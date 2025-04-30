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

//-------------------------------------------------------------------------------
// Interpreter
//-------------------------------------------------------------------------------

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
