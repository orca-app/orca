/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <stdio.h>
#include <math.h>

#include "warm.h"
#include "reader.h"

typedef struct wa_parser
{
    oc_arena* arena;
    wa_module* module;
    wa_reader reader;
} wa_parser;

//-------------------------------------------------------------------------
// errors
//-------------------------------------------------------------------------

void wa_parse_error(wa_parser* parser, const char* fmt, ...)
{
    wa_module_error* error = oc_arena_push_type(parser->arena, wa_module_error);

    error->loc = wa_reader_offset(&parser->reader);
    error->status = WA_PARSE_ERROR;

    va_list ap;
    va_start(ap, fmt);
    error->string = oc_str8_pushfv(parser->arena, fmt, ap);
    va_end(ap);

    oc_list_push_back(&parser->module->errors, &error->listElt);
}

void wa_parse_error_str8(wa_parser* parser, u64 offset, oc_str8 message)
{
    wa_module_error* error = oc_arena_push_type(parser->arena, wa_module_error);

    error->loc = offset;
    error->status = WA_PARSE_ERROR;
    error->string = oc_str8_push_copy(parser->arena, message);
    oc_list_push_back(&parser->module->errors, &error->listElt);
}

//------------------------------------------------------------------------
// Parsing
//------------------------------------------------------------------------

oc_str8 wa_parse_name(wa_parser* parser)
{
    u32 len = wa_read_leb128_u32(&parser->reader);
    oc_str8 string = wa_read_bytes(&parser->reader, len);

    oc_utf8_dec dec = {};
    for(u64 i = 0; i < string.len; i += dec.size)
    {
        dec = oc_utf8_decode_at(string, i);
        if(dec.status != OC_UTF8_OK)
        {
            break;
        }
    }

    switch(dec.status)
    {
        case OC_UTF8_OK:
            break;
        case OC_UTF8_OUT_OF_BOUNDS:
            wa_parse_error(parser,
                           "Invalid UTF8 encoding: out of bounds.\n");
            break;
        case OC_UTF8_UNEXPECTED_CONTINUATION_BYTE:
            wa_parse_error(parser,
                           "Invalid UTF8 encoding: unexpected continuation byte.\n");
            break;
        case OC_UTF8_UNEXPECTED_LEADING_BYTE:
            wa_parse_error(parser,
                           "Invalid UTF8 encoding: unexpected leading byte.\n");
            break;
        case OC_UTF8_INVALID_BYTE:
            wa_parse_error(parser,
                           "Invalid UTF8 encoding: invalid byte.\n");
            break;
        case OC_UTF8_INVALID_CODEPOINT:
            wa_parse_error(parser,
                           "Invalid UTF8 encoding: invalid codepoint.\n");
            break;
        case OC_UTF8_OVERLONG_ENCODING:
            wa_parse_error(parser,
                           "Invalid UTF8 encoding: overlong encoding.\n");
            break;
    }

    return string;
}

oc_str8 wa_parse_bytes_vector(wa_parser* parser)
{
    u32 len = wa_read_leb128_u32(&parser->reader);
    oc_str8 string = wa_read_bytes(&parser->reader, len);
    return (string);
}

void wa_parse_sections(wa_parser* parser, wa_module* module)
{
    while(wa_reader_has_more(&parser->reader))
    {
        u8 sectionID = wa_read_u8(&parser->reader);
        u32 sectionLen = wa_read_leb128_u32(&parser->reader);

        u64 contentOffset = wa_reader_offset(&parser->reader);

        //TODO: check if section was already defined...

        wa_section* entry = 0;
        switch(sectionID)
        {
            case 0:
            {
                oc_str8 name = wa_parse_name(parser);

                if(!oc_str8_cmp(name, OC_STR8("name")))
                {
                    entry = &module->toc.names;
                }
                else
                {
                    entry = oc_arena_push_type(parser->arena, wa_section);
                    memset(entry, 0, sizeof(wa_section));
                }
                entry->name = name;

                if(wa_reader_offset(&parser->reader) - contentOffset > sectionLen)
                {
                    wa_parse_error(parser,
                                   "Unexpected end of custom section.\n",
                                   sectionID);
                }
            }
            break;

            case 1:
                entry = &module->toc.types;
                break;

            case 2:
                entry = &module->toc.imports;
                break;

            case 3:
                entry = &module->toc.functions;
                break;

            case 4:
                entry = &module->toc.tables;
                break;

            case 5:
                entry = &module->toc.memory;
                break;

            case 6:
                entry = &module->toc.globals;
                break;

            case 7:
                entry = &module->toc.exports;
                break;

            case 8:
                entry = &module->toc.start;
                break;

            case 9:
                entry = &module->toc.elements;
                break;

            case 10:
                entry = &module->toc.code;
                break;

            case 11:
                entry = &module->toc.data;
                break;

            case 12:
                entry = &module->toc.dataCount;
                break;

            default:
            {
                wa_parse_error(parser,
                               "Unknown section identifier %i.\n",
                               sectionID);
            }
            break;
        }

        if(entry)
        {
            //TODO: check redeclaration
            /*
            {
                wa_parse_error(parser, "Redeclaration of %.*s.\n", oc_str8_ip(ast->label));
            }
            */
            entry->id = sectionID;
            entry->offset = wa_reader_offset(&parser->reader);
            entry->len = sectionLen;

            if(entry->id == 0)
            {
                entry->len = sectionLen - (entry->offset - contentOffset);
                oc_list_push_back(&module->toc.customSections, &entry->listElt);
            }
        }
        if(contentOffset + sectionLen > parser->reader.contents.len || contentOffset + sectionLen < contentOffset)
        {
            wa_parse_error(parser,
                           "Length of section out of bounds.\n",
                           sectionID);
        }
        wa_reader_seek(&parser->reader, contentOffset + sectionLen);
    }
}

void wa_parse_names(wa_parser* parser, wa_module* module)
{
    if(!module->toc.names.len)
    {
        return;
    }

    //TODO: use subreader
    wa_reader_seek(&parser->reader, module->toc.names.offset);
    u64 startOffset = wa_reader_offset(&parser->reader);

    while(wa_reader_offset(&parser->reader) - startOffset < module->toc.names.len)
    {
        u8 id = wa_read_u8(&parser->reader);
        u32 size = wa_read_leb128_u32(&parser->reader);
        u32 subStartOffset = wa_reader_offset(&parser->reader);

        switch(id)
        {
            case 0:
            {
                //NOTE: module name
            }
            break;
            case 1:
            {
                //NOTE: function names
                module->functionNameCount = wa_read_leb128_u32(&parser->reader);
                module->functionNames = oc_arena_push_array(module->arena, wa_name_entry, module->functionNameCount);

                for(u32 entryIndex = 0; entryIndex < module->functionNameCount; entryIndex++)
                {
                    module->functionNames[entryIndex].index = wa_read_leb128_u32(&parser->reader);
                    module->functionNames[entryIndex].name = wa_parse_name(parser);
                }
            }
            break;
            case 2:
            {
                //NOTE: local names
            }
            break;
            case 0x07:
            {
                //NOTE: unstandardized global names
            }
            break;
            default:
            {
                oc_log_warning("Unexpected subsection id %hhi at offset 0x%02x.\n", id, wa_reader_offset(&parser->reader));
            }
            break;
        }
        wa_reader_seek(&parser->reader, subStartOffset + size);
    }
    //NOTE: check section size
    if(wa_reader_offset(&parser->reader) - startOffset != module->toc.names.len)
    {
        wa_parse_error(parser,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.names.len,
                       wa_reader_offset(&parser->reader) - startOffset);
    }
}

wa_value_type wa_parse_value_type(wa_parser* parser)
{
    wa_value_type type = wa_read_leb128_u32(&parser->reader);

    if(!wa_is_value_type(type))
    {
        wa_parse_error(parser,
                       "unrecognized value type 0x%02x\n",
                       type);
    }
    return type;
}

void wa_parse_types(wa_parser* parser, wa_module* module)
{
    //NOTE: parse types section
    if(!module->toc.types.len)
    {
        return;
    }

    //TODO: use subreader
    wa_reader_seek(&parser->reader, module->toc.types.offset);
    u64 startOffset = wa_reader_offset(&parser->reader);

    module->typeCount = wa_read_leb128_u32(&parser->reader);
    module->types = oc_arena_push_array(parser->arena, wa_func_type, module->typeCount);

    for(u32 typeIndex = 0; typeIndex < module->typeCount; typeIndex++)
    {
        wa_func_type* type = &module->types[typeIndex];

        u8 b = wa_read_u8(&parser->reader);

        if(b != 0x60)
        {
            wa_parse_error(parser,
                           "Unexpected prefix 0x%02x for function type.\n",
                           b);
            return;
        }

        type->paramCount = wa_read_leb128_u32(&parser->reader);
        type->params = oc_arena_push_array(parser->arena, wa_value_type, type->paramCount);

        for(u32 typeIndex = 0; typeIndex < type->paramCount; typeIndex++)
        {
            type->params[typeIndex] = wa_parse_value_type(parser);
        }

        type->returnCount = wa_read_leb128_u32(&parser->reader);
        type->returns = oc_arena_push_array(parser->arena, wa_value_type, type->returnCount);

        for(u32 typeIndex = 0; typeIndex < type->returnCount; typeIndex++)
        {
            type->returns[typeIndex] = wa_parse_value_type(parser);
        }
    }

    //NOTE: check section size
    if(wa_reader_offset(&parser->reader) - startOffset != module->toc.types.len)
    {
        wa_parse_error(parser,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.types.len,
                       wa_reader_offset(&parser->reader) - startOffset);
    }
}

wa_limits wa_parse_limits(wa_parser* parser)
{
    wa_limits limits = { 0 };
    u8 kind = wa_read_u8(&parser->reader);

    if(kind != WA_LIMIT_MIN && kind != WA_LIMIT_MIN_MAX)
    {
        wa_parse_error(parser,
                       "Invalid limit kind 0x%02x\n",
                       kind);
    }
    else
    {
        limits.kind = kind;
        limits.min = wa_read_leb128_u32(&parser->reader);

        if(limits.kind == WA_LIMIT_MIN_MAX)
        {
            limits.max = wa_read_leb128_u32(&parser->reader);
        }
    }
    return limits;
}

void wa_parse_imports(wa_parser* parser, wa_module* module)
{
    //NOTE: parse import section
    if(!module->toc.imports.len)
    {
        return;
    }

    //TODO: use subreader
    wa_reader_seek(&parser->reader, module->toc.imports.offset);
    u64 startOffset = wa_reader_offset(&parser->reader);

    module->importCount = wa_read_leb128_u32(&parser->reader);
    module->imports = oc_arena_push_array(parser->arena, wa_import, module->importCount);

    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];

        import->moduleName = wa_parse_name(parser);
        import->importName = wa_parse_name(parser);
        import->kind = wa_read_u8(&parser->reader);

        switch((u32)import->kind)
        {
            case WA_IMPORT_FUNCTION:
            {
                import->index = wa_read_leb128_u32(&parser->reader);

                if(import->index >= module->typeCount)
                {
                    wa_parse_error(parser,
                                   "Out of bounds type index in function import (type count: %u, got index %u)\n",
                                   module->functionCount,
                                   import->index);
                }
                module->functionImportCount++;
            }
            break;
            case WA_IMPORT_TABLE:
            {
                import->type = wa_read_u8(&parser->reader);

                if(import->type != WA_TYPE_FUNC_REF && import->type != WA_TYPE_EXTERN_REF)
                {
                    wa_parse_error(parser,
                                   "Invalid type 0x%02x in table import \n",
                                   import->type);
                }
                import->limits = wa_parse_limits(parser);
                module->tableImportCount++;
            }
            break;

            case WA_IMPORT_MEMORY:
            {
                import->limits = wa_parse_limits(parser);
                module->memoryImportCount++;
            }
            break;

            case WA_IMPORT_GLOBAL:
            {
                //TODO: coalesce with globals section parsing
                import->type = wa_parse_value_type(parser);
                u8 mut = wa_read_u8(&parser->reader);

                if(mut == 0x00)
                {
                    import->mut = false;
                }
                else if(mut == 0x01)
                {
                    import->mut = true;
                }
                else
                {
                    wa_parse_error(parser,
                                   "invalid byte 0x%02hhx as global mutability.",
                                   mut);
                }
                module->globalImportCount++;
            }
            break;
            default:
            {
                wa_parse_error(parser,
                               "Unknown import kind 0x%02x\n",
                               (u8)import->kind);
                return;
            }
        }
    }

    //NOTE: check section size
    if(wa_reader_offset(&parser->reader) - startOffset != module->toc.imports.len)
    {
        wa_parse_error(parser,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.imports.len,
                       wa_reader_offset(&parser->reader) - startOffset);
    }
}

void wa_parse_functions(wa_parser* parser, wa_module* module)
{
    //NOTE: parse function section
    //TODO: use subreader
    wa_reader_seek(&parser->reader, module->toc.functions.offset);
    u64 startOffset = wa_reader_offset(&parser->reader);

    if(module->toc.functions.len)
    {
        module->functionCount = wa_read_leb128_u32(&parser->reader);
    }

    module->functions = oc_arena_push_array(parser->arena, wa_func, module->functionImportCount + module->functionCount);
    memset(module->functions, 0, (module->functionImportCount + module->functionCount) * sizeof(wa_func));

    //NOTE: re-read imports, because the format is kinda stupid -- they should have included imports in the function section
    u32 funcImportIndex = 0;
    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];
        if(import->kind == WA_IMPORT_FUNCTION)
        {
            wa_func* func = &module->functions[funcImportIndex];
            func->type = &module->types[import->index];
            func->import = import;
            funcImportIndex++;
        }
    }

    //NOTE: read non-import functions
    if(module->toc.functions.len)
    {
        for(u32 funcIndex = 0; funcIndex < module->functionCount; funcIndex++)
        {
            wa_func* func = &module->functions[module->functionImportCount + funcIndex];

            u32 typeIndex = wa_read_leb128_u32(&parser->reader);

            if(typeIndex >= module->typeCount)
            {
                wa_parse_error(parser,
                               "Invalid type index %i in function section\n",
                               typeIndex);
            }
            else
            {
                func->type = &module->types[typeIndex];
            }
        }
    }
    module->functionCount += module->functionImportCount;

    //NOTE: check section size
    if(wa_reader_offset(&parser->reader) - startOffset != module->toc.functions.len)
    {
        wa_parse_error(parser,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.functions.len,
                       wa_reader_offset(&parser->reader) - startOffset);
    }
}

void wa_parse_constant_expression(wa_parser* parser, oc_list* list);

void wa_parse_globals(wa_parser* parser, wa_module* module)
{
    //NOTE: parse global section
    //TODO: use subreader
    wa_reader_seek(&parser->reader, module->toc.globals.offset);
    u64 startOffset = wa_reader_offset(&parser->reader);

    if(module->toc.globals.len)
    {
        module->globalCount = wa_read_leb128_u32(&parser->reader);
    }

    module->globals = oc_arena_push_array(parser->arena, wa_global_desc, module->globalCount + module->globalImportCount);
    memset(module->globals, 0, (module->globalCount + module->globalImportCount) * sizeof(wa_global_desc));

    //NOTE: re-read imports, because the format is kinda stupid -- they should have included imports in the global section
    u32 globalImportIndex = 0;
    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];
        if(import->kind == WA_IMPORT_GLOBAL)
        {
            wa_global_desc* global = &module->globals[globalImportIndex];
            global->type = import->type;
            global->mut = import->mut;
            globalImportIndex++;
        }
    }

    if(module->toc.globals.len)
    {
        for(u32 globalIndex = 0; globalIndex < module->globalCount; globalIndex++)
        {
            wa_global_desc* global = &module->globals[globalIndex + module->globalImportCount];

            global->type = wa_parse_value_type(parser);
            u8 mut = wa_read_u8(&parser->reader);

            if(mut == 0x00)
            {
                global->mut = false;
            }
            else if(mut == 0x01)
            {
                global->mut = true;
            }
            else
            {
                wa_parse_error(parser,
                               "invalid byte 0x%02hhx as global mutability.",
                               mut);
            }
            wa_parse_constant_expression(parser, &global->init);
        }
    }
    module->globalCount += module->globalImportCount;

    //NOTE: check section size
    if(wa_reader_offset(&parser->reader) - startOffset != module->toc.globals.len)
    {
        wa_parse_error(parser,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.globals.len,
                       wa_reader_offset(&parser->reader) - startOffset);
    }
}

void wa_parse_tables(wa_parser* parser, wa_module* module)
{
    //NOTE: parse table section
    //TODO: use subreader
    wa_reader_seek(&parser->reader, module->toc.tables.offset);
    u64 startOffset = wa_reader_offset(&parser->reader);

    if(module->toc.tables.len)
    {
        module->tableCount = wa_read_leb128_u32(&parser->reader);
    }

    module->tables = oc_arena_push_array(parser->arena, wa_table_type, module->tableImportCount + module->tableCount);
    memset(module->tables, 0, (module->tableImportCount + module->tableCount) * sizeof(wa_table_type));

    //NOTE: re-read imports, because the format is kinda stupid -- they should have included imports in the tables section
    u32 tableImportIndex = 0;
    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];
        if(import->kind == WA_IMPORT_TABLE)
        {
            wa_table_type* table = &module->tables[tableImportIndex];
            table->type = import->type;
            table->limits = import->limits;
            tableImportIndex++;
        }
    }

    if(module->toc.tables.len)
    {
        //NOTE: read non-import tables
        for(u32 tableIndex = 0; tableIndex < module->tableCount; tableIndex++)
        {
            wa_table_type* table = &module->tables[tableIndex + module->tableImportCount];

            //TODO coalesce with parsing of table in imports
            table->type = wa_read_u8(&parser->reader);

            if(table->type != WA_TYPE_FUNC_REF && table->type != WA_TYPE_EXTERN_REF)
            {
                wa_parse_error(parser,
                               "Invalid type 0x%02x in table import \n",
                               table->type);
            }
            table->limits = wa_parse_limits(parser);
        }
    }
    module->tableCount += module->tableImportCount;

    //NOTE: check section size
    if(wa_reader_offset(&parser->reader) - startOffset != module->toc.tables.len)
    {
        wa_parse_error(parser,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.tables.len,
                       wa_reader_offset(&parser->reader) - startOffset);
    }
}

void wa_parse_memories(wa_parser* parser, wa_module* module)
{
    //NOTE: parse table section
    //TODO: use subreader
    wa_reader_seek(&parser->reader, module->toc.memory.offset);
    u64 startOffset = wa_reader_offset(&parser->reader);

    if(module->toc.memory.len)
    {
        module->memoryCount = wa_read_leb128_u32(&parser->reader);
    }

    module->memories = oc_arena_push_array(parser->arena, wa_limits, module->memoryImportCount + module->memoryCount);
    memset(module->memories, 0, (module->memoryImportCount + module->memoryCount) * sizeof(wa_limits));

    //NOTE: re-read imports, because the format is kinda stupid -- they should have included imports in the memories section
    u32 memoryImportIndex = 0;
    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];
        if(import->kind == WA_IMPORT_MEMORY)
        {
            module->memories[memoryImportIndex] = import->limits;
            memoryImportIndex++;
        }
    }

    if(module->toc.memory.len)
    {
        //NOTE: read non-import memories
        for(u32 memoryIndex = 0; memoryIndex < module->memoryCount; memoryIndex++)
        {
            module->memories[memoryIndex + module->memoryImportCount] = wa_parse_limits(parser);
        }
    }
    module->memoryCount += module->memoryImportCount;

    //NOTE: check section size
    if(wa_reader_offset(&parser->reader) - startOffset != module->toc.memory.len)
    {
        wa_parse_error(parser,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.memory.len,
                       wa_reader_offset(&parser->reader) - startOffset);
    }
}

void wa_parse_exports(wa_parser* parser, wa_module* module)
{
    //NOTE: parse export section
    if(!module->toc.exports.len)
    {
        return;
    }

    //TODO: use subreader
    wa_reader_seek(&parser->reader, module->toc.exports.offset);
    u64 startOffset = wa_reader_offset(&parser->reader);

    module->exportCount = wa_read_leb128_u32(&parser->reader);
    module->exports = oc_arena_push_array(parser->arena, wa_export, module->exportCount);

    for(u32 exportIndex = 0; exportIndex < module->exportCount; exportIndex++)
    {
        wa_export* export = &module->exports[exportIndex];

        export->name = wa_parse_name(parser);
        export->kind = wa_read_u8(&parser->reader);
        export->index = wa_read_leb128_u32(&parser->reader);

        switch((u32) export->kind)
        {
            case WA_EXPORT_FUNCTION:
            {
                if(export->index >= module->functionCount)
                {
                    wa_parse_error(parser,
                                   "Invalid type index in function export (function count: %u, got index %u)\n",
                                   module->functionCount,
                                   export->index);
                }
            }
            break;
            case WA_EXPORT_TABLE:
            {
                //TODO
            }
            break;
            case WA_EXPORT_MEMORY:
            {
                //TODO
            }
            break;
            case WA_EXPORT_GLOBAL:
            {
                //TODO
            }
            break;
            default:
            {
                wa_parse_error(parser,
                               "Unknown export kind 0x%02x\n",
                               (u8) export->kind);
                //TODO end parsing section?
            }
        }
    }

    //NOTE: check section size
    if(wa_reader_offset(&parser->reader) - startOffset != module->toc.exports.len)
    {
        wa_parse_error(parser,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.exports.len,
                       wa_reader_offset(&parser->reader) - startOffset);
    }
}

void wa_parse_start(wa_parser* parser, wa_module* module)
{
    //NOTE: parse export section
    if(!module->toc.start.len)
    {
        return;
    }

    //TODO: use subreader
    wa_reader_seek(&parser->reader, module->toc.start.offset);
    u64 startOffset = wa_reader_offset(&parser->reader);

    module->hasStart = true;
    module->startIndex = wa_read_leb128_u32(&parser->reader);

    //NOTE: check section size
    if(wa_reader_offset(&parser->reader) - startOffset != module->toc.start.len)
    {
        wa_parse_error(parser,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.start.len,
                       wa_reader_offset(&parser->reader) - startOffset);
    }
}

void wa_parse_expression(wa_parser* parser, u32 localCount, oc_list* list, bool constant)
{
    wa_module* module = parser->module;

    //TODO: we should validate block nesting here?

    i64 blockDepth = 0;
    wa_instr* instr = 0;

    while(wa_reader_has_more(&parser->reader) && blockDepth >= 0)
    {
        instr = oc_arena_push_type(parser->arena, wa_instr);
        memset(instr, 0, sizeof(wa_instr));
        oc_list_push_back(list, &instr->listElt);

        instr->loc.start = wa_reader_offset(&parser->reader);

        u8 byte = wa_read_u8(&parser->reader);

        if(byte == WA_INSTR_PREFIX_EXTENDED)
        {
            u32 code = wa_read_leb128_u32(&parser->reader);

            if(code >= wa_instr_decode_extended_len)
            {
                wa_parse_error(parser,
                               "Invalid extended instruction %i\n",
                               code);
                break;
            }
            instr->op = wa_instr_decode_extended[code];
        }
        else if(byte == WA_INSTR_PREFIX_VECTOR)
        {
            u32 code = wa_read_leb128_u32(&parser->reader);

            if(code >= wa_instr_decode_vector_len)
            {
                wa_parse_error(parser,
                               "Invalid vector instruction %i\n",
                               code);
                break;
            }
            instr->op = wa_instr_decode_vector[code];
        }
        else
        {
            if(byte >= wa_instr_decode_basic_len)
            {
                wa_parse_error(parser,
                               "Invalid basic instruction 0x%02x\n",
                               byte);
                break;
            }
            instr->op = wa_instr_decode_basic[byte];
        }

        const wa_instr_info* info = &wa_instr_infos[instr->op];

        if(!info->defined)
        {
            wa_parse_error(parser, "undefined instruction %s.\n", wa_instr_strings[instr->op]);
            break;
        }

        if(constant)
        {
            if(instr->op != WA_INSTR_i32_const
               && instr->op != WA_INSTR_i64_const
               && instr->op != WA_INSTR_f32_const
               && instr->op != WA_INSTR_f64_const
               && instr->op != WA_INSTR_ref_null
               && instr->op != WA_INSTR_ref_func
               && instr->op != WA_INSTR_global_get
               && instr->op != WA_INSTR_end)
            {
                wa_parse_error(parser,
                               "found non-constant instruction %s while parsing constant expression.\n",
                               wa_instr_strings[instr->op]);
            }
            //TODO add constraint on global get
        }

        //NOTE: memory.init and data.drop need a data count section
        if((instr->op == WA_INSTR_memory_init || instr->op == WA_INSTR_data_drop)
           && !module->toc.dataCount.len)
        {
            wa_parse_error(parser, "%s requires a data count section.\n", wa_instr_strings[instr->op]);
        }

        //NOTE: parse immediates, special cases first, then generic
        if(instr->op == WA_INSTR_block
           || instr->op == WA_INSTR_loop
           || instr->op == WA_INSTR_if)
        {
            //NOTE: parse block type
            i64 t = wa_read_leb128_i64(&parser->reader);
            if(t >= 0)
            {
                u64 typeIndex = (u64)t;

                if(typeIndex >= module->typeCount)
                {
                    wa_parse_error(parser,
                                   "unexpected type index %u (type count: %u)\n",
                                   typeIndex,
                                   module->typeCount);
                    break;
                }
                instr->blockType = &module->types[typeIndex];
            }
            else
            {
                if(t != -64 && !wa_is_value_type(t & 0x7f))
                {
                    wa_parse_error(parser,
                                   "unrecognized value type 0x%02hhx\n",
                                   t & 0x7f);
                    break;
                }
                t = (t == -64) ? 0 : -t;

                instr->blockType = (wa_func_type*)&WA_BLOCK_VALUE_TYPES[t];
            }
            blockDepth++;
        }
        else if(instr->op == WA_INSTR_end)
        {
            blockDepth--;
        }
        else if(instr->op == WA_INSTR_select_t)
        {
            instr->immCount = wa_read_leb128_u32(&parser->reader);

            if(instr->immCount != 1)
            {
                //TODO: should set the error on the count rather than the vector?
                wa_parse_error(parser,
                               "select instruction can have at most one immediate\n");
                break;
            }
            instr->imm = oc_arena_push_type(parser->arena, wa_code);
            instr->imm[0].valueType = wa_parse_value_type(parser);
        }
        else if(instr->op == WA_INSTR_br_table)
        {
            instr->immCount = wa_read_leb128_u32(&parser->reader);
            instr->immCount += 1;
            instr->imm = oc_arena_push_array(parser->arena, wa_code, instr->immCount);

            for(u32 i = 0; i < instr->immCount - 1; i++)
            {
                instr->imm[i].index = wa_read_leb128_u32(&parser->reader);
            }
            instr->imm[instr->immCount - 1].index = wa_read_leb128_u32(&parser->reader);
        }
        else
        {
            //generic case
            instr->immCount = info->immCount;
            instr->imm = oc_arena_push_array(parser->arena, wa_code, instr->immCount);

            for(int immIndex = 0; immIndex < info->immCount; immIndex++)
            {
                switch(info->imm[immIndex])
                {
                    case WA_IMM_ZERO:
                    {
                        instr->imm[immIndex].valI32 = wa_read_u8(&parser->reader);
                    }
                    break;
                    case WA_IMM_I32:
                    {
                        instr->imm[immIndex].valI32 = wa_read_leb128_i32(&parser->reader);
                    }
                    break;
                    case WA_IMM_I64:
                    {
                        instr->imm[immIndex].valI64 = wa_read_leb128_i64(&parser->reader);
                    }
                    break;
                    case WA_IMM_F32:
                    {
                        instr->imm[immIndex].valF32 = wa_read_f32(&parser->reader);
                    }
                    break;
                    case WA_IMM_F64:
                    {
                        instr->imm[immIndex].valF64 = wa_read_f64(&parser->reader);
                    }
                    break;
                    case WA_IMM_VALUE_TYPE:
                    {
                        instr->imm[immIndex].valueType = wa_parse_value_type(parser);
                    }
                    break;
                    case WA_IMM_REF_TYPE:
                    {
                        instr->imm[immIndex].valueType = wa_read_u8(&parser->reader);
                    }
                    break;

                    case WA_IMM_LOCAL_INDEX:
                    {
                        instr->imm[immIndex].index = wa_read_leb128_u32(&parser->reader);
                    }
                    break;

                    case WA_IMM_FUNC_INDEX:
                    {
                        instr->imm[immIndex].index = wa_read_leb128_u32(&parser->reader);
                    }
                    break;

                    case WA_IMM_GLOBAL_INDEX:
                    case WA_IMM_TYPE_INDEX:
                    case WA_IMM_TABLE_INDEX:
                    case WA_IMM_ELEM_INDEX:
                    case WA_IMM_DATA_INDEX:
                    case WA_IMM_LABEL:
                    {
                        instr->imm[immIndex].index = wa_read_leb128_u32(&parser->reader);
                    }
                    break;
                    case WA_IMM_MEM_ARG:
                    {
                        instr->imm[immIndex].memArg.align = wa_read_leb128_u32(&parser->reader);
                        instr->imm[immIndex].memArg.offset = wa_read_leb128_u32(&parser->reader);
                    }
                    break;
                    case WA_IMM_LANE_INDEX:
                    {
                        instr->imm[immIndex].laneIndex = wa_read_u8(&parser->reader);
                    }
                    break;
                    /*
                    case WA_IMM_V128:
                    {
                        //TODO
                    }
                    break;
                    */
                    default:
                        OC_ASSERT(0, "unsupported immediate type");
                        break;
                }
            }
        }
        instr->loc.len = wa_reader_offset(&parser->reader) - instr->loc.start;
    }

    if(!instr || instr->op != WA_INSTR_end)
    {
        wa_parse_error(parser, "unexpected end of expression\n");
    }

    //TODO check that we exited from an end instruction
}

void wa_parse_constant_expression(wa_parser* parser, oc_list* list)
{
    wa_parse_expression(parser, 0, list, true);
}

void wa_parse_elements(wa_parser* parser, wa_module* module)
{
    if(!module->toc.elements.len)
    {
        return;
    }

    //TODO: use subreader
    wa_reader_seek(&parser->reader, module->toc.elements.offset);
    u64 startOffset = wa_reader_offset(&parser->reader);

    module->elementCount = wa_read_leb128_u32(&parser->reader);
    module->elements = oc_arena_push_array(parser->arena, wa_element, module->elementCount);
    memset(module->elements, 0, module->elementCount * sizeof(wa_element));

    for(u32 eltIndex = 0; eltIndex < module->elementCount; eltIndex++)
    {
        wa_element* element = &module->elements[eltIndex];

        u32 prefix = wa_read_leb128_u32(&parser->reader);

        if(prefix > 7)
        {
            wa_parse_error(parser, "invalid element prefix %u\n", prefix);
            return;
        }

        if(prefix & 0x01)
        {
            if(prefix & 0x02)
            {
                element->mode = WA_ELEMENT_DECLARATIVE;
                //NOTE(martin): what the f* are they used for??
            }
            else
            {
                element->mode = WA_ELEMENT_PASSIVE;
            }
        }
        else
        {
            element->mode = WA_ELEMENT_ACTIVE;
            if(prefix & 0x02)
            {
                //NOTE: explicit table index
                //TODO validate index
                element->tableIndex = wa_read_leb128_u32(&parser->reader);
            }
            wa_parse_constant_expression(parser, &element->tableOffset);
        }

        element->type = WA_TYPE_FUNC_REF;

        if(prefix & 0x04)
        {
            //NOTE: reftype? vec(expr)
            if(prefix & 0x03)
            {
                //NOTE ref type
                element->type = wa_read_u8(&parser->reader);
                if(element->type != WA_TYPE_FUNC_REF && element->type != WA_TYPE_EXTERN_REF)
                {
                    wa_parse_error(parser, "ref type should be externref or funcref.");
                }
            }

            element->initCount = wa_read_leb128_u32(&parser->reader);
            element->initInstr = oc_arena_push_array(parser->arena, oc_list, element->initCount);
            memset(element->initInstr, 0, element->initCount * sizeof(oc_list));

            for(u32 i = 0; i < element->initCount; i++)
            {
                wa_parse_constant_expression(parser, &element->initInstr[i]);
            }
        }
        else
        {
            //NOTE: refkind? vec(funcIdx)
            if(prefix & 0x03)
            {
                //NOTE refkind
                u8 refKind = wa_read_u8(&parser->reader);
                if(refKind != 0x00)
                {
                    wa_parse_error(parser, "ref kind should be 0.");
                }
            }

            element->initCount = wa_read_leb128_u32(&parser->reader);
            element->initInstr = oc_arena_push_array(parser->arena, oc_list, element->initCount);
            memset(element->initInstr, 0, element->initCount * sizeof(oc_list));

            for(u32 i = 0; i < element->initCount; i++)
            {
                //TODO validate index
                u32 funcIndex = wa_read_leb128_u32(&parser->reader);

                wa_instr* init = oc_arena_push_array(parser->arena, wa_instr, 2);
                memset(init, 0, 2 * sizeof(wa_instr));

                init[0].op = WA_INSTR_ref_func;
                init[0].immCount = 1;
                init[0].imm = oc_arena_push_type(parser->arena, wa_code);
                init[0].imm[0].index = funcIndex;
                oc_list_push_back(&element->initInstr[i], &init[0].listElt);

                init[1].op = WA_INSTR_end;
                oc_list_push_back(&element->initInstr[i], &init[1].listElt);
            }
        }
    }

    //NOTE: check section size
    if(wa_reader_offset(&parser->reader) - startOffset != module->toc.elements.len)
    {
        wa_parse_error(parser,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.elements.len,
                       wa_reader_offset(&parser->reader) - startOffset);
    }
}

void wa_parse_data_count(wa_parser* parser, wa_module* module)
{
    if(!module->toc.dataCount.len)
    {
        return;
    }

    //TODO: use subreader
    wa_reader_seek(&parser->reader, module->toc.dataCount.offset);
    u64 startOffset = wa_reader_offset(&parser->reader);

    module->dataCount = wa_read_leb128_u32(&parser->reader);

    //NOTE: check section size
    if(wa_reader_offset(&parser->reader) - startOffset != module->toc.dataCount.len)
    {
        wa_parse_error(parser,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.dataCount.len,
                       wa_reader_offset(&parser->reader) - startOffset);
    }
}

void wa_parse_data(wa_parser* parser, wa_module* module)
{
    if(!module->toc.data.len)
    {
        return;
    }

    //TODO use subreader
    wa_reader_seek(&parser->reader, module->toc.data.offset);
    u64 startOffset = wa_reader_offset(&parser->reader);

    u32 dataCount = wa_read_leb128_u32(&parser->reader);

    if(module->toc.dataCount.len && dataCount != module->dataCount)
    {
        wa_parse_error(parser,
                       "Number of data segments does not match data count section (expected %u, got %u).\n",
                       module->dataCount,
                       dataCount);
    }
    module->dataCount = dataCount;

    module->data = oc_arena_push_array(parser->arena, wa_data_segment, module->dataCount);
    memset(module->data, 0, module->dataCount * sizeof(wa_data_segment));

    for(u32 segIndex = 0; segIndex < module->dataCount; segIndex++)
    {
        wa_data_segment* seg = &module->data[segIndex];

        u32 prefix = wa_read_leb128_u32(&parser->reader);

        if(prefix > 2)
        {
            wa_parse_error(parser, "invalid segment prefix %u\n", prefix);
            return;
        }

        if(prefix & 0x01)
        {
            seg->mode = WA_DATA_PASSIVE;
        }
        else
        {
            seg->mode = WA_DATA_ACTIVE;
            if(prefix & 0x02)
            {
                //NOTE: explicit memory index
                //TODO validate index
                seg->memoryIndex = wa_read_leb128_u32(&parser->reader);
            }
            wa_parse_constant_expression(parser, &seg->memoryOffset);
        }

        //NOTE: parse vec(bytes)
        seg->init = wa_parse_bytes_vector(parser);
    }

    //NOTE: check section size
    if(wa_reader_offset(&parser->reader) - startOffset != module->toc.data.len)
    {
        wa_parse_error(parser,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.data.len,
                       wa_reader_offset(&parser->reader) - startOffset);
    }
}

void wa_parse_code(wa_parser* parser, wa_module* module)
{
    if(!module->toc.code.len)
    {
        if(module->functionCount - module->functionImportCount)
        {
            wa_parse_error(parser,
                           "Function section declares %i functions, but code section is absent",
                           module->functionCount - module->functionImportCount);
        }
        return;
    }

    wa_reader_seek(&parser->reader, module->toc.code.offset);
    u64 startOffset = wa_reader_offset(&parser->reader);

    u32 functionCount = wa_read_leb128_u32(&parser->reader);

    if(functionCount != module->functionCount - module->functionImportCount)
    {
        //TODO should set the error on the count, not the vector?
        wa_parse_error(parser,
                       "Function count mismatch (function section: %i, code section: %i\n",
                       module->functionCount - module->functionImportCount,
                       functionCount);
    }
    functionCount = oc_min(functionCount + module->functionImportCount, module->functionCount);

    for(u32 funcIndex = module->functionImportCount; funcIndex < functionCount; funcIndex++)
    {
        wa_func* func = &module->functions[funcIndex];

        oc_arena_scope scratch = oc_scratch_begin();

        u32 funcLen = wa_read_leb128_u32(&parser->reader);
        u32 funcStartOffset = wa_reader_offset(&parser->reader);

        //NOTE: parse locals
        u32 localEntryCount = wa_read_leb128_u32(&parser->reader);

        u32* counts = oc_arena_push_array(scratch.arena, u32, localEntryCount);
        wa_value_type* types = oc_arena_push_array(scratch.arena, wa_value_type, localEntryCount);

        func->localCount = func->type->paramCount;
        for(u32 localEntryIndex = 0; localEntryIndex < localEntryCount; localEntryIndex++)
        {
            counts[localEntryIndex] = wa_read_leb128_u32(&parser->reader);
            types[localEntryIndex] = wa_read_u8(&parser->reader);

            if(func->localCount + counts[localEntryIndex] < func->localCount)
            {
                //NOTE: overflow
                wa_parse_error(parser,
                               "Too many locals for function %i\n",
                               funcIndex);
                goto parse_function_end;
            }

            func->localCount += counts[localEntryIndex];

            //TODO: validate types? or validate later?
        }

        //NOTE: expand locals
        func->locals = oc_arena_push_array(parser->arena, wa_value_type, func->localCount);

        for(u32 paramIndex = 0; paramIndex < func->type->paramCount; paramIndex++)
        {
            func->locals[paramIndex] = func->type->params[paramIndex];
        }

        u32 localIndex = func->type->paramCount;
        for(u32 localEntryIndex = 0; localEntryIndex < localEntryCount; localEntryIndex++)
        {
            u32 count = counts[localEntryIndex];
            wa_value_type type = types[localEntryIndex];

            for(int i = 0; i < count; i++)
            {
                func->locals[localIndex + i] = type;
            }
            localIndex += count;
        }

        //NOTE: parse body
        wa_parse_expression(parser, func->localCount, &func->instructions, false);

        //NOTE: check entry length
        if(wa_reader_offset(&parser->reader) - funcStartOffset != funcLen)
        {
            wa_parse_error(parser,
                           "Size of code entry %i does not match declared size (declared %u, got %u)\n",
                           funcIndex,
                           funcLen,
                           wa_reader_offset(&parser->reader) - funcStartOffset);
            goto parse_function_end;
        }

    parse_function_end:
        oc_scratch_end(scratch);
        wa_reader_seek(&parser->reader, funcStartOffset + funcLen);
    }

    //NOTE: check section size
    if(wa_reader_offset(&parser->reader) - startOffset != module->toc.code.len)
    {
        wa_parse_error(parser,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.code.len,
                       wa_reader_offset(&parser->reader) - startOffset);
    }
}

void wa_parser_read_error_callback(wa_reader* reader, oc_str8 message, void* user)
{
    wa_parser* parser = (wa_parser*)user;
    wa_parse_error_str8(parser, wa_reader_offset(reader), message);
}

void wa_parse_module(wa_module* module, oc_str8 contents)
{
    wa_parser parser = {
        .module = module,
        .arena = module->arena,
        .reader = wa_reader_from_str8(contents),
    };
    wa_reader_set_error_callback(&parser.reader, wa_parser_read_error_callback, &parser);

    u32 magic = wa_read_u32(&parser.reader);

    if(magic != 0x6d736100)
    {
        wa_parse_error(&parser, "wrong wasm magic number");
        return;
    }

    u32 version = wa_read_u32(&parser.reader);
    if(version != 1)
    {
        wa_parse_error(&parser, "wrong wasm version");
        return;
    }

    wa_parse_sections(&parser, module);
    wa_parse_names(&parser, module);
    wa_parse_types(&parser, module);
    wa_parse_imports(&parser, module);
    wa_parse_functions(&parser, module);
    wa_parse_globals(&parser, module);
    wa_parse_tables(&parser, module);
    wa_parse_memories(&parser, module);
    wa_parse_exports(&parser, module);
    wa_parse_start(&parser, module);
    wa_parse_elements(&parser, module);
    wa_parse_data_count(&parser, module);
    wa_parse_code(&parser, module);
    wa_parse_data(&parser, module);
}
