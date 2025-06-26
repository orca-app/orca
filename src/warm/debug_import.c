/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "warm.h"
#include "debug_info.h"
#include "dwarf.h"

//------------------------------------------------------------------------
// Dwarf line processing
//------------------------------------------------------------------------

wa_source_node* wa_source_node_alloc(wa_module* module)
{
    wa_source_node* node = oc_arena_push_type(module->arena, wa_source_node);
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
        ////////////////////////////////////////////////////////////////
        //TODO: review the way we deduplicate files...
        ////////////////////////////////////////////////////////////////
        if(!oc_str8_cmp(elt->file.fullPath, fullPath))
        {
            file = elt;
        }
    }

    if(!file)
    {
        file = oc_arena_push_type(scratchArena, wa_source_file_elt);
        file->file.fullPath = oc_str8_push_copy(pathArena, fullPath);
        file->file.rootPath = oc_str8_slice(file->file.fullPath, 0, rootPath.len);
        file->index = *fileCount;
        oc_list_push_back(files, &file->listElt);
        (*fileCount)++;
    }
    return file;
}

void wa_dwarf_error_callback(dw_parser* parser, u64 loc, oc_str8 message, void* user)
{
    wa_module* module = (wa_module*)user;

    wa_module_error* error = oc_arena_push_type(module->arena, wa_module_error);

    error->loc = loc;
    error->status = WA_PARSE_ERROR;
    error->string = oc_str8_push_copy(module->arena, message);
    oc_list_push_back(&module->errors, &error->listElt);
}

//-------------------------------------------------------------------------
// Debug type processing
//-------------------------------------------------------------------------

wa_type* wa_type_alloc(oc_arena* arena, u64 typeRef, wa_type_kind kind, oc_list* types)
{
    wa_type* type = oc_arena_push_type(arena, wa_type);
    type->kind = kind;
    type->dwarfRef = typeRef;
    oc_list_push_back(types, &type->listElt);
    return type;
}

wa_type* wa_type_strip(wa_type* type)
{
    while(type && type->kind == WA_TYPE_NAMED)
    {
        type = type->type;
    }
    return type;
}

wa_type* wa_build_debug_type_from_dwarf(oc_arena* arena, dw_info* dwarf, u64 typeRef, oc_list* types)
{
    //NOTE: if we already parsed that type, return it
    oc_list_for(*types, t, wa_type, listElt)
    {
        if(t->dwarfRef == typeRef)
        {
            return (t);
        }
    }

    wa_type* type = 0;

    //NOTE: find die corresponding to typeRef
    u64 addressSize = 4;
    dw_die* die = 0;
    for(u64 unitIndex = 0; unitIndex < dwarf->unitCount; unitIndex++)
    {
        dw_unit* unit = &dwarf->units[unitIndex];
        u64 unitSize = unit->initialLength + (unit->format == DW_DWARF32 ? 4 : 8);

        if(typeRef >= unit->start && typeRef < unit->start + unitSize)
        {
            dw_die* rootDie = oc_catch_ptr(unit->rootDie)
            {
                break;
            }

            die = dw_die_next(rootDie, rootDie);
            while(die && die->start != typeRef)
            {
                die = dw_die_next(rootDie, die);
            }

            if(die)
            {
                addressSize = unit->addressSize;
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
                type = wa_type_alloc(arena, typeRef, WA_TYPE_BASIC, types);

                //TODO: endianity

                dw_attr* encoding = dw_die_get_attr(die, DW_AT_encoding);
                if(encoding)
                {
                    switch(encoding->valU64)
                    {
                        case DW_ATE_boolean:
                            type->encoding = WA_TYPE_BOOL;
                            break;
                        case DW_ATE_address:
                        case DW_ATE_unsigned:
                        case DW_ATE_unsigned_char:
                            type->encoding = WA_TYPE_UNSIGNED;
                            break;
                        case DW_ATE_signed:
                        case DW_ATE_signed_char:
                            type->encoding = WA_TYPE_SIGNED;
                            break;
                        case DW_ATE_float:
                            type->encoding = WA_TYPE_FLOAT;
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
                type = wa_type_alloc(arena, typeRef, WA_TYPE_VOID, types);
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
                type = wa_type_alloc(arena, typeRef, WA_TYPE_POINTER, types);
                type->size = addressSize;

                dw_attr* typeAttr = dw_die_get_attr(die, DW_AT_type);
                if(typeAttr)
                {
                    type->type = wa_build_debug_type_from_dwarf(arena, dwarf, typeAttr->valU64, types);
                    //TODO: should we assert that type->type is always non null?
                    if(type->type)
                    {
                        type->name = oc_str8_pushf(arena, "%.*s*", oc_str8_ip(type->type->name));
                    }
                }
            }
            break;

            case DW_TAG_typedef:
            {
                type = wa_type_alloc(arena, typeRef, WA_TYPE_NAMED, types);

                dw_attr* typeAttr = dw_die_get_attr(die, DW_AT_type);
                if(typeAttr)
                {
                    type->type = wa_build_debug_type_from_dwarf(arena, dwarf, typeAttr->valU64, types);
                }
            }
            break;

            case DW_TAG_array_type:
            {
                type = wa_type_alloc(arena, typeRef, WA_TYPE_ARRAY, types);

                dw_attr* typeAttr = dw_die_get_attr(die, DW_AT_type);
                if(typeAttr)
                {
                    type->array.type = wa_build_debug_type_from_dwarf(arena, dwarf, typeAttr->valU64, types);
                }

                //TODO stride

                ///////////////////////////////////////
                //TODO: multi dim arrays...
                ///////////////////////////////////////
                u64 size = wa_type_strip(type->array.type)->size;

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
                wa_type_kind kind = (die->abbrev->tag == DW_TAG_structure_type) ? WA_TYPE_STRUCT
                                                                                : WA_TYPE_UNION;
                type = wa_type_alloc(arena, typeRef, kind, types);

                oc_list_for(die->children, child, dw_die, parentElt)
                {
                    if(child->abbrev && child->abbrev->tag == DW_TAG_member)
                    {
                        wa_type_field* member = oc_arena_push_type(arena, wa_type_field);

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
                type = wa_type_alloc(arena, typeRef, WA_TYPE_ENUM, types);

                dw_attr* valType = dw_die_get_attr(die, DW_AT_type);
                if(valType)
                {
                    type->enumType.type = wa_build_debug_type_from_dwarf(arena, dwarf, valType->valU64, types);
                    type->size = wa_type_strip(type->enumType.type)->size;
                }

                oc_list_for(die->children, child, dw_die, parentElt)
                {
                    if(child->abbrev && child->abbrev->tag == DW_TAG_enumerator)
                    {
                        wa_type_enumerator* enumerator = oc_arena_push_type(arena, wa_type_enumerator);
                        dw_attr* name = dw_die_get_attr(child, DW_AT_name);
                        if(name)
                        {
                            enumerator->name = name->string;
                        }
                        //TODO: extract const values

                        oc_list_push_back(&type->enumType.enumerators, &enumerator->listElt);
                    }
                }
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

wa_debug_variable wa_debug_import_variable(oc_arena* arena, dw_die* varDie, dw_info* dwarf, oc_list* types)
{
    wa_debug_variable var = { 0 };

    dw_attr* name = dw_die_get_attr(varDie, DW_AT_name);
    if(name)
    {
        var.name = oc_str8_push_copy(arena, name->string);
    }

    dw_attr* loc = dw_die_get_attr(varDie, DW_AT_location);
    if(loc)
    {
        var.loc = &loc->loc;
        //TODO: compile the expr to wasm
    }

    dw_attr* type = dw_die_get_attr(varDie, DW_AT_type);
    if(type)
    {
        var.type = wa_build_debug_type_from_dwarf(arena, dwarf, type->valU64, types);
    }
    return var;
}

void wa_debug_extract_vars_from_scope(oc_arena* arena, wa_debug_function* funcInfo, wa_debug_scope* scope, dw_die* scopeDie, u64 unitBaseAddress, dw_info* dwarf, oc_list* types)
{
    //NOTE: set scope's extents
    dw_attr* lowPC = dw_die_get_attr(scopeDie, DW_AT_low_pc);
    if(lowPC)
    {
        dw_attr* highPC = dw_die_get_attr(scopeDie, DW_AT_high_pc);
        if(!highPC)
        {
            ////////////////////////////////////////////////////////////////////
            //TODO: process error
            ////////////////////////////////////////////////////////////////////
            OC_ASSERT(0, "TODO: dwarf error");
        }
        else
        {
            scope->rangeCount = 1;
            scope->ranges = oc_arena_push_type(arena, wa_debug_range);

            scope->ranges[0].low = lowPC->valU64;

            dw_attr_class attrClass = dw_attr_get_class(DW_AT_high_pc, highPC->abbrev->form);

            if(attrClass == DW_AT_CLASS_address)
            {
                scope->ranges[0].high = highPC->valU64;
            }
            else if(attrClass == DW_AT_CLASS_constant)
            {
                scope->ranges[0].high = scope->ranges[0].low + highPC->valU64;
            }
            else
            {
                ////////////////////////////////////////////////////////////////////
                //TODO: process error? or should have detected it earlier?
                ////////////////////////////////////////////////////////////////////
                OC_ASSERT(0, "TODO: dwarf error");
            }
        }
    }
    else
    {
        dw_attr* rangesAttr = dw_die_get_attr(scopeDie, DW_AT_ranges);
        if(rangesAttr)
        {
            //NOTE: get base address of unit
            u64 baseAddress = unitBaseAddress;

            //NOTE: allocate a scratch buffer of ranges. This is because we don't know the final count of ranges,
            // since some dwarf range entries are base selection entries that don't make it into the final range list
            oc_arena_scope scratch = oc_scratch_begin();
            wa_debug_range* ranges = oc_arena_push_array(scratch.arena, wa_debug_range, rangesAttr->ranges.entryCount);

            for(u64 i = 0; i < rangesAttr->ranges.entryCount; i++)
            {
                dw_range_entry* entry = &rangesAttr->ranges.entries[i];
                if(entry->start == 0xffffffffffffffff)
                {
                    // base selection entry, change base addres and skip
                    baseAddress = entry->end;
                }
                else
                {
                    // normal entry, record it in ranges
                    ranges[scope->rangeCount].low = baseAddress + entry->start;
                    ranges[scope->rangeCount].high = baseAddress + entry->end;
                    scope->rangeCount++;
                }
            }
            //NOTE: allocate scope ranges with the final size, and copy ranges there.
            scope->ranges = oc_arena_push_array(arena, wa_debug_range, scope->rangeCount);
            memcpy(scope->ranges, ranges, sizeof(wa_debug_range) * scope->rangeCount);

            oc_scratch_end(scratch);
        }
    }

    //NOTE: count scope vars
    u32 varTagCount = 2;
    dw_tag varTags[2] = { DW_TAG_variable, DW_TAG_formal_parameter };

    oc_list_for(scopeDie->children, varDie, dw_die, parentElt)
    {
        if(varDie->abbrev && (varDie->abbrev->tag == DW_TAG_variable || varDie->abbrev->tag == DW_TAG_formal_parameter))
        {
            scope->count++;
        }
    }

    //NOTE: extract scope vars
    scope->vars = oc_arena_push_array(arena, wa_debug_variable, scope->count);

    u64 varIndex = 0;
    oc_list_for(scopeDie->children, varDie, dw_die, parentElt)
    {
        if(varDie->abbrev && (varDie->abbrev->tag == DW_TAG_variable || varDie->abbrev->tag == DW_TAG_formal_parameter))
        {
            scope->vars[varIndex] = wa_debug_import_variable(arena, varDie, dwarf, types);
            scope->vars[varIndex].uid = funcInfo->totalVarDecl + varIndex;
        }
        varIndex++;
    }

    funcInfo->totalVarDecl += scope->count;

    //NOTE: create children scopes
    oc_list_for(scopeDie->children, childDie, dw_die, parentElt)
    {
        if(childDie->abbrev && childDie->abbrev->tag == DW_TAG_lexical_block)
        {
            wa_debug_scope* childScope = oc_arena_push_type(arena, wa_debug_scope);
            childScope->parent = scope;
            oc_list_push_back(&scope->children, &childScope->listElt);

            wa_debug_extract_vars_from_scope(arena, funcInfo, childScope, childDie, unitBaseAddress, dwarf, types);
        }
    }
}

void wa_debug_info_import_variables(wa_module* module, wa_debug_info* info, dw_info* dwarf)
{
    //TODO: could we get function count from somewhere else so we dont need to pass module here?

    //NOTE: allocate arrays for units and function infos
    info->units = oc_arena_push_array(module->arena, wa_debug_unit, dwarf->unitCount);
    info->functionLocals = oc_arena_push_array(module->arena, wa_debug_function, module->functionCount);

    //NOTE: list of all types to deduplicate types
    oc_list types = { 0 };

    for(u64 unitIndex = 0; unitIndex < dwarf->unitCount; unitIndex++)
    {
        dw_die* rootDie = oc_catch_ptr(dwarf->units[unitIndex].rootDie)
        {
            continue;
        }

        OC_ASSERT(rootDie && rootDie->abbrev && rootDie->abbrev->tag == DW_TAG_compile_unit);
        u64 unitBaseAddress = 0;
        dw_attr* lowPC = dw_die_get_attr(rootDie, DW_AT_low_pc);
        if(lowPC)
        {
            unitBaseAddress = lowPC->valU64;
        }

        wa_debug_unit* unit = &info->units[unitIndex];

        //NOTE: count globals
        oc_list_for(rootDie->children, varDie, dw_die, parentElt)
        {
            if(varDie->abbrev && varDie->abbrev->tag == DW_TAG_variable)
            {
                dw_attr* name = dw_die_get_attr(varDie, DW_AT_name);
                if(name)
                {
                    unit->globalCount++;
                }
            }
        }

        //NOTE: extract named globals
        unit->globals = oc_arena_push_array(module->arena, wa_debug_variable, unit->globalCount);

        u64 globalIndex = 0;
        oc_list_for(rootDie->children, varDie, dw_die, parentElt)
        {
            if(varDie->abbrev && varDie->abbrev->tag == DW_TAG_variable)
            {
                dw_attr* name = dw_die_get_attr(varDie, DW_AT_name);
                if(name)
                {
                    unit->globals[globalIndex] = wa_debug_import_variable(module->arena, varDie, dwarf, &types);
                    globalIndex++;
                }
            }
        }

        //NOTE: extract per-function locals
        for(dw_die* funcDie = dw_die_find_next_with_tag(rootDie, rootDie, DW_TAG_subprogram);
            funcDie != 0;
            funcDie = dw_die_find_next_with_tag(rootDie, funcDie, DW_TAG_subprogram))
        {
            dw_attr* funcNameAttr = dw_die_get_attr(funcDie, DW_AT_name);
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
                    wa_debug_function* funcInfo = &info->functionLocals[funcIndex];
                    funcInfo->unit = unit;

                    //NOTE: get frame base expr loc
                    dw_attr* frameBase = dw_die_get_attr(funcDie, DW_AT_frame_base);
                    if(frameBase)
                    {
                        OC_DEBUG_ASSERT(frameBase->abbrev->form == DW_FORM_exprloc);
                        funcInfo->frameBase = &frameBase->loc;
                    }

                    wa_debug_extract_vars_from_scope(module->arena, funcInfo, &funcInfo->body, funcDie, unitBaseAddress, dwarf, &types);
                }
            }
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

    u64 id = mapping->instr->loc.start;
    u64 hash = oc_hash_xx64_string((oc_str8){ .ptr = (char*)&id, .len = 8 });
    u64 index = hash % module->debugInfo->wasmToWarmMapLen;

    oc_list_push_back(&module->debugInfo->wasmToWarmMap[index], &mapping->listElt);
}

//-------------------------------------------------------------------------
// Create debug info from dwarf
//-------------------------------------------------------------------------

void wa_debug_info_import_line_table(oc_arena* arena, wa_debug_info* info, dw_info* dwarf)
{
    dw_line_info* lineInfo = &dwarf->line;

    //NOTE: alloc wasm to line map
    for(u64 tableIndex = 0; tableIndex < lineInfo->tableCount; tableIndex++)
    {
        info->wasmToLineCount += lineInfo->tables[tableIndex].entryCount;
    }
    info->wasmToLine = oc_arena_push_array(arena, wa_wasm_to_line_entry, info->wasmToLineCount);

    //NOTE: build a global file table and build wasm line map
    oc_arena_scope scratch = oc_scratch_begin_next(arena);

    wa_source_info* sourceInfo = &info->sourceInfo;
    oc_list files = { 0 };
    sourceInfo->fileCount = 1; // index 0 is reserved for a "nil" wa_source_file entry

    u64 wasmToLineIndex = 0;

    for(u64 tableIndex = 0; tableIndex < lineInfo->tableCount; tableIndex++)
    {
        dw_line_table* table = &lineInfo->tables[tableIndex];

        //NOTE: How the paths of compile unit current directory and current file are stored in Dwarf differs
        //      between dwarf 4 and 5. But our dwarf parsing code always put the current file/current directory
        //      at index 0 of the file/directory list.
        OC_DEBUG_ASSERT(table->header.dirEntryCount);
        OC_DEBUG_ASSERT(table->header.fileEntryCount);

        //NOTE: allocate a table to map dwarf file indices to our global file table indices
        u64* fileIndices = oc_arena_push_array(scratch.arena, u64, table->header.fileEntryCount);

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

                if(fileEntry->dirIndex < table->header.dirEntryCount)
                {
                    dw_file_entry* dirEntry = &table->header.dirEntries[fileEntry->dirIndex];
                    dirPath = dirEntry->path;
                }
                else
                {
                    oc_log_error("Directory index %llu for file entry %llu is out of bounds\n",
                                 fileEntry->dirIndex,
                                 fileIndex);
                    //////////////////////
                    //Error
                }

                if(dirPath.len && dirPath.ptr[0] == '/')
                {
                    // absolute dir path
                    rootPath = dirPath;
                }
                else
                {
                    // dir path relative to current directory of compilation
                    rootPath = table->header.dirEntries[0].path;
                    filePath = oc_path_append(scratch.arena, dirPath, filePath);
                }
                fullPath = oc_path_append(scratch.arena, rootPath, filePath);

                OC_DEBUG_ASSERT(!oc_str8_cmp(rootPath, oc_str8_slice(fullPath, 0, rootPath.len)),
                                "rootPath should be a prefix of fullPath");
            }

            //NOTE: now we create a wa_source_file entry with the given root path and file path, if it doesn't exist
            wa_source_file_elt* file = wa_find_or_add_source_file(scratch.arena,
                                                                  arena,
                                                                  &files,
                                                                  &sourceInfo->fileCount,
                                                                  rootPath,
                                                                  fullPath);

            /////////////////////////////////////////////////////////////////////////////////////////
            //TODO: review how we infer/use root path
            /////////////////////////////////////////////////////////////////////////////////////////
            //NOTE: sometimes the compiler puts the current file in a dwarf v4 table, which means
            //      we'd get a duplicate. In this case, we prefer the root path put by the compiler
            if(file->index == fileIndices[0])
            {
                file->file.fullPath = oc_str8_push_copy(arena, fullPath);
                file->file.rootPath = oc_str8_slice(file->file.fullPath, 0, rootPath.len);
            }

            fileIndices[fileIndex] = file->index;
        }

        //NOTE: add the table line entries to our module's wasmToLineEntry table
        for(u64 entryIndex = 0; entryIndex < table->entryCount; entryIndex++)
        {
            dw_line_entry* lineEntry = &table->entries[entryIndex];
            wa_wasm_to_line_entry* wasmToLineEntry = &info->wasmToLine[wasmToLineIndex];

            wasmToLineEntry->wasmOffset = lineEntry->address;
            wasmToLineEntry->loc.fileIndex = fileIndices[lineEntry->file];
            wasmToLineEntry->loc.line = lineEntry->line;

            wasmToLineIndex++;
        }
    }

    //NOTE: now copy all wa_source_file entries to the module's global file array, and finally clear the scratch
    //      the first element of the array is a zeroed wa_source_file (index 0 is used as an invalid fileIndex)
    sourceInfo->files = oc_arena_push_array(arena, wa_source_file, sourceInfo->fileCount);
    oc_list_for_indexed(files, it, wa_source_file_elt, listElt)
    {
        sourceInfo->files[it.index + 1] = it.elt->file;
    }

    oc_scratch_end(scratch);
}

wa_debug_info* wa_debug_info_create(wa_module* module, oc_str8 contents)
{
    wa_debug_info* info = oc_arena_push_type(module->arena, wa_debug_info);

    //NOTE: alloc warm to wasm maps
    //TODO: tune size
    info->warmToWasmMapLen = 4096;
    info->warmToWasmMap = oc_arena_push_array(module->arena, oc_list, 4096);
    info->wasmToWarmMapLen = 4096;
    info->wasmToWarmMap = oc_arena_push_array(module->arena, oc_list, 4096);

    //NOTE: get dwarf sections
    dw_sections dwarfSections = { 0 };

    oc_list_for(module->toc.customSections, section, wa_section, listElt)
    {
        dw_section sectionContents = {
            .offset = section->offset,
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
        else if(!oc_str8_cmp(section->name, OC_STR8(".debug_ranges")))
        {
            dwarfSections.ranges = sectionContents;
        }
    }

    //NOTE: parse dwarf info and process it
    oc_arena_scope scratch = oc_scratch_begin_next(module->arena);

    dw_parser parser = {
        .arena = scratch.arena,
        .sections = dwarfSections,
        .errorCallback = wa_dwarf_error_callback,
        .userData = module,
        .rootReader = wa_reader_from_str8(contents),
    };

    dw_info dwarf = dw_parse_dwarf(&parser);

    wa_debug_info_import_line_table(module->arena, info, &dwarf);
    wa_debug_info_import_variables(module, info, &dwarf);

    oc_scratch_end(scratch);

    return info;
}
