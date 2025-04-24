/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "warm.h"
#include "dwarf.c"

#include "warm_internal.h"

static const wa_value_type WA_BLOCK_TYPE_I32_VAL = WA_TYPE_I32;
static const wa_value_type WA_BLOCK_TYPE_I64_VAL = WA_TYPE_I64;
static const wa_value_type WA_BLOCK_TYPE_F32_VAL = WA_TYPE_F32;
static const wa_value_type WA_BLOCK_TYPE_F64_VAL = WA_TYPE_F64;
static const wa_value_type WA_BLOCK_TYPE_V128_VAL = WA_TYPE_V128;
static const wa_value_type WA_BLOCK_TYPE_FUNC_REF_VAL = WA_TYPE_FUNC_REF;
static const wa_value_type WA_BLOCK_TYPE_EXTERN_REF_VAL = WA_TYPE_EXTERN_REF;

static const wa_func_type WA_BLOCK_VALUE_TYPES[] = {
    [0] = { 0 },
    [1] = {
        .returnCount = 1,
        .returns = (wa_value_type*)&WA_BLOCK_TYPE_I32_VAL,
    },
    [2] = {
        .returnCount = 1,
        .returns = (wa_value_type*)&WA_BLOCK_TYPE_I64_VAL,
    },
    [3] = {
        .returnCount = 1,
        .returns = (wa_value_type*)&WA_BLOCK_TYPE_F32_VAL,
    },
    [4] = {
        .returnCount = 1,
        .returns = (wa_value_type*)&WA_BLOCK_TYPE_F64_VAL,
    },
    [5] = {
        .returnCount = 1,
        .returns = (wa_value_type*)&WA_BLOCK_TYPE_V128_VAL,
    },
    [16] = {
        .returnCount = 1,
        .returns = (wa_value_type*)&WA_BLOCK_TYPE_FUNC_REF_VAL,
    },
    [17] = {
        .returnCount = 1,
        .returns = (wa_value_type*)&WA_BLOCK_TYPE_EXTERN_REF_VAL,
    },
};

bool wa_is_value_type(u64 t)
{
    switch(t)
    {
        case WA_TYPE_I32:
        case WA_TYPE_I64:
        case WA_TYPE_F32:
        case WA_TYPE_F64:
        case WA_TYPE_V128:
        case WA_TYPE_FUNC_REF:
        case WA_TYPE_EXTERN_REF:
            return true;
        default:
            return false;
    }
}

bool wa_is_value_type_numeric(u64 t)
{
    switch(t)
    {
        case WA_TYPE_UNKNOWN:
        case WA_TYPE_I32:
        case WA_TYPE_I64:
        case WA_TYPE_F32:
        case WA_TYPE_F64:
            return true;
        default:
            return false;
    }
}

#include "wasm_tables.c"

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

oc_str8 wa_module_get_function_name(wa_module* module, u32 index);

void wa_parse_dwarf(oc_str8 contents, wa_module* module)
{
    /////////////////////////////////////////////////////////////////////////
    //WIP: Dwarf stuff

    //NOTE: load dwarf sections
    //TODO: - put all that in its own wa_load_debug_info function
    //      ? split dw_xxx struct (for parsing dwarf specific stuff) from wa_debug_xxx (for internal representation)

    oc_arena_scope scratch = oc_scratch_begin();

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

    module->debugInfo.dwarf = oc_arena_push_type(module->arena, dw_info);
    memset(module->debugInfo.dwarf, 0, sizeof(dw_info));

    dw_parse_info(module->arena, &dwarfSections, module->debugInfo.dwarf);

    //NOTE: load line info if it exists
    if(dwarfSections.line.len)
    {
        module->debugInfo.dwarf->line = oc_arena_push_type(module->arena, dw_line_info);
        *module->debugInfo.dwarf->line = dw_load_line_info(module->arena, &dwarfSections);

        //        dw_print_debug_info(module->debugInfo.dwarf);

        //dw_dump_info(dwarfSections);
        //dw_print_line_info(module->debugInfo.dwarf->line);

        dw_line_info* lineInfo = module->debugInfo.dwarf->line;

        //NOTE: alloc wasm to line map
        for(u64 tableIndex = 0; tableIndex < lineInfo->tableCount; tableIndex++)
        {
            module->debugInfo.wasmToLineCount += lineInfo->tables[tableIndex].entryCount;
        }
        module->debugInfo.wasmToLine = oc_arena_push_array(module->arena, wa_wasm_to_line_entry, module->debugInfo.wasmToLineCount);
        u64 wasmToLineIndex = 0;

        //NOTE: build a global file table and build wasm line map
        wa_source_info* sourceInfo = &module->debugInfo.sourceInfo;
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
                OC_ASSERT(tableIndex < module->debugInfo.dwarf->unitCount);
                dw_unit* unit = &module->debugInfo.dwarf->units[tableIndex];
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
                wa_wasm_to_line_entry* wasmToLineEntry = &module->debugInfo.wasmToLine[wasmToLineIndex];

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

    oc_scratch_end(scratch);
}

//-------------------------------------------------------------------------
// Debug type processing
//-------------------------------------------------------------------------

typedef enum wa_debug_type_kind
{
    WA_DEBUG_TYPE_VOID,
    WA_DEBUG_TYPE_BASIC,
    WA_DEBUG_TYPE_POINTER,
    WA_DEBUG_TYPE_STRUCT,
    WA_DEBUG_TYPE_UNION,
    WA_DEBUG_TYPE_ENUM,
    WA_DEBUG_TYPE_ARRAY,
    WA_DEBUG_TYPE_NAMED,
    //...
} wa_debug_type_kind;

typedef enum wa_debug_type_encoding
{
    WA_DEBUG_TYPE_SIGNED,
    WA_DEBUG_TYPE_UNSIGNED,
    WA_DEBUG_TYPE_FLOAT,
    WA_DEBUG_TYPE_BOOL,
} wa_debug_type_encoding;

typedef struct wa_debug_type_field
{
    oc_list_elt listElt;
    oc_str8 name;
    wa_debug_type* type;
    u64 offset;
} wa_debug_type_field;

typedef struct wa_debug_type
{
    oc_list_elt listElt;
    u64 dwarfRef;
    oc_str8 name;
    wa_debug_type_kind kind;
    u64 size;

    union
    {
        wa_debug_type_encoding encoding;
        wa_debug_type* type;
        oc_list fields;

        struct
        {
            wa_debug_type* type;
            u64 count;
        } array;

        //TODO enum, etc...
    };
} wa_debug_type;

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

//-------------------------------------------------------------------------
// emitting bytecode mapping
//-------------------------------------------------------------------------

typedef struct wa_bytecode_mapping
{
    oc_list_elt listElt;

    u32 funcIndex;
    u32 codeIndex;
    wa_instr* instr;

} wa_bytecode_mapping;

void wa_warm_to_wasm_loc_push(wa_module* module, u32 funcIndex, u32 codeIndex, wa_instr* instr)
{
    wa_bytecode_mapping* mapping = oc_arena_push_type(module->arena, wa_bytecode_mapping);
    mapping->funcIndex = funcIndex;
    mapping->codeIndex = codeIndex;
    mapping->instr = instr;

    u64 id = (u64)funcIndex << 32 | (u64)codeIndex;
    u64 hash = oc_hash_xx64_string((oc_str8){ .ptr = (char*)&id, .len = 8 });
    u64 index = hash % module->debugInfo.warmToWasmMapLen;

    oc_list_push_back(&module->debugInfo.warmToWasmMap[index], &mapping->listElt);
}

void wa_wasm_to_warm_loc_push(wa_module* module, u32 funcIndex, u32 codeIndex, wa_instr* instr)
{
    wa_bytecode_mapping* mapping = oc_arena_push_type(module->arena, wa_bytecode_mapping);
    mapping->funcIndex = funcIndex;
    mapping->codeIndex = codeIndex;
    mapping->instr = instr;

    u64 id = mapping->instr->ast->loc.start - module->toc.code.offset;
    u64 hash = oc_hash_xx64_string((oc_str8){ .ptr = (char*)&id, .len = 8 });
    u64 index = hash % module->debugInfo.wasmToWarmMapLen;

    oc_list_push_back(&module->debugInfo.wasmToWarmMap[index], &mapping->listElt);
}

//-------------------------------------------------------------------------
// instance
//-------------------------------------------------------------------------

typedef struct wa_instance
{
    wa_status status;

    oc_arena* arena;
    wa_module* module;

    wa_func* functions;
    wa_global** globals;
    wa_table** tables;
    wa_memory** memories;

    wa_data_segment* data;
    wa_element* elements;
} wa_instance;

wa_status wa_instance_status(wa_instance* instance)
{
    return instance->status;
}

bool wa_check_function_type(wa_func_type* t1, wa_func_type* t2)
{
    if(t1->paramCount != t2->paramCount || t1->returnCount != t2->returnCount)
    {
        return (false);
    }
    for(u32 i = 0; i < t1->paramCount; i++)
    {
        if(t1->params[i] != t2->params[i])
        {
            return (false);
        }
    }
    for(u32 i = 0; i < t1->returnCount; i++)
    {
        if(t1->returns[i] != t2->returns[i])
        {
            return (false);
        }
    }
    return true;
}

bool wa_check_limits_match(wa_limits* l1, wa_limits* l2)
{
    return ((l1->min >= l2->min)
            && (l2->kind == WA_LIMIT_MIN
                || (l1->kind == WA_LIMIT_MIN_MAX && l1->max <= l2->max)));
}

wa_status wa_instance_link_imports(wa_instance* instance, wa_instance_options* options)
{
    wa_module* module = instance->module;
    //NOTE: link imports
    u32 funcIndex = 0;
    u32 globalIndex = 0;
    u32 tableIndex = 0;
    u32 memIndex = 0;

    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];

        for(u32 packageIndex = 0; packageIndex < options->packageCount; packageIndex++)
        {
            wa_import_package* package = &options->importPackages[packageIndex];
            if(!oc_str8_cmp(package->name, import->moduleName))
            {
                oc_list_for(package->bindings, elt, wa_import_package_elt, listElt)
                {
                    wa_import_binding* binding = &elt->binding;
                    if(!oc_str8_cmp(binding->name, import->importName))
                    {
                        switch(import->kind)
                        {
                            case WA_IMPORT_GLOBAL:
                            {
                                if(binding->kind != WA_BINDING_WASM_GLOBAL
                                   && binding->kind != WA_BINDING_HOST_GLOBAL)
                                {
                                    return WA_FAIL_IMPORT_TYPE_MISMATCH;
                                }

                                wa_global_desc* globalDesc = &module->globals[globalIndex];
                                wa_global* global = 0;

                                if(binding->kind == WA_BINDING_WASM_GLOBAL)
                                {
                                    global = binding->instance->globals[binding->wasmGlobal];
                                }
                                else
                                {
                                    global = binding->hostGlobal;
                                }

                                if(globalDesc->type != global->type || globalDesc->mut != global->mut)
                                {
                                    return WA_FAIL_IMPORT_TYPE_MISMATCH;
                                }
                                instance->globals[globalIndex] = global;
                            }
                            break;

                            case WA_IMPORT_FUNCTION:
                            {
                                if(binding->kind != WA_BINDING_WASM_FUNCTION
                                   && binding->kind != WA_BINDING_HOST_FUNCTION)
                                {
                                    return WA_FAIL_IMPORT_TYPE_MISMATCH;
                                }

                                wa_func* importFunc = &instance->functions[funcIndex];
                                wa_func_type* importType = importFunc->type;

                                wa_func_type* boundType = 0;
                                if(binding->kind == WA_BINDING_WASM_FUNCTION)
                                {
                                    boundType = binding->instance->functions[binding->wasmFunction].type;
                                }
                                else
                                {
                                    boundType = &binding->hostFunction.type;
                                }

                                if(importType->paramCount != boundType->paramCount
                                   || importType->returnCount != boundType->returnCount)
                                {
                                    //log error to module
                                    return WA_FAIL_IMPORT_TYPE_MISMATCH;
                                }
                                for(u32 paramIndex = 0; paramIndex < importType->paramCount; paramIndex++)
                                {
                                    if(importType->params[paramIndex] != boundType->params[paramIndex])
                                    {
                                        return WA_FAIL_IMPORT_TYPE_MISMATCH;
                                    }
                                }
                                for(u32 returnIndex = 0; returnIndex < importType->returnCount; returnIndex++)
                                {
                                    if(importType->returns[returnIndex] != boundType->returns[returnIndex])
                                    {
                                        return WA_FAIL_IMPORT_TYPE_MISMATCH;
                                    }
                                }

                                if(binding->kind == WA_BINDING_WASM_FUNCTION)
                                {
                                    importFunc->extInstance = binding->instance;
                                    importFunc->extIndex = binding->wasmFunction;
                                }
                                else
                                {
                                    importFunc->proc = binding->hostFunction.proc;
                                    importFunc->user = binding->hostFunction.userData;
                                }
                            }
                            break;

                            case WA_IMPORT_MEMORY:
                            {
                                wa_memory* memory = 0;
                                if(binding->kind == WA_BINDING_WASM_MEMORY)
                                {
                                    memory = binding->instance->memories[binding->wasmMemory];
                                }
                                else if(binding->kind == WA_BINDING_HOST_MEMORY)
                                {
                                    memory = binding->hostMemory;
                                }
                                else
                                {
                                    return WA_FAIL_IMPORT_TYPE_MISMATCH;
                                }

                                if(!wa_check_limits_match(&memory->limits, &module->memories[memIndex]))
                                {
                                    return WA_FAIL_IMPORT_TYPE_MISMATCH;
                                }

                                instance->memories[memIndex] = memory;
                            }
                            break;

                            case WA_IMPORT_TABLE:
                            {
                                wa_table* table = 0;
                                if(binding->kind == WA_BINDING_WASM_TABLE)
                                {
                                    table = binding->instance->tables[binding->wasmTable];
                                }
                                else if(binding->kind == WA_BINDING_HOST_TABLE)
                                {
                                    table = binding->hostTable;
                                }
                                else
                                {
                                    return WA_FAIL_IMPORT_TYPE_MISMATCH;
                                }

                                if(table->type != module->tables[tableIndex].type
                                   || !wa_check_limits_match(&table->limits, &module->tables[tableIndex].limits))
                                {
                                    return WA_FAIL_IMPORT_TYPE_MISMATCH;
                                }

                                instance->tables[tableIndex] = table;
                            }
                            break;
                        }
                    }
                }
            }
        }

        switch(import->kind)
        {
            case WA_IMPORT_GLOBAL:
                globalIndex++;
                break;
            case WA_IMPORT_FUNCTION:
                funcIndex++;
                break;
            case WA_IMPORT_TABLE:
                tableIndex++;
                break;
            case WA_IMPORT_MEMORY:
                memIndex++;
                break;
        }
    }

    //NOTE: check that all imports are satisfied
    //TODO: hoist in in loop
    for(u32 funcIndex = 0; funcIndex < module->functionImportCount; funcIndex++)
    {
        wa_func* func = &instance->functions[funcIndex];
        if(!func->proc && !func->extInstance)
        {
            //oc_log_error("Couldn't link instance: import %.*s not satisfied.\n", oc_str8_ip(func->import->importName));
            return WA_FAIL_MISSING_IMPORT;
        }
    }

    for(u32 globalIndex = 0; globalIndex < module->globalImportCount; globalIndex++)
    {
        if(instance->globals[globalIndex] == 0)
        {
            return WA_FAIL_MISSING_IMPORT;
        }
    }

    for(u32 tableIndex = 0; tableIndex < module->tableImportCount; tableIndex++)
    {
        if(instance->tables[tableIndex] == 0)
        {
            //oc_log_error("Coulnd't link instance: table import is not satisfied\n");
            return WA_FAIL_MISSING_IMPORT;
        }
    }

    for(u32 memIndex = 0; memIndex < module->memoryImportCount; memIndex++)
    {
        if(instance->memories[memIndex] == 0)
        {
            //oc_log_error("Coulnd't link instance: memory import is not satisfied\n");
            return WA_FAIL_MISSING_IMPORT;
        }
    }

    return WA_OK;
}

wa_status wa_instance_interpret_expr(wa_instance* instance,
                                     wa_func* func,
                                     wa_func_type* type,
                                     wa_code* code,
                                     u32 argCount,
                                     wa_value* args,
                                     u32 retCount,
                                     wa_value* returns);

/*
wa_status wa_instance_invoke(wa_instance* instance,
                             wa_func* func,
                             u32 argCount,
                             wa_value* args,
                             u32 retCount,
                             wa_value* returns);
*/

wa_status wa_instance_initialize(wa_instance* instance)
{
    wa_module* module = instance->module;

    for(u64 globalIndex = module->globalImportCount; globalIndex < module->globalCount; globalIndex++)
    {
        wa_global_desc* global = &module->globals[globalIndex];
        if(global->code)
        {
            i64 t = 0x7f - (i64)global->type + 1;
            wa_func_type* exprType = (wa_func_type*)&WA_BLOCK_VALUE_TYPES[t];

            wa_status status = wa_instance_interpret_expr(instance, 0, exprType, global->code, 0, 0, 1, &instance->globals[globalIndex]->value);
            if(status != WA_OK)
            {
                //oc_log_error("Couldn't link instance: couldn't execute global initialization expression.\n");
                return status;
            }
        }
    }

    for(u64 eltIndex = 0; eltIndex < module->elementCount; eltIndex++)
    {
        wa_element* element = &instance->elements[eltIndex];

        i64 t = 0x7f - (i64)element->type + 1;
        wa_func_type* exprType = (wa_func_type*)&WA_BLOCK_VALUE_TYPES[t];

        for(u32 exprIndex = 0; exprIndex < element->initCount; exprIndex++)
        {
            wa_value* result = &element->refs[exprIndex];
            wa_status status = wa_instance_interpret_expr(instance, 0, exprType, element->code[exprIndex], 0, 0, 1, result);
            if(status != WA_OK)
            {
                //oc_log_error("Couldn't link instance: couldn't execute element initialization expression.\n");
                return status;
            }
        }

        if(element->mode == WA_ELEMENT_ACTIVE)
        {
            //TODO: check table size?
            wa_table_type* desc = &module->tables[element->tableIndex];
            wa_table* table = instance->tables[element->tableIndex];

            wa_value offset = { 0 };
            wa_status status = wa_instance_interpret_expr(instance,
                                                          0,
                                                          (wa_func_type*)&WA_BLOCK_VALUE_TYPES[1],
                                                          element->tableOffsetCode,
                                                          0, 0,
                                                          1, &offset);
            if(status != WA_OK)
            {
                //oc_log_error("Couldn't link instance: couldn't execute element offset expression.\n");
                return status;
            }
            if(offset.valI32 + element->initCount > table->limits.min || offset.valI32 + element->initCount < offset.valI32)
            {
                return WA_TRAP_TABLE_OUT_OF_BOUNDS;
            }
            memcpy(&table->contents[offset.valI32], element->refs, element->initCount * sizeof(wa_value));
        }
        if(element->mode == WA_ELEMENT_ACTIVE || element->mode == WA_ELEMENT_DECLARATIVE)
        {
            //NOTE: drop the element
            element->initCount = 0;
        }
    }

    for(u32 dataIndex = 0; dataIndex < module->dataCount; dataIndex++)
    {
        wa_data_segment* seg = &module->data[dataIndex];

        if(seg->mode == WA_DATA_ACTIVE)
        {
            wa_limits* limits = &module->memories[seg->memoryIndex];
            wa_memory* mem = instance->memories[seg->memoryIndex];

            wa_value offsetVal = { 0 };
            wa_status status = wa_instance_interpret_expr(instance,
                                                          0,
                                                          (wa_func_type*)&WA_BLOCK_VALUE_TYPES[1],
                                                          seg->memoryOffsetCode,
                                                          0, 0,
                                                          1, &offsetVal);
            if(status != WA_OK)
            {
                //oc_log_error("Couldn't link instance: couldn't execute data offset expression.\n");
                return status;
            }
            u32 offset = *(u32*)&offsetVal.valI32;

            if(offset + seg->init.len > mem->limits.min * WA_PAGE_SIZE || offset + seg->init.len < offset)
            {
                //oc_log_error("Couldn't link instance: data offset out of bounds.\n");
                return WA_TRAP_MEMORY_OUT_OF_BOUNDS;
            }
            memcpy(mem->ptr + offset, seg->init.ptr, seg->init.len);
        }
    }

    if(module->hasStart)
    {
        if(module->startIndex >= module->functionCount)
        {
            oc_log_error("Couldn't link instance: invalid start function index.\n");
            return WA_FAIL_MISSING_IMPORT; //TODO: change this
        }
        wa_func* func = &instance->functions[module->startIndex];

        if(func->type->paramCount || func->type->returnCount)
        {
            oc_log_error("Couldn't link instance: invalid start function type.\n");
            return WA_FAIL_MISSING_IMPORT; //TODO: change this
        }

        //TODO: later take an interpreter as input of instantiate?
        oc_arena_scope scratch = oc_scratch_begin();
        wa_interpreter* interpreter = wa_interpreter_create(scratch.arena);
        wa_status status = wa_interpreter_invoke(interpreter, instance, func, 0, 0, 0, 0);
        oc_scratch_end(scratch);

        if(status != WA_OK)
        {
            return status;
        }
    }
    return WA_OK;
}

wa_instance* wa_instance_create(oc_arena* arena, wa_module* module, wa_instance_options* options)
{
    wa_instance* instance = oc_arena_push_type(arena, wa_instance);
    memset(instance, 0, sizeof(wa_instance));

    instance->arena = arena;
    instance->module = module;

    //NOTE: allocate functions
    instance->functions = oc_arena_push_array(arena, wa_func, module->functionCount);
    memcpy(instance->functions, module->functions, module->functionCount * sizeof(wa_func));

    //NOTE: allocate globals
    instance->globals = oc_arena_push_array(arena, wa_global*, module->globalCount);
    memset(instance->globals, 0, module->globalImportCount * sizeof(wa_global*));

    for(u32 globalIndex = module->globalImportCount; globalIndex < module->globalCount; globalIndex++)
    {
        instance->globals[globalIndex] = oc_arena_push_type(arena, wa_global);
        instance->globals[globalIndex]->type = module->globals[globalIndex].type;
        instance->globals[globalIndex]->mut = module->globals[globalIndex].mut;
        memset(&instance->globals[globalIndex]->value, 0, sizeof(wa_value));
    }

    //NOTE: allocate tables
    instance->tables = oc_arena_push_array(arena, wa_table*, module->tableCount);
    memset(instance->tables, 0, module->tableImportCount * sizeof(wa_table*));

    for(u32 tableIndex = module->tableImportCount; tableIndex < module->tableCount; tableIndex++)
    {
        wa_table_type* desc = &module->tables[tableIndex];
        wa_table* table = oc_arena_push_type(arena, wa_table);

        table->type = desc->type;
        table->limits = desc->limits;
        table->contents = oc_arena_push_array(arena, wa_value, table->limits.min);
        memset(table->contents, 0, table->limits.min * sizeof(wa_value));

        instance->tables[tableIndex] = table;
    }

    //NOTE: allocate elements
    //TODO: we could refer to passive elements directly in the module?
    instance->elements = oc_arena_push_array(arena, wa_element, module->elementCount);
    memcpy(instance->elements, module->elements, module->elementCount * sizeof(wa_element));
    for(u32 eltIndex = 0; eltIndex < module->elementCount; eltIndex++)
    {
        wa_element* elt = &instance->elements[eltIndex];
        elt->refs = oc_arena_push_array(arena, wa_value, elt->initCount);
        memset(elt->refs, 0, elt->initCount * sizeof(wa_element));
    }

    //NOTE: allocate memories
    oc_base_allocator* allocator = oc_base_allocator_default();

    instance->memories = oc_arena_push_array(arena, wa_memory*, module->memoryCount);
    memset(instance->memories, 0, module->memoryImportCount * sizeof(wa_memory*));

    for(u32 memIndex = module->memoryImportCount; memIndex < module->memoryCount; memIndex++)
    {
        wa_limits* limits = &module->memories[memIndex];
        wa_memory* mem = oc_arena_push_type(arena, wa_memory);

        ////////////////////////////////////////////////////////
        //TODO: validate limit before that
        ////////////////////////////////////////////////////////
        mem->limits.kind = limits->kind;
        mem->limits.min = limits->min;
        mem->limits.max = (limits->kind == WA_LIMIT_MIN)
                            ? UINT32_MAX / WA_PAGE_SIZE
                            : limits->max;

        mem->ptr = oc_base_reserve(allocator, mem->limits.max * WA_PAGE_SIZE);
        oc_base_commit(allocator, mem->ptr, mem->limits.min * WA_PAGE_SIZE);

        instance->memories[memIndex] = mem;
    }

    //NOTE: allocate data
    instance->data = oc_arena_push_array(arena, wa_data_segment, module->dataCount);
    memcpy(instance->data, module->data, module->dataCount * sizeof(wa_data_segment));

    //NOTE: link
    instance->status = wa_instance_link_imports(instance, options);
    if(instance->status != WA_OK)
    {
        return instance;
    }

    //NOTE: initialize
    instance->status = wa_instance_initialize(instance);

    return (instance);
}

wa_import_package wa_instance_exports(oc_arena* arena, wa_instance* instance, oc_str8 name)
{
    wa_module* module = instance->module;

    wa_import_package package = {
        .name = oc_str8_push_copy(arena, name),
        .bindingCount = module->exportCount,
    };

    for(u32 exportIndex = 0; exportIndex < module->exportCount; exportIndex++)
    {
        wa_export* export = &module->exports[exportIndex];
        wa_import_binding binding = { 0 };

        binding.name = export->name;
        binding.instance = instance;

        switch(export->kind)
        {
            case WA_EXPORT_GLOBAL:
            {
                binding.kind = WA_BINDING_WASM_GLOBAL;
                binding.wasmGlobal = export->index;
            }
            break;
            case WA_EXPORT_FUNCTION:
            {
                binding.kind = WA_BINDING_WASM_FUNCTION;
                binding.wasmFunction = export->index;
            }
            break;
            case WA_EXPORT_TABLE:
            {
                binding.kind = WA_BINDING_WASM_TABLE;
                binding.wasmTable = export->index;
            }
            break;
            case WA_EXPORT_MEMORY:
            {
                binding.kind = WA_BINDING_WASM_MEMORY;
                binding.wasmMemory = export->index;
            }
            break;
        }

        wa_import_package_push_binding(arena, &package, &binding);
    }

    return (package);
}

wa_func* wa_instance_find_function(wa_instance* instance, oc_str8 name)
{
    wa_module* module = instance->module;

    wa_func* func = 0;
    for(u32 exportIndex = 0; exportIndex < module->exportCount; exportIndex++)
    {
        wa_export* export = &module->exports[exportIndex];
        if(export->kind == WA_EXPORT_FUNCTION && !oc_str8_cmp(export->name, name))
        {
            func = &instance->functions[export->index];
            break;
        }
    }
    return (func);
}

wa_func_type wa_func_get_type(oc_arena* arena, wa_instance* instance, wa_func* func)
{
    wa_func_type* type = func->type;

    wa_func_type res = (wa_func_type){
        .params = oc_arena_push_array(arena, wa_value_type, type->paramCount),
        .returns = oc_arena_push_array(arena, wa_value_type, type->returnCount),
        .paramCount = type->paramCount,
        .returnCount = type->returnCount,
    };
    memcpy(res.params, type->params, type->paramCount * sizeof(wa_value_type));
    memcpy(res.returns, type->returns, type->returnCount * sizeof(wa_value_type));

    return (res);
}

wa_global* wa_instance_find_global(wa_instance* instance, oc_str8 name)
{
    wa_module* module = instance->module;

    wa_global* global = 0;
    for(u32 exportIndex = 0; exportIndex < module->exportCount; exportIndex++)
    {
        wa_export* export = &module->exports[exportIndex];
        if(export->kind == WA_EXPORT_GLOBAL && !oc_str8_cmp(export->name, name))
        {
            global = instance->globals[export->index];
            break;
        }
    }
    return (global);
}

//-------------------------------------------------------------------------------
// Interpreter
//-------------------------------------------------------------------------------
typedef struct wa_control
{
    wa_instance* instance;
    wa_func* func;
    wa_code* returnPC;
    wa_value* returnFrame;
} wa_control;

enum
{
    WA_CONTROL_STACK_SIZE = 256,
    WA_LOCALS_BUFFER_SIZE = WA_MAX_SLOT_COUNT * 256,
};

typedef struct wa_interpreter
{
    wa_instance* instance;
    wa_code* code;

    u32 argCount;
    wa_value* args;
    u32 retCount;
    wa_value* returns;

    wa_control controlStack[WA_CONTROL_STACK_SIZE];
    u32 controlStackTop;

    wa_value* localsBuffer;
    wa_value* locals;
    wa_code* pc;

    _Atomic(bool) suspend;
    bool terminated;

    oc_arena arena;
    oc_list breakpoints;
    oc_list breakpointFreeList;

    wa_code cachedRegs[WA_MAX_REG];

} wa_interpreter;

wa_status wa_interpreter_init(wa_interpreter* interpreter,
                              wa_instance* instance,
                              wa_func* func,
                              wa_func_type* type,
                              wa_code* code,
                              u32 argCount,
                              wa_value* args,
                              u32 retCount,
                              wa_value* returns)
{
    interpreter->instance = instance;
    interpreter->code = code;
    interpreter->argCount = argCount;
    interpreter->args = args;
    interpreter->retCount = retCount;
    interpreter->returns = returns;
    interpreter->pc = code;
    interpreter->controlStack[0] = (wa_control){
        .instance = instance,
        .func = func,
    };
    interpreter->controlStackTop = 0;

    interpreter->terminated = false;

    interpreter->locals = interpreter->localsBuffer;
    memcpy(interpreter->locals, args, argCount * sizeof(wa_value));

    return WA_OK;
}

wa_status wa_interpreter_run(wa_interpreter* interpreter, bool step)
{
    if(interpreter->terminated)
    {
        return WA_TRAP_TERMINATED;
    }
    interpreter->suspend = false;

    wa_instance* instance = interpreter->instance;
    wa_memory* memory = 0;
    char* memPtr = 0;
    if(instance->memories)
    {
        memory = instance->memories[0];
        if(memory)
        {
            memPtr = memory->ptr;
        }
    }

    u32 localsIndex = interpreter->locals - interpreter->localsBuffer;
    if(localsIndex + WA_MAX_SLOT_COUNT >= WA_LOCALS_BUFFER_SIZE)
    {
        return (WA_TRAP_STACK_OVERFLOW);
    }

    while(!interpreter->suspend)
    {
        wa_instr_op opcode = interpreter->pc->opcode;
        interpreter->pc++;

        switch(opcode)
        {

#define I0 interpreter->pc[0]
#define I1 interpreter->pc[1]
#define I2 interpreter->pc[2]
#define I3 interpreter->pc[3]

#define L0 interpreter->locals[interpreter->pc[0].valI32]
#define L1 interpreter->locals[interpreter->pc[1].valI32]
#define L2 interpreter->locals[interpreter->pc[2].valI32]
#define L3 interpreter->locals[interpreter->pc[3].valI32]
#define L4 interpreter->locals[interpreter->pc[4].valI32]

#define G0 interpreter->instance->globals[interpreter->pc[0].valI32]->value
#define G1 interpreter->instance->globals[interpreter->pc[1].valI32]->value

            case WA_INSTR_breakpoint:
            {
                interpreter->pc--;
                return WA_TRAP_BREAKPOINT;
            }
            break;

            case WA_INSTR_unreachable:
            {
                return WA_TRAP_UNREACHABLE;
            }
            break;

            case WA_INSTR_i32_const:
            {
                L1.valI32 = I0.valI32;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_const:
            {
                L1.valI64 = I0.valI64;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_const:
            {
                L1.valF32 = I0.valF32;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f64_const:
            {
                L1.valF64 = I0.valF64;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_move:
            {
                memcpy(&L1, &L0, sizeof(wa_value));
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_global_get:
            {
                memcpy(&L1, &G0, sizeof(wa_value));
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_global_set:
            {
                memcpy(&G0, &L1, sizeof(wa_value));
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_select:
            {
                if(L2.valI32)
                {
                    memcpy(&L3, &L0, sizeof(u64));
                }
                else
                {
                    memcpy(&L3, &L1, sizeof(u64));
                }
                interpreter->pc += 4;
            }
            break;

#define WA_CHECK_READ_ACCESS(t)                                                                  \
    u32 offset = I0.memArg.offset + (u32)L1.valI32;                                              \
    if(offset < I0.memArg.offset                                                                 \
       || offset + sizeof(t) > memory->limits.min * WA_PAGE_SIZE || offset + sizeof(t) < offset) \
    {                                                                                            \
        /*OC_ASSERT(0, "read out of bounds");*/                                                  \
        return WA_TRAP_MEMORY_OUT_OF_BOUNDS;                                                     \
    }

            case WA_INSTR_i32_load:
            {
                WA_CHECK_READ_ACCESS(i32);
                L2.valI32 = *(i32*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_load:
            {
                WA_CHECK_READ_ACCESS(i64);
                L2.valI64 = *(i64*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f32_load:
            {
                WA_CHECK_READ_ACCESS(f32);
                L2.valF32 = *(f32*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f64_load:
            {
                WA_CHECK_READ_ACCESS(f64);
                L2.valF64 = *(f64*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_load8_s:
            {
                WA_CHECK_READ_ACCESS(u8);
                L2.valI32 = (i32) * (i8*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_load8_u:
            {
                WA_CHECK_READ_ACCESS(u8);
                *(u32*)&L2.valI32 = (u32) * (u8*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_load16_s:
            {
                WA_CHECK_READ_ACCESS(u16);
                L2.valI32 = (i32) * (i16*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_load16_u:
            {
                WA_CHECK_READ_ACCESS(u16);
                *(u32*)&L2.valI32 = (u32) * (u16*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_load8_s:
            {
                WA_CHECK_READ_ACCESS(u8);
                L2.valI64 = (i64) * (i8*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_load8_u:
            {
                WA_CHECK_READ_ACCESS(u8);
                *(u32*)&L2.valI64 = (u64) * (u8*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_load16_s:
            {
                WA_CHECK_READ_ACCESS(u16);
                L2.valI64 = (i64) * (i16*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_load16_u:
            {
                WA_CHECK_READ_ACCESS(u16);
                *(u32*)&L2.valI64 = (u64) * (u16*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_load32_s:
            {
                WA_CHECK_READ_ACCESS(u32);
                L2.valI64 = (i64) * (i32*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_load32_u:
            {
                WA_CHECK_READ_ACCESS(u32);
                *(u32*)&L2.valI64 = (u64) * (u32*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

#define WA_CHECK_WRITE_ACCESS(t)                                                                 \
    u32 offset = I0.memArg.offset + (u32)L1.valI32;                                              \
    if(offset < I0.memArg.offset                                                                 \
       || offset + sizeof(t) > memory->limits.min * WA_PAGE_SIZE || offset + sizeof(t) < offset) \
    {                                                                                            \
        /*OC_ASSERT(0, "write out of bounds");*/                                                 \
        return WA_TRAP_MEMORY_OUT_OF_BOUNDS;                                                     \
    }

            case WA_INSTR_i32_store:
            {
                WA_CHECK_WRITE_ACCESS(u32);
                *(i32*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = L2.valI32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_store:
            {
                WA_CHECK_WRITE_ACCESS(u64);
                *(i64*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = L2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f32_store:
            {
                WA_CHECK_WRITE_ACCESS(f32);
                *(f32*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = L2.valF32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f64_store:
            {
                WA_CHECK_WRITE_ACCESS(f64);
                *(f64*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = L2.valF64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_store8:
            {
                WA_CHECK_WRITE_ACCESS(u8);
                *(u8*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = *(u8*)&L2.valI32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_store16:
            {
                WA_CHECK_WRITE_ACCESS(u16);
                *(u16*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = *(u16*)&L2.valI32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_store8:
            {
                WA_CHECK_WRITE_ACCESS(u8);
                *(u8*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = *(u8*)&L2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_store16:
            {
                WA_CHECK_WRITE_ACCESS(u16);
                *(u16*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = *(u16*)&L2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_store32:
            {
                WA_CHECK_WRITE_ACCESS(u32);
                *(u32*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = *(u32*)&L2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_jump:
            {
                interpreter->pc += I0.valI64;
            }
            break;

            case WA_INSTR_jump_if_zero:
            {
                if(L1.valI32 == 0)
                {
                    interpreter->pc += I0.valI64;
                }
                else
                {
                    interpreter->pc += 2;
                }
            }
            break;

            case WA_INSTR_jump_table:
            {
                u32 count = I0.valU32;
                u32 index = L1.valI32;

                if(index >= count)
                {
                    index = count - 1;
                }

                interpreter->pc += interpreter->pc[2 + index].valI64;
            }
            break;

            case WA_INSTR_call:
            {
                wa_func* callee = &instance->functions[I0.valI64];
                i64 maxUsedSlot = I1.valI64;

                wa_instance* calleeInstance = instance;

                while(callee->extInstance)
                {
                    calleeInstance = callee->extInstance;
                    callee = &calleeInstance->functions[callee->extIndex];
                }

                if(callee->code)
                {
                    interpreter->controlStackTop++;
                    if(interpreter->controlStackTop >= WA_CONTROL_STACK_SIZE)
                    {
                        return (WA_TRAP_STACK_OVERFLOW);
                    }

                    interpreter->controlStack[interpreter->controlStackTop] = (wa_control){
                        .instance = calleeInstance,
                        .func = callee,
                        .returnPC = interpreter->pc + 2,
                        .returnFrame = interpreter->locals,
                    };

                    interpreter->locals += maxUsedSlot;
                    interpreter->pc = callee->code;
                    instance = calleeInstance;

                    if(interpreter->locals - interpreter->localsBuffer + WA_MAX_SLOT_COUNT >= WA_LOCALS_BUFFER_SIZE)
                    {
                        return (WA_TRAP_STACK_OVERFLOW);
                    }

                    if(instance->memories)
                    {
                        memory = instance->memories[0];
                        if(memory)
                        {
                            memPtr = memory->ptr;
                        }
                    }
                }
                else
                {
                    wa_value* saveLocals = interpreter->locals;
                    interpreter->locals += I1.valI64;
                    callee->proc(interpreter, interpreter->locals, interpreter->locals, callee->user);
                    interpreter->pc += 2;
                    interpreter->locals = saveLocals;
                }
            }
            break;

            case WA_INSTR_call_indirect:
            {
                u32 typeIndex = *(u32*)&I0.valI32;
                u32 tableIndex = *(u32*)&I1.valI32;
                i64 maxUsedSlot = I2.valI64;
                u32 index = *(u32*)&(L3.valI32);

                wa_table* table = instance->tables[tableIndex];

                if(index >= table->limits.min)
                {
                    return WA_TRAP_TABLE_OUT_OF_BOUNDS;
                }

                wa_instance* calleeInstance = table->contents[index].refInstance;
                u32 funcIndex = table->contents[index].refIndex;

                if(calleeInstance == 0)
                {
                    return WA_TRAP_REF_NULL;
                }

                wa_func* callee = &calleeInstance->functions[funcIndex];
                wa_func_type* t1 = callee->type;
                wa_func_type* t2 = &instance->module->types[typeIndex];

                if(!wa_check_function_type(t1, t2))
                {
                    return (WA_TRAP_INDIRECT_CALL_TYPE_MISMATCH);
                }

                while(callee->extInstance)
                {
                    calleeInstance = callee->extInstance;
                    callee = &calleeInstance->functions[callee->extIndex];
                }

                if(callee->code)
                {
                    interpreter->controlStackTop++;
                    if(interpreter->controlStackTop >= WA_CONTROL_STACK_SIZE)
                    {
                        return (WA_TRAP_STACK_OVERFLOW);
                    }

                    interpreter->controlStack[interpreter->controlStackTop] = (wa_control){
                        .instance = calleeInstance,
                        .func = callee,
                        .returnPC = interpreter->pc + 4,
                        .returnFrame = interpreter->locals,
                    };

                    interpreter->locals += maxUsedSlot;
                    interpreter->pc = callee->code;
                    instance = calleeInstance;

                    if(interpreter->locals - interpreter->localsBuffer + WA_MAX_SLOT_COUNT >= WA_LOCALS_BUFFER_SIZE)
                    {
                        return (WA_TRAP_STACK_OVERFLOW);
                    }

                    if(instance->memories)
                    {
                        memory = instance->memories[0];
                        if(memory)
                        {
                            memPtr = memory->ptr;
                        }
                    }
                }
                else
                {
                    wa_value* saveLocals = interpreter->locals;
                    interpreter->locals += maxUsedSlot;
                    callee->proc(interpreter, interpreter->locals, interpreter->locals, callee->user);
                    interpreter->pc += 4;
                    interpreter->locals = saveLocals;
                }
            }
            break;

            case WA_INSTR_return:
            {
                if(!interpreter->controlStackTop)
                {
                    goto end;
                }

                wa_control control = interpreter->controlStack[interpreter->controlStackTop];

                interpreter->locals = control.returnFrame;
                interpreter->pc = control.returnPC;

                interpreter->controlStackTop--;

                instance = interpreter->controlStack[interpreter->controlStackTop].instance;
                if(instance->memories)
                {
                    memory = instance->memories[0];
                    if(memory)
                    {
                        memPtr = memory->ptr;
                    }
                }
            }
            break;

            case WA_INSTR_ref_null:
            {
                L0.refInstance = 0;
                L0.refIndex = 0;
                interpreter->pc += 1;
            }
            break;

            case WA_INSTR_ref_is_null:
            {
                L1.valI32 = (L0.refInstance == 0) ? 1 : 0;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_ref_func:
            {
                L1.refInstance = instance,
                L1.refIndex = I0.valI64;
                interpreter->pc += 2;
            }
            break;

#define OPD1 L0
#define OPD2 L1
#define BRES L2
#define URES L1

            case WA_INSTR_i32_add:
            {
                BRES.valI32 = OPD1.valI32 + OPD2.valI32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_sub:
            {
                BRES.valI32 = OPD1.valI32 - OPD2.valI32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_mul:
            {
                BRES.valI32 = OPD1.valI32 * OPD2.valI32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_div_s:
            {
                if(OPD2.valI32 == 0)
                {
                    return WA_TRAP_DIVIDE_BY_ZERO;
                }
                else if(OPD1.valI32 == INT32_MIN && OPD2.valI32 == -1)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }
                else
                {
                    BRES.valI32 = OPD1.valI32 / OPD2.valI32;
                }
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_div_u:
            {
                if(OPD2.valI32 == 0)
                {
                    return WA_TRAP_DIVIDE_BY_ZERO;
                }
                else
                {
                    *(u32*)&BRES.valI32 = *(u32*)&OPD1.valI32 / *(u32*)&OPD2.valI32;
                }
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_rem_s:
            {
                if(OPD2.valI32 == 0)
                {
                    return WA_TRAP_DIVIDE_BY_ZERO;
                }
                else if(OPD1.valI32 == INT32_MIN && OPD2.valI32 == -1)
                {
                    BRES.valI32 = 0;
                }
                else
                {
                    BRES.valI32 = OPD1.valI32 % OPD2.valI32;
                }
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_rem_u:
            {
                if(OPD2.valI32 == 0)
                {
                    return WA_TRAP_DIVIDE_BY_ZERO;
                }
                else
                {
                    *(u32*)&BRES.valI32 = *(u32*)&OPD1.valI32 % *(u32*)&OPD2.valI32;
                }
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_and:
            {
                BRES.valI32 = OPD1.valI32 & OPD2.valI32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_or:
            {
                BRES.valI32 = OPD1.valI32 | OPD2.valI32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_xor:
            {
                BRES.valI32 = OPD1.valI32 ^ OPD2.valI32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_shl:
            {
                BRES.valI32 = OPD1.valI32 << OPD2.valI32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_shr_s:
            {
                BRES.valI32 = OPD1.valI32 >> OPD2.valI32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_shr_u:
            {
                *(u32*)&BRES.valI32 = *(u32*)&OPD1.valI32 >> *(u32*)&OPD2.valI32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_rotr:
            {
                u32 n = *(u32*)&OPD1.valI32;
                u32 r = *(u32*)&OPD2.valI32;
                *(u32*)&BRES.valI32 = (n >> r) | (n << (32 - r));
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_rotl:
            {
                u32 n = *(u32*)&OPD1.valI32;
                u32 r = *(u32*)&OPD2.valI32;
                *(u32*)&BRES.valI32 = (n << r) | (n >> (32 - r));
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_clz:
            {
                if(OPD1.valI32 == 0)
                {
                    URES.valI32 = 32;
                }
                else
                {
                    URES.valI32 = __builtin_clz(*(u32*)&OPD1.valI32);
                }
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_ctz:
            {
                if(OPD1.valI32 == 0)
                {
                    URES.valI32 = 32;
                }
                else
                {
                    URES.valI32 = __builtin_ctz(*(u32*)&OPD1.valI32);
                }
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_popcnt:
            {
                URES.valI32 = __builtin_popcount(*(u32*)&OPD1.valI32);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_extend8_s:
            {
                URES.valI32 = (i32)(i8)(OPD1.valI32 & 0xff);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_extend16_s:
            {
                URES.valI32 = (i32)(i16)(OPD1.valI32 & 0xffff);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_eqz:
            {
                URES.valI32 = (OPD1.valI32 == 0) ? 1 : 0;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_eq:
            {
                BRES.valI32 = (OPD1.valI32 == OPD2.valI32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_ne:
            {
                BRES.valI32 = (OPD1.valI32 != OPD2.valI32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_lt_s:
            {
                BRES.valI32 = (OPD1.valI32 < OPD2.valI32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_lt_u:
            {
                BRES.valI32 = (*(u32*)&OPD1.valI32 < *(u32*)&OPD2.valI32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_le_s:
            {
                BRES.valI32 = (OPD1.valI32 <= OPD2.valI32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_le_u:
            {
                BRES.valI32 = (*(u32*)&OPD1.valI32 <= *(u32*)&OPD2.valI32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_gt_s:
            {
                BRES.valI32 = (OPD1.valI32 > OPD2.valI32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_gt_u:
            {
                BRES.valI32 = (*(u32*)&OPD1.valI32 > *(u32*)&OPD2.valI32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_ge_s:
            {
                BRES.valI32 = (OPD1.valI32 >= OPD2.valI32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_ge_u:
            {
                BRES.valI32 = (*(u32*)&OPD1.valI32 >= *(u32*)&OPD2.valI32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_add:
            {
                BRES.valI64 = OPD1.valI64 + OPD2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_sub:
            {
                BRES.valI64 = OPD1.valI64 - OPD2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_mul:
            {
                BRES.valI64 = OPD1.valI64 * OPD2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_div_s:
            {
                if(OPD2.valI64 == 0)
                {
                    return WA_TRAP_DIVIDE_BY_ZERO;
                }
                else if(OPD1.valI64 == INT64_MIN && OPD2.valI64 == -1)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }
                else
                {
                    BRES.valI64 = OPD1.valI64 / OPD2.valI64;
                }
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_div_u:
            {
                if(OPD2.valI64 == 0)
                {
                    return WA_TRAP_DIVIDE_BY_ZERO;
                }
                else
                {
                    *(u64*)&BRES.valI64 = *(u64*)&OPD1.valI64 / *(u64*)&OPD2.valI64;
                }
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_rem_s:
            {
                if(OPD2.valI64 == 0)
                {
                    return WA_TRAP_DIVIDE_BY_ZERO;
                }
                else if(OPD1.valI64 == INT64_MIN && OPD2.valI64 == -1)
                {
                    BRES.valI64 = 0;
                }
                else
                {
                    BRES.valI64 = OPD1.valI64 % OPD2.valI64;
                }
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_rem_u:
            {
                if(OPD2.valI64 == 0)
                {
                    return WA_TRAP_DIVIDE_BY_ZERO;
                }
                else
                {
                    *(u64*)&BRES.valI64 = *(u64*)&OPD1.valI64 % *(u64*)&OPD2.valI64;
                }
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_and:
            {
                BRES.valI64 = OPD1.valI64 & OPD2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_or:
            {
                BRES.valI64 = OPD1.valI64 | OPD2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_xor:
            {
                BRES.valI64 = OPD1.valI64 ^ OPD2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_shl:
            {
                BRES.valI64 = OPD1.valI64 << OPD2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_shr_s:
            {
                BRES.valI64 = OPD1.valI64 >> OPD2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_shr_u:
            {
                *(u64*)&BRES.valI64 = *(u64*)&OPD1.valI64 >> *(u64*)&OPD2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_rotr:
            {
                u64 n = *(u64*)&OPD1.valI64;
                u64 r = *(u64*)&OPD2.valI64;
                *(u64*)&BRES.valI64 = (n >> r) | (n << (64 - r));
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_rotl:
            {
                u64 n = *(u64*)&OPD1.valI64;
                u64 r = *(u64*)&OPD2.valI64;
                *(u64*)&BRES.valI64 = (n << r) | (n >> (64 - r));
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_clz:
            {
                if(OPD1.valI64 == 0)
                {
                    URES.valI64 = 64;
                }
                else
                {
                    URES.valI64 = __builtin_clzl(*(u64*)&OPD1.valI64);
                }
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_ctz:
            {
                if(OPD1.valI64 == 0)
                {
                    URES.valI64 = 64;
                }
                else
                {
                    URES.valI64 = __builtin_ctzl(*(u64*)&OPD1.valI64);
                }
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_popcnt:
            {
                URES.valI64 = __builtin_popcountl(*(u64*)&OPD1.valI64);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_extend8_s:
            {
                URES.valI64 = (i64)(i8)(OPD1.valI64 & 0xff);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_extend16_s:
            {
                URES.valI64 = (i64)(i16)(OPD1.valI64 & 0xffff);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_extend32_s:
            {
                URES.valI64 = (i64)(i32)(OPD1.valI64 & 0xffffffff);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_eqz:
            {
                URES.valI32 = (OPD1.valI64 == 0) ? 1 : 0;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_eq:
            {
                BRES.valI32 = (OPD1.valI64 == OPD2.valI64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_ne:
            {
                BRES.valI32 = (OPD1.valI64 != OPD2.valI64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_lt_s:
            {
                BRES.valI32 = (OPD1.valI64 < OPD2.valI64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_lt_u:
            {
                BRES.valI32 = (*(u64*)&OPD1.valI64 < *(u64*)&OPD2.valI64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_le_s:
            {
                BRES.valI32 = (OPD1.valI64 <= OPD2.valI64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_le_u:
            {
                BRES.valI32 = (*(u64*)&OPD1.valI64 <= *(u64*)&OPD2.valI64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_gt_s:
            {
                BRES.valI32 = (OPD1.valI64 > OPD2.valI64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_gt_u:
            {
                BRES.valI32 = (*(u64*)&OPD1.valI64 > *(u64*)&OPD2.valI64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_ge_s:
            {
                BRES.valI32 = (OPD1.valI64 >= OPD2.valI64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_ge_u:
            {
                BRES.valI32 = (*(u64*)&OPD1.valI64 >= *(u64*)&OPD2.valI64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f32_eq:
            {
                BRES.valI32 = (OPD1.valF32 == OPD2.valF32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;
            case WA_INSTR_f32_ne:
            {
                BRES.valI32 = (OPD1.valF32 != OPD2.valF32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;
            case WA_INSTR_f32_lt:
            {
                BRES.valI32 = (OPD1.valF32 < OPD2.valF32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;
            case WA_INSTR_f32_gt:
            {
                BRES.valI32 = (OPD1.valF32 > OPD2.valF32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;
            case WA_INSTR_f32_le:
            {
                BRES.valI32 = (OPD1.valF32 <= OPD2.valF32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;
            case WA_INSTR_f32_ge:
            {
                BRES.valI32 = (OPD1.valF32 >= OPD2.valF32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f64_eq:
            {
                BRES.valI32 = (OPD1.valF64 == OPD2.valF64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;
            case WA_INSTR_f64_ne:
            {
                BRES.valI32 = (OPD1.valF64 != OPD2.valF64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;
            case WA_INSTR_f64_lt:
            {
                BRES.valI32 = (OPD1.valF64 < OPD2.valF64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;
            case WA_INSTR_f64_gt:
            {
                BRES.valI32 = (OPD1.valF64 > OPD2.valF64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;
            case WA_INSTR_f64_le:
            {
                BRES.valI32 = (OPD1.valF64 <= OPD2.valF64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;
            case WA_INSTR_f64_ge:
            {
                BRES.valI32 = (OPD1.valF64 >= OPD2.valF64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f32_abs:
            {
                URES.valF32 = fabsf(OPD1.valF32);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_neg:
            {
                URES.valF32 = -OPD1.valF32;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_ceil:
            {
                URES.valF32 = ceilf(OPD1.valF32);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_floor:
            {
                URES.valF32 = floorf(OPD1.valF32);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_trunc:
            {
                URES.valF32 = truncf(OPD1.valF32);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_nearest:
            {
                URES.valF32 = rintf(OPD1.valF32);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_sqrt:
            {
                URES.valF32 = sqrtf(OPD1.valF32);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_add:
            {
                BRES.valF32 = OPD1.valF32 + OPD2.valF32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f32_sub:
            {
                BRES.valF32 = OPD1.valF32 - OPD2.valF32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f32_mul:
            {
                BRES.valF32 = OPD1.valF32 * OPD2.valF32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f32_div:
            {
                BRES.valF32 = OPD1.valF32 / OPD2.valF32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f32_min:
            {
                f32 a = OPD1.valF32;
                f32 b = OPD2.valF32;
                if(isnan(a) || isnan(b))
                {
                    u32 u = 0x7fc00000;
                    memcpy(&BRES.valF32, &u, sizeof(f32));
                }
                else if(a == 0 && b == 0)
                {
                    BRES.valF32 = signbit(a) ? a : b;
                }
                else
                {
                    BRES.valF32 = oc_min(a, b);
                }
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f32_max:
            {
                f32 a = OPD1.valF32;
                f32 b = OPD2.valF32;
                if(isnan(a) || isnan(b))
                {
                    u32 u = 0x7fc00000;
                    memcpy(&BRES.valF32, &u, sizeof(f32));
                }
                else if(a == 0 && b == 0)
                {
                    BRES.valF32 = signbit(a) ? b : a;
                }

                else
                {
                    BRES.valF32 = oc_max(a, b);
                }
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f32_copysign:
            {
                BRES.valF32 = copysignf(OPD1.valF32, OPD2.valF32);
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f64_abs:
            {
                URES.valF64 = fabs(OPD1.valF64);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f64_neg:
            {
                URES.valF64 = -OPD1.valF64;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f64_ceil:
            {
                URES.valF64 = ceil(OPD1.valF64);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f64_floor:
            {
                URES.valF64 = floor(OPD1.valF64);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f64_trunc:
            {
                URES.valF64 = trunc(OPD1.valF64);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f64_nearest:
            {
                URES.valF64 = rint(OPD1.valF64);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f64_sqrt:
            {
                URES.valF64 = sqrt(OPD1.valF64);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f64_add:
            {
                BRES.valF64 = OPD1.valF64 + OPD2.valF64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f64_sub:
            {
                BRES.valF64 = OPD1.valF64 - OPD2.valF64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f64_mul:
            {
                BRES.valF64 = OPD1.valF64 * OPD2.valF64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f64_div:
            {
                BRES.valF64 = OPD1.valF64 / OPD2.valF64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f64_min:
            {
                f64 a = OPD1.valF64;
                f64 b = OPD2.valF64;

                if(isnan(a) || isnan(b))
                {
                    u64 u = 0x7ff8000000000000;
                    memcpy(&BRES.valF64, &u, sizeof(f64));
                }
                else if(a == 0 && b == 0)
                {
                    BRES.valF64 = signbit(a) ? a : b;
                }
                else
                {
                    BRES.valF64 = oc_min(a, b);
                }
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f64_max:
            {
                f64 a = OPD1.valF64;
                f64 b = OPD2.valF64;

                if(isnan(a) || isnan(b))
                {
                    u64 u = 0x7ff8000000000000;
                    memcpy(&BRES.valF64, &u, sizeof(f64));
                }
                else if(a == 0 && b == 0)
                {
                    BRES.valF64 = signbit(a) ? b : a;
                }
                else
                {
                    BRES.valF64 = oc_max(a, b);
                }
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f64_copysign:
            {
                BRES.valF64 = copysign(OPD1.valF64, OPD2.valF64);
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_wrap_i64:
            {
                URES.valI32 = (OPD1.valI64 & 0x00000000ffffffff);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_f32_s:
            {
                if(isnan(OPD1.valF32))
                {
                    return WA_TRAP_INVALID_INTEGER_CONVERSION;
                }
                else if(OPD1.valF32 >= 2147483648.0f || OPD1.valF32 < -2147483648.0f)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }

                URES.valI32 = (i32)truncf(OPD1.valF32);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_f32_u:
            {
                if(isnan(OPD1.valF32))
                {
                    return WA_TRAP_INVALID_INTEGER_CONVERSION;
                }

                if(OPD1.valF32 >= 4294967296.0f || OPD1.valF32 <= -1.0f)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }

                *(u32*)&URES.valI32 = (u32)truncf(OPD1.valF32);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_f64_s:
            {
                if(isnan(OPD1.valF64))
                {
                    return WA_TRAP_INVALID_INTEGER_CONVERSION;
                }

                if(OPD1.valF64 >= 2147483648.0 || OPD1.valF64 <= -2147483649.0)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }

                URES.valI32 = (i32)trunc(OPD1.valF64);
                interpreter->pc += 2;
            }
            break;
            case WA_INSTR_i32_trunc_f64_u:
            {
                if(isnan(OPD1.valF64))
                {
                    return WA_TRAP_INVALID_INTEGER_CONVERSION;
                }

                if(OPD1.valF64 >= 4294967296.0 || OPD1.valF64 <= -1.0)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }

                *(u32*)&URES.valI32 = (u32)trunc(OPD1.valF64);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_f32_s:
            {
                if(isnan(OPD1.valF32))
                {
                    return WA_TRAP_INVALID_INTEGER_CONVERSION;
                }

                if(OPD1.valF32 >= 9223372036854775808.0f || OPD1.valF32 < -9223372036854775808.0f)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }

                URES.valI64 = (i64)truncf(OPD1.valF32);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_f32_u:
            {
                if(isnan(OPD1.valF32))
                {
                    return WA_TRAP_INVALID_INTEGER_CONVERSION;
                }

                if(OPD1.valF32 >= 18446744073709551616.0f || OPD1.valF32 <= -1.0f)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }

                *(u64*)&URES.valI64 = (u64)truncf(OPD1.valF32);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_f64_s:
            {
                if(isnan(OPD1.valF64))
                {
                    return WA_TRAP_INVALID_INTEGER_CONVERSION;
                }

                if(OPD1.valF64 >= 9223372036854775808.0 || OPD1.valF64 < -9223372036854775808.0)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }

                URES.valI64 = (i64)trunc(OPD1.valF64);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_f64_u:
            {
                if(isnan(OPD1.valF64))
                {
                    return WA_TRAP_INVALID_INTEGER_CONVERSION;
                }

                if(OPD1.valF64 >= 18446744073709551616.0 || OPD1.valF64 <= -1.0)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }

                *(u64*)&URES.valI64 = (u64)trunc(OPD1.valF64);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_convert_i32_s:
            {
                URES.valF32 = (f32)OPD1.valI32;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_convert_i32_u:
            {
                URES.valF32 = (f32) * (u32*)&OPD1.valI32;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_convert_i64_s:
            {
                URES.valF32 = (f32)OPD1.valI64;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_convert_i64_u:
            {
                URES.valF32 = (f32) * (u64*)&OPD1.valI64;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_demote_f64:
            {
                URES.valF32 = (f32)OPD1.valF64;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f64_convert_i32_s:
            {
                URES.valF64 = (f64)OPD1.valI32;
                interpreter->pc += 2;
            }
            break;
            case WA_INSTR_f64_convert_i32_u:
            {
                URES.valF64 = (f64) * (u32*)&OPD1.valI32;
                interpreter->pc += 2;
            }
            break;
            case WA_INSTR_f64_convert_i64_s:
            {
                URES.valF64 = (f64)OPD1.valI64;
                interpreter->pc += 2;
            }
            break;
            case WA_INSTR_f64_convert_i64_u:
            {
                URES.valF64 = (f64) * (u64*)&OPD1.valI64;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f64_promote_f32:
            {
                URES.valF64 = (f64)OPD1.valF32;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_reinterpret_f32:
            {
                URES.valI32 = *(i32*)&OPD1.valF32;
                interpreter->pc += 2;
            }
            break;
            case WA_INSTR_i64_reinterpret_f64:
            {
                URES.valI64 = *(i64*)&OPD1.valF64;
                interpreter->pc += 2;
            }
            break;
            case WA_INSTR_f32_reinterpret_i32:
            {
                URES.valF32 = *(f32*)&OPD1.valI32;
                interpreter->pc += 2;
            }
            break;
            case WA_INSTR_f64_reinterpret_i64:
            {
                URES.valF64 = *(f64*)&OPD1.valI64;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_sat_f32_s:
            {
                if(isnan(OPD1.valF32))
                {
                    URES.valI32 = 0;
                }
                else if(OPD1.valF32 >= 2147483648.0f)
                {
                    URES.valI32 = INT32_MAX;
                }
                else if(OPD1.valF32 < -2147483648.0f)
                {
                    URES.valI32 = INT32_MIN;
                }
                else
                {
                    URES.valI32 = (i32)truncf(OPD1.valF32);
                }
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_sat_f32_u:
            {
                if(isnan(OPD1.valF32))
                {
                    URES.valI32 = 0;
                }
                else if(OPD1.valF32 >= 4294967296.0f)
                {
                    *(u32*)&URES.valI32 = 0xffffffff;
                }
                else if(OPD1.valF32 <= -1.0f)
                {
                    URES.valI32 = 0;
                }
                else
                {
                    *(u32*)&URES.valI32 = (u32)truncf(OPD1.valF32);
                }
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_sat_f64_s:
            {
                if(isnan(OPD1.valF64))
                {
                    URES.valI32 = 0;
                }
                else if(OPD1.valF64 >= 2147483648.0)
                {
                    URES.valI32 = INT32_MAX;
                }
                else if(OPD1.valF64 <= -2147483649.0)
                {
                    URES.valI32 = INT32_MIN;
                }
                else
                {
                    URES.valI32 = (i32)trunc(OPD1.valF64);
                }
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_sat_f64_u:
            {
                if(isnan(OPD1.valF64))
                {
                    URES.valI32 = 0;
                }
                else if(OPD1.valF64 >= 4294967296.0)
                {
                    *(u32*)&URES.valI32 = 0xffffffff;
                }
                else if(OPD1.valF64 <= -1.0)
                {
                    URES.valI32 = 0;
                }
                else
                {
                    *(u32*)&URES.valI32 = (u32)trunc(OPD1.valF64);
                }
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_sat_f32_s:
            {
                if(isnan(OPD1.valF32))
                {
                    URES.valI64 = 0;
                }
                else if(OPD1.valF32 >= 9223372036854775808.0f)
                {
                    URES.valI64 = INT64_MAX;
                }
                else if(OPD1.valF32 < -9223372036854775808.0f)
                {
                    URES.valI64 = INT64_MIN;
                }
                else
                {
                    URES.valI64 = (i64)truncf(OPD1.valF32);
                }
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_sat_f32_u:
            {
                if(isnan(OPD1.valF32))
                {
                    URES.valI64 = 0;
                }
                else if(OPD1.valF32 >= 18446744073709551616.0f)
                {
                    *(u64*)&URES.valI64 = 0xffffffffffffffffLLU;
                }
                else if(OPD1.valF32 <= -1.0f)
                {
                    URES.valI64 = 0;
                }
                else
                {
                    *(u64*)&URES.valI64 = (u64)truncf(OPD1.valF32);
                }
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_sat_f64_s:
            {
                if(isnan(OPD1.valF64))
                {
                    URES.valI64 = 0;
                }
                else if(OPD1.valF64 >= 9223372036854775808.0)
                {
                    URES.valI64 = INT64_MAX;
                }
                else if(OPD1.valF64 < -9223372036854775808.0)
                {
                    URES.valI64 = INT64_MIN;
                }
                else
                {
                    URES.valI64 = (i64)trunc(OPD1.valF64);
                }
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_sat_f64_u:
            {
                if(isnan(OPD1.valF64))
                {
                    URES.valI64 = 0;
                }
                else if(OPD1.valF64 >= 18446744073709551616.0)
                {
                    *(u64*)&URES.valI64 = 0xffffffffffffffffLLU;
                }
                else if(OPD1.valF64 <= -1.0)
                {
                    URES.valI64 = 0;
                }
                else
                {
                    *(u64*)&URES.valI64 = (u64)trunc(OPD1.valF64);
                }
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_extend_i32_s:
            {
                URES.valI64 = (i64)(i32)(OPD1.valI32);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_extend_i32_u:
            {
                URES.valI64 = *(u32*)&(OPD1.valI32);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_memory_size:
            {
                wa_memory* mem = instance->memories[0];
                L0.valI32 = (i32)(mem->limits.min);
                interpreter->pc += 1;
            }
            break;

            case WA_INSTR_memory_grow:
            {
                wa_memory* mem = instance->memories[0];

                i32 res = -1;
                u32 n = *(u32*)&(L0.valI32);
                oc_base_allocator* allocator = oc_base_allocator_default();

                if(mem->limits.min + n <= mem->limits.max
                   && (mem->limits.min + n >= mem->limits.min))
                {
                    res = mem->limits.min;
                    oc_base_commit(allocator, mem->ptr + mem->limits.min * WA_PAGE_SIZE, n * WA_PAGE_SIZE);
                    mem->limits.min += n;
                }

                L1.valI32 = res;

                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_memory_fill:
            {
                wa_memory* mem = instance->memories[0];

                u32 d = *(u32*)&L0.valI32;
                i32 val = L1.valI32;
                u32 n = *(u32*)&L2.valI32;

                if(d + n > mem->limits.min * WA_PAGE_SIZE || d + n < d)
                {
                    //OC_ASSERT(0, "fill out of bounds");
                    return WA_TRAP_MEMORY_OUT_OF_BOUNDS;
                }
                else
                {
                    memset(mem->ptr + d, val, n);
                }
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_memory_copy:
            {
                wa_memory* mem = instance->memories[0];
                u32 d = *(u32*)&L0.valI32;
                u32 s = *(u32*)&L1.valI32;
                u32 n = *(u32*)&L2.valI32;

                if(s + n > mem->limits.min * WA_PAGE_SIZE || s + n < s
                   || d + n > mem->limits.min * WA_PAGE_SIZE || d + n < d)
                {
                    //OC_ASSERT(0, "copy out of bounds");
                    return WA_TRAP_MEMORY_OUT_OF_BOUNDS;
                }
                memmove(mem->ptr + d, mem->ptr + s, n);
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_memory_init:
            {
                wa_memory* mem = instance->memories[0];
                wa_data_segment* seg = &instance->data[I0.valI32];

                u32 d = *(u32*)&L1.valI32;
                u32 s = *(u32*)&L2.valI32;
                u32 n = *(u32*)&L3.valI32;

                if(s + n > seg->init.len || s + n < s
                   || d + n > mem->limits.min * WA_PAGE_SIZE || d + n < d)
                {
                    //OC_ASSERT(0, "memory init out of bounds");
                    return WA_TRAP_MEMORY_OUT_OF_BOUNDS;
                }
                memmove(mem->ptr + d, seg->init.ptr + s, n);
                interpreter->pc += 4;
            }
            break;

            case WA_INSTR_data_drop:
            {
                wa_data_segment* seg = &instance->data[I0.valI32];
                seg->init.len = 0;
                interpreter->pc += 1;
            }
            break;

            case WA_INSTR_table_init:
            {
                wa_element* elt = &instance->elements[I0.valI32];
                wa_table* table = instance->tables[I1.valI32];

                u32 d = *(u32*)&L2.valI32;
                u32 s = *(u32*)&L3.valI32;
                u32 n = *(u32*)&L4.valI32;

                if(n + s > elt->initCount || n + s < n
                   || d + n > table->limits.min || d + n < d)
                {
                    return WA_TRAP_TABLE_OUT_OF_BOUNDS;
                }
                memmove(table->contents + d, elt->refs + s, n * sizeof(wa_value));
                interpreter->pc += 5;
            }
            break;

            case WA_INSTR_table_fill:
            {
                wa_table* table = instance->tables[I0.valI32];

                u32 d = *(u32*)&L1.valI32;
                wa_value val = L2;
                u32 n = *(u32*)&L3.valI32;

                if(d + n > table->limits.min || d + n < d)
                {
                    return WA_TRAP_TABLE_OUT_OF_BOUNDS;
                }
                for(u32 i = 0; i < n; i++)
                {
                    table->contents[d + i] = val;
                }

                interpreter->pc += 4;
            }
            break;

            case WA_INSTR_table_copy:
            {
                wa_table* tx = instance->tables[I0.valI32];
                wa_table* ty = instance->tables[I1.valI32];

                u32 d = *(u32*)&L2.valI32;
                u32 s = *(u32*)&L3.valI32;
                u32 n = *(u32*)&L4.valI32;

                if(s + n > ty->limits.min || s + n < s
                   || d + n > tx->limits.min || d + n < d)
                {
                    return WA_TRAP_TABLE_OUT_OF_BOUNDS;
                }
                memmove(tx->contents + d, ty->contents + s, n * sizeof(wa_value));
                interpreter->pc += 5;
            }
            break;

            case WA_INSTR_table_size:
            {
                wa_table* table = instance->tables[I0.valI32];
                L1.valI32 = table->limits.min;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_table_grow:
            {
                wa_table* table = instance->tables[I0.valI32];
                wa_limits limits = table->limits;
                wa_value val = L1;
                u32 size = L2.valI32;

                i32 ret = -1;
                if((u64)limits.min + (u64)size <= UINT32_MAX
                   && (limits.kind != WA_LIMIT_MIN_MAX || limits.min + size <= limits.max))
                {
                    wa_value* contents = oc_arena_push_array(instance->arena, wa_value, limits.min + size);
                    memcpy(contents, table->contents, limits.min * sizeof(wa_value));
                    for(u32 i = 0; i < size; i++)
                    {
                        contents[limits.min + i] = val;
                    }
                    ret = limits.min;
                    table->limits.min += size;
                    table->contents = contents;
                }
                L3.valI32 = ret;
                interpreter->pc += 4;
            }
            break;

            case WA_INSTR_table_get:
            {
                wa_table* table = instance->tables[I0.valI32];
                u32 eltIndex = L1.valI32;

                if(eltIndex >= table->limits.min)
                {
                    return WA_TRAP_TABLE_OUT_OF_BOUNDS;
                }
                L2 = table->contents[eltIndex];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_table_set:
            {
                wa_table* table = instance->tables[I0.valI32];
                u32 eltIndex = L1.valI32;
                wa_value val = L2;

                if(eltIndex >= table->limits.min)
                {
                    return WA_TRAP_TABLE_OUT_OF_BOUNDS;
                }
                table->contents[eltIndex] = val;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_elem_drop:
            {
                wa_element* elt = &instance->elements[I0.valI32];
                elt->initCount = 0;
                interpreter->pc += 1;
            }
            break;

            default:
                oc_log_error("invalid opcode %s\n", wa_instr_strings[opcode]);
                return WA_TRAP_INVALID_OP;
        }

        if(step)
        {
            break;
        }
    }

    if(step)
    {
        return WA_TRAP_STEP;
    }
    else
    {
        return WA_TRAP_SUSPENDED;
    }

end:
    interpreter->terminated = true;
    for(u32 retIndex = 0; retIndex < interpreter->retCount; retIndex++)
    {
        interpreter->returns[retIndex] = interpreter->locals[retIndex];
    }

    return WA_OK;
}

wa_status wa_instance_interpret_expr(wa_instance* instance,
                                     wa_func* func,
                                     wa_func_type* type,
                                     wa_code* code,
                                     u32 argCount,
                                     wa_value* args,
                                     u32 retCount,
                                     wa_value* returns)
{
    oc_arena_scope scratch = oc_scratch_begin();
    wa_interpreter* interpreter = wa_interpreter_create(scratch.arena);
    wa_interpreter_init(interpreter, instance, func, type, code, argCount, args, retCount, returns);
    wa_status status = wa_interpreter_run(interpreter, false);

    wa_interpreter_destroy(interpreter);
    oc_scratch_end(scratch);

    return status;
}

/*
wa_status wa_instance_invoke(wa_instance* instance,
                             wa_func* func,
                             u32 argCount,
                             wa_value* args,
                             u32 retCount,
                             wa_value* returns)
{
    if(argCount != func->type->paramCount || retCount != func->type->returnCount)
    {
        return WA_FAIL_INVALID_ARGS;
    }

    if(func->code)
    {
        return (wa_instance_interpret_expr(instance, func, func->type, func->code, argCount, args, retCount, returns));
    }
    else if(func->extInstance)
    {
        wa_func* extFunc = &func->extInstance->functions[func->extIndex];
        return wa_instance_invoke(func->extInstance, extFunc, argCount, args, retCount, returns);
    }
    else
    {
        /////////////////////////////////////////////////////
        //TODO: temporary
        /////////////////////////////////////////////////////

        wa_interpreter interpreter = {
            .instance = instance,
        };

        //TODO: host proc should return a status
        func->proc(&interpreter, args, returns, func->user);
        return WA_OK;
    }
}
*/

//-------------------------------------------------------------------------
// interpreter API
//-------------------------------------------------------------------------

wa_interpreter* wa_interpreter_create(oc_arena* arena)
{
    wa_interpreter* interpreter = oc_arena_push_type(arena, wa_interpreter);
    memset(interpreter, 0, sizeof(wa_interpreter));

    oc_base_allocator* alloc = oc_base_allocator_default();

    //TODO: should we rather allocate it in arena?
    interpreter->localsBuffer = oc_base_reserve(alloc, WA_LOCALS_BUFFER_SIZE * sizeof(wa_value));
    oc_base_commit(alloc, interpreter->localsBuffer, WA_LOCALS_BUFFER_SIZE * sizeof(wa_value));

    oc_arena_init(&interpreter->arena);

    return (interpreter);
}

void wa_interpreter_destroy(wa_interpreter* interpreter)
{
    oc_base_allocator* alloc = oc_base_allocator_default();
    oc_base_release(alloc, interpreter->localsBuffer, WA_LOCALS_BUFFER_SIZE * sizeof(wa_value));

    oc_arena_init(&interpreter->arena);
}

wa_instance* wa_interpreter_current_instance(wa_interpreter* interpreter)
{
    return (interpreter->instance);
}

//TODO
wa_status wa_interpreter_invoke(wa_interpreter* interpreter,
                                wa_instance* instance,
                                wa_func* function,
                                u32 argCount,
                                wa_value* args,
                                u32 retCount,
                                wa_value* returns)
{
    if(argCount != function->type->paramCount || retCount != function->type->returnCount)
    {
        return WA_FAIL_INVALID_ARGS;
    }

    if(function->code)
    {
        wa_interpreter_init(interpreter, instance, function, function->type, function->code, argCount, args, retCount, returns);
        wa_status status = wa_interpreter_run(interpreter, false);

        return status;
    }
    else if(function->extInstance)
    {
        wa_func* extFunc = &function->extInstance->functions[function->extIndex];
        return wa_interpreter_invoke(interpreter, function->extInstance, extFunc, argCount, args, retCount, returns);
    }
    else
    {
        //TODO: host proc should return a status
        function->proc(interpreter, args, returns, function->user);
        return WA_OK;
    }
}

//-------------------------------------------------------------------------
// debug helpers
//-------------------------------------------------------------------------

oc_str8 wa_module_get_function_name(wa_module* module, u32 index)
{
    oc_str8 res = { 0 };
    for(u32 entryIndex = 0; entryIndex < module->functionNameCount; entryIndex++)
    {
        wa_name_entry* entry = &module->functionNames[entryIndex];
        if(entry->index == index)
        {
            res = entry->name;
        }
    }
    return res;
}

void wa_print_stack_trace(wa_interpreter* interpreter)
{
    for(u32 level = 0; level <= interpreter->controlStackTop; level++)
    {
        wa_func* func = interpreter->controlStack[level].func;
        u64 addr = 0;
        if(level == interpreter->controlStackTop)
        {
            addr = interpreter->pc - func->code;
        }
        else
        {
            addr = interpreter->controlStack[level + 1].returnPC - 2 - func->code;
        }
        u32 functionIndex = func - interpreter->instance->functions;
        oc_str8 name = wa_module_get_function_name(interpreter->instance->module, functionIndex);

        printf("[%i] %.*s + 0x%08llx\n", level, oc_str8_ip(name), addr);
    }
}

//-------------------------------------------------------------------------
// debugger
//-------------------------------------------------------------------------
//TODO: move this in its own place

typedef struct wa_breakpoint
{
    oc_list_elt listElt;
    bool isLine;
    wa_line_loc lineLoc;
    wa_warm_loc warmLoc;
    wa_code savedOpcode;

} wa_breakpoint;

wa_breakpoint* wa_interpreter_find_breakpoint_any(wa_interpreter* interpreter, wa_warm_loc* loc)
{
    wa_breakpoint* result = 0;

    oc_list_for(interpreter->breakpoints, bp, wa_breakpoint, listElt)
    {
        if(bp->warmLoc.module == loc->module
           && bp->warmLoc.funcIndex == loc->funcIndex
           && bp->warmLoc.codeIndex == loc->codeIndex)
        {
            result = bp;
            break;
        }
    }

    return result;
}

wa_breakpoint* wa_interpreter_find_breakpoint(wa_interpreter* interpreter, wa_warm_loc* loc)
{
    wa_breakpoint* result = 0;

    oc_list_for(interpreter->breakpoints, bp, wa_breakpoint, listElt)
    {
        if(bp->isLine == false
           && bp->warmLoc.module == loc->module
           && bp->warmLoc.funcIndex == loc->funcIndex
           && bp->warmLoc.codeIndex == loc->codeIndex)
        {
            result = bp;
            break;
        }
    }

    return result;
}

wa_breakpoint* wa_interpreter_add_breakpoint(wa_interpreter* interpreter, wa_warm_loc* loc)
{
    wa_breakpoint* bp = wa_interpreter_find_breakpoint(interpreter, loc);
    if(bp == 0)
    {
        bp = oc_list_pop_front_entry(&interpreter->breakpointFreeList, wa_breakpoint, listElt);
        if(!bp)
        {
            bp = oc_arena_push_type(&interpreter->arena, wa_breakpoint);
        }
        bp->isLine = false;
        bp->warmLoc = *loc;
        oc_list_push_back(&interpreter->breakpoints, &bp->listElt);

        wa_func* func = &interpreter->instance->functions[bp->warmLoc.funcIndex];
        bp->savedOpcode = func->code[bp->warmLoc.codeIndex];
        func->code[bp->warmLoc.codeIndex].opcode = WA_INSTR_breakpoint;
    }
    return bp;
}

wa_breakpoint* wa_interpreter_find_breakpoint_line(wa_interpreter* interpreter, wa_line_loc* loc)
{
    wa_breakpoint* result = 0;

    oc_list_for(interpreter->breakpoints, bp, wa_breakpoint, listElt)
    {
        if(bp->isLine == true
           && bp->lineLoc.fileIndex == loc->fileIndex
           && bp->lineLoc.line == loc->line)
        {
            result = bp;
            break;
        }
    }

    return result;
}

wa_warm_loc wa_warm_loc_from_line_loc(wa_module* module, wa_line_loc loc);
wa_line_loc wa_line_loc_from_warm_loc(wa_module* module, wa_warm_loc loc);

wa_breakpoint* wa_interpreter_add_breakpoint_line(wa_interpreter* interpreter, wa_line_loc* loc)
{
    wa_breakpoint* bp = wa_interpreter_find_breakpoint_line(interpreter, loc);
    if(bp == 0)
    {
        wa_warm_loc warmLoc = wa_warm_loc_from_line_loc(interpreter->instance->module,
                                                        *loc);
        if(warmLoc.module)
        {
            bp = oc_list_pop_front_entry(&interpreter->breakpointFreeList, wa_breakpoint, listElt);
            if(!bp)
            {
                bp = oc_arena_push_type(&interpreter->arena, wa_breakpoint);
            }
            bp->isLine = true;
            bp->warmLoc = warmLoc;
            bp->lineLoc = *loc;
            oc_list_push_back(&interpreter->breakpoints, &bp->listElt);

            wa_func* func = &interpreter->instance->functions[bp->warmLoc.funcIndex];
            bp->savedOpcode = func->code[bp->warmLoc.codeIndex];
            func->code[bp->warmLoc.codeIndex].opcode = WA_INSTR_breakpoint;
        }
    }
    return bp;
}

void wa_interpreter_remove_breakpoint(wa_interpreter* interpreter, wa_breakpoint* bp)
{
    wa_func* func = &interpreter->instance->functions[bp->warmLoc.funcIndex];
    func->code[bp->warmLoc.codeIndex] = bp->savedOpcode;

    oc_list_remove(&interpreter->breakpoints, &bp->listElt);
    oc_list_push_back(&interpreter->breakpointFreeList, &bp->listElt);
}

wa_instr_op wa_breakpoint_saved_opcode(wa_breakpoint* bp)
{
    return bp->savedOpcode.opcode;
}

void wa_interpreter_cache_registers(wa_interpreter* interpreter)
{
    wa_func* execFunc = interpreter->controlStack[interpreter->controlStackTop].func;
    u32 funcIndex = execFunc - interpreter->instance->functions;
    u32 codeIndex = interpreter->pc - execFunc->code;

    for(u64 regIndex = 0; regIndex < execFunc->maxRegCount; regIndex++)
    {
        interpreter->cachedRegs[regIndex].valI64 = interpreter->locals[regIndex].valI64;
    }
}

wa_status wa_interpreter_continue(wa_interpreter* interpreter)
{
    //TODO: if we're on a breakpoint, deactivate it, step, reactivate, continue
    wa_interpreter_cache_registers(interpreter);

    wa_func* func = interpreter->controlStack[interpreter->controlStackTop].func;
    u64 funcIndex = func - interpreter->instance->functions;

    wa_breakpoint* bp = wa_interpreter_find_breakpoint_any(
        interpreter,
        &(wa_warm_loc){
            .module = interpreter->instance->module,
            .funcIndex = funcIndex,
            .codeIndex = interpreter->pc - func->code,
        });

    if(bp)
    {
        func->code[bp->warmLoc.codeIndex] = bp->savedOpcode;

        wa_status status = wa_interpreter_run(interpreter, true);
        //TODO: check if program terminated

        func->code[bp->warmLoc.codeIndex].opcode = WA_INSTR_breakpoint;
    }

    return wa_interpreter_run(interpreter, false);
}

wa_status wa_interpreter_step(wa_interpreter* interpreter)
{
    wa_interpreter_cache_registers(interpreter);

    wa_func* func = interpreter->controlStack[interpreter->controlStackTop].func;
    u64 funcIndex = func - interpreter->instance->functions;

    wa_breakpoint* bp = wa_interpreter_find_breakpoint_any(
        interpreter,
        &(wa_warm_loc){
            .module = interpreter->instance->module,
            .funcIndex = funcIndex,
            .codeIndex = interpreter->pc - func->code,
        });

    if(bp)
    {
        func->code[bp->warmLoc.codeIndex] = bp->savedOpcode;
    }

    wa_status status = wa_interpreter_run(interpreter, true);

    if(bp)
    {
        func->code[bp->warmLoc.codeIndex].opcode = WA_INSTR_breakpoint;
    }

    return status;
}

wa_status wa_interpreter_step_line(wa_interpreter* interpreter)
{
    wa_interpreter_cache_registers(interpreter);

    //NOTE: step the dumbest possible way
    wa_status status = WA_OK;
    wa_func* func = interpreter->controlStack[interpreter->controlStackTop].func;
    u64 funcIndex = func - interpreter->instance->functions;

    wa_line_loc startLoc = wa_line_loc_from_warm_loc(
        interpreter->instance->module,
        (wa_warm_loc){
            .module = interpreter->instance->module,
            .funcIndex = funcIndex,
            .codeIndex = interpreter->pc - func->code,
        });

    wa_line_loc lineLoc = startLoc;
    while((lineLoc.fileIndex == startLoc.fileIndex && lineLoc.line == startLoc.line)
          || lineLoc.line == 0)
    {
        wa_breakpoint* bp = wa_interpreter_find_breakpoint_any(
            interpreter,
            &(wa_warm_loc){
                .module = interpreter->instance->module,
                .funcIndex = funcIndex,
                .codeIndex = interpreter->pc - func->code,
            });

        if(bp)
        {
            func->code[bp->warmLoc.codeIndex] = bp->savedOpcode;
        }

        status = wa_interpreter_run(interpreter, true);

        if(bp)
        {
            func->code[bp->warmLoc.codeIndex].opcode = WA_INSTR_breakpoint;
        }

        if(status != WA_TRAP_STEP && status != WA_TRAP_BREAKPOINT)
        {
            break;
        }

        func = interpreter->controlStack[interpreter->controlStackTop].func;
        funcIndex = func - interpreter->instance->functions;

        lineLoc = wa_line_loc_from_warm_loc(
            interpreter->instance->module,
            (wa_warm_loc){
                .module = interpreter->instance->module,
                .funcIndex = funcIndex,
                .codeIndex = interpreter->pc - func->code,
            });
    }

    return status;
}

void wa_interpreter_suspend(wa_interpreter* interpreter)
{
    interpreter->suspend = true;
}

//-------------------------------------------------------------------------
// bytecode -> instr map
//-------------------------------------------------------------------------

typedef struct wa_wasm_loc
{
    wa_module* module;
    u32 offset;
} wa_wasm_loc;

wa_wasm_loc wa_wasm_loc_from_warm_loc(wa_warm_loc loc)
{
    u64 id = (u64)loc.funcIndex << 32 | (u64)loc.codeIndex;
    u64 hash = oc_hash_xx64_string((oc_str8){ .ptr = (char*)&id, .len = 8 });
    u64 index = hash % loc.module->debugInfo.warmToWasmMapLen;

    wa_instr* instr = 0;
    oc_list_for(loc.module->debugInfo.warmToWasmMap[index], mapping, wa_bytecode_mapping, listElt)
    {
        if(mapping->funcIndex == loc.funcIndex && mapping->codeIndex == loc.codeIndex)
        {
            instr = mapping->instr;
            break;
        }
    }
    wa_wasm_loc result = { 0 };
    if(instr)
    {
        result.module = loc.module;
        result.offset = instr->ast->loc.start - loc.module->toc.code.offset;
    }
    return (result);
}

wa_warm_loc wa_warm_loc_from_wasm_loc(wa_wasm_loc loc)
{
    wa_warm_loc result = { 0 };

    u64 id = loc.offset;
    u64 hash = oc_hash_xx64_string((oc_str8){ .ptr = (char*)&id, .len = 8 });
    u64 index = hash % loc.module->debugInfo.wasmToWarmMapLen;

    wa_instr* instr = 0;
    oc_list_for(loc.module->debugInfo.wasmToWarmMap[index], mapping, wa_bytecode_mapping, listElt)
    {
        if((mapping->instr->ast->loc.start - loc.module->toc.code.offset) == loc.offset)
        {
            result.module = loc.module;
            result.funcIndex = mapping->funcIndex;
            result.codeIndex = mapping->codeIndex;
            break;
        }
    }

    return (result);
}

wa_line_loc wa_line_loc_from_warm_loc(wa_module* module, wa_warm_loc loc)
{
    wa_line_loc res = { 0 };
    wa_wasm_loc wasmLoc = wa_wasm_loc_from_warm_loc(loc);

    for(u64 entryIndex = 0; entryIndex < module->debugInfo.wasmToLineCount; entryIndex++)
    {
        wa_wasm_to_line_entry* entry = &module->debugInfo.wasmToLine[entryIndex];
        if(entry->wasmOffset > wasmLoc.offset)
        {
            if(entryIndex)
            {
                res = module->debugInfo.wasmToLine[entryIndex - 1].loc;
            }
            break;
        }
    }

    return (res);
}

wa_warm_loc wa_warm_loc_from_line_loc(wa_module* module, wa_line_loc loc)
{
    wa_warm_loc result = { 0 };
    //TODO: this is super dumb for now, just pick the lowest line that's >= to the line we're looking for
    wa_wasm_loc wasmLoc = { 0 };

    u64 currentLine = UINT64_MAX;

    for(u64 entryIndex = 0; entryIndex < module->debugInfo.wasmToLineCount; entryIndex++)
    {
        wa_wasm_to_line_entry* entry = &module->debugInfo.wasmToLine[entryIndex];

        if(loc.fileIndex == entry->loc.fileIndex
           && entry->loc.line >= loc.line
           && entry->loc.line < currentLine)
        {
            currentLine = entry->loc.line;
            wasmLoc.module = module;
            wasmLoc.offset = entry->wasmOffset;
        }
    }

    if(wasmLoc.module)
    {
        result = wa_warm_loc_from_wasm_loc(wasmLoc);
    }
    return result;
}

typedef enum dw_stack_value_type
{
    DW_STACK_VALUE_ADDRESS,
    DW_STACK_VALUE_OPERAND,
    DW_STACK_VALUE_LOCAL,
    DW_STACK_VALUE_GLOBAL,
    DW_STACK_VALUE_U64,
    //...

} dw_stack_value_type;

typedef struct dw_stack_value
{
    dw_stack_value_type type;

    union
    {
        u32 valU32;
        u64 valU64;
        //...
    };
} dw_stack_value;

dw_stack_value wa_interpret_dwarf_expr(wa_interpreter* interpreter, wa_debug_function* funcInfo, oc_str8 expr)
{
    u64 sp = 0;

    const u64 DW_STACK_MAX = 1024;
    dw_stack_value stack[DW_STACK_MAX];

    dw_reader reader = {
        .contents = expr,
    };

    while(dw_reader_has_more(&reader))
    {
        dw_op op = dw_read_u8(&reader);

        switch(op)
        {
            case DW_OP_addr:
            {
                u32 opd = dw_read_u32(&reader);

                stack[sp] = (dw_stack_value){
                    .type = DW_STACK_VALUE_ADDRESS,
                    .valU32 = opd,
                };
                sp++;
            }
            break;

            case DW_OP_fbreg:
            {
                i64 offset = dw_read_leb128_i64(&reader);

                OC_ASSERT(funcInfo->frameBase->single && funcInfo->frameBase->entryCount == 1);
                dw_stack_value frameBase = wa_interpret_dwarf_expr(interpreter, funcInfo, funcInfo->frameBase->entries[0].desc);

                /*NOTE: what the spec says and what clang does seem to differ:
                    - dwarf says that DW_OP_stack_value means the _value_ of the object (not its location) is on the top of the stack
                    - But clang often produces expressions of the form (DW_OP_WASM_location 0x00 idx, DW_OP_stack_value) for frame bases,
                    and the expected result is the memory address stored in local idx.
                    So, if we get a wasm local location here, we load its contents before terminating the expression...
                */
                if(frameBase.type == DW_STACK_VALUE_LOCAL)
                {
                    frameBase.type = DW_STACK_VALUE_ADDRESS;
                    frameBase.valU32 = interpreter->locals[frameBase.valU32].valI32;
                }
                //TODO: otherwise load anyway???

                stack[sp] = (dw_stack_value){
                    .type = DW_STACK_VALUE_ADDRESS,
                    .valU32 = frameBase.valU32 + offset,
                };
                sp++;
            }
            break;

            case DW_OP_WASM_location:
            {
                u8 kind = dw_read_u8(&reader);
                switch(kind)
                {
                    case 0x00:
                    {
                        //NOTE: wasm local
                        u32 index = dw_read_leb128_u32(&reader);
                        stack[sp] = (dw_stack_value){
                            .type = DW_STACK_VALUE_LOCAL,
                            .valU32 = index,
                        };
                        sp++;
                    }
                    break;
                    case 0x01:
                    {
                        //NOTE: wasm global, leb128
                        u32 index = dw_read_leb128_u32(&reader);
                        stack[sp] = (dw_stack_value){
                            .type = DW_STACK_VALUE_GLOBAL,
                            .valU32 = index,
                        };
                        sp++;
                    }
                    break;
                    case 0x02:
                    {
                        //NOTE: wasm operand stack
                        u32 index = dw_read_leb128_u32(&reader);
                        stack[sp] = (dw_stack_value){
                            .type = DW_STACK_VALUE_OPERAND,
                            .valU32 = index,
                        };
                        sp++;
                    }
                    break;
                    case 0x03:
                    {
                        //NOTE: wasm global, u32
                        u32 index = dw_read_u32(&reader);
                        stack[sp] = (dw_stack_value){
                            .type = DW_STACK_VALUE_GLOBAL,
                            .valU32 = index,
                        };
                        sp++;
                    }
                    break;
                    default:
                        oc_log_error("unrecognized WASM location kind %hhu\n", kind);
                        goto end;
                }
            }
            break;

            case DW_OP_stack_value:
            {
                goto end;
            }
            break;

            default:
                oc_log_error("unsupported dwarf op %s\n", dw_op_get_string(op));
                goto end;
        }
    }

end:
    OC_ASSERT(sp > 0);
    return (stack[sp - 1]);
}

oc_str8 wa_debug_variable_get_value(oc_arena* arena, wa_interpreter* interpreter, wa_debug_function* funcInfo, wa_debug_variable* var)
{
    if(!var->loc)
    {
        return (oc_str8){ 0 };
    }
    wa_debug_type* type = wa_debug_type_strip(var->type);

    oc_str8 res = {
        .len = type->size,
        .ptr = oc_arena_push_aligned(arena, type->size, 8),
    };
    memset(res.ptr, 0, res.len);

    dw_loc* loc = var->loc;

    for(u64 entryIndex = 0; entryIndex < loc->entryCount; entryIndex++)
    {
        dw_loc_entry* entry = &loc->entries[entryIndex];

        dw_stack_value val = wa_interpret_dwarf_expr(interpreter, funcInfo, entry->desc);

        switch(val.type)
        {
            case DW_STACK_VALUE_ADDRESS:
            {
                //TODO: bounds check
                if(loc->single)
                {
                    memcpy(res.ptr, interpreter->instance->memories[0]->ptr + val.valU32, res.len);
                }
                else
                {
                    memcpy(res.ptr + entry->start, interpreter->instance->memories[0]->ptr + val.valU32, entry->end - entry->start);
                }
            }
            break;

            default:
                break;
        }
    end:
        continue;
    }

    return res;
}

#include "parser.c"
#include "compiler.c"
#include "module.c"
