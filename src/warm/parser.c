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
#include "module.h"

typedef struct wa_parser
{
    oc_arena* arena;
    wa_module* module;
    char* contents;
    u64 len;
    u64 offset;

} wa_parser;

//-------------------------------------------------------------------------
// errors
//-------------------------------------------------------------------------

void wa_parse_error(wa_parser* parser, wa_ast* ast, const char* fmt, ...)
{
    wa_module_error* error = oc_arena_push_type(parser->arena, wa_module_error);

    error->status = WA_PARSE_ERROR;

    va_list ap;
    va_start(ap, fmt);
    error->string = oc_str8_pushfv(parser->arena, fmt, ap);
    va_end(ap);

    error->ast = ast;

    oc_list_push_back(&parser->module->errors, &error->moduleElt);
    oc_list_push_back(&ast->errors, &error->astElt);
}

bool wa_module_has_errors(wa_module* module)
{
    return (!oc_list_empty(module->errors));
}

void wa_module_print_errors(wa_module* module)
{
    oc_list_for(module->errors, err, wa_module_error, moduleElt)
    {
        printf("%.*s", oc_str8_ip(err->string));
    }
}

bool wa_ast_has_errors(wa_ast* ast)
{
    return (!oc_list_empty(ast->errors));
}

//-------------------------------------------------------------------------
// AST
//-------------------------------------------------------------------------

wa_ast* wa_ast_alloc(wa_parser* parser, wa_ast_kind kind)
{
    wa_ast* ast = oc_arena_push_type(parser->arena, wa_ast);
    memset(ast, 0, sizeof(wa_ast));

    ast->kind = kind;

    return (ast);
}

void wa_ast_add_child(wa_ast* parent, wa_ast* child)
{
    child->parent = parent;
    oc_list_push_back(&parent->children, &child->parentElt);
}

wa_ast* wa_ast_begin(wa_parser* parser, wa_ast* parent, wa_ast_kind kind)
{
    wa_ast* ast = oc_arena_push_type(parser->arena, wa_ast);
    memset(ast, 0, sizeof(wa_ast));

    ast->kind = kind;
    ast->loc.start = parser->offset;

    if(parent)
    {
        wa_ast_add_child(parent, ast);
    }

    return ast;
}

void wa_ast_end(wa_parser* parser, wa_ast* ast)
{
    ast->loc.len = parser->offset - ast->loc.start;
}

char* wa_parser_head(wa_parser* parser)
{
    return (parser->contents + parser->offset);
}

bool wa_parser_end(wa_parser* parser)
{
    return (parser->offset >= parser->len);
}

void wa_parser_seek(wa_parser* parser, u64 offset, oc_str8 label)
{
    parser->offset = offset;
}

wa_ast* wa_parser_read_byte(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_ast_begin(parser, parent, WA_AST_U8);
    ast->label = label;

    if(parser->offset + sizeof(u8) > parser->len)
    {
        wa_parse_error(parser,
                       ast,
                       "Couldn't read %.*s: unexpected end of parser.\n",
                       oc_str8_ip(label));
    }
    else
    {
        ast->valU8 = *(u8*)&parser->contents[parser->offset];
        parser->offset += sizeof(u8);
    }

    wa_ast_end(parser, ast);
    return (ast);
}

wa_ast* wa_parser_read_raw_u32(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_ast_begin(parser, parent, WA_AST_U32);
    ast->label = label;

    if(parser->offset + sizeof(u32) > parser->len)
    {
        wa_parse_error(parser,
                       ast,
                       "Couldn't read %.*s: unexpected end of parser.\n",
                       oc_str8_ip(label));
    }
    else
    {
        ast->valU32 = *(u32*)&parser->contents[parser->offset];
        parser->offset += sizeof(u32);
    }

    wa_ast_end(parser, ast);
    return (ast);
}

wa_ast* wa_parser_read_leb128(wa_parser* parser, wa_ast* parent, oc_str8 label, u32 bitWidth, bool isSigned)
{
    wa_ast* ast = wa_ast_begin(parser, parent, WA_AST_U64);
    ast->label = label;

    char byte = 0;
    u64 shift = 0;
    u64 res = 0;
    u32 count = 0;
    u32 maxCount = (u32)ceil(bitWidth / 7.);

    do
    {
        if(parser->offset + sizeof(char) > parser->len)
        {
            wa_parse_error(parser,
                           ast,
                           "Couldn't read %.*s: unexpected end of parser.\n",
                           oc_str8_ip(label));
            res = 0;
            break;
        }

        if(count >= maxCount)
        {
            wa_parse_error(parser,
                           ast,
                           "Couldn't read %.*s: leb128 u64 representation too long.\n",
                           oc_str8_ip(label));
            res = 0;
            break;
        }

        byte = parser->contents[parser->offset];
        parser->offset++;

        res |= ((u64)byte & 0x7f) << shift;
        shift += 7;
        count++;
    }
    while(byte & 0x80);

    if(isSigned)
    {
        if(count == maxCount)
        {
            //NOTE: the spec mandates that unused bits must match the sign bit,
            // so we construct a mask to select the sign bit and the unused bits,
            // and we check that they're either all 1 or all 0
            u8 bitsInLastByte = (bitWidth - (maxCount - 1) * 7);
            u8 lastByteMask = (0xff << (bitsInLastByte - 1)) & 0x7f;
            u8 bits = byte & lastByteMask;

            if(bits != 0 && bits != lastByteMask)
            {
                wa_parse_error(parser,
                               ast,
                               "Couldn't read %.*s: leb128 overflow.\n",
                               oc_str8_ip(label));
                res = 0;
            }
        }

        if(shift < 64 && (byte & 0x40))
        {
            res |= (~0ULL << shift);
        }
    }
    else
    {
        if(count == maxCount)
        {
            //NOTE: for signed the spec mandates that unused bits must be zero,
            // so we construct a mask to select only unused bits,
            // and we check that they're all 0
            u8 bitsInLastByte = (bitWidth - (maxCount - 1) * 7);
            u8 lastByteMask = (0xff << (bitsInLastByte)) & 0x7f;
            u8 bits = byte & lastByteMask;

            if(bits != 0)
            {
                wa_parse_error(parser,
                               ast,
                               "Couldn't read %.*s: leb128 overflow.\n",
                               oc_str8_ip(label));
                res = 0;
            }
        }
    }

    ast->valU64 = res;
    wa_ast_end(parser, ast);

    return (ast);
}

wa_ast* wa_parser_read_leb128_u64(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    return (wa_parser_read_leb128(parser, parent, label, 64, false));
}

/*
wa_ast* wa_parser_read_sleb128(wa_parser* parser, wa_ast* parent, oc_str8 label, u32 bitWidth)
{
    wa_ast* ast = wa_ast_begin(parser, parent, WA_AST_I64);
    ast->label = label;

    char byte = 0;
    u64 shift = 0;
    i64 res = 0;

    u32 maxShift = 7 * (u32)ceil(bitWidth / 7.);
    do
    {
        if(parser->offset + sizeof(char) > parser->len)
        {
            wa_parse_error(parser,
                           ast,
                           "Couldn't read %.*s: unexpected end of parser.\n",
                           oc_str8_ip(label));
            byte = 0;
            res = 0;
            break;
        }

        if(shift >= maxShift)
        {
            wa_parse_error(parser,
                           ast,
                           "Couldn't read %.*s: leb128 i64 representation too long.\n",
                           oc_str8_ip(label));
            res = 0;
            break;
        }

        byte = parser->contents[parser->offset];
        parser->offset++;

        if(shift == maxShift && (byte & 0x7e))
        {
            wa_parse_error(parser,
                           ast,
                           "Couldn't read %.*s: leb128 overflow.\n",
                           oc_str8_ip(label));
            res = 0;
            byte = 0;
            break;
        }

        if(shift >= 64)
        {
            wa_parse_error(parser,
                           ast,
                           "Couldn't read %.*s: leb128 overflow.\n",
                           oc_str8_ip(label));
            byte = 0;
            res = 0;
            break;
        }

        res |= ((u64)byte & 0x7f) << shift;
        shift += 7;
    }
    while(byte & 0x80);

    if(shift < 64 && (byte & 0x40))
    {
        res |= (~0ULL << shift);
    }

    ast->valI64 = res;

    wa_ast_end(parser, ast);
    return (ast);
}
*/

wa_ast* wa_parser_read_leb128_i64(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    return (wa_parser_read_leb128(parser, parent, label, 64, true));
}

wa_ast* wa_parser_read_leb128_u32(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_parser_read_leb128(parser, parent, label, 32, false);
    ast->kind = WA_AST_U32;
    ast->label = label;

    if(ast->valU64 > UINT32_MAX)
    {
        ast->valU64 = 0;
        if(!wa_ast_has_errors(ast))
        {
            wa_parse_error(parser,
                           ast,
                           "Couldn't read %.*s: leb128 overflow.\n",
                           oc_str8_ip(label));
        }
    }
    else
    {
        ast->valU32 = (u32)ast->valU64;
    }
    return ast;
}

wa_ast* wa_parser_read_leb128_i32(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_parser_read_leb128(parser, parent, label, 32, true);
    ast->kind = WA_AST_I32;
    ast->label = label;

    if(ast->valI64 > INT32_MAX || ast->valI64 < INT32_MIN)
    {
        ast->valI64 = 0;
        if(!wa_ast_has_errors(ast))
        {
            wa_parse_error(parser,
                           ast,
                           "Couldn't read %.*s: leb128 overflow.\n",
                           oc_str8_ip(label));
        }
    }
    else
    {
        ast->valI32 = (i32)ast->valI64;
    }
    return ast;
}

wa_ast* wa_parser_read_f32(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_ast_begin(parser, parent, WA_AST_F32);
    ast->label = label;

    if(parser->offset + sizeof(f32) > parser->len)
    {
        wa_parse_error(parser,
                       ast,
                       "Couldn't read %.*s: unexpected end of parser.\n",
                       oc_str8_ip(label));
    }
    else
    {
        ast->valF32 = *(f32*)&parser->contents[parser->offset];
        parser->offset += sizeof(f32);
    }

    wa_ast_end(parser, ast);
    return ast;
}

wa_ast* wa_parser_read_f64(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_ast_begin(parser, parent, WA_AST_F64);
    ast->label = label;

    if(parser->offset + sizeof(f64) > parser->len)
    {
        wa_parse_error(parser,
                       ast,
                       "Couldn't read %.*s: unexpected end of parser.\n",
                       oc_str8_ip(label));
    }
    else
    {
        ast->valF64 = *(f64*)&parser->contents[parser->offset];
        parser->offset += sizeof(f64);
    }

    wa_ast_end(parser, ast);
    return ast;
}

//------------------------------------------------------------------------
// Parsing
//------------------------------------------------------------------------

wa_ast* wa_parser_read_name(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_ast_begin(parser, parent, WA_AST_NAME);
    ast->label = label;

    wa_ast* lenAst = wa_parser_read_leb128_u32(parser, ast, label);
    if(!wa_ast_has_errors(lenAst))
    {
        u32 len = lenAst->valU32;

        if(parser->offset + len > parser->len)
        {
            wa_parse_error(parser,
                           ast,
                           "Couldn't read %.*s: unexpected end of parser.\n",
                           oc_str8_ip(label));
        }
        else
        {
            ast->str8 = oc_str8_push_buffer(parser->arena, len, &parser->contents[parser->offset]);
            parser->offset += len;

            oc_utf8_dec dec = {};
            for(u64 i = 0; i < ast->str8.len; i += dec.size)
            {
                dec = oc_utf8_decode_at(ast->str8, i);
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
                                   ast,
                                   "Invalid UTF8 encoding: out of bounds.\n");
                    break;
                case OC_UTF8_UNEXPECTED_CONTINUATION_BYTE:
                    wa_parse_error(parser,
                                   ast,
                                   "Invalid UTF8 encoding: unexpected continuation byte.\n");
                    break;
                case OC_UTF8_UNEXPECTED_LEADING_BYTE:
                    wa_parse_error(parser,
                                   ast,
                                   "Invalid UTF8 encoding: unexpected leading byte.\n");
                    break;
                case OC_UTF8_INVALID_BYTE:
                    wa_parse_error(parser,
                                   ast,
                                   "Invalid UTF8 encoding: invalid byte.\n");
                    break;
                case OC_UTF8_INVALID_CODEPOINT:
                    wa_parse_error(parser,
                                   ast,
                                   "Invalid UTF8 encoding: invalid codepoint.\n");
                    break;
                case OC_UTF8_OVERLONG_ENCODING:
                    wa_parse_error(parser,
                                   ast,
                                   "Invalid UTF8 encoding: overlong encoding.\n");
                    break;
            }
        }
    }
    wa_ast_end(parser, ast);
    return (ast);
}

wa_ast* wa_parser_read_bytes_vector(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_ast_begin(parser, parent, WA_AST_VECTOR);
    ast->label = label;

    wa_ast* lenAst = wa_parser_read_leb128_u32(parser, ast, label);
    if(!wa_ast_has_errors(lenAst))
    {
        u32 len = lenAst->valU32;

        if(parser->offset + len > parser->len)
        {
            wa_parse_error(parser,
                           ast,
                           "Couldn't read %.*s: unexpected end of parser.\n",
                           oc_str8_ip(label));
        }
        else
        {
            ast->str8 = oc_str8_push_buffer(parser->arena, len, &parser->contents[parser->offset]);
            parser->offset += len;
        }
    }
    wa_ast_end(parser, ast);
    return (ast);
}

wa_ast* wa_parse_value_type(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_parser_read_leb128_u32(parser, parent, label);
    ast->kind = WA_AST_VALUE_TYPE;
    ast->valueType = (wa_value_type)ast->valU32;

    if(!wa_is_value_type(ast->valU32))
    {
        wa_parse_error(parser,
                       ast,
                       "unrecognized value type 0x%02x\n",
                       ast->valU32);
    }
    return (ast);
}

void wa_parse_sections(wa_parser* parser, wa_module* module)
{
    while(!wa_parser_end(parser))
    {
        wa_ast* ast = wa_ast_begin(parser, module->root, WA_AST_SECTION);
        wa_ast* sectionID = wa_parser_read_byte(parser, ast, OC_STR8("section ID"));
        if(wa_ast_has_errors(sectionID))
        {
            return;
        }

        wa_ast* sectionLen = wa_parser_read_leb128_u32(parser, ast, OC_STR8("section length"));
        if(wa_ast_has_errors(sectionLen))
        {
            return;
        }

        u64 contentOffset = parser->offset;

        //TODO: check if section was already defined...

        wa_section* entry = 0;
        switch(sectionID->valU8)
        {
            case 0:
            {
                wa_ast* name = wa_parser_read_name(parser, ast, OC_STR8("section name"));

                if(!oc_str8_cmp(name->str8, OC_STR8("name")))
                {
                    entry = &module->toc.names;
                    ast->label = OC_STR8("Names section");
                }
                else
                {
                    entry = oc_arena_push_type(parser->arena, wa_section);
                    memset(entry, 0, sizeof(wa_section));
                    ast->label = OC_STR8("Custom section");
                }
                entry->name = name->str8;

                if(parser->offset - contentOffset > sectionLen->valU32)
                {
                    wa_parse_error(parser,
                                   ast,
                                   "Unexpected end of custom section.\n",
                                   sectionID);
                }
            }
            break;

            case 1:
                entry = &module->toc.types;
                ast->label = OC_STR8("Types section");
                break;

            case 2:
                entry = &module->toc.imports;
                ast->label = OC_STR8("Imports section");
                break;

            case 3:
                entry = &module->toc.functions;
                ast->label = OC_STR8("Functions section");
                break;

            case 4:
                entry = &module->toc.tables;
                ast->label = OC_STR8("Tables section");
                break;

            case 5:
                entry = &module->toc.memory;
                ast->label = OC_STR8("Memory section");
                break;

            case 6:
                entry = &module->toc.globals;
                ast->label = OC_STR8("Globals section");
                break;

            case 7:
                entry = &module->toc.exports;
                ast->label = OC_STR8("Exports section");
                break;

            case 8:
                entry = &module->toc.start;
                ast->label = OC_STR8("Start section");
                break;

            case 9:
                entry = &module->toc.elements;
                ast->label = OC_STR8("Elements section");
                break;

            case 10:
                entry = &module->toc.code;
                ast->label = OC_STR8("Code section");
                break;

            case 11:
                entry = &module->toc.data;
                ast->label = OC_STR8("Data section");
                break;

            case 12:
                entry = &module->toc.dataCount;
                ast->label = OC_STR8("Data count section");
                break;

            default:
            {
                wa_parse_error(parser,
                               ast,
                               "Unknown section identifier %i.\n",
                               sectionID);
            }
            break;
        }

        if(entry)
        {
            if(entry->ast)
            {
                wa_parse_error(parser, ast, "Redeclaration of %.*s.\n", oc_str8_ip(ast->label));
            }
            entry->id = sectionID->valU8;
            entry->offset = parser->offset;
            entry->len = sectionLen->valU32;
            entry->ast = ast;

            if(entry->id == 0)
            {
                entry->len = sectionLen->valU32 - (parser->offset - contentOffset);
                oc_list_push_back(&module->toc.customSections, &entry->listElt);
            }
        }
        if(contentOffset + sectionLen->valU32 > parser->len || contentOffset + sectionLen->valU32 < contentOffset)
        {
            wa_parse_error(parser,
                           ast,
                           "Length of section out of bounds.\n",
                           sectionID);
        }
        wa_parser_seek(parser, contentOffset + sectionLen->valU32, OC_STR8("next section"));
        wa_ast_end(parser, ast);
    }
}

wa_ast* wa_ast_begin_vector(wa_parser* parser, wa_ast* parent, u32* count)
{
    wa_ast* vectorAst = wa_ast_begin(parser, parent, WA_AST_VECTOR);
    wa_ast* countAst = wa_parser_read_leb128_u32(parser, vectorAst, OC_STR8("count"));

    if(wa_ast_has_errors(countAst))
    {
        *count = 0;
    }
    else
    {
        *count = countAst->valU32;
    }
    return vectorAst;
}

void wa_parse_names(wa_parser* parser, wa_module* module)
{
    wa_ast* section = module->toc.names.ast;
    if(!section)
    {
        return;
    }

    wa_parser_seek(parser, module->toc.names.offset, OC_STR8("names section"));
    u64 startOffset = parser->offset;

    while(parser->offset - startOffset < module->toc.names.len)
    {
        wa_ast* subsection = wa_ast_begin(parser, section, WA_AST_NAME_SUBSECTION);
        wa_ast* id = wa_parser_read_byte(parser, subsection, OC_STR8("subsection id"));
        wa_ast* size = wa_parser_read_leb128_u32(parser, subsection, OC_STR8("subsection size"));
        u32 subStartOffset = parser->offset;

        switch(id->valU8)
        {
            case 0:
            {
                //NOTE: module name
            }
            break;
            case 1:
            {
                //NOTE: function names
                wa_ast* vector = wa_ast_begin_vector(parser, subsection, &module->functionNameCount);
                module->functionNames = oc_arena_push_array(module->arena, wa_name_entry, module->functionNameCount);

                for(u32 entryIndex = 0; entryIndex < module->functionNameCount; entryIndex++)
                {
                    wa_ast* entry = wa_ast_begin(parser, vector, WA_AST_NAME_ENTRY);
                    wa_ast* index = wa_parser_read_leb128_u32(parser, entry, OC_STR8("index"));
                    wa_ast* name = wa_parser_read_name(parser, entry, OC_STR8("name"));

                    module->functionNames[entryIndex] = (wa_name_entry){
                        .index = index->valU32,
                        .name = name->str8,
                    };

                    wa_ast_end(parser, entry);
                }
                wa_ast_end(parser, vector);
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
                oc_log_warning("Unexpected subsection id %hhi at offset 0x%02x.\n", id->valU8, parser->offset);
            }
            break;
        }
        wa_ast_end(parser, subsection);
        wa_parser_seek(parser, subStartOffset + size->valU32, OC_STR8("next subsection"));
    }
    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.names.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.names.len,
                       parser->offset - startOffset);
    }
}

void wa_parse_types(wa_parser* parser, wa_module* module)
{
    //NOTE: parse types section
    wa_ast* section = module->toc.types.ast;
    if(!section)
    {
        return;
    }

    wa_parser_seek(parser, module->toc.types.offset, OC_STR8("types section"));
    u64 startOffset = parser->offset;

    wa_ast* vector = wa_ast_begin_vector(parser, section, &module->typeCount);

    module->types = oc_arena_push_array(parser->arena, wa_func_type, module->typeCount);

    for(u32 typeIndex = 0; typeIndex < module->typeCount; typeIndex++)
    {
        wa_func_type* type = &module->types[typeIndex];

        wa_ast* typeAst = wa_ast_begin(parser, vector, WA_AST_TYPE);
        typeAst->type = type;

        wa_ast* b = wa_parser_read_byte(parser, typeAst, OC_STR8("type prefix"));

        if(b->valU8 != 0x60)
        {
            wa_parse_error(parser,
                           b,
                           "Unexpected prefix 0x%02x for function type.\n",
                           b->valU8);
            return;
        }

        wa_ast* paramCountAst = wa_parser_read_leb128_u32(parser, typeAst, OC_STR8("parameter count"));
        type->paramCount = paramCountAst->valU32;
        type->params = oc_arena_push_array(parser->arena, wa_value_type, type->paramCount);

        for(u32 typeIndex = 0; typeIndex < type->paramCount; typeIndex++)
        {
            wa_ast* paramAst = wa_parse_value_type(parser, typeAst, OC_STR8("parameter type"));
            type->params[typeIndex] = paramAst->valU32;
        }

        wa_ast* returnCountAst = wa_parser_read_leb128_u32(parser, typeAst, OC_STR8("return count"));
        type->returnCount = returnCountAst->valU32;
        type->returns = oc_arena_push_array(parser->arena, wa_value_type, type->returnCount);

        for(u32 typeIndex = 0; typeIndex < type->returnCount; typeIndex++)
        {
            wa_ast* returnAst = wa_parse_value_type(parser, typeAst, OC_STR8("return type"));
            type->returns[typeIndex] = returnAst->valU32;
        }

        wa_ast_end(parser, typeAst);
    }
    wa_ast_end(parser, vector);

    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.types.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.types.len,
                       parser->offset - startOffset);
    }
}

wa_ast* wa_parse_limits(wa_parser* parser, wa_ast* parent, wa_limits* limits)
{
    wa_ast* limitsAst = wa_ast_begin(parser, parent, WA_AST_LIMITS);
    wa_ast* kindAst = wa_parser_read_byte(parser, limitsAst, OC_STR8("limit kind"));
    u8 kind = kindAst->valU8;

    if(kind != WA_LIMIT_MIN && kind != WA_LIMIT_MIN_MAX)
    {
        wa_parse_error(parser,
                       kindAst,
                       "Invalid limit kind 0x%02x\n",
                       kind);
    }
    else
    {
        limits->kind = kind;

        wa_ast* minAst = wa_parser_read_leb128_u32(parser, limitsAst, OC_STR8("min limit"));
        limits->min = minAst->valU32;

        if(limits->kind == WA_LIMIT_MIN_MAX)
        {
            wa_ast* maxAst = wa_parser_read_leb128_u32(parser, limitsAst, OC_STR8("max limit"));
            limits->max = maxAst->valU32;
        }
    }
    wa_ast_end(parser, limitsAst);
    return limitsAst;
}

void wa_parse_imports(wa_parser* parser, wa_module* module)
{
    //NOTE: parse import section
    wa_ast* section = module->toc.imports.ast;
    if(!section)
    {
        return;
    }

    wa_parser_seek(parser, module->toc.imports.offset, OC_STR8("import section"));
    u64 startOffset = parser->offset;

    wa_ast* vector = wa_ast_begin_vector(parser, section, &module->importCount);

    module->imports = oc_arena_push_array(parser->arena, wa_import, module->importCount);

    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];

        wa_ast* importAst = wa_ast_begin(parser, vector, WA_AST_IMPORT);
        wa_ast* moduleNameAst = wa_parser_read_name(parser, importAst, OC_STR8("module name"));
        wa_ast* importNameAst = wa_parser_read_name(parser, importAst, OC_STR8("import name"));
        wa_ast* kindAst = wa_parser_read_byte(parser, importAst, OC_STR8("import kind"));

        import->moduleName = moduleNameAst->str8;
        import->importName = importNameAst->str8;
        import->kind = kindAst->valU8;

        switch((u32)import->kind)
        {
            case WA_IMPORT_FUNCTION:
            {
                wa_ast* indexAst = wa_parser_read_leb128_u32(parser, importAst, OC_STR8("type index"));
                indexAst->kind = WA_AST_TYPE_INDEX;
                import->index = indexAst->valU32;

                if(import->index >= module->typeCount)
                {
                    wa_parse_error(parser,
                                   indexAst,
                                   "Out of bounds type index in function import (type count: %u, got index %u)\n",
                                   module->functionCount,
                                   import->index);
                }
                module->functionImportCount++;
            }
            break;
            case WA_IMPORT_TABLE:
            {
                wa_ast* typeAst = wa_parser_read_byte(parser, importAst, OC_STR8("table type"));
                import->type = typeAst->valU8;

                if(import->type != WA_TYPE_FUNC_REF && import->type != WA_TYPE_EXTERN_REF)
                {
                    wa_parse_error(parser,
                                   typeAst,
                                   "Invalid type 0x%02x in table import \n",
                                   import->type);
                }
                wa_ast* limitsAst = wa_parse_limits(parser, importAst, &import->limits);
                module->tableImportCount++;
            }
            break;

            case WA_IMPORT_MEMORY:
            {
                wa_ast* limitAst = wa_parse_limits(parser, importAst, &import->limits);
                module->memoryImportCount++;
            }
            break;

            case WA_IMPORT_GLOBAL:
            {
                //TODO: coalesce with globals section parsing

                wa_ast* typeAst = wa_parse_value_type(parser, importAst, OC_STR8("type"));
                wa_ast* mutAst = wa_parser_read_byte(parser, importAst, OC_STR8("mut"));

                import->type = typeAst->valueType;

                if(mutAst->valU8 == 0x00)
                {
                    import->mut = false;
                }
                else if(mutAst->valU8 == 0x01)
                {
                    import->mut = true;
                }
                else
                {
                    wa_parse_error(parser,
                                   mutAst,
                                   "invalid byte 0x%02hhx as global mutability.",
                                   mutAst->valU8);
                }
                module->globalImportCount++;
            }
            break;
            default:
            {
                wa_parse_error(parser,
                               importAst,
                               "Unknown import kind 0x%02x\n",
                               (u8)import->kind);
                return;
            }
        }
        wa_ast_end(parser, importAst);
    }
    wa_ast_end(parser, vector);

    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.imports.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.imports.len,
                       parser->offset - startOffset);
    }
}

void wa_parse_functions(wa_parser* parser, wa_module* module)
{
    //NOTE: parse function section
    wa_parser_seek(parser, module->toc.functions.offset, OC_STR8("functions section"));
    u64 startOffset = parser->offset;

    wa_ast* section = module->toc.functions.ast;
    wa_ast* vector = 0;
    if(section)
    {
        vector = wa_ast_begin_vector(parser, section, &module->functionCount);
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
    if(section)
    {
        for(u32 funcIndex = 0; funcIndex < module->functionCount; funcIndex++)
        {
            wa_func* func = &module->functions[module->functionImportCount + funcIndex];

            wa_ast* typeIndexAst = wa_parser_read_leb128_u32(parser, vector, OC_STR8("type index"));
            typeIndexAst->kind = WA_AST_FUNC_ENTRY;
            u32 typeIndex = typeIndexAst->valU32;

            if(typeIndex >= module->typeCount)
            {
                wa_parse_error(parser,
                               typeIndexAst,
                               "Invalid type index %i in function section\n",
                               typeIndex);
            }
            else
            {
                func->type = &module->types[typeIndex];
            }
        }
        wa_ast_end(parser, vector);
    }
    module->functionCount += module->functionImportCount;

    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.functions.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.functions.len,
                       parser->offset - startOffset);
    }
}

wa_ast* wa_parse_constant_expression(wa_parser* parser, wa_ast* parent, oc_list* list);

void wa_parse_globals(wa_parser* parser, wa_module* module)
{
    //NOTE: parse global section
    wa_parser_seek(parser, module->toc.globals.offset, OC_STR8("globals section"));
    u64 startOffset = parser->offset;

    wa_ast* section = module->toc.globals.ast;
    wa_ast* vector = 0;
    if(section)
    {
        vector = wa_ast_begin_vector(parser, section, &module->globalCount);
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

    if(section)
    {
        for(u32 globalIndex = 0; globalIndex < module->globalCount; globalIndex++)
        {
            wa_global_desc* global = &module->globals[globalIndex + module->globalImportCount];

            wa_ast* globalAst = wa_ast_begin(parser, vector, WA_AST_GLOBAL);
            wa_ast* typeAst = wa_parse_value_type(parser, globalAst, OC_STR8("type"));
            wa_ast* mutAst = wa_parser_read_byte(parser, globalAst, OC_STR8("mut"));

            global->type = typeAst->valueType;

            if(mutAst->valU8 == 0x00)
            {
                global->mut = false;
            }
            else if(mutAst->valU8 == 0x01)
            {
                global->mut = true;
            }
            else
            {
                wa_parse_error(parser,
                               mutAst,
                               "invalid byte 0x%02hhx as global mutability.",
                               mutAst->valU8);
            }
            wa_parse_constant_expression(parser, globalAst, &global->init);

            wa_ast_end(parser, globalAst);
        }
        wa_ast_end(parser, vector);
    }
    module->globalCount += module->globalImportCount;

    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.globals.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.globals.len,
                       parser->offset - startOffset);
    }
}

void wa_parse_tables(wa_parser* parser, wa_module* module)
{
    //NOTE: parse table section
    wa_parser_seek(parser, module->toc.tables.offset, OC_STR8("tables section"));
    u64 startOffset = parser->offset;

    wa_ast* section = module->toc.tables.ast;
    wa_ast* vector = 0;

    if(section)
    {
        vector = wa_ast_begin_vector(parser, section, &module->tableCount);
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

    if(section)
    {
        //NOTE: read non-import tables
        for(u32 tableIndex = 0; tableIndex < module->tableCount; tableIndex++)
        {
            wa_table_type* table = &module->tables[tableIndex + module->tableImportCount];

            wa_ast* tableAst = wa_ast_begin(parser, vector, WA_AST_TABLE_TYPE);

            //TODO coalesce with parsing of table in imports
            wa_ast* typeAst = wa_parser_read_byte(parser, tableAst, OC_STR8("table type"));
            table->type = typeAst->valU8;

            if(table->type != WA_TYPE_FUNC_REF && table->type != WA_TYPE_EXTERN_REF)
            {
                wa_parse_error(parser,
                               typeAst,
                               "Invalid type 0x%02x in table import \n",
                               table->type);
            }
            wa_ast* limitsAst = wa_parse_limits(parser, tableAst, &table->limits);

            wa_ast_end(parser, tableAst);
        }
        wa_ast_end(parser, vector);
    }
    module->tableCount += module->tableImportCount;

    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.tables.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.tables.len,
                       parser->offset - startOffset);
    }
}

void wa_parse_memories(wa_parser* parser, wa_module* module)
{
    //NOTE: parse table section
    wa_parser_seek(parser, module->toc.memory.offset, OC_STR8("memory section"));
    u64 startOffset = parser->offset;

    wa_ast* section = module->toc.memory.ast;
    wa_ast* vector = 0;

    if(section)
    {
        vector = wa_ast_begin_vector(parser, section, &module->memoryCount);
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

    if(section)
    {
        //NOTE: read non-import memories
        for(u32 memoryIndex = 0; memoryIndex < module->memoryCount; memoryIndex++)
        {
            wa_limits* memory = &module->memories[memoryIndex + module->memoryImportCount];
            wa_ast* memoryAst = wa_parse_limits(parser, vector, memory);
        }
        wa_ast_end(parser, vector);
    }
    module->memoryCount += module->memoryImportCount;

    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.memory.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.memory.len,
                       parser->offset - startOffset);
    }
}

void wa_parse_exports(wa_parser* parser, wa_module* module)
{
    //NOTE: parse export section
    wa_ast* section = module->toc.exports.ast;
    if(!section)
    {
        return;
    }

    wa_parser_seek(parser, module->toc.exports.offset, OC_STR8("exports section"));
    u64 startOffset = parser->offset;

    wa_ast* vector = wa_ast_begin_vector(parser, section, &module->exportCount);

    module->exports = oc_arena_push_array(parser->arena, wa_export, module->exportCount);

    for(u32 exportIndex = 0; exportIndex < module->exportCount; exportIndex++)
    {
        wa_export* export = &module->exports[exportIndex];

        wa_ast* exportAst = wa_ast_begin(parser, vector, WA_AST_EXPORT);
        wa_ast* nameAst = wa_parser_read_name(parser, exportAst, OC_STR8("export name"));
        wa_ast* kindAst = wa_parser_read_byte(parser, exportAst, OC_STR8("export kind"));
        wa_ast* indexAst = wa_parser_read_leb128_u32(parser, exportAst, OC_STR8("export index"));

        export->name = nameAst->str8;
        export->kind = kindAst->valU8;
        export->index = indexAst->valU32;

        switch((u32) export->kind)
        {
            case WA_EXPORT_FUNCTION:
            {
                if(export->index >= module->functionCount)
                {
                    wa_parse_error(parser,
                                   indexAst,
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
                               kindAst,
                               "Unknown export kind 0x%02x\n",
                               (u8) export->kind);
                //TODO end parsing section?
            }
        }
        wa_ast_end(parser, exportAst);
    }
    wa_ast_end(parser, vector);

    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.exports.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.exports.len,
                       parser->offset - startOffset);
    }
}

void wa_parse_start(wa_parser* parser, wa_module* module)
{
    //NOTE: parse export section
    wa_ast* section = module->toc.start.ast;
    if(!section)
    {
        return;
    }

    wa_parser_seek(parser, module->toc.start.offset, OC_STR8("start section"));
    u64 startOffset = parser->offset;

    wa_ast* startAst = wa_parser_read_leb128_u32(parser, section, OC_STR8("start index"));

    module->hasStart = true;
    module->startIndex = startAst->valU32;

    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.start.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.start.len,
                       parser->offset - startOffset);
    }
}

wa_ast* wa_parse_expression(wa_parser* parser, wa_ast* parent, u32 localCount, oc_list* list, bool constant)
{
    wa_module* module = parser->module;

    wa_ast* exprAst = wa_ast_begin(parser, parent, WA_AST_FUNC_BODY);

    //TODO: we should validate block nesting here?

    i64 blockDepth = 0;
    wa_instr* instr = 0;

    while(!wa_parser_end(parser) && blockDepth >= 0)
    {
        instr = oc_arena_push_type(parser->arena, wa_instr);
        memset(instr, 0, sizeof(wa_instr));
        oc_list_push_back(list, &instr->listElt);

        wa_ast* instrAst = wa_ast_begin(parser, exprAst, WA_AST_INSTR);
        instrAst->instr = instr;

        instr->ast = instrAst;

        wa_ast* byteAst = wa_parser_read_byte(parser, instrAst, OC_STR8("opcode"));
        u8 byte = byteAst->valU8;

        if(byte == WA_INSTR_PREFIX_EXTENDED)
        {
            wa_ast* extAst = wa_parser_read_leb128_u32(parser, instrAst, OC_STR8("extended instruction"));
            u32 code = extAst->valU32;

            if(code >= wa_instr_decode_extended_len)
            {
                wa_parse_error(parser,
                               extAst,
                               "Invalid extended instruction %i\n",
                               code);
                break;
            }
            instr->op = wa_instr_decode_extended[code];
        }
        else if(byte == WA_INSTR_PREFIX_VECTOR)
        {
            wa_ast* extAst = wa_parser_read_leb128_u32(parser, instrAst, OC_STR8("vector instruction"));
            u32 code = extAst->valU32;

            if(code >= wa_instr_decode_vector_len)
            {
                wa_parse_error(parser,
                               extAst,
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
                               byteAst,
                               "Invalid basic instruction 0x%02x\n",
                               byte);
                break;
            }
            instr->op = wa_instr_decode_basic[byte];
        }

        const wa_instr_info* info = &wa_instr_infos[instr->op];

        if(!info->defined)
        {
            wa_parse_error(parser, instrAst, "undefined instruction %s.\n", wa_instr_strings[instr->op]);
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
                               instrAst,
                               "found non-constant instruction %s while parsing constant expression.\n",
                               wa_instr_strings[instr->op]);
            }
            //TODO add constraint on global get
        }

        //NOTE: memory.init and data.drop need a data count section
        if((instr->op == WA_INSTR_memory_init || instr->op == WA_INSTR_data_drop)
           && !module->toc.dataCount.ast)
        {
            wa_parse_error(parser, instrAst, "%s requires a data count section.\n", wa_instr_strings[instr->op]);
        }

        //NOTE: parse immediates, special cases first, then generic
        if(instr->op == WA_INSTR_block
           || instr->op == WA_INSTR_loop
           || instr->op == WA_INSTR_if)
        {
            //NOTE: parse block type
            wa_ast* blockTypeAst = wa_parser_read_leb128_i64(parser, instrAst, OC_STR8("block type"));
            i64 t = blockTypeAst->valI64;
            if(t >= 0)
            {
                u64 typeIndex = (u64)t;

                if(typeIndex >= module->typeCount)
                {
                    wa_parse_error(parser,
                                   blockTypeAst,
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
                                   blockTypeAst,
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
            wa_ast* vector = wa_ast_begin_vector(parser, instrAst, &instr->immCount);

            if(instr->immCount != 1)
            {
                //TODO: should set the error on the count rather than the vector?
                wa_parse_error(parser,
                               vector,
                               "select instruction can have at most one immediate\n");
                break;
            }

            wa_ast* immAst = wa_parse_value_type(parser, vector, OC_STR8("type"));

            instr->imm = oc_arena_push_type(parser->arena, wa_code);
            instr->imm[0].valueType = immAst->valU32;

            wa_ast_end(parser, vector);
        }
        else if(instr->op == WA_INSTR_br_table)
        {
            wa_ast* vector = wa_ast_begin_vector(parser, instrAst, &instr->immCount);

            instr->immCount += 1;
            instr->imm = oc_arena_push_array(parser->arena, wa_code, instr->immCount);

            for(u32 i = 0; i < instr->immCount - 1; i++)
            {
                wa_ast* immAst = wa_parser_read_leb128_u32(parser, vector, OC_STR8("label"));
                instr->imm[i].index = immAst->valU32;
            }
            wa_ast* immAst = wa_parser_read_leb128_u32(parser, vector, OC_STR8("label"));
            instr->imm[instr->immCount - 1].index = immAst->valU32;

            wa_ast_end(parser, vector);
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
                        wa_ast* immAst = wa_parser_read_byte(parser, instrAst, OC_STR8("zero"));
                        instr->imm[immIndex].valI32 = immAst->valU8;
                    }
                    break;
                    case WA_IMM_I32:
                    {
                        wa_ast* immAst = wa_parser_read_leb128_i32(parser, instrAst, OC_STR8("i32"));
                        instr->imm[immIndex].valI32 = immAst->valI32;
                    }
                    break;
                    case WA_IMM_I64:
                    {
                        wa_ast* immAst = wa_parser_read_leb128_i64(parser, instrAst, OC_STR8("i64"));
                        instr->imm[immIndex].valI64 = immAst->valI64;
                    }
                    break;
                    case WA_IMM_F32:
                    {
                        wa_ast* immAst = wa_parser_read_f32(parser, instrAst, OC_STR8("f32"));
                        instr->imm[immIndex].valF32 = immAst->valF32;
                    }
                    break;
                    case WA_IMM_F64:
                    {
                        wa_ast* immAst = wa_parser_read_f64(parser, instrAst, OC_STR8("f64"));
                        instr->imm[immIndex].valF64 = immAst->valF64;
                    }
                    break;
                    case WA_IMM_VALUE_TYPE:
                    {
                        wa_ast* immAst = wa_parse_value_type(parser, instrAst, OC_STR8("value type"));
                        instr->imm[immIndex].valueType = immAst->valU32;
                    }
                    break;
                    case WA_IMM_REF_TYPE:
                    {
                        wa_ast* immAst = wa_parser_read_byte(parser, instrAst, OC_STR8("ref type"));
                        instr->imm[immIndex].valueType = immAst->valU8;
                    }
                    break;

                    case WA_IMM_LOCAL_INDEX:
                    {
                        wa_ast* immAst = wa_parser_read_leb128_u32(parser, instrAst, OC_STR8("index"));
                        instr->imm[immIndex].index = immAst->valU32;
                    }
                    break;

                    case WA_IMM_FUNC_INDEX:
                    {
                        wa_ast* immAst = wa_parser_read_leb128_u32(parser, instrAst, OC_STR8("function index"));
                        instr->imm[immIndex].index = immAst->valU32;
                        immAst->kind = WA_AST_FUNC_INDEX;
                    }
                    break;

                    case WA_IMM_GLOBAL_INDEX:
                    case WA_IMM_TYPE_INDEX:
                    case WA_IMM_TABLE_INDEX:
                    case WA_IMM_ELEM_INDEX:
                    case WA_IMM_DATA_INDEX:
                    case WA_IMM_LABEL:
                    {
                        wa_ast* immAst = wa_parser_read_leb128_u32(parser, instrAst, OC_STR8("index"));
                        instr->imm[immIndex].index = immAst->valU32;
                    }
                    break;
                    case WA_IMM_MEM_ARG:
                    {
                        wa_ast* memArgAst = wa_ast_begin(parser, instrAst, WA_AST_MEM_ARG);
                        wa_ast* alignAst = wa_parser_read_leb128_u32(parser, memArgAst, OC_STR8("mem arg"));
                        wa_ast* offsetAst = wa_parser_read_leb128_u32(parser, memArgAst, OC_STR8("mem arg"));

                        instr->imm[immIndex].memArg.align = alignAst->valU32;
                        instr->imm[immIndex].memArg.offset = offsetAst->valU32;

                        wa_ast_end(parser, memArgAst);
                    }
                    break;
                    case WA_IMM_LANE_INDEX:
                    {
                        wa_ast* immAst = wa_parser_read_byte(parser, instrAst, OC_STR8("lane index"));
                        instr->imm[immIndex].laneIndex = immAst->valU8;
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
        wa_ast_end(parser, instrAst);
    }

    if(!instr || instr->op != WA_INSTR_end)
    {
        wa_parse_error(parser, exprAst, "unexpected end of expression\n");
    }

    //TODO check that we exited from an end instruction

    wa_ast_end(parser, exprAst);
    return (exprAst);
}

wa_ast* wa_parse_constant_expression(wa_parser* parser, wa_ast* parent, oc_list* list)
{
    return wa_parse_expression(parser, parent, 0, list, true);
}

void wa_parse_elements(wa_parser* parser, wa_module* module)
{
    wa_ast* section = module->toc.elements.ast;
    if(!section)
    {
        return;
    }

    wa_parser_seek(parser, module->toc.elements.offset, OC_STR8("elements section"));
    u64 startOffset = parser->offset;

    wa_ast* vector = wa_ast_begin_vector(parser, section, &module->elementCount);

    module->elements = oc_arena_push_array(parser->arena, wa_element, module->elementCount);
    memset(module->elements, 0, module->elementCount * sizeof(wa_element));

    for(u32 eltIndex = 0; eltIndex < module->elementCount; eltIndex++)
    {
        wa_element* element = &module->elements[eltIndex];

        wa_ast* elementAst = wa_ast_begin(parser, vector, WA_AST_ELEMENT);
        wa_ast* prefixAst = wa_parser_read_leb128_u32(parser, elementAst, OC_STR8("prefix"));
        u32 prefix = prefixAst->valU32;

        if(prefix > 7)
        {
            wa_parse_error(parser, prefixAst, "invalid element prefix %u\n", prefix);
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
                wa_ast* tableIndexAst = wa_parser_read_leb128_u32(parser, elementAst, OC_STR8("table index"));
                //TODO validate index
                element->tableIndex = tableIndexAst->valU32;
            }
            wa_parse_constant_expression(parser, elementAst, &element->tableOffset);
        }

        element->type = WA_TYPE_FUNC_REF;

        if(prefix & 0x04)
        {
            //NOTE: reftype? vec(expr)
            if(prefix & 0x03)
            {
                //NOTE ref type
                wa_ast* refTypeAst = wa_parser_read_byte(parser, elementAst, OC_STR8("refType"));
                if(refTypeAst->valU8 != WA_TYPE_FUNC_REF && refTypeAst->valU8 != WA_TYPE_EXTERN_REF)
                {
                    wa_parse_error(parser, refTypeAst, "ref type should be externref or funcref.");
                }
                element->type = refTypeAst->valU8;
            }

            wa_ast* exprVec = wa_ast_begin(parser, elementAst, WA_AST_VECTOR);
            wa_ast* exprVecCount = wa_parser_read_leb128_u32(parser, exprVec, OC_STR8("count"));
            element->initCount = exprVecCount->valU32;
            element->initInstr = oc_arena_push_array(parser->arena, oc_list, element->initCount);
            memset(element->initInstr, 0, element->initCount * sizeof(oc_list));

            for(u32 i = 0; i < element->initCount; i++)
            {
                wa_parse_constant_expression(parser, elementAst, &element->initInstr[i]);
            }
        }
        else
        {
            //NOTE: refkind? vec(funcIdx)
            if(prefix & 0x03)
            {
                //NOTE refkind
                wa_ast* refKindAst = wa_parser_read_byte(parser, elementAst, OC_STR8("refKind"));
                if(refKindAst->valU8 != 0x00)
                {
                    wa_parse_error(parser, refKindAst, "ref kind should be 0.");
                }
            }

            wa_ast* funcVec = wa_ast_begin(parser, elementAst, WA_AST_VECTOR);
            wa_ast* funcVecCount = wa_parser_read_leb128_u32(parser, funcVec, OC_STR8("count"));
            element->initCount = funcVecCount->valU32;
            element->initInstr = oc_arena_push_array(parser->arena, oc_list, element->initCount);
            memset(element->initInstr, 0, element->initCount * sizeof(oc_list));

            for(u32 i = 0; i < element->initCount; i++)
            {
                //TODO validate index
                wa_ast* funcIndexAst = wa_parser_read_leb128_u32(parser, funcVec, OC_STR8("index"));
                funcIndexAst->kind = WA_AST_FUNC_INDEX;

                wa_instr* init = oc_arena_push_array(parser->arena, wa_instr, 2);
                memset(init, 0, 2 * sizeof(wa_instr));

                init[0].op = WA_INSTR_ref_func;
                init[0].immCount = 1;
                init[0].imm = oc_arena_push_type(parser->arena, wa_code);
                init[0].imm[0].index = funcIndexAst->valU32;
                oc_list_push_back(&element->initInstr[i], &init[0].listElt);

                init[1].op = WA_INSTR_end;
                oc_list_push_back(&element->initInstr[i], &init[1].listElt);
            }
        }
        wa_ast_end(parser, elementAst);
    }
    wa_ast_end(parser, vector);

    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.elements.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.elements.len,
                       parser->offset - startOffset);
    }
}

void wa_parse_data_count(wa_parser* parser, wa_module* module)
{
    wa_ast* section = module->toc.dataCount.ast;
    if(!section)
    {
        return;
    }

    wa_parser_seek(parser, module->toc.dataCount.offset, OC_STR8("data count section"));
    u64 startOffset = parser->offset;

    wa_ast* dataCount = wa_parser_read_leb128_u32(parser, section, OC_STR8("data count"));
    module->dataCount = dataCount->valU32;

    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.dataCount.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.dataCount.len,
                       parser->offset - startOffset);
    }
}

void wa_parse_data(wa_parser* parser, wa_module* module)
{
    wa_ast* section = module->toc.data.ast;
    if(!section)
    {
        return;
    }

    wa_parser_seek(parser, module->toc.data.offset, OC_STR8("data section"));
    u64 startOffset = parser->offset;

    u32 dataCount = 0;
    wa_ast* vector = wa_ast_begin_vector(parser, section, &dataCount);

    if(module->toc.dataCount.ast && dataCount != module->dataCount)
    {
        wa_parse_error(parser,
                       vector,
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

        wa_ast* segmentAst = wa_ast_begin(parser, vector, WA_AST_DATA_SEGMENT);
        wa_ast* prefixAst = wa_parser_read_leb128_u32(parser, segmentAst, OC_STR8("prefix"));
        u32 prefix = prefixAst->valU32;

        if(prefix > 2)
        {
            wa_parse_error(parser, prefixAst, "invalid segment prefix %u\n", prefix);
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
                wa_ast* memoryIndexAst = wa_parser_read_leb128_u32(parser, segmentAst, OC_STR8("memory index"));
                //TODO validate index
                seg->memoryIndex = memoryIndexAst->valU32;
            }
            wa_parse_constant_expression(parser, segmentAst, &seg->memoryOffset);
        }

        //NOTE: parse vec(bytes)
        wa_ast* initVec = wa_parser_read_bytes_vector(parser, segmentAst, OC_STR8("init"));
        seg->init = initVec->str8;

        wa_ast_end(parser, segmentAst);
    }
    wa_ast_end(parser, vector);

    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.data.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.data.len,
                       parser->offset - startOffset);
    }
}

void wa_parse_code(wa_parser* parser, wa_module* module)
{
    wa_ast* section = module->toc.code.ast;
    if(!section)
    {
        if(module->functionCount - module->functionImportCount)
        {
            wa_parse_error(parser,
                           module->toc.functions.ast,
                           "Function section declares %i functions, but code section is absent",
                           module->functionCount - module->functionImportCount);
        }
        return;
    }

    wa_parser_seek(parser, module->toc.code.offset, OC_STR8("code section"));
    u64 startOffset = parser->offset;

    u32 functionCount = 0;
    wa_ast* vector = wa_ast_begin_vector(parser, section, &functionCount);

    if(functionCount != module->functionCount - module->functionImportCount)
    {
        //TODO should set the error on the count, not the vector?
        wa_parse_error(parser,
                       vector,
                       "Function count mismatch (function section: %i, code section: %i\n",
                       module->functionCount - module->functionImportCount,
                       functionCount);
    }
    functionCount = oc_min(functionCount + module->functionImportCount, module->functionCount);

    for(u32 funcIndex = module->functionImportCount; funcIndex < functionCount; funcIndex++)
    {
        wa_func* func = &module->functions[funcIndex];

        wa_ast* funcAst = wa_ast_begin(parser, vector, WA_AST_FUNC);
        funcAst->func = func;

        oc_arena_scope scratch = oc_scratch_begin();

        wa_ast* lenAst = wa_parser_read_leb128_u32(parser, funcAst, OC_STR8("function length"));

        u32 funcLen = lenAst->valU32;
        u32 funcStartOffset = parser->offset;

        //NOTE: parse locals
        u32 localEntryCount = 0;
        wa_ast* localsVector = wa_ast_begin_vector(parser, funcAst, &localEntryCount);

        u32* counts = oc_arena_push_array(scratch.arena, u32, localEntryCount);
        wa_value_type* types = oc_arena_push_array(scratch.arena, wa_value_type, localEntryCount);

        func->localCount = func->type->paramCount;
        for(u32 localEntryIndex = 0; localEntryIndex < localEntryCount; localEntryIndex++)
        {
            wa_ast* localEntryAst = wa_ast_begin(parser, localsVector, WA_AST_LOCAL_ENTRY);
            wa_ast* countAst = wa_parser_read_leb128_u32(parser, localEntryAst, OC_STR8("count"));
            wa_ast* typeAst = wa_parser_read_byte(parser, localEntryAst, OC_STR8("type"));
            typeAst->kind = WA_AST_VALUE_TYPE;

            counts[localEntryIndex] = countAst->valU32;
            types[localEntryIndex] = typeAst->valU8;

            if(func->localCount + counts[localEntryIndex] < func->localCount)
            {
                //NOTE: overflow
                wa_parse_error(parser,
                               funcAst,
                               "Too many locals for function %i\n",
                               funcIndex);
                goto parse_function_end;
            }

            func->localCount += counts[localEntryIndex];

            wa_ast_end(parser, localEntryAst);
            //TODO: validate types? or validate later?
        }
        wa_ast_end(parser, localsVector);

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
        wa_ast* bodyAst = wa_parse_expression(parser, funcAst, func->localCount, &func->instructions, false);

        wa_ast_end(parser, funcAst);

        //NOTE: check entry length
        if(parser->offset - funcStartOffset != funcLen)
        {
            wa_parse_error(parser,
                           funcAst,
                           "Size of code entry %i does not match declared size (declared %u, got %u)\n",
                           funcIndex,
                           funcLen,
                           parser->offset - funcStartOffset);
            goto parse_function_end;
        }

    parse_function_end:
        oc_scratch_end(scratch);
        wa_parser_seek(parser, funcStartOffset + funcLen, OC_STR8("next function"));
    }

    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.code.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.code.len,
                       parser->offset - startOffset);
    }
}

void wa_parse_module(wa_module* module, oc_str8 contents)
{
    wa_parser parser = {
        .module = module,
        .arena = module->arena,
        .len = contents.len,
        .contents = contents.ptr,
    };

    module->root = wa_ast_alloc(&parser, WA_AST_ROOT);

    wa_ast* magic = wa_parser_read_raw_u32(&parser, module->root, OC_STR8("wasm magic number"));
    if(magic->kind == WA_AST_U32
       && magic->valU32 != 0x6d736100)
    {
        wa_parse_error(&parser, magic, "wrong wasm magic number");
        return;
    }
    magic->kind = WA_AST_MAGIC;

    wa_ast* version = wa_parser_read_raw_u32(&parser, module->root, OC_STR8("wasm version"));
    if(version->kind == WA_AST_U32
       && version->valU32 != 1)
    {
        wa_parse_error(&parser, version, "wrong wasm version");
        return;
    }

    wa_parse_sections(&parser, module);

    wa_parse_dwarf(contents, module);

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
