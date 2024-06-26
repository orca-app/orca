#include <stdio.h>

#define OC_NO_APP_LAYER 1
#include "orca.h"

#include "wa_types.h"
#include "reader.c"

//-------------------------------------------------------------------------
// build context
//-------------------------------------------------------------------------

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

typedef struct wa_build_context
{
    oc_arena internalArena;

    oc_arena_scope arenaScope;
    wa_module* module;

    u64 opdStackLen;
    u64 opdStackCap;
    wa_operand_slot* opdStack;

    u64 saveStackLen;
    u64 saveStackCap;
    wa_operand_slot* saveStack;

    u64 scopeStackLen;
    u64 scopeStackCap;
    u64* scopeStack;

    u64 controlStackLen;
    u64 controlStackCap;
    wa_instr** controlStack;

    u64 nextRegIndex;
    u64 freeRegLen;
    u64 freeRegs[WA_MAX_REG];

    u64 codeCap;
    u64 codeLen;
    wa_code* code;

} wa_build_context;

void wa_scope_stack_push(wa_build_context* context, u64 inputCount)
{
    OC_DEBUG_ASSERT(inputCount < context->opdStackLen);

    if(context->scopeStack == 0 || context->scopeStackLen >= context->scopeStackCap)
    {
        context->scopeStackCap = (context->scopeStackCap + 8) * 2;
        u64* tmp = context->scopeStack;
        context->scopeStack = oc_arena_push_array(&context->internalArena, u64, context->scopeStackCap);
        OC_ASSERT(context->scopeStack, "out of memory");

        if(tmp)
        {
            memcpy(context->scopeStack, tmp, context->scopeStackLen * sizeof(u64));
        }
    }
    context->scopeStack[context->scopeStackLen] = context->opdStackLen - inputCount;
    context->scopeStackLen++;
}

void wa_scope_stack_pop(wa_build_context* context)
{
    OC_DEBUG_ASSERT(context->scopeStackLen);
    context->scopeStackLen--;
}

u64 wa_scope_stack_top(wa_build_context* context)
{
    u64 scopeBase = 0;
    if(context->scopeStackLen)
    {
        scopeBase = context->scopeStack[context->scopeStackLen - 1];
    }
    return (scopeBase);
}

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
    u64 scopeBase = wa_scope_stack_top(context);

    if(context->opdStackLen > scopeBase)
    {
        context->opdStackLen--;
        slot = &context->opdStack[context->opdStackLen];

        if(slot->kind == WA_OPERAND_SLOT_REG)
        {
            OC_DEBUG_ASSERT(context->freeRegLen >= WA_MAX_REG);
            context->freeRegs[context->freeRegLen] = slot->index;
            context->freeRegLen++;
        }
    }
    return (slot);
}

void wa_operand_stack_pop_slots(wa_build_context* context, u64 count)
{
    OC_DEBUG_ASSERT(count <= context->opdStackLen);

    for(u64 i = 0; i < count; i++)
    {
        wa_operand_slot* slot = &context->opdStack[context->opdStackLen - count + i];
        if(slot->kind == WA_OPERAND_SLOT_REG)
        {
            OC_DEBUG_ASSERT(context->freeRegLen >= WA_MAX_REG);
            context->freeRegs[context->freeRegLen] = slot->index;
            context->freeRegLen++;
        }
    }

    context->opdStackLen -= count;
}

wa_operand_slot* wa_operand_stack_top(wa_build_context* context)
{
    wa_operand_slot* slot = 0;
    u64 scopeBase = wa_scope_stack_top(context);

    if(context->opdStackLen > scopeBase)
    {
        slot = &context->opdStack[context->opdStackLen - 1];
    }
    return (slot);
}

void wa_operand_stack_save_slots(wa_build_context* context, u64 count)
{
    OC_DEBUG_ASSERT(count <= context->opdStackLen);

    if(context->saveStackLen + count > context->saveStackCap)
    {
        wa_operand_slot* tmp = context->saveStack;
        context->saveStackCap = (context->saveStackLen + count) * 2;
        context->saveStack = oc_arena_push_array(&context->internalArena, wa_operand_slot, context->saveStackCap);

        if(tmp)
        {
            memcpy(context->saveStack, tmp, context->saveStackLen * sizeof(wa_operand_slot));
        }
    }
    for(u64 i = 0; i < count; i++)
    {
        context->saveStack[context->saveStackLen + i] = context->opdStack[context->opdStackLen - count + 1];
    }
    context->saveStackLen += count;
}

void wa_operand_stack_restore_slots(wa_build_context* context, u64 count)
{
    OC_DEBUG_ASSERT(count <= context->saveStackLen);

    if(context->opdStackLen + count > context->opdStackCap)
    {
        wa_operand_slot* tmp = context->opdStack;
        context->opdStackCap = (context->opdStackLen + count) * 2;
        context->opdStack = oc_arena_push_array(&context->internalArena, wa_operand_slot, context->opdStackCap);

        if(tmp)
        {
            memcpy(context->opdStack, tmp, context->opdStackLen * sizeof(wa_operand_slot));
        }
    }
    for(u64 i = 0; i < count; i++)
    {
        context->opdStack[context->opdStackLen + i] = context->saveStack[context->saveStackLen - count + 1];
    }
    context->opdStackLen += count;
}

void wa_operand_stack_forget_slots(wa_build_context* context, u64 count)
{
    OC_DEBUG_ASSERT(count <= context->saveStackLen);
    context->saveStackLen -= count;
}

void wa_control_stack_push(wa_build_context* context, wa_instr* instr)
{
    if(context->controlStack == 0 || context->controlStackLen >= context->controlStackCap)
    {
        context->controlStackCap = (context->controlStackCap + 8) * 2;
        wa_instr** tmp = context->controlStack;
        context->controlStack = oc_arena_push_array(&context->internalArena, wa_instr*, context->controlStackCap);
        OC_ASSERT(context->controlStack, "out of memory");

        if(tmp)
        {
            memcpy(context->controlStack, tmp, context->controlStackLen * sizeof(wa_instr*));
        }
    }
    context->controlStack[context->controlStackLen] = instr;
    context->controlStackLen++;
}

wa_instr* wa_control_stack_pop(wa_build_context* context)
{
    wa_instr* instr = 0;
    if(context->controlStackLen)
    {
        context->controlStackLen--;
        instr = context->controlStack[context->controlStackLen];
    }
    return (instr);
}

wa_instr* wa_control_stack_lookup(wa_build_context* context, u32 label)
{
    wa_instr* instr = 0;
    if(label < context->controlStackLen)
    {
        instr = context->controlStack[context->controlStackLen - label - 1];
    }
    return (instr);
}

wa_instr* wa_control_stack_top(wa_build_context* context)
{
    return wa_control_stack_lookup(context, 0);
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

//TODO: free register

void wa_move_slot_if_used(wa_build_context* context, u32 slotIndex)
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
    wa_input_seek(input, module->toc.code.offset);

    u32 functionCount = wa_read_leb128_u32(input);
    if(functionCount != module->functionCount)
    {
        oc_log_error("Function count mismatch (function section: %i, code section: %i\n",
                     module->functionCount,
                     functionCount);
        exit(-1);
    }

    for(u32 funcIndex = 0; funcIndex < module->functionCount; funcIndex++)
    {
        wa_func* func = &module->functions[funcIndex];

        oc_arena_scope scratch = oc_scratch_begin();

        u32 len = wa_read_leb128_u32(input);
        u32 startOffset = input->offset;

        // read locals
        u32 localEntryCount = wa_read_leb128_u32(input);
        u32* counts = oc_arena_push_array(scratch.arena, u32, localEntryCount);
        wa_value_type* types = oc_arena_push_array(scratch.arena, wa_value_type, localEntryCount);

        func->localCount = func->type->paramCount;
        for(u32 localEntryIndex = 0; localEntryIndex < localEntryCount; localEntryIndex++)
        {
            counts[localEntryIndex] = wa_read_leb128_u32(input);
            types[localEntryIndex] = wa_read_byte(input);
            func->localCount += counts[localEntryIndex];

            //TODO: validate types
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

        wa_build_context context = {
            .arenaScope = oc_arena_scope_begin(arena),
            .module = module,
            .codeCap = 4,
            .code = oc_arena_push_array(scratch.arena, wa_code, 4),
            .nextRegIndex = func->localCount,
        };
        oc_arena_init(&context.internalArena);

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

            u8 byte = wa_read_byte(input);

            if(byte == WA_INSTR_PREFIX_EXTENDED)
            {
                u32 code = wa_read_leb128_u32(input);
                if(code >= wa_instr_decode_extended_len)
                {
                    oc_log_error("Invalid extended instruction %i\n", code);
                    exit(-1);
                }
                instr->op = wa_instr_decode_extended[code];
            }
            else if(byte == WA_INSTR_PREFIX_VECTOR)
            {
                u32 code = wa_read_leb128_u32(input);
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
                        wa_read_byte(input);
                    }
                    break;
                    case WA_IMM_I32:
                    {
                        instr->imm[immIndex].immI32 = wa_read_leb128_u32(input);
                    }
                    break;
                    case WA_IMM_I64:
                    {
                        instr->imm[immIndex].immI64 = wa_read_leb128_u64(input);
                    }
                    break;
                    case WA_IMM_F32:
                    {
                        instr->imm[immIndex].immF32 = wa_read_f32(input);
                    }
                    break;
                    case WA_IMM_F64:
                    {
                        instr->imm[immIndex].immF64 = wa_read_f64(input);
                    }
                    break;
                    case WA_IMM_VALUE_TYPE:
                    {
                        instr->imm[immIndex].valueType = wa_read_byte(input);
                    }
                    break;
                    case WA_IMM_REF_TYPE:
                    {
                        instr->imm[immIndex].valueType = wa_read_byte(input);
                    }
                    break;

                    case WA_IMM_LOCAL_INDEX:
                    case WA_IMM_GLOBAL_INDEX:
                    case WA_IMM_FUNC_INDEX:
                    case WA_IMM_TYPE_INDEX:
                    case WA_IMM_TABLE_INDEX:
                    case WA_IMM_ELEM_INDEX:
                    case WA_IMM_DATA_INDEX:
                    {
                        instr->imm[immIndex].index = wa_read_leb128_u32(input);
                    }
                    break;
                    case WA_IMM_MEM_ARG:
                    {
                        instr->imm[immIndex].memArg.align = wa_read_leb128_u32(input);
                        instr->imm[immIndex].memArg.offset = wa_read_leb128_u32(input);
                    }
                    break;
                    case WA_IMM_LANE_INDEX:
                    {
                        instr->imm[immIndex].laneIndex = wa_read_byte(input);
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
                i64 t = wa_read_leb128_i64(input);
                if(t >= 0)
                {
                    instr->block.typeKind = WA_BLOCK_TYPE_USER;
                    instr->block.type.index = (u64)t;
                    if(instr->block.type.index >= module->typeCount)
                    {
                        oc_log_error("unexpected type index %u (type count: %u)\n",
                                     instr->block.type.index,
                                     module->typeCount);
                        exit(-1);
                    }
                }
                else
                {
                    t &= 0x7f;
                    if(t == 0x40)
                    {
                        instr->block.typeKind = WA_BLOCK_TYPE_VOID;
                    }
                    else
                    {
                        if(!wa_is_value_type(t))
                        {
                            oc_log_error("unrecognized value type 0x%02x\n", t);
                            exit(-1);
                        }
                        instr->block.typeKind = WA_BLOCK_TYPE_VALUE_TYPE;
                        instr->block.type.valueType = t;
                    }
                }
            }

            instrCount++;
        }

        // check entry length
        if(input->offset - startOffset != len)
        {
            oc_log_error("Size of code entry %i does not match declared size (declared %u, got %u)\n",
                         funcIndex,
                         len,
                         input->offset - startOffset);
            exit(-1);
        }

        func->instrCount = instrCount;
        func->instructions = oc_arena_push_array(arena, wa_instr, func->instrCount);
        memcpy(func->instructions, instructions, func->instrCount * sizeof(wa_instr));

        oc_scratch_end(scratch);
    }
}

void wa_compile_code(oc_arena* arena, wa_module* module)
{
    for(u32 funcIndex = 0; funcIndex < module->functionCount; funcIndex++)
    {
        wa_func* func = &module->functions[funcIndex];

        oc_arena_scope scratch = oc_scratch_begin();

        wa_build_context context = {
            .arenaScope = oc_arena_scope_begin(arena),
            .module = module,
            .codeCap = 4,
            .code = oc_arena_push_array(scratch.arena, wa_code, 4),
            .nextRegIndex = func->localCount,
        };
        oc_arena_init(&context.internalArena);

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
                        break;

                    case WA_IMM_LOCAL_INDEX:
                    {
                        if(instr->imm[immIndex].index >= func->localCount)
                        {
                            oc_log_error("invalid local index %u (localCount: %u)\n",
                                         instr->imm[immIndex].index,
                                         func->localCount);
                            exit(-1);
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

            if(instr->op == WA_INSTR_if)
            {
                wa_control_stack_push(&context, instr);

                wa_operand_slot* slot = wa_operand_stack_pop(&context);
                if(!slot)
                {
                    oc_log_error("unbalanced stack\n");
                    exit(-1);
                }

                if(slot->type != WA_TYPE_I32)
                {
                    oc_log_error("operand type mismatch (expected %s, got %s)\n",
                                 wa_value_type_string(WA_TYPE_I32),
                                 wa_value_type_string(slot->type));
                    exit(-1);
                }

                //NOTE: check block type input
                u64 inputCount = 0;
                if(instr->block.typeKind == WA_BLOCK_TYPE_USER)
                {
                    wa_func_type* type = &module->types[instr->block.type.index];
                    if(type->paramCount > context.opdStackLen)
                    {
                        oc_log_error("not enough operands on stack (expected %i, got %i)\n",
                                     type->paramCount,
                                     context.opdStackLen);
                        exit(-1);
                    }
                    for(u64 paramIndex = 0; paramIndex < type->paramCount; paramIndex++)
                    {
                        wa_value_type opdType = context.opdStack[context.opdStackLen - type->paramCount + paramIndex].type;
                        wa_value_type expectedType = type->params[paramIndex];
                        if(opdType != expectedType)
                        {
                            oc_log_error("operand type mismatch for param %i (expected %s, got %s)\n",
                                         paramIndex,
                                         wa_value_type_string(expectedType),
                                         wa_value_type_string(opdType));
                            exit(-1);
                        }
                    }
                    inputCount = type->paramCount;
                }
                wa_operand_stack_save_slots(&context, inputCount);
                wa_scope_stack_push(&context, inputCount);

                wa_move_locals_to_registers(&context);

                wa_emit(&context, (wa_code){ .opcode = WA_INSTR_jump_if_zero });
                wa_emit(&context, (wa_code){ .operand.immI64 = slot->index });

                //TODO: record a jump target
                wa_emit(&context, (wa_code){ .operand.immI64 = 0 });
            }
            else if(instr->op == WA_INSTR_else)
            {
                wa_instr* block = wa_control_stack_top(&context);
                if(!block
                   || block->op != WA_INSTR_if
                   || block->block.elseBranch)
                {
                    oc_log_error("unexpected else block\n");
                    exit(-1);
                }

                //NOTE validate block type output
                u64 inputCount = 0;
                u64 outputCount = 0;

                if(block->block.typeKind == WA_BLOCK_TYPE_VALUE_TYPE)
                {
                    wa_operand_slot* slot = wa_operand_stack_top(&context);
                    if(!slot)
                    {
                        oc_log_error("operand type mismatch (expected %s, got void)\n",
                                     wa_value_type_string(block->block.type.valueType));
                        exit(-1);
                    }
                    else if(slot->type != block->block.type.valueType)
                    {
                        oc_log_error("operand type mismatch (expected %s, got %s)\n",
                                     wa_value_type_string(block->block.type.valueType),
                                     wa_value_type_string(slot->type));
                        exit(-1);
                    }
                    outputCount = 1;
                }
                else if(block->block.typeKind == WA_BLOCK_TYPE_USER)
                {
                    wa_func_type* type = &module->types[block->block.type.index];
                    ///////////////////////////////////////////////////////////////////
                    //TODO check that we have the _exact_ number of outputs
                    ///////////////////////////////////////////////////////////////////
                    if(type->returnCount > context.opdStackLen)
                    {
                        oc_log_error("not enough operands on stack (expected %i, got %i)\n",
                                     type->returnCount,
                                     context.opdStackLen);
                        exit(-1);
                    }
                    for(u64 returnIndex = 0; returnIndex < type->returnCount; returnIndex++)
                    {
                        wa_value_type opdType = context.opdStack[context.opdStackLen - type->returnCount + returnIndex].type;
                        wa_value_type expectedType = type->returns[returnIndex];
                        if(opdType != expectedType)
                        {
                            oc_log_error("operand type mismatch for return %i (expected %s, got %s)\n",
                                         returnIndex,
                                         wa_value_type_string(expectedType),
                                         wa_value_type_string(opdType));
                            exit(-1);
                        }
                    }
                    inputCount = type->paramCount;
                    outputCount = type->returnCount;
                }
                wa_operand_stack_pop_slots(&context, outputCount);
                wa_operand_stack_restore_slots(&context, inputCount);

                block->block.elseBranch = instr;

                wa_emit(&context, (wa_code){ .opcode = WA_INSTR_jump });
                //TODO: record a jump target
                wa_emit(&context, (wa_code){ .operand.immI64 = 0 });
            }
            else if(instr->op == WA_INSTR_end)
            {
                wa_instr* block = wa_control_stack_pop(&context);
                if(!block)
                {
                    break;
                }

                u64 inputCount = 0;
                u64 outputCount = 0;

                if(block->block.typeKind == WA_BLOCK_TYPE_VALUE_TYPE)
                {
                    wa_operand_slot* slot = wa_operand_stack_top(&context);
                    if(!slot)
                    {
                        oc_log_error("operand type mismatch (expected %s, got void)\n",
                                     wa_value_type_string(block->block.type.valueType));
                        exit(-1);
                    }
                    else if(slot->type != block->block.type.valueType)
                    {
                        oc_log_error("operand type mismatch (expected %s, got %s)\n",
                                     wa_value_type_string(block->block.type.valueType),
                                     wa_value_type_string(slot->type));
                        exit(-1);
                    }
                    outputCount = 1;
                }
                else if(block->block.typeKind == WA_BLOCK_TYPE_USER)
                {
                    wa_func_type* type = &module->types[block->block.type.index];
                    if(type->returnCount > context.opdStackLen)
                    {
                        oc_log_error("not enough operands on stack (expected %i, got %i)\n",
                                     type->returnCount,
                                     context.opdStackLen);
                        exit(-1);
                    }
                    for(u64 returnIndex = 0; returnIndex < type->returnCount; returnIndex++)
                    {
                        wa_value_type opdType = context.opdStack[context.opdStackLen - type->returnCount + returnIndex].type;
                        wa_value_type expectedType = type->returns[returnIndex];
                        if(opdType != expectedType)
                        {
                            oc_log_error("operand type mismatch for return %i (expected %s, got %s)\n",
                                         returnIndex,
                                         wa_value_type_string(expectedType),
                                         wa_value_type_string(opdType));
                            exit(-1);
                        }
                    }
                    inputCount = type->paramCount;
                    outputCount = type->returnCount;
                }
                wa_operand_stack_forget_slots(&context, inputCount);
                wa_scope_stack_pop(&context);

                //TODO: patch jump targets
            }
            else if(instr->op == WA_INSTR_call)
            {
                wa_func* callee = &module->functions[instr->imm[0].index];
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
                wa_move_slot_if_used(&context, localIndex);

                //TODO: check if the local was written to since the value was pushed, and if not, change
                //      the output operand of the value's origin instruction rather than issuing a move
                bool shortcutSet = false;
                if(slot->kind == WA_OPERAND_SLOT_REG && slot->originOpd)
                {
                    shortcutSet = true;
                    for(u32 instrIt = slot->originInstr; instrIt < instrIndex; instrIt++)
                    {
                        if(func->instructions[instrIt].op == WA_INSTR_local_set && func->instructions[instrIt].imm[0].immI32 == localIndex)
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

            //TODO: count blocks
            if(instr->op == WA_INSTR_end)
            {
                break;
            }
        }
        func->code = context.code;
        func->codeLen = context.codeLen;

        oc_scratch_end(scratch);
    }
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

    wa_input input = {};
    {
        oc_file file = oc_file_open(OC_STR8(modulePath), OC_FILE_ACCESS_READ, OC_FILE_OPEN_NONE);

        input.len = oc_file_size(file);
        input.contents = oc_arena_push(&arena, input.len);

        oc_file_read(file, input.len, input.contents);
        oc_file_close(file);
    }

    wa_module module = {
        .len = input.len,
        .bytes = input.contents,
    };

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
    wa_compile_code(&arena, &module);
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

    //    wa_interpret_func(&module, start);

    oc_scratch_end(scratch);

    return (0);
}
