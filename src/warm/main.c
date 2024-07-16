#include <stdio.h>

#define OC_NO_APP_LAYER 1
#include "orca.h"

#include "wa_types.h"
#include "wasm_tables.c"

//-------------------------------------------------------------------------
// errors
//-------------------------------------------------------------------------

void wa_parse_error(wa_module* module, const char* fmt, ...)
{
    wa_error_elt* error = oc_arena_push_type(module->arena, wa_error_elt);

    va_list ap;
    va_start(ap, fmt);
    error->string = oc_str8_pushfv(module->arena, fmt, ap);
    va_end(ap);

    oc_list_push_back(&module->errors, &error->listElt);
}

void wa_module_print_errors(wa_module* module)
{
    oc_list_for(module->errors, err, wa_error_elt, listElt)
    {
        printf("%.*s", oc_str8_ip(err->string));
    }
}

//-------------------------------------------------------------------------
// Input
//-------------------------------------------------------------------------
typedef enum wa_input_status
{
    WA_INPUT_OK = 0,
    WA_INPUT_EOF,
    WA_INPUT_OVERFLOW,
} wa_input_status;

typedef struct wa_input
{
    wa_module* module;
    char* contents;
    u64 len;
    u64 offset;
    wa_input_status status;

} wa_input;

char* wa_input_head(wa_input* input)
{
    return (input->contents + input->offset);
}

bool wa_input_end(wa_input* input)
{
    return (input->offset >= input->len);
}

void wa_input_seek(wa_input* input, u64 offset, oc_str8 label)
{
    if(offset > input->len)
    {
        input->offset = input->len;
        input->status = WA_INPUT_EOF;
        wa_parse_error(input->module,
                       "Couldn't seek %.*s: unexpected end of input.\n",
                       oc_str8_ip(label));
    }
    else
    {
        input->offset = offset;
    }
}

int wa_read_status(wa_input* input)
{
    return (input->status);
}

u8 wa_read_byte(wa_input* input, oc_str8 label)
{
    if(input->status)
    {
        return (0);
    }
    if(input->offset + sizeof(u8) > input->len)
    {
        input->status = WA_INPUT_EOF;
        wa_parse_error(input->module,
                       "Couldn't read %.*s: unexpected end of input.\n",
                       oc_str8_ip(label));
        return (0);
    }

    u8 r = *(u8*)&input->contents[input->offset];
    input->offset += sizeof(u8);
    return (r);
}

u32 wa_read_raw_u32(wa_input* input, oc_str8 label)
{
    if(input->status)
    {
        return (0);
    }
    if(input->offset + sizeof(u32) > input->len)
    {
        input->status = WA_INPUT_EOF;
        wa_parse_error(input->module,
                       "Couldn't read %.*s: unexpected end of input.\n",
                       oc_str8_ip(label));
        return (0);
    }

    u32 r = *(u32*)&input->contents[input->offset];
    input->offset += sizeof(u32);
    return (r);
}

u64 wa_read_leb128_u64(wa_input* input, oc_str8 label)
{
    if(input->status)
    {
        return (0);
    }

    char byte = 0;
    u64 shift = 0;
    u64 res = 0;

    do
    {
        if(input->offset + sizeof(char) > input->len)
        {
            input->status = WA_INPUT_EOF;
            wa_parse_error(input->module,
                           "Couldn't read %.*s: unexpected end of input.\n",
                           oc_str8_ip(label));
            return (0);
        }

        byte = input->contents[input->offset];
        input->offset++;

        if(shift >= 63 && byte > 1)
        {
            input->status = WA_INPUT_EOF;
            wa_parse_error(input->module,
                           "Couldn't read %.*s: leb128 overflow.\n",
                           oc_str8_ip(label));
            return (0);
        }

        res |= ((u64)byte & 0x7f) << shift;

        shift += 7;
    }
    while(byte & 0x80);

    return (res);
}

i64 wa_read_leb128_i64(wa_input* input, oc_str8 label)
{
    if(input->status)
    {
        return (0);
    }

    char byte = 0;
    u64 shift = 0;
    i64 res = 0;

    do
    {
        if(input->offset + sizeof(char) > input->len)
        {
            input->status = WA_INPUT_EOF;
            wa_parse_error(input->module,
                           "Couldn't read %.*s: unexpected end of input.\n",
                           oc_str8_ip(label));
            return (0);
        }
        byte = input->contents[input->offset];
        input->offset++;

        if(shift >= 63 && byte > 1)
        {
            input->status = WA_INPUT_EOF;
            wa_parse_error(input->module,
                           "Couldn't read %.*s: leb128 overflow.\n",
                           oc_str8_ip(label));
            return (0);
        }

        res |= ((u64)byte & 0x7f) << shift;
        shift += 7;
    }
    while(byte & 0x80);

    if(shift < 64 && (byte & 0x40))
    {
        res |= (~0ULL << shift);
    }

    return (res);
}

u32 wa_read_leb128_u32(wa_input* input, oc_str8 label)
{
    if(input->status)
    {
        return (0);
    }

    u64 r = wa_read_leb128_u64(input, label);
    if(r >= INT32_MAX)
    {
        r = 0;
        if(input->status == WA_INPUT_OK)
        {
            input->status = WA_INPUT_OVERFLOW;
            wa_parse_error(input->module,
                           "Couldn't read %.*s: leb128 overflow.\n",
                           oc_str8_ip(label));
        }
    }
    return (u32)r;
}

f32 wa_read_f32(wa_input* input, oc_str8 label)
{
    if(input->status)
    {
        return (0);
    }
    if(input->offset + sizeof(f32) > input->len)
    {
        input->status = WA_INPUT_EOF;
        wa_parse_error(input->module,
                       "Couldn't read %.*s: unexpected end of input.\n",
                       oc_str8_ip(label));
        return (0);
    }
    f32 f = *(f32*)&input->contents[input->offset];
    input->offset += sizeof(f32);
    return f;
}

f64 wa_read_f64(wa_input* input, oc_str8 label)
{
    if(input->status)
    {
        return (0);
    }
    if(input->offset + sizeof(f64) > input->len)
    {
        input->status = WA_INPUT_EOF;
        wa_parse_error(input->module,
                       "Couldn't read %.*s: unexpected end of input.\n",
                       oc_str8_ip(label));
        return (0);
    }
    f64 f = *(f64*)&input->contents[input->offset];
    input->offset += sizeof(f64);
    return f;
}

oc_str8 wa_read_name(wa_input* input, oc_arena* arena, oc_str8 label)
{
    u32 len = wa_read_leb128_u32(input, label);
    if(input->status)
    {
        return (oc_str8){ 0 };
    }
    if(input->offset + len > input->len)
    {
        input->status = WA_INPUT_EOF;
        wa_parse_error(input->module,
                       "Couldn't read %.*s: unexpected end of input.\n",
                       oc_str8_ip(label));
        return (oc_str8){ 0 };
    }
    oc_str8 res = {
        .ptr = &input->contents[input->offset],
        .len = len,
    };
    input->offset += len;
    return (res);
}

//-------------------------------------------------------------------------
// Parsing
//-------------------------------------------------------------------------

void wa_parse_sections(wa_input* input, wa_module* module)
{
    u32 magic = wa_read_raw_u32(input, OC_STR8("wasm magic number"));
    u32 version = wa_read_raw_u32(input, OC_STR8("wasm version"));

    if(wa_read_status(input))
    {
        return;
    }

    while(!wa_input_end(input))
    {
        u8 sectionID = wa_read_byte(input, OC_STR8("section ID"));
        u32 sectionLen = wa_read_leb128_u32(input, OC_STR8("section length"));

        if(wa_read_status(input))
        {
            return;
        }

        u64 contentOffset = input->offset;

        switch(sectionID)
        {
            case 0:
                break;

                //TODO: check if section was already defined...

            case 1:
            {
                module->toc.types.offset = input->offset;
                module->toc.types.len = sectionLen;
            }
            break;

            case 2:
            {
                module->toc.imports.offset = input->offset;
                module->toc.imports.len = sectionLen;
            }
            break;

            case 3:
            {
                module->toc.functions.offset = input->offset;
                module->toc.functions.len = sectionLen;
            }
            break;

            case 4:
            {
                module->toc.tables.offset = input->offset;
                module->toc.tables.len = sectionLen;
            }
            break;

            case 5:
            {
                module->toc.memory.offset = input->offset;
                module->toc.memory.len = sectionLen;
            }
            break;

            case 6:
            {
                module->toc.globals.offset = input->offset;
                module->toc.globals.len = sectionLen;
            }
            break;

            case 7:
            {
                module->toc.exports.offset = input->offset;
                module->toc.exports.len = sectionLen;
            }
            break;

            case 8:
            {
                module->toc.start.offset = input->offset;
                module->toc.start.len = sectionLen;
            }
            break;

            case 9:
            {
                module->toc.elements.offset = input->offset;
                module->toc.elements.len = sectionLen;
            }
            break;

            case 10:
            {
                module->toc.code.offset = input->offset;
                module->toc.code.len = sectionLen;
            }
            break;

            case 11:
            {
                module->toc.data.offset = input->offset;
                module->toc.data.len = sectionLen;
            }
            break;

            case 12:
            {
                module->toc.dataCount.offset = input->offset;
                module->toc.dataCount.len = sectionLen;
            }
            break;

            default:
            {
                wa_parse_error(module,
                               "Unknown section identifier %i.\n",
                               sectionID);
            }
        }
        wa_input_seek(input, contentOffset + sectionLen, OC_STR8("next section"));
    }
}

void wa_parse_types(oc_arena* arena, wa_input* input, wa_module* module)
{
    //NOTE: parse types section
    wa_input_seek(input, module->toc.types.offset, OC_STR8("types section"));

    u64 startOffset = input->offset;

    module->typeCount = wa_read_leb128_u32(input, OC_STR8("types count"));
    module->types = oc_arena_push_array(arena, wa_func_type, module->typeCount);

    for(u32 typeIndex = 0; typeIndex < module->typeCount; typeIndex++)
    {
        wa_func_type* type = &module->types[typeIndex];

        u8 b = wa_read_byte(input, OC_STR8("type prefix"));

        if(b != 0x60)
        {
            wa_parse_error(module,
                           "Unexpected prefix 0x%02x for function type.\n",
                           b);
            return;
        }

        type->paramCount = wa_read_leb128_u32(input, OC_STR8("parameter count"));

        type->params = oc_arena_push_array(arena, wa_value_type, type->paramCount);
        for(u32 typeIndex = 0; typeIndex < type->paramCount; typeIndex++)
        {
            type->params[typeIndex] = wa_read_leb128_u32(input, OC_STR8("parameter type"));
        }

        type->returnCount = wa_read_leb128_u32(input, OC_STR8("return count"));
        type->returns = oc_arena_push_array(arena, wa_value_type, type->returnCount);
        for(u32 typeIndex = 0; typeIndex < type->returnCount; typeIndex++)
        {
            type->returns[typeIndex] = wa_read_leb128_u32(input, OC_STR8("return type"));
        }
    }

    //NOTE: check section size
    if(input->offset - startOffset != module->toc.types.len)
    {
        wa_parse_error(module,
                       "Size of type section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.types.len,
                       input->offset - startOffset);
    }
}

void wa_parse_imports(oc_arena* arena, wa_input* input, wa_module* module)
{
    //NOTE: parse import section
    wa_input_seek(input, module->toc.imports.offset, OC_STR8("import section"));

    module->importCount = wa_read_leb128_u32(input, OC_STR8("imports count"));
    module->imports = oc_arena_push_array(arena, wa_import, module->importCount);

    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];

        import->moduleName = wa_read_name(input, arena, OC_STR8("module name"));
        import->importName = wa_read_name(input, arena, OC_STR8("import name"));
        import->kind = wa_read_byte(input, OC_STR8("import kind"));

        switch((u32)import->kind)
        {
            case WA_IMPORT_FUNCTION:
            {
                import->index = wa_read_leb128_u32(input, OC_STR8("function index"));
                if(import->index >= module->functionCount)
                {
                    wa_parse_error(module,
                                   "Out of bounds type index in function import (function count: %u, got index %u)\n",
                                   module->functionCount,
                                   import->index);
                }
            }
            break;
            case WA_IMPORT_TABLE:
            {
                import->type = wa_read_byte(input, OC_STR8("table type"));
                if(import->type != WA_TYPE_FUNC_REF && import->type != WA_TYPE_EXTERN_REF)
                {
                    wa_parse_error(module, "Invalid type 0x%02x in table import \n",
                                   import->type);
                }
                u8 b = wa_read_byte(input, OC_STR8("table limit kind"));
                if(b != WA_LIMIT_MIN && b != WA_LIMIT_MIN_MAX)
                {
                    wa_parse_error(module,
                                   "Invalid limit kind 0x%02x in table import\n",
                                   b);
                    return;
                }
                import->limits.kind = b;

                import->limits.min = wa_read_leb128_u32(input, OC_STR8("table min limit"));
                if(import->limits.kind == WA_LIMIT_MIN_MAX)
                {
                    import->limits.max = wa_read_leb128_u32(input, OC_STR8("table max limit"));
                }
            }
            break;
            case WA_IMPORT_MEMORY:
            {
                u8 b = wa_read_byte(input, OC_STR8("memory limit kind"));
                if(b != WA_LIMIT_MIN && b != WA_LIMIT_MIN_MAX)
                {
                    wa_parse_error(module,
                                   "Invalid limit kind 0x%02x in table import\n",
                                   b);
                    return;
                }
                import->limits.kind = b;

                import->limits.min = wa_read_leb128_u32(input, OC_STR8("memory min limit"));
                if(import->limits.kind == WA_LIMIT_MIN_MAX)
                {
                    import->limits.max = wa_read_leb128_u32(input, OC_STR8("memory max limit"));
                }
            }
            break;
            case WA_IMPORT_GLOBAL:
            {
                //TODO
            }
            break;
            default:
            {
                wa_parse_error(module,
                               "Unknown import kind 0x%02x\n",
                               (u8)import->kind);
                return;
            }
        }
    }
}

void wa_parse_functions(oc_arena* arena, wa_input* input, wa_module* module)
{
    //NOTE: parse function section
    wa_input_seek(input, module->toc.functions.offset, OC_STR8("functions section"));

    module->functionCount = wa_read_leb128_u32(input, OC_STR8("functions count"));
    module->functions = oc_arena_push_array(arena, wa_func, module->functionCount);

    for(u32 funcIndex = 0; funcIndex < module->functionCount; funcIndex++)
    {
        wa_func* func = &module->functions[funcIndex];
        u32 typeIndex = wa_read_leb128_u32(input, OC_STR8("function type index"));

        if(typeIndex >= module->typeCount)
        {
            wa_parse_error(module,
                           "Invalid type index %i in function section\n",
                           typeIndex);
        }
        func->type = &module->types[typeIndex];
    }
}

void wa_parse_exports(oc_arena* arena, wa_input* input, wa_module* module)
{
    //NOTE: parse export section
    wa_input_seek(input, module->toc.exports.offset, OC_STR8("exports section"));

    module->exportCount = wa_read_leb128_u32(input, OC_STR8("exports count"));
    module->exports = oc_arena_push_array(arena, wa_export, module->exportCount);

    for(u32 exportIndex = 0; exportIndex < module->exportCount; exportIndex++)
    {
        wa_export* export = &module->exports[exportIndex];

        export->name = wa_read_name(input, arena, OC_STR8("export name"));
        export->kind = wa_read_byte(input, OC_STR8("export kind"));
        export->index = wa_read_leb128_u32(input, OC_STR8("export index"));

        switch((u32) export->kind)
        {
            case WA_EXPORT_FUNCTION:
            {
                if(export->index >= module->functionCount)
                {
                    wa_parse_error(module,
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
                wa_parse_error(module,
                               "Unknown export kind 0x%02x\n",
                               (u8) export->kind);
            }
        }
    }
}

//-------------------------------------------------------------------------
// build context
//-------------------------------------------------------------------------

static const wa_value_type WA_BLOCK_TYPE_I32_VAL = WA_TYPE_I32;
static const wa_value_type WA_BLOCK_TYPE_I64_VAL = WA_TYPE_I64;
static const wa_value_type WA_BLOCK_TYPE_F32_VAL = WA_TYPE_F32;
static const wa_value_type WA_BLOCK_TYPE_F64_VAL = WA_TYPE_F64;
static const wa_value_type WA_BLOCK_TYPE_V128_VAL = WA_TYPE_V128;
static const wa_value_type WA_BLOCK_TYPE_FUNC_REF_VAL = WA_TYPE_FUNC_REF;
static const wa_value_type WA_BLOCK_TYPE_EXTERN_REF_VAL = WA_TYPE_EXTERN_REF;

static const wa_func_type WA_BLOCK_VALUE_TYPES[] = {
    { 0 },
    {
        .returnCount = 1,
        .returns = (wa_value_type*)&WA_BLOCK_TYPE_I32_VAL,
    },
    {
        .returnCount = 1,
        .returns = (wa_value_type*)&WA_BLOCK_TYPE_I64_VAL,
    },
    {
        .returnCount = 1,
        .returns = (wa_value_type*)&WA_BLOCK_TYPE_F32_VAL,
    },
    {
        .returnCount = 1,
        .returns = (wa_value_type*)&WA_BLOCK_TYPE_F64_VAL,
    },
    {
        .returnCount = 1,
        .returns = (wa_value_type*)&WA_BLOCK_TYPE_V128_VAL,
    },
    {
        .returnCount = 1,
        .returns = (wa_value_type*)&WA_BLOCK_TYPE_FUNC_REF_VAL,
    },
    {
        .returnCount = 1,
        .returns = (wa_value_type*)&WA_BLOCK_TYPE_EXTERN_REF_VAL,
    },
};

enum
{
    WA_TYPE_STACK_MAX_LEN = 512,
    WA_CONTROL_STACK_MAX_LEN = 512,
    WA_MAX_REG = 4096,
};

typedef enum wa_operand_slot_kind
{
    WA_OPERAND_SLOT_NIL = 0,
    WA_OPERAND_SLOT_LOCAL,
    WA_OPERAND_SLOT_REG,
} wa_operand_slot_kind;

typedef struct wa_operand_slot
{
    wa_operand_slot_kind kind;
    wa_value_type type;
    u32 index;
    u64 count;
    u64 originInstr;
    u64 originOpd;
} wa_operand_slot;

typedef struct wa_block
{
    wa_instr* begin;
    u64 beginOffset;
    u64 elseOffset;
    u64 scopeBase;
    oc_list jumpTargets;

} wa_block;

typedef struct wa_build_context
{
    oc_arena checkArena;
    oc_arena codeArena;
    wa_module* module;

    u64 opdStackLen;
    u64 opdStackCap;
    wa_operand_slot* opdStack;

    u64 controlStackLen;
    u64 controlStackCap;
    wa_block* controlStack;

    u64 nextRegIndex;
    u64 freeRegLen;
    u64 freeRegs[WA_MAX_REG];

    u64 codeCap;
    u64 codeLen;
    wa_code* code;

} wa_build_context;

bool wa_operand_slot_is_nil(wa_operand_slot* slot)
{
    return (slot->kind == WA_OPERAND_SLOT_NIL);
}

bool wa_block_is_nil(wa_block* block)
{
    return (block->begin == 0);
}

void wa_control_stack_push(wa_build_context* context, wa_instr* instr)
{
    if(context->controlStack == 0 || context->controlStackLen >= context->controlStackCap)
    {
        context->controlStackCap = (context->controlStackCap + 8) * 2;
        wa_block* tmp = context->controlStack;
        context->controlStack = oc_arena_push_array(&context->checkArena, wa_block, context->controlStackCap);
        OC_ASSERT(context->controlStack, "out of memory");

        if(tmp)
        {
            memcpy(context->controlStack, tmp, context->controlStackLen * sizeof(wa_block));
        }
    }
    context->controlStack[context->controlStackLen] = (wa_block){
        .begin = instr,
        .beginOffset = context->codeLen,
        .scopeBase = context->opdStackLen,
    };
    context->controlStackLen++;
}

wa_block wa_control_stack_pop(wa_build_context* context)
{
    wa_block block = { 0 };
    if(context->controlStackLen)
    {
        context->controlStackLen--;
        block = context->controlStack[context->controlStackLen];
    }
    return (block);
}

wa_block* wa_control_stack_lookup(wa_build_context* context, u32 label)
{
    wa_block* block = 0;
    if(label < context->controlStackLen)
    {
        block = &context->controlStack[context->controlStackLen - label - 1];
    }
    return (block);
}

wa_block* wa_control_stack_top(wa_build_context* context)
{
    return wa_control_stack_lookup(context, 0);
}

wa_block wa_control_stack_top_value(wa_build_context* context)
{
    wa_block* block = wa_control_stack_top(context);
    if(block)
    {
        return (*block);
    }
    else
    {
        return ((wa_block){ 0 });
    }
}

void wa_operand_stack_push(wa_build_context* context, wa_operand_slot s)
{
    if(context->opdStack == 0 || context->opdStackLen >= context->opdStackCap)
    {
        context->opdStackCap = (context->opdStackCap + 8) * 2;
        wa_operand_slot* tmp = context->opdStack;
        context->opdStack = oc_arena_push_array(&context->checkArena, wa_operand_slot, context->opdStackCap);
        OC_ASSERT(context->opdStack, "out of memory");

        if(tmp)
        {
            memcpy(context->opdStack, tmp, context->opdStackLen * sizeof(wa_operand_slot));
        }
    }
    context->opdStack[context->opdStackLen] = s;
    context->opdStackLen++;
}

void wa_free_slot(wa_build_context* context, u64 index)
{
    OC_DEBUG_ASSERT(context->freeRegLen >= WA_MAX_REG);
    context->freeRegs[context->freeRegLen] = index;
    context->freeRegLen++;
}

wa_operand_slot wa_operand_stack_pop(wa_build_context* context)
{
    wa_operand_slot slot = { 0 };
    u64 scopeBase = 0;
    wa_block* block = wa_control_stack_top(context);
    if(block)
    {
        scopeBase = block->scopeBase;
    }

    if(context->opdStackLen > scopeBase)
    {
        context->opdStackLen--;
        slot = context->opdStack[context->opdStackLen];

        if(slot.kind == WA_OPERAND_SLOT_REG && slot.count == 0)
        {
            wa_free_slot(context, slot.index);
        }
    }
    return (slot);
}

void wa_operand_stack_pop_slots(wa_build_context* context, u64 count)
{
    OC_DEBUG_ASSERT(count <= context->opdStackLen);

    for(u64 i = 0; i < count; i++)
    {
        wa_operand_slot* slot = &context->opdStack[context->opdStackLen - i - 1];
        if(slot->kind == WA_OPERAND_SLOT_REG && slot->count == 0)
        {
            wa_free_slot(context, slot->index);
        }
    }

    context->opdStackLen -= count;
}

wa_operand_slot wa_operand_stack_top(wa_build_context* context)
{
    wa_operand_slot slot = { 0 };
    u64 scopeBase = 0;
    wa_block* block = wa_control_stack_top(context);
    if(block)
    {
        scopeBase = block->scopeBase;
    }

    if(context->opdStackLen > scopeBase)
    {
        slot = context->opdStack[context->opdStackLen - 1];
    }
    return (slot);
}

const char* wa_value_type_string(wa_value_type t)
{
    switch(t)
    {
        case WA_TYPE_I32:
            return "i32";
        case WA_TYPE_I64:
            return "i64";
        case WA_TYPE_F32:
            return "f32";
        case WA_TYPE_F64:
            return "f64";
        case WA_TYPE_V128:
            return "vec128";
        case WA_TYPE_FUNC_REF:
            return "funcref";
        case WA_TYPE_EXTERN_REF:
            return "externref";
        default:
            return "invalid type";
    }
}

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

void wa_emit(wa_build_context* context, wa_code code)
{
    if(context->codeLen >= context->codeCap)
    {
        //TODO: better handle realloc
        oc_arena_scope scratch = oc_scratch_begin();
        wa_code* tmp = oc_arena_push_array(scratch.arena, wa_code, context->codeLen);
        memcpy(tmp, context->code, context->codeLen * sizeof(wa_code));

        context->codeCap = (context->codeCap + 1) * 1.5;

        oc_arena_clear(&context->codeArena);
        context->code = oc_arena_push_array(&context->codeArena, wa_code, context->codeCap);
        memcpy(context->code, tmp, context->codeLen * sizeof(wa_code));

        oc_scratch_end(scratch);
    }
    context->code[context->codeLen] = code;
    context->codeLen++;
}

u32 wa_allocate_register(wa_build_context* context)
{
    u32 index = 0;
    if(context->freeRegLen)
    {
        context->freeRegLen--;
        index = context->freeRegs[context->freeRegLen];
    }
    else
    {
        //TODO: prevent overflow
        index = context->nextRegIndex;
        context->nextRegIndex++;
    }
    return (index);
}

void wa_move_slot_if_used(wa_build_context* context, u32 slotIndex)
{
    u32 newReg = UINT32_MAX;
    u64 count = 0;
    for(u32 stackIndex = 0; stackIndex < context->opdStackLen; stackIndex++)
    {
        wa_operand_slot* slot = &context->opdStack[stackIndex];
        if(slot->index == slotIndex)
        {
            if(newReg == UINT32_MAX)
            {
                newReg = wa_allocate_register(context);
            }
            slot->kind = WA_OPERAND_SLOT_REG;
            slot->index = newReg;
            slot->count = count;
            count++;
        }
    }
    if(newReg != UINT32_MAX)
    {
        wa_emit(context, (wa_code){ .opcode = WA_INSTR_move });
        wa_emit(context, (wa_code){ .operand.immI32 = slotIndex });
        wa_emit(context, (wa_code){ .operand.immI32 = newReg });
    }
}

void wa_move_locals_to_registers(wa_build_context* context)
{
    for(u32 stackIndex = 0; stackIndex < context->opdStackLen; stackIndex++)
    {
        wa_operand_slot* slot = &context->opdStack[stackIndex];
        if(slot->kind == WA_OPERAND_SLOT_LOCAL)
        {
            wa_move_slot_if_used(context, slot->index);
        }
    }
}

void wa_print_bytecode(u64 len, wa_code* bytecode)
{
    for(u64 codeIndex = 0; codeIndex < len; codeIndex++)
    {
        wa_code* c = &bytecode[codeIndex];
        printf("0x%08llx ", codeIndex);
        printf("%-16s0x%02x ", wa_instr_strings[c->opcode], c->opcode);

        const wa_instr_info* info = &wa_instr_infos[c->opcode];
        for(u64 i = 0; i < info->opdCount; i++)
        {
            codeIndex++;
            if(codeIndex >= len)
            {
                break;
            }
            printf("0x%02llx ", bytecode[codeIndex].operand.immI64);
        }
        printf("\n");
    }
    printf("0x%08llx eof\n", len);
}

typedef struct wa_control
{
    wa_code* returnPC;
    wa_instr_immediate* returnFrame;
} wa_control;

void wa_interpret_func(wa_module* module, wa_func* func)
{
    wa_control controlStack[1024] = {};
    u32 controlStackTop = 0;

    wa_instr_immediate localsBuffer[1024];
    wa_instr_immediate* locals = localsBuffer;
    wa_code* code = func->code;
    wa_code* pc = code;

    while(1)
    {
        wa_instr_op opcode = pc->opcode;
        pc++;

        switch(opcode)
        {
            case WA_INSTR_i32_const:
            {
                locals[pc[1].operand.immI64].immI32 = pc[0].operand.immI32;
                pc += 2;
            }
            break;
            case WA_INSTR_move:
            {
                locals[pc[1].operand.immI64].immI64 = locals[pc[0].operand.immI64].immI64;
                pc += 2;
            }
            break;

            case WA_INSTR_jump:
            {
                pc += pc[0].operand.immI64;
            }
            break;

            case WA_INSTR_jump_if_zero:
            {
                if(locals[pc[1].operand.immI64].immI64 == 0)
                {
                    pc += pc[0].operand.immI64;
                }
                else
                {
                    pc += 2;
                }
            }
            break;

            case WA_INSTR_call:
            {
                controlStack[controlStackTop] = (wa_control){
                    .returnPC = pc + 2,
                    .returnFrame = locals,
                };
                controlStackTop++;
                wa_func* callee = &module->functions[pc[0].operand.immI64];
                locals += pc[1].operand.immI64;
                pc = callee->code;
            }
            break;
            case WA_INSTR_return:
            {
                if(!controlStackTop)
                {
                    goto end;
                }
                controlStackTop--;
                wa_control control = controlStack[controlStackTop];
                locals = control.returnFrame;
                pc = control.returnPC;
            }
            break;
            case WA_INSTR_end:
            {
                goto end;
            }
            break;

            default:
                OC_ASSERT(0, "unreachable, op = %s", wa_instr_strings[opcode]);
        }
    }
end:
    printf("result: %lli\n", locals[0].immI64);
}

/*TODO store all sections in order and print them
void wa_print_sections(wa_module* module)
{
    oc_log_info("section identifier %i at index 0x%08llx of size %u\n", sectionID, contentOffset, sectionLen);
}
*/

void wa_print_types(wa_module* module)
{
    printf("Types:\n");

    for(u32 typeIndex = 0; typeIndex < module->typeCount; typeIndex++)
    {
        wa_func_type* type = &module->types[typeIndex];

        printf("(");
        for(u32 typeIndex = 0; typeIndex < type->paramCount; typeIndex++)
        {
            printf("%s", wa_value_type_string(type->params[typeIndex]));
            if(typeIndex != type->paramCount - 1)
            {
                printf(" ");
            }
        }
        printf(") -> (");

        for(u32 typeIndex = 0; typeIndex < type->returnCount; typeIndex++)
        {
            printf("%s", wa_value_type_string(type->returns[typeIndex]));
            if(typeIndex != type->returnCount - 1)
            {
                printf(" ");
            }
        }
        printf(")\n");
    }
}

void wa_print_imports(wa_module* module)
{
    //NOTE: parse import section
    printf("Imports\n");

    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];

        switch(import->kind)
        {
            case WA_IMPORT_FUNCTION:
            {
                printf("function %.*s %.*s $%u\n",
                       oc_str8_ip(import->moduleName),
                       oc_str8_ip(import->importName),
                       import->index);
            }
            break;
            case WA_IMPORT_TABLE:
            {
                //TODO
            }
            break;
            case WA_IMPORT_MEMORY:
            {
                //TODO
            }
            break;
            case WA_IMPORT_GLOBAL:
            {
                //TODO
            }
            break;
            default:
                OC_ASSERT(0, "unreachable");
                break;
        }
    }
    printf("\n");
}

void wa_print_exports(wa_module* module)
{
    printf("Exports\n");

    for(u32 exportIndex = 0; exportIndex < module->exportCount; exportIndex++)
    {
        wa_export* export = &module->exports[exportIndex];

        switch((u32) export->kind)
        {
            case WA_EXPORT_FUNCTION:
            {
                printf("function %.*s $%u\n",
                       oc_str8_ip(export->name),
                       export->index);
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
                OC_ASSERT(0, "unreachable");
                break;
        }
    }
    printf("\n");
}

void wa_parse_code(oc_arena* arena, wa_input* input, wa_module* module)
{
    wa_input_seek(input, module->toc.code.offset, OC_STR8("code section"));

    u32 functionCount = wa_read_leb128_u32(input, OC_STR8("functions count"));
    if(functionCount != module->functionCount)
    {
        wa_parse_error(module,
                       "Function count mismatch (function section: %i, code section: %i\n",
                       module->functionCount,
                       functionCount);
    }
    functionCount = oc_min(functionCount, module->functionCount);

    for(u32 funcIndex = 0; funcIndex < functionCount; funcIndex++)
    {
        wa_func* func = &module->functions[funcIndex];

        oc_arena_scope scratch = oc_scratch_begin();

        u32 len = wa_read_leb128_u32(input, OC_STR8("function length"));
        u32 startOffset = input->offset;

        // read locals
        u32 localEntryCount = wa_read_leb128_u32(input, OC_STR8("local type entries count"));
        u32* counts = oc_arena_push_array(scratch.arena, u32, localEntryCount);
        wa_value_type* types = oc_arena_push_array(scratch.arena, wa_value_type, localEntryCount);

        func->localCount = func->type->paramCount;
        for(u32 localEntryIndex = 0; localEntryIndex < localEntryCount; localEntryIndex++)
        {
            counts[localEntryIndex] = wa_read_leb128_u32(input, OC_STR8("local entry count"));
            types[localEntryIndex] = wa_read_byte(input, OC_STR8("local entry type index"));
            func->localCount += counts[localEntryIndex];

            //TODO: validate types? or validate later?
        }

        func->locals = oc_arena_push_array(arena, wa_value_type, func->localCount);

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

        // read body
        u64 instrCap = 4;
        u64 instrCount = 0;
        wa_instr* instructions = oc_arena_push_array(scratch.arena, wa_instr, instrCap);

        while(!wa_input_end(input) && (input->offset - startOffset < len))
        {
            if(instrCount >= instrCap)
            {
                wa_instr* tmp = instructions;
                instrCap = instrCap * 1.5;
                instructions = oc_arena_push_array(scratch.arena, wa_instr, instrCap);
                memcpy(instructions, tmp, instrCount * sizeof(wa_instr));
            }
            wa_instr* instr = &instructions[instrCount];

            u8 byte = wa_read_byte(input, OC_STR8("instruction"));

            if(byte == WA_INSTR_PREFIX_EXTENDED)
            {
                u32 code = wa_read_leb128_u32(input, OC_STR8("extended instruction"));
                if(code >= wa_instr_decode_extended_len)
                {
                    wa_parse_error(module,
                                   "Invalid extended instruction %i\n",
                                   code);
                    goto parse_function_end;
                }
                instr->op = wa_instr_decode_extended[code];
            }
            else if(byte == WA_INSTR_PREFIX_VECTOR)
            {
                u32 code = wa_read_leb128_u32(input, OC_STR8("vector instructions"));
                if(code >= wa_instr_decode_vector_len)
                {
                    wa_parse_error(module,
                                   "Invalid vector instruction %i\n",
                                   code);
                    goto parse_function_end;
                }
                instr->op = wa_instr_decode_vector[code];
            }
            else
            {
                if(byte >= wa_instr_decode_basic_len)
                {
                    wa_parse_error(module,
                                   "Invalid basic instruction 0x%02x\n",
                                   byte);
                    goto parse_function_end;
                }
                instr->op = wa_instr_decode_basic[byte];
            }

            const wa_instr_info* info = &wa_instr_infos[instr->op];

            // parse immediate
            for(int immIndex = 0; immIndex < info->immCount; immIndex++)
            {
                switch(info->imm[immIndex])
                {
                    case WA_IMM_ZERO:
                    {
                        wa_read_byte(input, OC_STR8("zero immediate"));
                    }
                    break;
                    case WA_IMM_I32:
                    {
                        instr->imm[immIndex].immI32 = wa_read_leb128_u32(input, OC_STR8("i32 immediate"));
                    }
                    break;
                    case WA_IMM_I64:
                    {
                        instr->imm[immIndex].immI64 = wa_read_leb128_u64(input, OC_STR8("i64 immediate"));
                    }
                    break;
                    case WA_IMM_F32:
                    {
                        instr->imm[immIndex].immF32 = wa_read_f32(input, OC_STR8("f32 immediate"));
                    }
                    break;
                    case WA_IMM_F64:
                    {
                        instr->imm[immIndex].immF64 = wa_read_f64(input, OC_STR8("f64 immediate"));
                    }
                    break;
                    case WA_IMM_VALUE_TYPE:
                    {
                        instr->imm[immIndex].valueType = wa_read_byte(input, OC_STR8("value type immediate"));
                    }
                    break;
                    case WA_IMM_REF_TYPE:
                    {
                        instr->imm[immIndex].valueType = wa_read_byte(input, OC_STR8("ref type immediate"));
                    }
                    break;

                    case WA_IMM_LOCAL_INDEX:
                    case WA_IMM_GLOBAL_INDEX:
                    case WA_IMM_FUNC_INDEX:
                    case WA_IMM_TYPE_INDEX:
                    case WA_IMM_TABLE_INDEX:
                    case WA_IMM_ELEM_INDEX:
                    case WA_IMM_DATA_INDEX:
                    case WA_IMM_LABEL:
                    {
                        instr->imm[immIndex].index = wa_read_leb128_u32(input, OC_STR8("index immediate"));
                    }
                    break;
                    case WA_IMM_MEM_ARG:
                    {
                        instr->imm[immIndex].memArg.align = wa_read_leb128_u32(input, OC_STR8("memory argument immediate"));
                        instr->imm[immIndex].memArg.offset = wa_read_leb128_u32(input, OC_STR8("memory argument immediate"));
                    }
                    break;
                    case WA_IMM_LANE_INDEX:
                    {
                        instr->imm[immIndex].laneIndex = wa_read_byte(input, OC_STR8("lane index immediate"));
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

            if(instr->op == WA_INSTR_block
               || instr->op == WA_INSTR_loop
               || instr->op == WA_INSTR_if)
            {
                // parse block type
                i64 t = wa_read_leb128_i64(input, OC_STR8("block type"));
                if(t >= 0)
                {
                    u64 typeIndex = (u64)t;

                    if(typeIndex >= module->typeCount)
                    {
                        wa_parse_error(module,
                                       "unexpected type index %u (type count: %u)\n",
                                       typeIndex,
                                       module->typeCount);
                        goto parse_function_end;
                    }
                    instr->blockType = &module->types[typeIndex];
                }
                else
                {
                    if(!wa_is_value_type(t & 0x7f))
                    {
                        wa_parse_error(module,
                                       "unrecognized value type 0x%02x\n",
                                       t);
                        goto parse_function_end;
                    }

                    t = (t == -64) ? 0 : -t;
                    instr->blockType = (wa_func_type*)&WA_BLOCK_VALUE_TYPES[t];
                }
            }

            instrCount++;
        }

        // check entry length
        if(input->offset - startOffset != len)
        {
            wa_parse_error(module,
                           "Size of code entry %i does not match declared size (declared %u, got %u)\n",
                           funcIndex,
                           len,
                           input->offset - startOffset);
            goto parse_function_end;
        }

        func->instrCount = instrCount;
        func->instructions = oc_arena_push_array(arena, wa_instr, func->instrCount);
        memcpy(func->instructions, instructions, func->instrCount * sizeof(wa_instr));

    parse_function_end:
        oc_scratch_end(scratch);
        wa_input_seek(input, startOffset + len, OC_STR8("next function"));
    }
}

void wa_push_block_inputs(wa_build_context* context, u64 inputCount, u64 outputCount)
{
    //NOTE copy inputs on top of the stack for else branch
    u64 paramStart = context->opdStackLen - (inputCount + outputCount);
    for(u64 inIndex = 0; inIndex < inputCount; inIndex++)
    {
        wa_operand_slot slotCopy = context->opdStack[paramStart + inIndex];
        slotCopy.count++;
        wa_operand_stack_push(context, slotCopy);
    }
}

void wa_block_begin(wa_build_context* context, wa_instr* instr)
{
    //NOTE: check block type input
    wa_func_type* type = instr->blockType;

    if(type->paramCount > context->opdStackLen)
    {
        wa_parse_error(context->module,
                       "not enough operands on stack (expected %i, got %i)\n",
                       type->paramCount,
                       context->opdStackLen);
        //TODO: push fake inputs to continue validating the block correctly
    }
    else
    {
        for(u64 inIndex = 0; inIndex < type->paramCount; inIndex++)
        {
            wa_value_type opdType = context->opdStack[context->opdStackLen - type->paramCount + inIndex].type;
            wa_value_type expectedType = type->params[inIndex];
            if(opdType != expectedType)
            {
                wa_parse_error(context->module,
                               "operand type mismatch for param %i (expected %s, got %s)\n",
                               inIndex,
                               wa_value_type_string(expectedType),
                               wa_value_type_string(opdType));
            }
        }
    }

    //NOTE allocate block results and push them on the stack
    for(u64 outIndex = 0; outIndex < type->returnCount; outIndex++)
    {
        u32 index = wa_allocate_register(context);

        wa_operand_slot s = {
            .kind = WA_OPERAND_SLOT_REG,
            .index = index,
            .type = type->returns[outIndex],
        };
        wa_operand_stack_push(context, s);

        //TODO immediately put them in the freelist so they can be used in the branches (this might complicate copying results a bit...)
        // wa_free_slot(context, index);
    }

    wa_control_stack_push(context, instr);
    wa_push_block_inputs(context, type->paramCount, type->returnCount);
}

void wa_block_move_results_to_input_slots(wa_build_context* context, wa_block* block)
{
    //NOTE validate block type output
    wa_func_type* type = block->begin->blockType;

    if(type->paramCount > context->opdStackLen)
    {
        wa_parse_error(context->module,
                       "not enough operands on stack (expected %i, got %i)\n",
                       type->paramCount,
                       context->opdStackLen);
        return;
    }
    else
    {
        for(u64 paramIndex = 0; paramIndex < type->paramCount; paramIndex++)
        {
            wa_value_type opdType = context->opdStack[context->opdStackLen - type->paramCount + paramIndex].type;
            wa_value_type expectedType = type->params[paramIndex];
            if(opdType != expectedType)
            {
                wa_parse_error(context->module,
                               "operand type mismatch for param %i (expected %s, got %s)\n",
                               paramIndex,
                               wa_value_type_string(expectedType),
                               wa_value_type_string(opdType));
                return;
            }
        }
    }
    //NOTE move top operands to result slots
    u64 scopeBase = block->scopeBase;

    u64 resultOperandStart = context->opdStackLen - type->paramCount;
    u64 resultSlotStart = scopeBase - type->returnCount - type->paramCount;

    for(u64 outIndex = 0; outIndex < type->paramCount; outIndex++)
    {
        wa_operand_slot* src = &context->opdStack[resultOperandStart + outIndex];
        wa_operand_slot* dst = &context->opdStack[resultSlotStart + outIndex];

        wa_emit(context, (wa_code){ .opcode = WA_INSTR_move });
        wa_emit(context, (wa_code){ .operand.immI64 = src->index });
        wa_emit(context, (wa_code){ .operand.immI64 = dst->index });
    }
}

void wa_block_move_results_to_output_slots(wa_build_context* context, wa_block* block)
{
    //NOTE validate block type output
    wa_func_type* type = block->begin->blockType;

    if(type->returnCount > context->opdStackLen)
    {
        wa_parse_error(context->module,
                       "not enough operands on stack (expected %i, got %i)\n",
                       type->returnCount,
                       context->opdStackLen);
        return;
    }
    else
    {
        for(u64 returnIndex = 0; returnIndex < type->returnCount; returnIndex++)
        {
            wa_value_type opdType = context->opdStack[context->opdStackLen - type->returnCount + returnIndex].type;
            wa_value_type expectedType = type->returns[returnIndex];
            if(opdType != expectedType)
            {
                wa_parse_error(context->module,
                               "operand type mismatch for return %i (expected %s, got %s)\n",
                               returnIndex,
                               wa_value_type_string(expectedType),
                               wa_value_type_string(opdType));
                return;
            }
        }
    }

    //NOTE move top operands to result slots
    u64 scopeBase = block->scopeBase;

    u64 resultOperandStart = context->opdStackLen - type->returnCount;
    u64 resultSlotStart = scopeBase - type->returnCount;

    for(u64 outIndex = 0; outIndex < type->returnCount; outIndex++)
    {
        wa_operand_slot* src = &context->opdStack[resultOperandStart + outIndex];
        wa_operand_slot* dst = &context->opdStack[resultSlotStart + outIndex];

        wa_emit(context, (wa_code){ .opcode = WA_INSTR_move });
        wa_emit(context, (wa_code){ .operand.immI64 = src->index });
        wa_emit(context, (wa_code){ .operand.immI64 = dst->index });
    }
}

void wa_block_end(wa_build_context* context, wa_block* block, wa_instr* instr)
{
    wa_block_move_results_to_output_slots(context, block);

    //TODO: pop result operands, or pop until scope start ?? (shouldn't this already be done?)
    wa_func_type* type = block->begin->blockType;
    wa_operand_stack_pop_slots(context, type->returnCount);

    //NOTE: pop saved params, and push result slots on top of stack
    context->opdStackLen -= type->returnCount;
    for(u64 index = 0; index < type->paramCount; index++)
    {
        wa_operand_stack_pop(context);
    }
    memcpy(&context->opdStack[context->opdStackLen],
           &context->opdStack[context->opdStackLen + type->paramCount],
           type->returnCount * sizeof(wa_operand_slot));
    context->opdStackLen += type->returnCount;

    if(block->begin->op == WA_INSTR_if)
    {
        if(block->begin->elseBranch)
        {
            //NOTE: patch conditional jump to else branch
            context->code[block->beginOffset + 1].operand.immI64 = block->elseOffset - (block->beginOffset + 1);

            //NOTE: patch jump from end of if branch to end of else branch
            context->code[block->elseOffset - 1].operand.immI64 = context->codeLen - (block->elseOffset - 1);
        }
        else
        {
            //NOTE patch conditional jump to end of block
            context->code[block->beginOffset + 1].operand.immI64 = context->codeLen - (block->beginOffset + 1);
        }
    }

    wa_control_stack_pop(context);
}

void wa_emit_jump_target(wa_build_context* context, u32 level)
{
    //NOTE: emit a jump target operand for end of block at a given level up the control stack
    wa_block* block = wa_control_stack_lookup(context, level);
    if(!block)
    {
        wa_parse_error(context->module,
                       "block level %u not found\n",
                       level);
        return;
    }

    wa_jump_target* target = oc_arena_push_type(&context->checkArena, wa_jump_target);
    target->offset = context->codeLen;
    oc_list_push_back(&block->jumpTargets, &target->listElt);

    wa_emit(context, (wa_code){ .operand.immI64 = 0 });
}

void wa_patch_jump_targets(wa_build_context* context, wa_block* block)
{
    oc_list_for(block->jumpTargets, target, wa_jump_target, listElt)
    {
        context->code[target->offset].operand.immI64 = context->codeLen - target->offset;
    }
}

void wa_compile_branch(wa_build_context* context, wa_instr* instr)
{
    u32 label = instr->imm[0].index;
    wa_block* block = wa_control_stack_lookup(context, label);
    if(!block)
    {
        wa_parse_error(context->module,
                       "block level %u not found\n",
                       label);
        return;
    }

    if(block->begin->op == WA_INSTR_block
       || block->begin->op == WA_INSTR_if)
    {
        wa_block_move_results_to_output_slots(context, block);

        //jump to end
        wa_emit(context, (wa_code){ .opcode = WA_INSTR_jump });
        wa_emit_jump_target(context, label);
    }
    else if(block->begin->op == WA_INSTR_loop)
    {
        wa_block_move_results_to_input_slots(context, block);

        //jump to begin
        wa_emit(context, (wa_code){ .opcode = WA_INSTR_jump });
        wa_emit(context, (wa_code){ .operand.immI64 = block->beginOffset - context->codeLen });
    }
    else
    {
        OC_ASSERT(0, "unreachable");
    }
}

void wa_compile_code(oc_arena* arena, wa_module* module)
{
    wa_build_context context = {
        .module = module,
    };
    oc_arena_init(&context.codeArena);
    oc_arena_init(&context.checkArena);

    context.codeCap = 4;
    context.code = oc_arena_push_array(&context.codeArena, wa_code, 4);

    for(u32 funcIndex = 0; funcIndex < module->functionCount; funcIndex++)
    {
        wa_func* func = &module->functions[funcIndex];

        oc_arena_clear(&context.checkArena);
        context.nextRegIndex = func->localCount;

        for(u64 instrIndex = 0; instrIndex < func->instrCount; instrIndex++)
        {
            wa_instr* instr = &func->instructions[instrIndex];
            const wa_instr_info* info = &wa_instr_infos[instr->op];

            //NOTE: validate immediates
            for(int immIndex = 0; immIndex < info->immCount; immIndex++)
            {
                switch(info->imm[immIndex])
                {
                    case WA_IMM_ZERO:
                    case WA_IMM_I32:
                    case WA_IMM_I64:
                    case WA_IMM_F32:
                    case WA_IMM_F64:
                    case WA_IMM_VALUE_TYPE:
                    case WA_IMM_REF_TYPE:
                    case WA_IMM_LABEL:
                        break;

                    case WA_IMM_LOCAL_INDEX:
                    {
                        if(instr->imm[immIndex].index >= func->localCount)
                        {
                            wa_parse_error(module,
                                           "invalid local index %u (localCount: %u)\n",
                                           instr->imm[immIndex].index,
                                           func->localCount);
                        }
                    }
                    break;
                    case WA_IMM_GLOBAL_INDEX:
                    {
                        //TODO
                    }
                    break;
                    case WA_IMM_FUNC_INDEX:
                    {
                        //TODO
                    }
                    break;
                    case WA_IMM_TYPE_INDEX:
                    {
                        //TODO
                    }
                    break;
                    case WA_IMM_TABLE_INDEX:
                    {
                        //TODO
                    }
                    break;
                    case WA_IMM_ELEM_INDEX:
                    {
                        //TODO
                    }
                    break;
                    case WA_IMM_DATA_INDEX:
                    {
                        //TODO
                    }
                    break;
                    case WA_IMM_MEM_ARG:
                    {
                        //TODO
                    }
                    break;
                    case WA_IMM_LANE_INDEX:
                    {
                        //TODO
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
                        OC_ASSERT(0, "unreachable");
                        break;
                }
            }

            if(instr->op == WA_INSTR_nop)
            {
                // do nothing
            }
            else if(instr->op == WA_INSTR_block)
            {
                wa_move_locals_to_registers(&context);
                wa_block_begin(&context, instr);
            }
            else if(instr->op == WA_INSTR_br)
            {
                wa_compile_branch(&context, instr);
            }
            else if(instr->op == WA_INSTR_br_if)
            {
                wa_operand_slot slot = wa_operand_stack_pop(&context);
                if(wa_operand_slot_is_nil(&slot))
                {
                    wa_parse_error(module, "unbalanced stack\n");
                    goto compile_function_end;
                }

                if(slot.type != WA_TYPE_I32)
                {
                    wa_parse_error(module,
                                   "operand type mismatch (expected %s, got %s)\n",
                                   wa_value_type_string(WA_TYPE_I32),
                                   wa_value_type_string(slot.type));
                    goto compile_function_end;
                }

                wa_emit(&context, (wa_code){ .opcode = WA_INSTR_jump_if_zero });
                u64 jumpOffset = context.codeLen;
                wa_emit(&context, (wa_code){ .operand.immI64 = 0 });
                wa_emit(&context, (wa_code){ .operand.immI64 = slot.index });

                wa_compile_branch(&context, instr);

                context.code[jumpOffset].operand.immI64 = context.codeLen - jumpOffset;
            }
            else if(instr->op == WA_INSTR_if)
            {
                wa_operand_slot slot = wa_operand_stack_pop(&context);
                if(wa_operand_slot_is_nil(&slot))
                {
                    wa_parse_error(module, "unbalanced stack\n");
                    goto compile_function_end;
                }

                if(slot.type != WA_TYPE_I32)
                {
                    wa_parse_error(module,
                                   "operand type mismatch (expected %s, got %s)\n",
                                   wa_value_type_string(WA_TYPE_I32),
                                   wa_value_type_string(slot.type));
                    goto compile_function_end;
                }

                wa_move_locals_to_registers(&context);

                wa_block_begin(&context, instr);

                wa_emit(&context, (wa_code){ .opcode = WA_INSTR_jump_if_zero });
                wa_emit(&context, (wa_code){ 0 });
                wa_emit(&context, (wa_code){ .operand.immI64 = slot.index });
            }
            else if(instr->op == WA_INSTR_else)
            {
                wa_block* ifBlock = wa_control_stack_top(&context);
                if(!ifBlock
                   || ifBlock->begin->op != WA_INSTR_if
                   || ifBlock->begin->elseBranch)
                {
                    wa_parse_error(module, "unexpected else block\n");
                    goto compile_function_end;
                }
                wa_func_type* type = ifBlock->begin->blockType;

                wa_block_move_results_to_output_slots(&context, ifBlock);
                wa_operand_stack_pop_slots(&context, type->returnCount);

                wa_push_block_inputs(&context, type->paramCount, type->returnCount);

                wa_emit(&context, (wa_code){ .opcode = WA_INSTR_jump });
                wa_emit(&context, (wa_code){ 0 });

                ifBlock->begin->elseBranch = instr;
                ifBlock->elseOffset = context.codeLen;
            }
            else if(instr->op == WA_INSTR_end)
            {
                wa_block block = wa_control_stack_pop(&context);
                if(wa_block_is_nil(&block))
                {
                    //TODO should we sometimes emit a return here?
                    break;
                }
                wa_block_end(&context, &block, instr);
                wa_patch_jump_targets(&context, &block);
            }
            else if(instr->op == WA_INSTR_call)
            {
                wa_func* callee = &module->functions[instr->imm[0].index];
                u32 paramCount = callee->type->paramCount;

                if(context.opdStackLen < paramCount)
                {
                    wa_parse_error(module,
                                   "Not enough arguments on stack (expected: %u, got: %u)\n",
                                   paramCount,
                                   context.opdStackLen);
                    goto compile_function_end;
                }
                context.opdStackLen -= paramCount;

                u32 maxUsedSlot = 0;
                for(u32 stackIndex = 0; stackIndex < context.opdStackLen; stackIndex++)
                {
                    wa_operand_slot* slot = &context.opdStack[stackIndex];
                    maxUsedSlot = oc_max(slot->index, maxUsedSlot);
                }

                //TODO: first check if we args are already in order at the end of the frame?
                for(u32 argIndex = 0; argIndex < paramCount; argIndex++)
                {
                    wa_operand_slot* slot = &context.opdStack[context.opdStackLen + argIndex];
                    wa_value_type paramType = callee->type->params[argIndex];

                    if(slot->type != paramType)
                    {
                        wa_parse_error(module,
                                       "Type mismatch for argument %u (expected %s, got %s)\n",
                                       argIndex,
                                       wa_value_type_string(paramType),
                                       wa_value_type_string(slot->type));
                        goto compile_function_end;
                    }
                    wa_emit(&context, (wa_code){ .opcode = WA_INSTR_move });
                    wa_emit(&context, (wa_code){ .operand.immI32 = slot->index });
                    wa_emit(&context, (wa_code){ .operand.immI32 = maxUsedSlot + 1 + argIndex });
                }

                wa_emit(&context, (wa_code){ .opcode = WA_INSTR_call });
                wa_emit(&context, (wa_code){ .operand.immI64 = instr->imm[0].index });
                wa_emit(&context, (wa_code){ .operand.immI64 = maxUsedSlot + 1 });

                for(u32 retIndex = 0; retIndex < callee->type->returnCount; retIndex++)
                {
                    wa_operand_stack_push(&context,
                                          (wa_operand_slot){
                                              .kind = WA_OPERAND_SLOT_REG,
                                              .type = callee->type->returns[retIndex],
                                              .index = maxUsedSlot + 1 + retIndex,
                                          });
                }
            }
            else if(instr->op == WA_INSTR_return)
            {
                // Check that there's the correct number / types of values on the stack
                if(context.opdStackLen < func->type->returnCount)
                {
                    wa_parse_error(module,
                                   "Not enough return values (expected: %i, stack depth: %i)\n",
                                   func->type->returnCount,
                                   context.opdStackLen);
                    goto compile_function_end;
                }

                u32 retSlotStart = context.opdStackLen - func->type->returnCount;
                for(u32 retIndex = 0; retIndex < func->type->returnCount; retIndex++)
                {
                    wa_operand_slot* slot = &context.opdStack[retSlotStart + retIndex];
                    wa_value_type expectedType = func->type->returns[retIndex];
                    wa_value_type returnType = slot->type;

                    if(returnType != expectedType)
                    {
                        wa_parse_error(module,
                                       "Argument type doesn't match function signature (expected %s, got %s)\n",
                                       wa_value_type_string(expectedType),
                                       wa_value_type_string(returnType));
                        goto compile_function_end;
                    }

                    //NOTE store value to return slot
                    if(slot->index != retIndex)
                    {
                        wa_move_slot_if_used(&context, retIndex);

                        wa_emit(&context, (wa_code){ .opcode = WA_INSTR_move });
                        wa_emit(&context, (wa_code){ .operand.immI64 = slot->index });
                        wa_emit(&context, (wa_code){ .operand.immI64 = retIndex });
                    }
                }
                wa_emit(&context, (wa_code){ .opcode = instr->op });
            }
            else if(instr->op == WA_INSTR_local_get)
            {
                u32 localIndex = instr->imm[0].index;

                wa_operand_stack_push(
                    &context,
                    (wa_operand_slot){
                        .kind = WA_OPERAND_SLOT_LOCAL,
                        .type = func->locals[localIndex],
                        .index = localIndex,
                        .originInstr = instrIndex,
                        .originOpd = context.codeLen,
                    });
            }
            else if(instr->op == WA_INSTR_local_set)
            {
                u32 localIndex = instr->imm[0].index;

                wa_operand_slot slot = wa_operand_stack_pop(&context);
                if(wa_operand_slot_is_nil(&slot))
                {
                    wa_parse_error(module, "unbalanced stack\n");
                    goto compile_function_end;
                }

                if(slot.type != func->locals[localIndex])
                {
                    wa_parse_error(module,
                                   "type mismatch for local.set instruction (expected %s, got %s)\n",
                                   wa_value_type_string(func->locals[localIndex]),
                                   wa_value_type_string(slot.type));
                    goto compile_function_end;
                }

                // check if the local was used in the stack and if so save it to a slot
                wa_move_slot_if_used(&context, localIndex);

                //TODO: check if the local was written to since the value was pushed, and if not, change
                //      the output operand of the value's origin instruction rather than issuing a move
                bool shortcutSet = false;
                /*NOTE: this can't be used after a branch, since the branch might use the original slot index
                //      so for now, disable this optimization
                // later we can add a "touched" bit and set it for operands used in a branch?
                if(slot.kind == WA_OPERAND_SLOT_REG && slot.originOpd)
                {
                    shortcutSet = true;
                    for(u32 instrIt = slot.originInstr; instrIt < instrIndex; instrIt++)
                    {
                        if(func->instructions[instrIt].op == WA_INSTR_local_set && func->instructions[instrIt].imm[0].immI32 == localIndex)
                        {
                            shortcutSet = false;
                            break;
                        }
                    }
                }
                */
                if(shortcutSet)
                {
                    OC_DEBUG_ASSERT(context.code[slot.originOpd].immU64 == slot.index);
                    context.code[slot.originOpd].operand.immI64 = localIndex;
                }
                else
                {
                    wa_emit(&context, (wa_code){ .opcode = WA_INSTR_move });
                    wa_emit(&context, (wa_code){ .operand.immI32 = slot.index });
                    wa_emit(&context, (wa_code){ .operand.immI32 = localIndex });
                }
            }
            else
            {
                //TODO validate instruction
                //NOTE emit opcode
                wa_emit(&context, (wa_code){ .opcode = instr->op });

                //emit immediates
                for(int immIndex = 0; immIndex < info->immCount; immIndex++)
                {
                    wa_emit(&context, (wa_code){ .operand = instr->imm[immIndex] });
                }

                // validate stack, allocate slots and emit operands
                //TODO: this only works with no locals
                for(int opdIndex = 0; opdIndex < info->inCount; opdIndex++)
                {
                    wa_operand_slot slot = wa_operand_stack_pop(&context);
                    if(wa_operand_slot_is_nil(&slot))
                    {
                        wa_parse_error(module, "unbalanced stack\n");
                        goto compile_function_end;
                    }

                    if(slot.type != info->in[opdIndex])
                    {
                        wa_parse_error(module, "operand type mismatch\n");
                        goto compile_function_end;
                    }

                    wa_code opd = { .operand = slot.index };

                    wa_emit(&context, opd);
                }

                for(int opdIndex = 0; opdIndex < info->outCount; opdIndex++)
                {
                    u32 index = wa_allocate_register(&context);

                    wa_operand_slot s = {
                        .kind = WA_OPERAND_SLOT_REG,
                        .type = info->out[opdIndex],
                        .index = index,
                        .originInstr = instrIndex,
                        .originOpd = context.codeLen,
                    };
                    wa_operand_stack_push(&context, s);

                    wa_emit(&context, (wa_code){ .operand = s.index });
                }
            }
        }
        func->codeLen = context.codeLen;
        func->code = oc_arena_push_array(arena, wa_code, context.codeLen);
        memcpy(func->code, context.code, context.codeLen * sizeof(wa_code));

    compile_function_end:
        continue;
    }
    oc_arena_cleanup(&context.codeArena);
    oc_arena_cleanup(&context.checkArena);
}

void wa_print_code(wa_module* module)
{
    printf("Code:\n\n");
    for(u64 funcIndex = 0; funcIndex < module->functionCount; funcIndex++)
    {
        wa_func* func = &module->functions[funcIndex];

        for(u64 exportIndex = 0; exportIndex < module->exportCount; exportIndex++)
        {
            wa_export* export = &module->exports[exportIndex];
            if(export->kind == WA_EXPORT_FUNCTION && export->index == funcIndex)
            {
                printf("%.*s:\n", oc_str8_ip(export->name));
                break;
            }
        }
        wa_print_bytecode(func->codeLen, func->code);
    }
    printf("\n");
}

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        printf("missing argument: wasm module.\n");
        exit(-1);
    }
    const char* modulePath = argv[1];

    oc_arena arena = { 0 };
    oc_arena_init(&arena);

    wa_module module = {
        .arena = &arena,
    };

    wa_input input = {
        .module = &module,
    };

    {
        oc_file file = oc_file_open(OC_STR8(modulePath), OC_FILE_ACCESS_READ, OC_FILE_OPEN_NONE);

        input.len = oc_file_size(file);
        input.contents = oc_arena_push(&arena, input.len);

        oc_file_read(file, input.len, input.contents);
        oc_file_close(file);
    }

    wa_parse_sections(&input, &module);

    oc_arena_scope scratch = oc_scratch_begin();

    wa_parse_types(&arena, &input, &module);
    wa_print_types(&module);

    wa_parse_imports(&arena, &input, &module);
    wa_print_imports(&module);

    wa_parse_functions(&arena, &input, &module);

    wa_parse_exports(&arena, &input, &module);
    wa_print_exports(&module);

    wa_parse_code(&arena, &input, &module);

    if(!oc_list_empty(module.errors))
    {
        wa_module_print_errors(&module);
        exit(-1);
    }

    wa_compile_code(&arena, &module);
    if(!oc_list_empty(module.errors))
    {
        wa_module_print_errors(&module);
        exit(-1);
    }

    wa_print_code(&module);

    printf("Run:\n");
    wa_func* start = 0;
    for(u32 exportIndex = 0; exportIndex < module.exportCount; exportIndex++)
    {
        wa_export* export = &module.exports[exportIndex];
        if(export->kind == WA_EXPORT_FUNCTION && !oc_str8_cmp(export->name, OC_STR8("start")))
        {
            start = &module.functions[export->index];
            break;
        }
    }

    if(!start)
    {
        oc_log_error("Couldn't find function start.\n");
        exit(-1);
    }

    wa_interpret_func(&module, start);

    oc_scratch_end(scratch);

    return (0);
}
