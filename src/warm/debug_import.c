/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "warm.h"
#include "debug_info.h"
#include "dwarf.c"

//------------------------------------------------------------------------
// Dwarf line processing
//------------------------------------------------------------------------

wa_source_node* wa_source_node_alloc(wa_module* module)
{
    wa_source_node* node = oc_arena_push_type(module->arena, wa_source_node);
    memset(node, 0, sizeof(wa_source_node));
    return node;
}

typedef struct wa_source_file_elt
{
    oc_list_elt listElt;
    wa_source_file file;
    u64 index;
} wa_source_file_elt;

wa_source_file_elt* wa_find_or_add_source_file(oc_arena* scratchArena, oc_arena* pathArena, oc_list* files, u64* fileCount, oc_str8 rootPath, oc_str8 fullPath)
{
    wa_source_file_elt* file = 0;

    oc_list_for(*files, elt, wa_source_file_elt, listElt)
    {
        if(!oc_str8_cmp(elt->file.rootPath, rootPath) && !oc_str8_cmp(elt->file.fullPath, fullPath))
        {
            file = elt;
        }
    }

    if(!file)
    {
        file = oc_arena_push_type(scratchArena, wa_source_file_elt);
        memset(file, 0, sizeof(wa_source_file_elt));
        file->file.fullPath = oc_str8_push_copy(pathArena, fullPath);
        file->file.rootPath = oc_str8_slice(file->file.fullPath, 0, rootPath.len);
        file->index = *fileCount;
        oc_list_push_back(files, &file->listElt);
        (*fileCount)++;
    }
    return file;
}

void wa_dwarf_error_callback(dw_parser* parser, oc_str8 message, void* user)
{
    //TODO
}

void wa_import_dwarf(wa_module* module, oc_str8 contents)
{
    dw_sections dwarfSections = { 0 };

    oc_list_for(module->toc.customSections, section, wa_section, listElt)
    {
        oc_str8 sectionContents = (oc_str8){
            .ptr = contents.ptr + section->offset,
            .len = section->len,
        };

        if(!oc_str8_cmp(section->name, OC_STR8(".debug_abbrev")))
        {
            dwarfSections.abbrev = sectionContents;
        }
        else if(!oc_str8_cmp(section->name, OC_STR8(".debug_info")))
        {
            dwarfSections.info = sectionContents;
        }
        else if(!oc_str8_cmp(section->name, OC_STR8(".debug_str_offsets")))
        {
            dwarfSections.strOffsets = sectionContents;
        }
        else if(!oc_str8_cmp(section->name, OC_STR8(".debug_str")))
        {
            dwarfSections.str = sectionContents;
        }
        else if(!oc_str8_cmp(section->name, OC_STR8(".debug_addr")))
        {
            dwarfSections.addr = sectionContents;
        }
        else if(!oc_str8_cmp(section->name, OC_STR8(".debug_line")))
        {
            dwarfSections.line = sectionContents;
        }
        else if(!oc_str8_cmp(section->name, OC_STR8(".debug_line_str")))
        {
            dwarfSections.lineStr = sectionContents;
        }
        else if(!oc_str8_cmp(section->name, OC_STR8(".debug_loc")))
        {
            dwarfSections.loc = sectionContents;
        }
    }

    dw_parser parser = {
        .arena = module->arena,
        .sections = dwarfSections,
        .errorCallback = wa_dwarf_error_callback,
        .userData = module,
    };

    //TODO: just have plain dwarf struct here, and process after parsing. For now keep pointer stuff

    module->debugInfo->dwarf = dw_parse_dwarf(&parser);

    //NOTE: process line info if it exists
    if(dwarfSections.line.len)
    {
        dw_line_info* lineInfo = module->debugInfo->dwarf->line;

        //NOTE: alloc wasm to line map
        for(u64 tableIndex = 0; tableIndex < lineInfo->tableCount; tableIndex++)
        {
            module->debugInfo->wasmToLineCount += lineInfo->tables[tableIndex].entryCount;
        }
        module->debugInfo->wasmToLine = oc_arena_push_array(module->arena, wa_wasm_to_line_entry, module->debugInfo->wasmToLineCount);
        u64 wasmToLineIndex = 0;

        //NOTE: build a global file table and build wasm line map
        wa_source_info* sourceInfo = &module->debugInfo->sourceInfo;
        oc_arena_scope scratch = oc_scratch_begin_next(module->arena);

        oc_list files = { 0 };
        sourceInfo->fileCount = 1; // index 0 is reserved for a "nil" wa_source_file entry

        for(u64 tableIndex = 0; tableIndex < lineInfo->tableCount; tableIndex++)
        {
            dw_line_table* table = &lineInfo->tables[tableIndex];

            //NOTE: determine the paths of compile unit current directory, current file, and current file dir,
            //      which varies between dwarf 4 and 5.
            //WARN: currentFileDir can be different from currentDir if currentFile is an absolute path, or if it
            //      explicitly refers to another directory than the compile unit current directory.
            oc_str8 currentDir = { 0 };
            oc_str8 currentFile = { 0 };
            oc_str8 currentFileDir = { 0 };
            oc_str8 currentFileAbs = { 0 };

            bool currentFileInTable = false;

            if(table->header.version == 4)
            {
                //NOTE: set current dir current file from CU info
                OC_ASSERT(tableIndex < module->debugInfo->dwarf->unitCount);
                dw_unit* unit = &module->debugInfo->dwarf->units[tableIndex];
                dw_die* die = unit->rootDie;

                OC_ASSERT(die->abbrev->tag == DW_TAG_compile_unit);

                //TODO: make sure the die DW_AT_stmt_list corresponds to the current table
                dw_attr* dirAttr = dw_die_get_attr(die, DW_AT_comp_dir);
                if(dirAttr)
                {
                    currentDir = dirAttr->string;
                }

                dw_attr* fileAttr = dw_die_get_attr(die, DW_AT_name);
                if(fileAttr)
                {
                    currentFile = fileAttr->string;
                }
            }
            else
            {
                currentDir = table->header.dirEntries[0].path;
                currentFile = table->header.fileEntries[0].path;
            }

            if(currentFile.len && currentFile.ptr[0] == '/')
            {
                currentFileDir = currentFile;
                currentFileAbs = currentFile;
            }
            else
            {
                currentFileDir = currentDir;
                currentFileAbs = oc_path_append(scratch.arena, currentFileDir, currentFile);
            }
            OC_DEBUG_ASSERT(!oc_str8_cmp(currentFileDir, oc_str8_slice(currentFileAbs, 0, currentFileDir.len)),
                            "currentFileDir should be a prefix of currentFileAbs");

            //NOTE: allocate a table to map dwarf file indices to our global file table indices
            u64* fileIndices = oc_arena_push_array(scratch.arena, u64, table->header.fileEntryCount + 1);

            //NOTE: add the current table file entries to our global table
            for(u64 fileIndex = 0; fileIndex < table->header.fileEntryCount; fileIndex++)
            {
                //NOTE: first find the root path and full path of the file
                dw_file_entry* fileEntry = &table->header.fileEntries[fileIndex];
                oc_str8 filePath = fileEntry->path;
                oc_str8 rootPath = { 0 };
                oc_str8 fullPath = { 0 };

                if(filePath.len && filePath.ptr[0] == '/')
                {
                    //NOTE: absolute file path. This will be shown as a single file name at the root of the tree
                    rootPath = filePath;
                    fullPath = filePath;
                }
                else
                {
                    //NOTE: relative file path. This will be shown in a subtree whose root it either the name of the
                    // dir path (in case of an absolute dir path), or the name of the CU path (in case of a relative dir path)

                    oc_str8 dirPath = { 0 };
                    if(table->header.version == 5)
                    {
                        dw_file_entry* dirEntry = &table->header.dirEntries[fileEntry->dirIndex];
                        dirPath = dirEntry->path;
                    }
                    else if(fileEntry->dirIndex)
                    {
                        dw_file_entry* dirEntry = &table->header.dirEntries[fileEntry->dirIndex - 1];
                        dirPath = dirEntry->path;
                    }
                    else
                    {
                        dirPath = currentDir;
                    }

                    if(dirPath.len && dirPath.ptr[0] == '/')
                    {
                        // absolute dir path
                        rootPath = dirPath;
                    }
                    else
                    {
                        // dir path relative to current directory of compilation
                        rootPath = currentDir;
                        filePath = oc_path_append(scratch.arena, dirPath, filePath);
                    }
                    fullPath = oc_path_append(scratch.arena, rootPath, filePath);

                    OC_DEBUG_ASSERT(!oc_str8_cmp(rootPath, oc_str8_slice(fullPath, 0, rootPath.len)),
                                    "rootPath should be a prefix of fullPath");
                }

                //NOTE: now we create a wa_source_file entry with the given root path and file path, if it doesn't exist
                wa_source_file_elt* file = wa_find_or_add_source_file(scratch.arena,
                                                                      module->arena,
                                                                      &files,
                                                                      &sourceInfo->fileCount,
                                                                      rootPath,
                                                                      fullPath);

                //NOTE: if the found/created wa_source_file represents the current file, we mark it as found.
                //      this avoids duplicates when toolchain explicity put the current file in dwarf version 4
                //      file entries rather than using index 0 as the implicit current file.
                if(!oc_str8_cmp(currentFileAbs, file->file.fullPath))
                {
                    currentFileInTable = true;
                    if(table->header.version == 4)
                    {
                        fileIndices[0] = file->index;
                    }
                }

                if(table->header.version == 4)
                {
                    fileIndices[fileIndex + 1] = file->index;
                }
                else
                {
                    fileIndices[fileIndex] = file->index;
                }
            }

            if(table->header.version == 4 && currentFile.len && !currentFileInTable)
            {
                //NOTE: if the compile unit "current file" is only referenced implicitly by index 0,
                //      we add it to our global table here

                wa_source_file_elt* file = wa_find_or_add_source_file(scratch.arena,
                                                                      module->arena,
                                                                      &files,
                                                                      &sourceInfo->fileCount,
                                                                      currentFileDir,
                                                                      currentFileAbs);

                fileIndices[0] = file->index;
            }

            //NOTE: add the table line entries to our module's wasmToLineEntry table
            for(u64 entryIndex = 0; entryIndex < table->entryCount; entryIndex++)
            {
                dw_line_entry* lineEntry = &table->entries[entryIndex];
                wa_wasm_to_line_entry* wasmToLineEntry = &module->debugInfo->wasmToLine[wasmToLineIndex];

                wasmToLineEntry->wasmOffset = lineEntry->address;
                wasmToLineEntry->loc.fileIndex = fileIndices[lineEntry->file];
                wasmToLineEntry->loc.line = lineEntry->line;

                wasmToLineIndex++;
            }
        }

        //NOTE: now copy all wa_source_file entries to the module's global file array, and finally clear the scratch
        //      the first element of the array is a zeroed wa_source_file (index 0 is used as an invalid fileIndex)
        sourceInfo->files = oc_arena_push_array(module->arena, wa_source_file, sourceInfo->fileCount);
        memset(&sourceInfo->files[0], 0, sizeof(wa_source_file));
        oc_list_for_indexed(files, it, wa_source_file_elt, listElt)
        {
            sourceInfo->files[it.index + 1] = it.elt->file;
        }

        oc_scratch_end(scratch);
    }
}

//-------------------------------------------------------------------------
// Debug type processing
//-------------------------------------------------------------------------

wa_debug_type* wa_debug_type_alloc(oc_arena* arena, u64 typeRef, wa_debug_type_kind kind, oc_list* types)
{
    wa_debug_type* type = oc_arena_push_type(arena, wa_debug_type);
    memset(type, 0, sizeof(wa_debug_type));
    type->kind = kind;
    type->dwarfRef = typeRef;
    oc_list_push_back(types, &type->listElt);
    return type;
}

wa_debug_type* wa_debug_type_strip(wa_debug_type* type)
{
    while(type && type->kind == WA_DEBUG_TYPE_NAMED)
    {
        type = type->type;
    }
    return type;
}

wa_debug_type* wa_build_debug_type_from_dwarf(oc_arena* arena, dw_info* dwarf, u64 typeRef, oc_list* types)
{
    //NOTE: if we already parsed that type, return it
    oc_list_for(*types, t, wa_debug_type, listElt)
    {
        if(t->dwarfRef == typeRef)
        {
            return (t);
        }
    }

    wa_debug_type* type = 0;

    //NOTE: find die corresponding to typeRef
    dw_die* die = 0;
    for(u64 unitIndex = 0; unitIndex < dwarf->unitCount; unitIndex++)
    {
        dw_unit* unit = &dwarf->units[unitIndex];
        u64 unitSize = unit->initialLength + (unit->format == DW_DWARF32 ? 4 : 8);

        if(typeRef >= unit->start && typeRef < unit->start + unitSize)
        {
            die = dw_die_next(unit->rootDie, unit->rootDie);
            while(die && die->start != typeRef)
            {
                die = dw_die_next(unit->rootDie, die);
            }

            if(die)
            {
                break;
            }
        }
    }

    if(die)
    {
        //NOTE: process die
        switch(die->abbrev->tag)
        {
            case DW_TAG_base_type:
            {
                type = wa_debug_type_alloc(arena, typeRef, WA_DEBUG_TYPE_BASIC, types);

                //TODO: endianity

                dw_attr* encoding = dw_die_get_attr(die, DW_AT_encoding);
                if(encoding)
                {
                    switch(encoding->valU64)
                    {
                        case DW_ATE_boolean:
                            type->encoding = WA_DEBUG_TYPE_BOOL;
                            break;
                        case DW_ATE_address:
                        case DW_ATE_unsigned:
                        case DW_ATE_unsigned_char:
                            type->encoding = WA_DEBUG_TYPE_UNSIGNED;
                            break;
                        case DW_ATE_signed:
                        case DW_ATE_signed_char:
                            type->encoding = WA_DEBUG_TYPE_SIGNED;
                            break;
                        case DW_ATE_float:
                            type->encoding = WA_DEBUG_TYPE_FLOAT;
                            break;
                        default:
                            oc_log_error("unrecognized type encoding %s\n", dw_get_encoding_string(encoding->valU64));
                            break;
                    }
                }
            }
            break;

            case DW_TAG_unspecified_type:
            {
                type = wa_debug_type_alloc(arena, typeRef, WA_DEBUG_TYPE_VOID, types);
            }
            break;

            case DW_TAG_atomic_type:
            case DW_TAG_const_type:
            case DW_TAG_restrict_type:
            case DW_TAG_volatile_type:
            {
                dw_attr* typeAttr = dw_die_get_attr(die, DW_AT_type);
                if(typeAttr)
                {
                    type = wa_build_debug_type_from_dwarf(arena, dwarf, typeAttr->valU64, types);
                }
            }
            break;

            case DW_TAG_pointer_type:
            {
                type = wa_debug_type_alloc(arena, typeRef, WA_DEBUG_TYPE_POINTER, types);

                dw_attr* typeAttr = dw_die_get_attr(die, DW_AT_type);
                if(typeAttr)
                {
                    type->type = wa_build_debug_type_from_dwarf(arena, dwarf, typeAttr->valU64, types);
                }
            }
            break;

            case DW_TAG_typedef:
            {
                type = wa_debug_type_alloc(arena, typeRef, WA_DEBUG_TYPE_NAMED, types);

                dw_attr* typeAttr = dw_die_get_attr(die, DW_AT_type);
                if(typeAttr)
                {
                    type->type = wa_build_debug_type_from_dwarf(arena, dwarf, typeAttr->valU64, types);
                }
            }
            break;

            case DW_TAG_array_type:
            {
                type = wa_debug_type_alloc(arena, typeRef, WA_DEBUG_TYPE_ARRAY, types);

                dw_attr* typeAttr = dw_die_get_attr(die, DW_AT_type);
                if(typeAttr)
                {
                    type->array.type = wa_build_debug_type_from_dwarf(arena, dwarf, typeAttr->valU64, types);
                }

                //TODO stride

                ///////////////////////////////////////
                //TODO: multi dim arrays...
                ///////////////////////////////////////
                u64 size = wa_debug_type_strip(type->array.type)->size;

                oc_list_for(die->children, child, dw_die, parentElt)
                {
                    if(child->abbrev)
                    {
                        if(child->abbrev->tag == DW_TAG_subrange_type)
                        {
                            dw_attr* count = dw_die_get_attr(child, DW_AT_count);
                            if(count)
                            {
                                type->array.count = count->valU64;
                                size *= type->array.count;
                            }
                        }
                        else if(child->abbrev->tag == DW_TAG_enumeration_type)
                        {
                            //TODO
                        }
                    }
                }
                type->size = size;
                //TODO: dynamic ranks

                /*TODO: array count
                dw_attr* countAttr = dw_die_get_attr(die, DW_AT_subrange_type);
                if(countAttr)
                {
                    type->array.type = wa_build_debug_type_from_dwarf(arena, dwarf, typeAttr->valU64, types);
                }
                */
            }
            break;

            case DW_TAG_structure_type:
            case DW_TAG_union_type:
            {
                wa_debug_type_kind kind = (die->abbrev->tag == DW_TAG_structure_type) ? WA_DEBUG_TYPE_STRUCT
                                                                                      : WA_DEBUG_TYPE_UNION;
                type = wa_debug_type_alloc(arena, typeRef, kind, types);

                oc_list_for(die->children, child, dw_die, parentElt)
                {
                    if(child->abbrev && child->abbrev->tag == DW_TAG_member)
                    {
                        wa_debug_type_field* member = oc_arena_push_type(arena, wa_debug_type_field);
                        memset(member, 0, sizeof(wa_debug_type_field));

                        dw_attr* memberName = dw_die_get_attr(child, DW_AT_name);
                        if(memberName)
                        {
                            member->name = memberName->string;
                        }

                        dw_attr* memberType = dw_die_get_attr(child, DW_AT_type);
                        if(memberType)
                        {
                            member->type = wa_build_debug_type_from_dwarf(arena, dwarf, memberType->valU64, types);
                        }

                        dw_attr* memberOffset = dw_die_get_attr(child, DW_AT_data_member_location);
                        if(memberOffset)
                        {
                            ///////////////////////////////////////////////////////////////
                            //TODO: this could also be a location description, ensure this is not the case or bailout for now
                            ///////////////////////////////////////////////////////////////
                            member->offset = memberOffset->valU64;
                        }

                        //TODO: bit offset
                        oc_list_push_back(&type->fields, &member->listElt);
                    }
                }
            }
            break;

            case DW_TAG_enumeration_type:
            {
                //TODO
            }
            break;

            case DW_TAG_subroutine_type:
            {
                //TODO
            }
            break;

            default:
            {
                oc_log_error("unrecognized DIE type tag %s\n", dw_get_tag_string(die->abbrev->tag));
            }
            break;
        }
        if(type)
        {
            dw_attr* byteSize = dw_die_get_attr(die, DW_AT_byte_size);
            if(byteSize)
            {
                type->size = byteSize->valU64;
            }
            //TODO: bitSize and offset

            dw_attr* name = dw_die_get_attr(die, DW_AT_name);
            if(name)
            {
                type->name = name->string;
            }
        }
    }
    else
    {
        oc_log_error("Could not find referenced type DIE.\n");
    }
    return type;
}

void wa_import_debug_locals(wa_module* module)
{
    //NOTE: extract per-function local variables
    module->debugInfo->functionLocals = oc_arena_push_array(module->arena, wa_debug_function, module->functionCount);
    memset(module->debugInfo->functionLocals, 0, module->functionCount * sizeof(wa_debug_function));

    //NOTE: list of all types to deduplicate types
    oc_list types = { 0 };

    for(u64 unitIndex = 0; unitIndex < module->debugInfo->dwarf->unitCount; unitIndex++)
    {
        dw_unit* unit = &module->debugInfo->dwarf->units[unitIndex];
        dw_die* die = dw_die_find_next_with_tag(unit->rootDie, unit->rootDie, DW_TAG_subprogram);
        while(die)
        {
            dw_attr* funcNameAttr = dw_die_get_attr(die, DW_AT_name);
            if(funcNameAttr)
            {
                //TODO: better way of finding function
                bool found = false;
                u64 funcIndex = 0;
                for(; funcIndex < module->functionCount; funcIndex++)
                {
                    oc_str8 funcName = wa_module_get_function_name(module, funcIndex);
                    if(!oc_str8_cmp(funcName, funcNameAttr->string))
                    {
                        found = true;
                        break;
                    }
                }

                if(found)
                {
                    wa_debug_function* funcInfo = &module->debugInfo->functionLocals[funcIndex];
                    funcInfo->count = 0;

                    //NOTE: get frame base expr loc
                    dw_attr* frameBase = dw_die_get_attr(die, DW_AT_frame_base);
                    if(frameBase)
                    {
                        OC_DEBUG_ASSERT(frameBase->abbrev->form == DW_FORM_exprloc);
                        funcInfo->frameBase = &frameBase->loc;
                    }

                    //NOTE: get variables
                    {
                        //TODO: get with multiple tags, eg here we also need formal_parameter
                        dw_die* var = dw_die_find_next_with_tag(die, die, DW_TAG_variable);
                        while(var)
                        {
                            funcInfo->count++;
                            var = dw_die_find_next_with_tag(die, var, DW_TAG_variable);
                        }
                    }

                    funcInfo->vars = oc_arena_push_array(module->arena, wa_debug_variable, funcInfo->count);

                    {
                        dw_die* var = dw_die_find_next_with_tag(die, die, DW_TAG_variable);
                        u64 varIndex = 0;
                        while(var)
                        {
                            dw_attr* name = dw_die_get_attr(var, DW_AT_name);
                            if(name)
                            {
                                funcInfo->vars[varIndex].name = oc_str8_push_copy(module->arena, name->string);
                            }

                            dw_attr* loc = dw_die_get_attr(var, DW_AT_location);
                            if(loc)
                            {
                                funcInfo->vars[varIndex].loc = &loc->loc;
                                //TODO: compile the expr to wasm
                            }

                            dw_attr* type = dw_die_get_attr(var, DW_AT_type);
                            if(type)
                            {
                                funcInfo->vars[varIndex].type = wa_build_debug_type_from_dwarf(module->arena, module->debugInfo->dwarf, type->valU64, &types);
                            }

                            var = dw_die_find_next_with_tag(die, var, DW_TAG_variable);
                            varIndex++;
                        }
                    }
                }
            }

            die = dw_die_find_next_with_tag(unit->rootDie, die, DW_TAG_subprogram);
        }
    }
}

//-------------------------------------------------------------------------
// emitting bytecode mapping
//-------------------------------------------------------------------------

void wa_warm_to_wasm_loc_push(wa_module* module, u32 funcIndex, u32 codeIndex, wa_instr* instr)
{
    wa_bytecode_mapping* mapping = oc_arena_push_type(module->arena, wa_bytecode_mapping);
    mapping->funcIndex = funcIndex;
    mapping->codeIndex = codeIndex;
    mapping->instr = instr;

    u64 id = (u64)funcIndex << 32 | (u64)codeIndex;
    u64 hash = oc_hash_xx64_string((oc_str8){ .ptr = (char*)&id, .len = 8 });
    u64 index = hash % module->debugInfo->warmToWasmMapLen;

    oc_list_push_back(&module->debugInfo->warmToWasmMap[index], &mapping->listElt);
}

void wa_wasm_to_warm_loc_push(wa_module* module, u32 funcIndex, u32 codeIndex, wa_instr* instr)
{
    wa_bytecode_mapping* mapping = oc_arena_push_type(module->arena, wa_bytecode_mapping);
    mapping->funcIndex = funcIndex;
    mapping->codeIndex = codeIndex;
    mapping->instr = instr;

    u64 id = mapping->instr->loc.start - module->toc.code.offset;
    u64 hash = oc_hash_xx64_string((oc_str8){ .ptr = (char*)&id, .len = 8 });
    u64 index = hash % module->debugInfo->wasmToWarmMapLen;

    oc_list_push_back(&module->debugInfo->wasmToWarmMap[index], &mapping->listElt);
}
