#include <stdio.h>

#define OC_NO_APP_LAYER 1
#include "orca.h"

#include "wa_types.h"
#include "wasm_tables.c"

char* wa_input_head(wa_input* file)
{
    return (file->contents + file->offset);
}

bool wa_input_end(wa_input* file)
{
    return (file->offset >= file->len);
}

void wa_input_seek(wa_input* file, u64 offset)
{
    if(offset > file->len)
    {
        file->offset = file->len;
        file->status = WA_INPUT_EOF;
    }
    else
    {
        file->offset = offset;
    }
}

int wa_read_status(wa_input* file)
{
    return (file->status);
}

u8 wa_read_byte(wa_input* file)
{
    if(file->status)
    {
        return (0);
    }
    if(file->offset + sizeof(u8) > file->len)
    {
        file->status = WA_INPUT_EOF;
        return (0);
    }

    u8 r = *(u8*)&file->contents[file->offset];
    file->offset += sizeof(u8);
    return (r);
}

u32 wa_read_raw_u32(wa_input* file)
{
    if(file->status)
    {
        return (0);
    }
    if(file->offset + sizeof(u32) > file->len)
    {
        file->status = WA_INPUT_EOF;
        return (0);
    }

    u32 r = *(u32*)&file->contents[file->offset];
    file->offset += sizeof(u32);
    return (r);
}

u64 wa_read_leb128_u64(wa_input* file)
{
    if(file->status)
    {
        return (0);
    }

    char byte = 0;
    u64 shift = 0;
    u64 res = 0;

    do
    {
        if(file->offset + sizeof(char) > file->len)
        {
            file->status = WA_INPUT_EOF;
            return (0);
        }
        byte = file->contents[file->offset];
        file->offset++;

        res |= ((u64)byte & 0x7f) << shift;
        shift += 7;
    }
    while(byte & 0x80);

    return (res);
}

i64 wa_read_leb128_i64(wa_input* file)
{
    if(file->status)
    {
        return (0);
    }

    char byte = 0;
    u64 shift = 0;
    i64 res = 0;

    do
    {
        if(file->offset + sizeof(char) > file->len)
        {
            file->status = WA_INPUT_EOF;
            return (0);
        }
        byte = file->contents[file->offset];
        file->offset++;

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

u32 wa_read_leb128_u32(wa_input* file)
{
    if(file->status)
    {
        return (0);
    }

    u64 r = wa_read_leb128_u64(file);
    if(r >= INT32_MAX)
    {
        r = 0;
        file->status = WA_INPUT_OVERFLOW;
    }
    return (u32)r;
}

f32 wa_read_f32(wa_input* file)
{
    if(file->status)
    {
        return (0);
    }
    if(file->offset + sizeof(f32) > file->len)
    {
        file->status = WA_INPUT_EOF;
        return (0);
    }
    f32 f = *(f32*)&file->contents[file->offset];
    file->offset += sizeof(f32);
    return f;
}

f64 wa_read_f64(wa_input* file)
{
    if(file->status)
    {
        return (0);
    }
    if(file->offset + sizeof(f64) > file->len)
    {
        file->status = WA_INPUT_EOF;
        return (0);
    }
    f64 f = *(f64*)&file->contents[file->offset];
    file->offset += sizeof(f64);
    return f;
}

oc_str8 wa_read_name(wa_input* file, oc_arena* arena)
{
    u32 len = wa_read_leb128_u32(file);
    if(file->status)
    {
        return (oc_str8){ 0 };
    }
    if(file->offset + len > file->len)
    {
        file->status = WA_INPUT_EOF;
        return (oc_str8){ 0 };
    }
    oc_str8 res = {
        .ptr = &file->contents[file->offset],
        .len = len,
    };
    file->offset += len;
    return (res);
}

enum
{
    WA_TYPE_STACK_MAX_LEN = 512,
    WA_CONTROL_STACK_MAX_LEN = 512,
    WA_MAX_REG = 4096,
};

typedef enum wa_operand_slot_kind
{
    WA_OPERAND_SLOT_LOCAL = 0,
    WA_OPERAND_SLOT_REG,
} wa_operand_slot_kind;

typedef struct wa_operand_slot
{
    wa_operand_slot_kind kind;
    wa_value_type type;
    u32 index;
    u64 originInstr;
    u64 originOpd;
} wa_operand_slot;

typedef enum wa_block_type_kind
{
    WA_BLOCK_TYPE_VOID,
    WA_BLOCK_TYPE_VALUE_TYPE,
    WA_BLOCK_TYPE_USER,
} wa_block_type_kind;

typedef struct wa_block wa_block;

struct wa_block
{
    wa_block_type_kind typeKind;

    union
    {
        wa_value_type valueType;
        u32 index;
    } type;

    wa_module_instruction* start;
    wa_module_instruction* elseBranch;
};

typedef struct wa_build_context
{
    oc_arena internalArena;

    oc_arena_scope arenaScope;
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

void wa_operand_stack_push(wa_build_context* context, wa_operand_slot s)
{
    if(context->opdStack == 0 || context->opdStackLen >= context->opdStackCap)
    {
        context->opdStackCap = (context->opdStackCap + 8) * 2;
        wa_operand_slot* tmp = context->opdStack;
        context->opdStack = oc_arena_push_array(&context->internalArena, wa_operand_slot, context->opdStackCap);
        OC_ASSERT(context->opdStack, "out of memory");

        if(tmp)
        {
            memcpy(context->opdStack, tmp, context->opdStackLen * sizeof(wa_operand_slot));
        }
    }
    context->opdStack[context->opdStackLen] = s;
    context->opdStackLen++;
}

wa_operand_slot* wa_operand_stack_pop(wa_build_context* context)
{
    wa_operand_slot* slot = 0;
    if(context->opdStackLen)
    {
        context->opdStackLen--;
        slot = &context->opdStack[context->opdStackLen];
    }
    return (slot);
}

void wa_control_stack_push(wa_build_context* context, wa_block b)
{
    if(context->controlStack == 0 || context->controlStackLen >= context->controlStackCap)
    {
        context->controlStackCap = (context->controlStackCap + 8) * 2;
        wa_block* tmp = context->controlStack;
        context->controlStack = oc_arena_push_array(&context->internalArena, wa_block, context->controlStackCap);
        OC_ASSERT(context->controlStack, "out of memory");

        if(tmp)
        {
            memcpy(context->controlStack, tmp, context->controlStackLen * sizeof(wa_block));
        }
    }
    context->controlStack[context->controlStackLen] = b;
    context->controlStackLen++;
}

wa_block* wa_control_stack_pop(wa_build_context* context)
{
    wa_block* block = 0;
    if(context->controlStackLen)
    {
        context->controlStackLen--;
        block = &context->controlStack[context->controlStackLen];
    }
    return (block);
}

wa_block* wa_control_stack_lookup(wa_build_context* context, u32 label)
{
    wa_block* block = 0;
    if(label <= context->controlStackLen)
    {
        block = &context->controlStack[context->controlStackLen - label];
    }
    return (block);
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
        oc_arena_scope scratch = oc_scratch_begin_next(context->arenaScope.arena);
        wa_code* tmp = oc_arena_push_array(scratch.arena, wa_code, context->codeLen);
        memcpy(tmp, context->code, context->codeLen * sizeof(wa_code));

        oc_arena* arena = context->arenaScope.arena;
        oc_arena_scope_end(context->arenaScope);

        context->codeCap = (context->codeCap + 1) * 1.5;

        context->arenaScope = oc_arena_scope_begin(arena);
        context->code = oc_arena_push_array(arena, wa_code, context->codeCap);
        memcpy(context->code, tmp, context->codeLen * sizeof(wa_code));
    }
    context->code[context->codeLen] = code;
    context->codeLen++;
}

u32 wa_allocate_register(wa_build_context* context)
{
    u32 index = 0;
    if(context->freeRegLen)
    {
        index = context->freeRegs[context->freeRegLen];
        context->freeRegLen--;
    }
    else
    {
        //TODO: prevent overflow
        index = context->nextRegIndex;
        context->nextRegIndex++;
    }
    return (index);
}

void wa_save_slot_if_used(wa_build_context* context, u32 slotIndex)
{
    u32 newReg = UINT32_MAX;
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
        }
    }
    if(newReg != UINT32_MAX)
    {
        wa_emit(context, (wa_code){ .opcode = WA_INSTR_move });
        wa_emit(context, (wa_code){ .operand.immI32 = slotIndex });
        wa_emit(context, (wa_code){ .operand.immI32 = newReg });
    }
}

void wa_print_bytecode(u64 len, wa_code* bytecode)
{
    for(u64 codeIndex = 0; codeIndex < len; codeIndex++)
    {
        wa_code* c = &bytecode[codeIndex];
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

int main(int argc, char** argv)
{
    oc_arena arena = { 0 };
    oc_arena_init(&arena);

    wa_input input = {};
    {
        oc_file file = oc_file_open(OC_STR8("test.wasm"), OC_FILE_ACCESS_READ, OC_FILE_OPEN_NONE);

        input.len = oc_file_size(file);
        input.contents = oc_arena_push(&arena, input.len);

        oc_file_read(file, input.len, input.contents);
        oc_file_close(file);
    }

    wa_module module = {
        .len = input.len,
        .bytes = input.contents,
    };

    u32 magic = wa_read_raw_u32(&input);
    u32 version = wa_read_raw_u32(&input);

    if(wa_read_status(&input))
    {
        oc_log_error("Couldn't read wasm magic number and version.\n");
        exit(-1);
    }
    //TODO check magic / version

    while(!wa_input_end(&input))
    {
        u8 sectionID = wa_read_byte(&input);
        u32 sectionLen = wa_read_leb128_u32(&input);

        u64 contentOffset = input.offset;
        //TODO: check read errors

        if(wa_read_status(&input))
        {
            oc_log_error("Input error.\n");
            exit(-1);
        }

        oc_log_info("section identifier %i at index 0x%08llx of size %u\n", sectionID, contentOffset, sectionLen);

        switch(sectionID)
        {
            case 0:
                break;

                //TODO: check if section was already defined...

            case 1:
            {
                module.toc.types.offset = input.offset;
                module.toc.types.len = sectionLen;
            }
            break;

            case 2:
            {
                module.toc.imports.offset = input.offset;
                module.toc.imports.len = sectionLen;
            }
            break;

            case 3:
            {
                module.toc.functions.offset = input.offset;
                module.toc.functions.len = sectionLen;
            }
            break;

            case 4:
            {
                module.toc.tables.offset = input.offset;
                module.toc.tables.len = sectionLen;
            }
            break;

            case 5:
            {
                module.toc.memory.offset = input.offset;
                module.toc.memory.len = sectionLen;
            }
            break;

            case 6:
            {
                module.toc.globals.offset = input.offset;
                module.toc.globals.len = sectionLen;
            }
            break;

            case 7:
            {
                module.toc.exports.offset = input.offset;
                module.toc.exports.len = sectionLen;
            }
            break;

            case 8:
            {
                module.toc.start.offset = input.offset;
                module.toc.start.len = sectionLen;
            }
            break;

            case 9:
            {
                module.toc.elements.offset = input.offset;
                module.toc.elements.len = sectionLen;
            }
            break;

            case 10:
            {
                module.toc.code.offset = input.offset;
                module.toc.code.len = sectionLen;
            }
            break;

            case 11:
            {
                module.toc.data.offset = input.offset;
                module.toc.data.len = sectionLen;
            }
            break;

            case 12:
            {
                module.toc.dataCount.offset = input.offset;
                module.toc.dataCount.len = sectionLen;
            }
            break;

            default:
            {
                oc_log_error("Unknown section identifier %i.\n", sectionID);
                exit(-1);
            }
        }
        wa_input_seek(&input, contentOffset + sectionLen);
    }

    oc_arena_scope scratch = oc_scratch_begin();

    //NOTE: parse types section
    printf("Types:\n");
    wa_input_seek(&input, module.toc.types.offset);
    {
        u64 startOffset = input.offset;

        module.typeCount = wa_read_leb128_u32(&input);
        module.types = oc_arena_push_array(&arena, wa_func_type, module.typeCount);

        for(u32 typeIndex = 0; typeIndex < module.typeCount; typeIndex++)
        {
            wa_func_type* type = &module.types[typeIndex];

            u8 b = wa_read_byte(&input);

            if(b != 0x60)
            {
                oc_log_error("Unexpected prefix 0x%02x for function type.\n", b);
                exit(-1);
            }

            type->paramCount = wa_read_leb128_u32(&input);
            type->params = oc_arena_push_array(&arena, wa_value_type, type->paramCount);
            printf("(");
            for(u32 typeIndex = 0; typeIndex < type->paramCount; typeIndex++)
            {
                type->params[typeIndex] = wa_read_leb128_u32(&input);

                printf("%s", wa_value_type_string(type->params[typeIndex]));
                if(typeIndex != type->paramCount - 1)
                {
                    printf(" ");
                }
            }
            printf(") -> (");

            type->returnCount = wa_read_leb128_u32(&input);
            type->returns = oc_arena_push_array(&arena, wa_value_type, type->returnCount);
            for(u32 typeIndex = 0; typeIndex < type->returnCount; typeIndex++)
            {
                type->returns[typeIndex] = wa_read_leb128_u32(&input);
                printf("%s", wa_value_type_string(type->returns[typeIndex]));
                if(typeIndex != type->returnCount - 1)
                {
                    printf(" ");
                }
            }
            printf(")\n");
        }

        //NOTE: check section size
        if(input.offset - startOffset != module.toc.types.len)
        {
            oc_log_error("Size of type section does not match declared size (declared = %llu, actual = %llu)\n",
                         module.toc.types.len,
                         input.offset - startOffset);
            exit(-1);
        }
    }

    //NOTE: parse import section
    printf("Imports\n");
    wa_input_seek(&input, module.toc.imports.offset);
    {
        module.importCount = wa_read_leb128_u32(&input);
        module.imports = oc_arena_push_array(&arena, wa_import, module.importCount);

        for(u32 importIndex = 0; importIndex < module.importCount; importIndex++)
        {
            wa_import* import = &module.imports[importIndex];

            import->moduleName = wa_read_name(&input, &arena);
            import->importName = wa_read_name(&input, &arena);
            import->kind = wa_read_byte(&input);

            switch((u32)import->kind)
            {
                case WA_IMPORT_FUNCTION:
                {
                    import->index = wa_read_leb128_u32(&input);
                    if(import->index >= module.functionCount)
                    {
                        oc_log_error("Invalid type index in function import (function count: %u, got index %u)\n",
                                     module.functionCount,
                                     import->index);
                        exit(-1);
                    }
                    printf("function %.*s %.*s $%u\n",
                           oc_str8_ip(import->moduleName),
                           oc_str8_ip(import->importName),
                           import->index);
                }
                break;
                case WA_IMPORT_TABLE:
                {
                    import->type = wa_read_byte(&input);
                    if(import->type != WA_TYPE_FUNC_REF && import->type != WA_TYPE_EXTERN_REF)
                    {
                        oc_log_error("Invalid type 0x%02x in table import \n",
                                     import->type);
                        exit(-1);
                    }
                    u8 b = wa_read_byte(&input);
                    if(b != WA_LIMIT_MIN && b != WA_LIMIT_MIN_MAX)
                    {
                        oc_log_error("Invalid limit kind 0x%02x in table import\n",
                                     b);
                        exit(-1);
                    }
                    import->limits.kind = b;

                    import->limits.min = wa_read_leb128_u32(&input);
                    if(import->limits.kind == WA_LIMIT_MIN_MAX)
                    {
                        import->limits.max = wa_read_leb128_u32(&input);
                    }
                }
                break;
                case WA_IMPORT_MEMORY:
                {
                    u8 b = wa_read_byte(&input);
                    if(b != WA_LIMIT_MIN && b != WA_LIMIT_MIN_MAX)
                    {
                        oc_log_error("Invalid limit kind 0x%02x in table import\n",
                                     b);
                        exit(-1);
                    }
                    import->limits.kind = b;

                    import->limits.min = wa_read_leb128_u32(&input);
                    if(import->limits.kind == WA_LIMIT_MIN_MAX)
                    {
                        import->limits.max = wa_read_leb128_u32(&input);
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
                    oc_log_error("Unknown import kind 0x%02x\n", (u8)import->kind);
                    exit(-1);
                }
            }
        }
    }
    printf("\n");

    //NOTE: parse function section
    wa_input_seek(&input, module.toc.functions.offset);
    {
        module.functionCount = wa_read_leb128_u32(&input);
        module.functions = oc_arena_push_array(&arena, wa_func, module.functionCount);

        for(u32 funcIndex = 0; funcIndex < module.functionCount; funcIndex++)
        {
            wa_func* func = &module.functions[funcIndex];
            u32 typeIndex = wa_read_leb128_u32(&input);

            if(typeIndex >= module.typeCount)
            {
                oc_log_error("Invalid type index %i in function section\n", typeIndex);
                exit(-1);
            }
            func->type = &module.types[typeIndex];
        }
    }

    //NOTE: parse export section
    printf("Exports\n");
    wa_input_seek(&input, module.toc.exports.offset);
    {
        module.exportCount = wa_read_leb128_u32(&input);
        module.exports = oc_arena_push_array(&arena, wa_export, module.exportCount);

        for(u32 exportIndex = 0; exportIndex < module.exportCount; exportIndex++)
        {
            wa_export* export = &module.exports[exportIndex];

            export->name = wa_read_name(&input, &arena);
            export->kind = wa_read_byte(&input);
            export->index = wa_read_leb128_u32(&input);

            switch((u32) export->kind)
            {
                case WA_EXPORT_FUNCTION:
                {
                    if(export->index >= module.functionCount)
                    {
                        oc_log_error("Invalid type index in function export (function count: %u, got index %u)\n",
                                     module.functionCount,
                                     export->index);
                        exit(-1);
                    }
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
                {
                    oc_log_error("Unknown export kind 0x%02x\n", (u8) export->kind);
                    exit(-1);
                }
            }
        }
    }
    printf("\n");

    //NOTE: parse code section
    printf("\nCode:\n");
    wa_input_seek(&input, module.toc.code.offset);

    u32 functionCount = wa_read_leb128_u32(&input);
    if(functionCount != module.functionCount)
    {
        oc_log_error("Function count mismatch (function section: %i, code section: %i\n",
                     module.functionCount,
                     functionCount);
        exit(-1);
    }

    for(u32 funcIndex = 0; funcIndex < module.functionCount; funcIndex++)
    {
        wa_func* func = &module.functions[funcIndex];

        u32 len = wa_read_leb128_u32(&input);
        u32 startOffset = input.offset;

        // read locals
        u32 localEntryCount = wa_read_leb128_u32(&input);
        u32* counts = oc_arena_push_array(scratch.arena, u32, localEntryCount);
        wa_value_type* types = oc_arena_push_array(scratch.arena, wa_value_type, localEntryCount);

        func->localCount = func->type->paramCount;
        for(u32 localEntryIndex = 0; localEntryIndex < localEntryCount; localEntryIndex++)
        {
            counts[localEntryIndex] = wa_read_leb128_u32(&input);
            types[localEntryIndex] = wa_read_byte(&input);
            func->localCount += counts[localEntryIndex];

            //TODO: validate types
        }

        func->locals = oc_arena_push_array(&arena, wa_value_type, func->localCount);

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
        wa_module_instruction* instructions = oc_arena_push_array(scratch.arena, wa_module_instruction, instrCap);

        wa_build_context context = {
            .arenaScope = oc_arena_scope_begin(&arena),
            .module = &module,
            .codeCap = 4,
            .code = oc_arena_push_array(scratch.arena, wa_code, 4),
            .nextRegIndex = func->localCount,
        };

        oc_arena_init(&context.internalArena);

        while(!wa_input_end(&input))
        {
            if(instrCount >= instrCap)
            {
                wa_module_instruction* tmp = instructions;
                instrCap = instrCap * 1.5;
                instructions = oc_arena_push_array(scratch.arena, wa_module_instruction, instrCap);
                memcpy(instructions, tmp, instrCount * sizeof(wa_module_instruction));
            }
            wa_module_instruction* instr = &instructions[instrCount];

            u8 byte = wa_read_byte(&input);

            if(byte == WA_INSTR_PREFIX_EXTENDED)
            {
                u32 code = wa_read_leb128_u32(&input);
                if(code >= wa_instr_decode_extended_len)
                {
                    oc_log_error("Invalid extended instruction %i\n", code);
                    exit(-1);
                }
                instr->op = wa_instr_decode_extended[code];
            }
            else if(byte == WA_INSTR_PREFIX_VECTOR)
            {
                u32 code = wa_read_leb128_u32(&input);
                if(code >= wa_instr_decode_vector_len)
                {
                    oc_log_error("Invalid vector instruction %i\n", code);
                    exit(-1);
                }
                instr->op = wa_instr_decode_vector[code];
            }
            else
            {
                if(byte >= wa_instr_decode_basic_len)
                {
                    oc_log_error("Invalid basic instruction 0x%02x\n", byte);
                    exit(-1);
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
                        wa_read_byte(&input);
                    }
                    break;
                    case WA_IMM_I32:
                    {
                        instr->imm[immIndex].immI32 = wa_read_leb128_u32(&input);
                    }
                    break;
                    case WA_IMM_I64:
                    {
                        instr->imm[immIndex].immI64 = wa_read_leb128_u64(&input);
                    }
                    break;
                    case WA_IMM_F32:
                    {
                        instr->imm[immIndex].immF32 = wa_read_f32(&input);
                    }
                    break;
                    case WA_IMM_F64:
                    {
                        instr->imm[immIndex].immF64 = wa_read_f64(&input);
                    }
                    break;
                    case WA_IMM_VALUE_TYPE:
                    {
                        instr->imm[immIndex].valueType = wa_read_byte(&input);
                    }
                    break;
                    case WA_IMM_REF_TYPE:
                    {
                        instr->imm[immIndex].valueType = wa_read_byte(&input);
                    }
                    break;
                    //TODO check indices
                    case WA_IMM_LOCAL_INDEX:
                    {
                        u32 localIndex = wa_read_leb128_u32(&input);

                        if(localIndex >= func->localCount)
                        {
                            oc_log_error("invalid local index %u (localCount: %u)\n",
                                         localIndex,
                                         func->localCount);
                            exit(-1);
                        }

                        instr->imm[immIndex].index = localIndex;
                    }
                    break;
                    case WA_IMM_GLOBAL_INDEX:
                    {
                        instr->imm[immIndex].index = wa_read_leb128_u32(&input);
                    }
                    break;
                    case WA_IMM_FUNC_INDEX:
                    {
                        instr->imm[immIndex].index = wa_read_leb128_u32(&input);
                    }
                    break;
                    case WA_IMM_TYPE_INDEX:
                    {
                        instr->imm[immIndex].index = wa_read_leb128_u32(&input);
                    }
                    break;
                    case WA_IMM_TABLE_INDEX:
                    {
                        instr->imm[immIndex].index = wa_read_leb128_u32(&input);
                    }
                    break;
                    case WA_IMM_ELEM_INDEX:
                    {
                        instr->imm[immIndex].index = wa_read_leb128_u32(&input);
                    }
                    break;
                    case WA_IMM_DATA_INDEX:
                    {
                        instr->imm[immIndex].index = wa_read_leb128_u32(&input);
                    }
                    break;
                    case WA_IMM_MEM_ARG:
                    {
                        instr->imm[immIndex].memArg.align = wa_read_leb128_u32(&input);
                        instr->imm[immIndex].memArg.offset = wa_read_leb128_u32(&input);
                    }
                    break;
                    case WA_IMM_LANE_INDEX:
                    {
                        instr->imm[immIndex].laneIndex = wa_read_byte(&input);
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

            /*
            if(instr->op == WA_INSTR_if)
            {
                // parse block type
                wa_block block = {
                    .start = instr,
                };
                i64 t = wa_read_leb128_i64(&input);
                if(t >= 0)
                {
                    block.typeKind = WA_BLOCK_TYPE_USER;
                    block.type.index = (u64)t;
                    if(block.type.index >= module.typeCount)
                    {
                        oc_log_error("unexpected type index %u (type count: %u)\n",
                                     block.type.index,
                                     module.typeCount);
                        exit(-1);
                    }
                }
                else
                {
                    t = -t;
                    if(t == 0x40)
                    {
                        block.typeKind = WA_BLOCK_TYPE_VOID;
                    }
                    else
                    {
                        if(!wa_is_value_type(t))
                        {
                            oc_log_error("unrecognized value type 0x%02x\n", t);
                            exit(-1);
                        }
                        block.typeKind = WA_BLOCK_TYPE_VALUE_TYPE;
                        block.type.valueType = t;
                    }
                }

                // push new block
                if(context.controlStackLen >= WA_CONTROL_STACK_MAX_LEN)
                {
                    oc_log_error("control stack overflow\n");
                    exit(-1);
                }
                context.controlStack[context.controlStackLen] = block;
                context.controlStackLen++;

                // get operand
                wa_operand_slot* slot = wa_operand_stack_pop(&context);
                if(!slot)
                {
                    oc_log_error("unbalanced stack\n");
                    exit(-1);
                }

                if(slot->type != WA_TYPE_I64)
                {
                    oc_log_error("operand type mismatch (expected %s, got %s)\n",
                                 wa_value_type_string(WA_TYPE_I64),
                                 wa_value_type_string(slot->type));
                    exit(-1);
                }
                wa_emit(&context, (wa_code){ .opcode = WA_INSTR_jump_if_zero });
                wa_emit(&context, (wa_code){ .operand.immI64 = slot->index });

                //TODO: record a jump target
                wa_emit(&context, (wa_code){ .operand.immI64 = 0 });
            }
            else if(instr->op == WA_INSTR_else)
            {
                //TODO check that we're validating an if block and pop it
                if(!context.controlStackLen
                   || context.controlStack[context.controlStackLen - 1].start->op != WA_INSTR_if
                   || context.controlStack[context.controlStackLen - 1].elseBranch)
                {
                    oc_log_error("unexpected else block\n");
                    exit(-1);
                }
                //TODO validate block type

                context.controlStack[context.controlStackLen - 1].elseBranch = instr;

                wa_emit(&context, (wa_code){ .opcode = WA_INSTR_jump });
                //TODO: record a jump target
                wa_emit(&context, (wa_code){ .operand.immI64 = 0 });
            }
            else if(instr->op == WA_INSTR_end)
            {
                if(!context.controlStackLen)
                {
                    break;
                }
                //TODO check block type and pop block

                context.controlStackLen--;

                //TODO: patch jump targets
            }
            else */

            if(instr->op == WA_INSTR_call)
            {
                wa_func* callee = &module.functions[instr->imm[0].index];
                u32 paramCount = callee->type->paramCount;

                if(context.opdStackLen < paramCount)
                {
                    oc_log_error("Not enough arguments on stack (expected: %u, got: %u)\n",
                                 paramCount,
                                 context.opdStackLen);
                    exit(-1);
                }
                context.opdStackLen -= paramCount;

                u32 maxUsedSlot = 0;
                for(u32 stackIndex = 0; stackIndex < context.opdStackLen; stackIndex++)
                {
                    wa_operand_slot* slot = &context.opdStack[stackIndex];
                    maxUsedSlot = oc_max(slot->index, maxUsedSlot);
                }

                //TODO: first check if we args are already in order at the end of the frame
                for(u32 argIndex = 0; argIndex < paramCount; argIndex++)
                {
                    wa_operand_slot* slot = &context.opdStack[context.opdStackLen + argIndex];
                    wa_value_type paramType = callee->type->params[argIndex];

                    if(slot->type != paramType)
                    {
                        oc_log_error("Type mismatch for argument %u (expected %s, got %s)\n",
                                     argIndex,
                                     wa_value_type_string(paramType),
                                     wa_value_type_string(slot->type));
                        exit(-1);
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
                    oc_log_error("Not enough return values (expected: %i, stack depth: %i)\n",
                                 func->type->returnCount,
                                 context.opdStackLen);
                    exit(-1);
                }

                u32 retSlotStart = context.opdStackLen - func->type->returnCount;
                for(u32 retIndex = 0; retIndex < func->type->returnCount; retIndex++)
                {
                    wa_operand_slot* slot = &context.opdStack[retSlotStart + retIndex];
                    wa_value_type expectedType = func->type->returns[retIndex];
                    wa_value_type returnType = slot->type;

                    if(returnType != expectedType)
                    {
                        oc_log_error("Argument type doesn't match function signature (expected %s, got %s)\n",
                                     wa_value_type_string(expectedType),
                                     wa_value_type_string(returnType));
                        exit(-1);
                    }

                    //NOTE store value to return slot
                    if(slot->index != retIndex)
                    {
                        wa_save_slot_if_used(&context, retIndex);

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
                        .originInstr = instrCount,
                        .originOpd = context.codeLen,
                    });
            }
            else if(instr->op == WA_INSTR_local_set)
            {
                u32 localIndex = instr->imm[0].index;

                wa_operand_slot* slot = wa_operand_stack_pop(&context);
                if(slot == 0)
                {
                    oc_log_error("unbalanced stack\n");
                    exit(-1);
                }

                if(slot->type != func->locals[localIndex])
                {
                    oc_log_error("type mismatch for local.set instruction (expected %s, got %s)\n",
                                 wa_value_type_string(func->locals[localIndex]),
                                 wa_value_type_string(slot->type));
                    exit(-1);
                }

                // check if the local was used in the stack and if so save it to a slot
                wa_save_slot_if_used(&context, localIndex);

                //TODO: check if the local was written to since the value was pushed, and if not, change
                //      the output operand of the value's origin instruction rather than issuing a move
                bool shortcutSet = false;
                if(slot->kind == WA_OPERAND_SLOT_REG && slot->originOpd)
                {
                    shortcutSet = true;
                    for(u32 instrIt = slot->originInstr; instrIt < instrCount; instrIt++)
                    {
                        if(instructions[instrIt].op == WA_INSTR_local_set && instructions[instrIt].imm[0].immI32 == localIndex)
                        {
                            shortcutSet = false;
                            break;
                        }
                    }
                }
                if(shortcutSet)
                {
                    OC_DEBUG_ASSERT(context.code[slot->originOpd].immU64 == slot->index);
                    context.code[slot->originOpd].operand.immI64 = localIndex;
                }
                else
                {
                    wa_emit(&context, (wa_code){ .opcode = WA_INSTR_move });
                    wa_emit(&context, (wa_code){ .operand.immI32 = slot->index });
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
                    wa_operand_slot* slot = wa_operand_stack_pop(&context);
                    if(slot == 0)
                    {
                        oc_log_error("unbalanced stack\n");
                        exit(-1);
                    }

                    if(slot->type != info->in[opdIndex])
                    {
                        oc_log_error("operand type mismatch\n");
                        exit(-1);
                    }

                    wa_code opd = { .operand = slot->index };

                    if(slot->kind == WA_OPERAND_SLOT_REG)
                    {
                        OC_DEBUG_ASSERT(context.freeRegLen >= WA_MAX_REG);
                        context.freeRegs[context.freeRegLen] = slot->index;
                        context.freeRegLen++;
                    }
                    wa_emit(&context, opd);
                }

                for(int opdIndex = 0; opdIndex < info->outCount; opdIndex++)
                {
                    u32 index = wa_allocate_register(&context);

                    wa_operand_slot s = {
                        .kind = WA_OPERAND_SLOT_REG,
                        .type = info->out[opdIndex],
                        .index = index,
                        .originInstr = instrCount,
                        .originOpd = context.codeLen,
                    };
                    wa_operand_stack_push(&context, s);

                    wa_emit(&context, (wa_code){ .operand = s.index });
                }
            }

            instrCount++;
            if(instr->op == WA_INSTR_end)
            {
                break;
            }
        }

        // check entry length
        if(input.offset - startOffset != len)
        {
            oc_log_error("Size of code entry %i does not match declared size (declared %u, got %u)\n",
                         funcIndex,
                         len,
                         input.offset - startOffset);
            exit(-1);
        }

        func->instrCount = instrCount;
        func->instructions = oc_arena_push_array(&arena, wa_module_instruction, func->instrCount);
        memcpy(func->instructions, instructions, func->instrCount * sizeof(wa_module_instruction));

        func->code = context.code;
        func->codeLen = context.codeLen;

        wa_print_bytecode(func->codeLen, func->code);
    }

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
