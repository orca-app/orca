#include <stdio.h>

#define OC_NO_APP_LAYER 1
#include "orca.h"

#include "wa_types.h"
#include "reader.c"

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
    WA_OPERAND_SLOT_LOCAL = 0,
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

typedef struct wa_build_context
{
    oc_arena checkArena;
    oc_arena codeArena;
    wa_module* module;

    u64 opdStackLen;
    u64 opdStackCap;
    wa_operand_slot* opdStack;

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

void wa_scope_stack_push(wa_build_context* context)
{
    if(context->scopeStack == 0 || context->scopeStackLen >= context->scopeStackCap)
    {
        context->scopeStackCap = (context->scopeStackCap + 8) * 2;
        u64* tmp = context->scopeStack;
        context->scopeStack = oc_arena_push_array(&context->checkArena, u64, context->scopeStackCap);
        OC_ASSERT(context->scopeStack, "out of memory");

        if(tmp)
        {
            memcpy(context->scopeStack, tmp, context->scopeStackLen * sizeof(u64));
        }
    }
    context->scopeStack[context->scopeStackLen] = context->opdStackLen;
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

wa_operand_slot* wa_operand_stack_pop(wa_build_context* context)
{
    wa_operand_slot* slot = 0;
    u64 scopeBase = wa_scope_stack_top(context);

    if(context->opdStackLen > scopeBase)
    {
        context->opdStackLen--;
        slot = &context->opdStack[context->opdStackLen];

        if(slot->kind == WA_OPERAND_SLOT_REG && slot->count == 0)
        {
            wa_free_slot(context, slot->index);
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

void wa_control_stack_push(wa_build_context* context, wa_instr* instr)
{
    if(context->controlStack == 0 || context->controlStackLen >= context->controlStackCap)
    {
        context->controlStackCap = (context->controlStackCap + 8) * 2;
        wa_instr** tmp = context->controlStack;
        context->controlStack = oc_arena_push_array(&context->checkArena, wa_instr*, context->controlStackCap);
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
                if(pc[1].operand.immI64 == 0)
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
                    u64 typeIndex = (u64)t;

                    if(typeIndex >= module->typeCount)
                    {
                        oc_log_error("unexpected type index %u (type count: %u)\n",
                                     typeIndex,
                                     module->typeCount);
                        exit(-1);
                    }
                    instr->block.type = &module->types[typeIndex];
                }
                else
                {
                    if(!wa_is_value_type(t & 0x7f))
                    {
                        oc_log_error("unrecognized value type 0x%02x\n", t);
                        exit(-1);
                    }

                    t = (t == -64) ? 0 : -t;
                    instr->block.type = (wa_func_type*)&WA_BLOCK_VALUE_TYPES[t];
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
    wa_func_type* type = instr->block.type;

    if(type->paramCount > context->opdStackLen)
    {
        oc_log_error("not enough operands on stack (expected %i, got %i)\n",
                     type->paramCount,
                     context->opdStackLen);
        exit(-1);
    }
    for(u64 inIndex = 0; inIndex < type->paramCount; inIndex++)
    {
        wa_value_type opdType = context->opdStack[context->opdStackLen - type->paramCount + inIndex].type;
        wa_value_type expectedType = type->params[inIndex];
        if(opdType != expectedType)
        {
            oc_log_error("operand type mismatch for param %i (expected %s, got %s)\n",
                         inIndex,
                         wa_value_type_string(expectedType),
                         wa_value_type_string(opdType));
            exit(-1);
        }
    }

    //NOTE allocate block results and push them on the stack
    //TODO immediately put them in the freelist so they can be used in the branches (this might complicate copying results a bit...)
    for(u64 outIndex = 0; outIndex < type->returnCount; outIndex++)
    {
        u32 index = wa_allocate_register(context);

        wa_operand_slot s = {
            .kind = WA_OPERAND_SLOT_REG,
            .index = index,
            .type = type->returns[outIndex],
        };
        wa_operand_stack_push(context, s);

        // wa_free_slot(context, index);
    }

    wa_scope_stack_push(context);
    wa_push_block_inputs(context, type->paramCount, type->returnCount);
}

void wa_block_end(wa_build_context* context, wa_instr* block, wa_instr* instr)
{
    //NOTE validate block type output
    wa_func_type* type = block->block.type;

    if(type->returnCount > context->opdStackLen)
    {
        oc_log_error("not enough operands on stack (expected %i, got %i)\n",
                     type->returnCount,
                     context->opdStackLen);
        exit(-1);
    }
    for(u64 returnIndex = 0; returnIndex < type->returnCount; returnIndex++)
    {
        wa_value_type opdType = context->opdStack[context->opdStackLen - type->returnCount + returnIndex].type;
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

    //NOTE move top operands to result slots
    u64 scopeBase = context->scopeStack[context->scopeStackLen - 1];

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

    //TODO: pop result operands, or pop until scope start ?? (shouldn't this already be done?)
    wa_operand_stack_pop_slots(context, type->returnCount);

    if(instr->op != WA_INSTR_else)
    {
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

        wa_scope_stack_pop(context);
    }
}

void wa_emit_jump_target(wa_build_context* context, wa_instr* block)
{
    wa_jump_target* target = oc_arena_push_type(&context->checkArena, wa_jump_target);
    target->offset = context->codeLen;
    oc_list_push_back(&block->block.jumpTargets, &target->listElt);

    wa_emit(context, (wa_code){ .operand.immI64 = 0 });
}

void wa_patch_jump_targets(wa_build_context* context, wa_instr* block)
{
    oc_list_for(block->block.jumpTargets, target, wa_jump_target, listElt)
    {
        context->code[target->offset].operand.immI64 = context->codeLen - target->offset;
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

                wa_move_locals_to_registers(&context);

                wa_block_begin(&context, instr);

                wa_emit(&context, (wa_code){ .opcode = WA_INSTR_jump_if_zero });
                wa_emit_jump_target(&context, instr);
                wa_emit(&context, (wa_code){ .operand.immI64 = slot->index });
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

                wa_block_end(&context, block, instr);

                wa_func_type* blockType = block->block.type;
                wa_push_block_inputs(&context, blockType->paramCount, blockType->returnCount);

                block->block.elseBranch = instr;

                wa_emit(&context, (wa_code){ .opcode = WA_INSTR_jump });
                wa_emit_jump_target(&context, instr);

                wa_patch_jump_targets(&context, block);
            }
            else if(instr->op == WA_INSTR_end)
            {
                wa_instr* block = wa_control_stack_pop(&context);
                if(!block)
                {
                    //TODO should we sometimes emit a return here?
                    break;
                }
                wa_block_end(&context, block, instr);

                if(block->op == WA_INSTR_if && block->block.elseBranch)
                {
                    block = block->block.elseBranch;
                }
                wa_patch_jump_targets(&context, block);
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

                //TODO: first check if we args are already in order at the end of the frame?
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
        }
        func->codeLen = context.codeLen;
        func->code = oc_arena_push_array(arena, wa_code, context.codeLen);
        memcpy(func->code, context.code, context.codeLen * sizeof(wa_code));
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

    wa_interpret_func(&module, start);

    oc_scratch_end(scratch);

    return (0);
}
