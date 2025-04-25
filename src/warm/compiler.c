/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "warm.h"
#include "instructions.h"

//-------------------------------------------------------------------------
// Compile
//-------------------------------------------------------------------------

typedef struct wa_jump_target
{
    oc_list_elt listElt;
    u64 offset;
} wa_jump_target;

typedef struct wa_block
{
    wa_instr* begin;
    u64 beginOffset;
    u64 elseOffset;
    oc_list jumpTargets;

    u64 scopeBase;
    bool polymorphic;
    bool prevPolymorphic;

} wa_block;

typedef struct wa_operand_slot
{
    u32 index;
    wa_instr* originInstr;
    u64 originOpd;
} wa_operand_slot;

typedef struct wa_register_slot
{
    u32 refCount;
    i32 nextFree;
    wa_value_type type;

    u64 startInstr;
} wa_register_slot;

typedef struct wa_operand
{
    wa_value_type type;
    u32 index;

} wa_operand;

typedef struct wa_build_context
{
    oc_arena* arena;     // the module's arena
    oc_arena checkArena; // temp arena for checking
    oc_arena codeArena;  // temp arena for building code
    wa_module* module;

    u32 regCount;
    wa_register_slot regs[WA_MAX_REG];
    i32 firstFreeReg;

    u64 opdStackLen;
    u64 opdStackCap;
    wa_operand_slot* opdStack;

    u64 controlStackLen;
    u64 controlStackCap;
    wa_block* controlStack;

    wa_func* currentFunction;
    wa_instr* currentInstr;
    wa_func_type* exprType;

    u64 codeCap;
    u64 codeLen;
    wa_code* code;

    u32 registerMapCounts[WA_MAX_REG];
    oc_list registerMap[WA_MAX_REG];

} wa_build_context;

typedef struct wa_register_range_elt
{
    oc_list_elt listElt;
    wa_register_range range;
} wa_register_range_elt;

void wa_register_mapping_push(wa_build_context* context, u32 regIndex, u64 start, u64 end, wa_value_type type)
{
    wa_register_range_elt* elt = oc_arena_push_type(&context->checkArena, wa_register_range_elt);
    elt->range = (wa_register_range){
        .start = start,
        .end = end,
        .type = type,
    };
    oc_list_push_back(&context->registerMap[regIndex], &elt->listElt);
    context->registerMapCounts[regIndex]++;
}

void wa_compile_error(wa_build_context* context, wa_ast* ast, const char* fmt, ...)
{
    wa_module_error* error = oc_arena_push_type(context->arena, wa_module_error);
    memset(error, 0, sizeof(wa_module_error));

    va_list ap;
    va_start(ap, fmt);
    error->string = oc_str8_pushfv(context->arena, fmt, ap);
    va_end(ap);

    error->ast = ast;
    if(ast)
    {
        oc_list_push_back(&ast->errors, &error->astElt);
    }

    oc_list_push_back(&context->module->errors, &error->moduleElt);
}

void wa_compile_error_block_end(wa_build_context* context, wa_ast* ast, const char* fmt, ...)
{
    wa_module_error* error = oc_arena_push_type(context->arena, wa_module_error);
    memset(error, 0, sizeof(wa_module_error));

    va_list ap;
    va_start(ap, fmt);
    error->string = oc_str8_pushfv(context->arena, fmt, ap);
    va_end(ap);

    error->ast = ast;
    error->blockEnd = true;

    oc_list_push_back(&ast->errors, &error->astElt);

    oc_list_push_back(&context->module->errors, &error->moduleElt);
}

bool wa_operand_is_nil(wa_operand* opd)
{
    return (opd->type == WA_TYPE_INVALID);
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

    if(context->controlStackLen && context->controlStack[context->controlStackLen - 1].polymorphic)
    {
        context->controlStack[context->controlStackLen].prevPolymorphic = true;
    }
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

bool wa_operand_slot_is_unknown(wa_operand_slot* opd)
{
    return opd->index == UINT32_MAX;
}

u32 wa_allocate_register(wa_build_context* context, wa_value_type type)
{
    u32 index = 0;
    wa_register_slot* reg = 0;

    if(context->firstFreeReg >= 0)
    {
        reg = &context->regs[context->firstFreeReg];
        context->firstFreeReg = reg->nextFree;
    }
    else
    {
        if(context->regCount >= WA_MAX_REG)
        {
            wa_compile_error(context, context->currentInstr->ast, "register allocation overflow\n");
        }
        else
        {
            reg = &context->regs[context->regCount];
            context->regCount++;
        }
    }

    if(reg)
    {
        reg->type = type;
        reg->refCount = 0;
        reg->startInstr = context->codeLen;
        index = reg - context->regs;
    }
    return (index);
}

void wa_retain_register(wa_build_context* context, u32 index)
{
    OC_DEBUG_ASSERT(index < WA_MAX_REG);
    wa_register_slot* reg = &context->regs[index];
    reg->refCount++;
}

void wa_release_register(wa_build_context* context, u32 index)
{
    OC_DEBUG_ASSERT(index < WA_MAX_REG);
    wa_register_slot* reg = &context->regs[index];

    OC_DEBUG_ASSERT(reg->refCount);
    reg->refCount--;

    u32 localCount = context->currentFunction ? context->currentFunction->localCount : 0;
    if(index >= localCount && reg->refCount == 0)
    {
        //NOTE range is inclusive, so it starts right after the first instruction, hence the +1
        wa_register_mapping_push(context, index, reg->startInstr + 1, context->codeLen, reg->type);

        reg->nextFree = context->firstFreeReg;
        context->firstFreeReg = index;
    }
}

void wa_operand_stack_push(wa_build_context* context, wa_operand_slot opd)
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
    context->opdStack[context->opdStackLen] = opd;
    context->opdStackLen++;

    if(!wa_operand_slot_is_unknown(&opd))
    {
        wa_retain_register(context, opd.index);
    }
}

void wa_operand_stack_push_return_slots(wa_build_context* context, u32 maxUsedSlot, u32 returnCount, wa_value_type* returns)
{
    //NOTE: return slots will start at maxUsedSlot, so we must reserve returnCount slots after that.
    context->regCount = oc_max(maxUsedSlot + 1 + returnCount, context->regCount);

    for(u32 retIndex = 0; retIndex < returnCount; retIndex++)
    {
        u64 slotIndex = maxUsedSlot + 1 + retIndex;

        //NOTE: if that slot was in the free list, we remove it.
        //      otherwise it is a fresh slot
        {
            i32 freeRegIndex = context->firstFreeReg;
            wa_register_slot* prev = 0;
            while(freeRegIndex >= 0)
            {
                wa_register_slot* reg = &context->regs[freeRegIndex];

                if(freeRegIndex == slotIndex)
                {
                    if(prev)
                    {
                        prev->nextFree = reg->nextFree;
                    }
                    else
                    {
                        context->firstFreeReg = reg->nextFree;
                    }
                    break;
                }

                prev = reg;
                freeRegIndex = reg->nextFree;
            }
        }

        context->regs[slotIndex].refCount = 0;
        context->regs[slotIndex].type = returns[retIndex];

        //NOTE: push the slot
        wa_operand_stack_push(context,
                              (wa_operand_slot){
                                  .index = slotIndex,
                              });
    }
}

u32 wa_operand_stack_push_reg(wa_build_context* context, wa_value_type type, wa_instr* instr)
{
    wa_operand_slot opd = {
        .index = wa_allocate_register(context, type),
        .originInstr = instr,
        .originOpd = context->codeLen,
    };
    wa_operand_stack_push(context, opd);

    return opd.index;
}

u32 wa_operand_stack_push_local(wa_build_context* context, u32 index)
{
    wa_operand_slot opd = {
        .index = index,
        //        .originInstr = instr,
        //        .originOpd = context->codeLen,
    };
    wa_operand_stack_push(context, opd);
    return opd.index;
}

u32 wa_operand_stack_push_unknown(wa_build_context* context)
{
    wa_operand_slot opd = {
        .index = UINT32_MAX,
    };
    wa_operand_stack_push(context, opd);
    return opd.index;
}

void wa_operand_stack_push_copy(wa_build_context* context, u32 index)
{
    wa_operand_slot opdCopy = context->opdStack[index];
    wa_operand_stack_push(context, opdCopy);
}

wa_operand wa_operand_stack_lookup(wa_build_context* context, u32 index)
{
    wa_operand opd = { 0 };
    wa_block* block = wa_control_stack_top(context);
    OC_ASSERT(block);

    if(index < context->opdStackLen - block->scopeBase)
    {
        wa_operand_slot* slot = &context->opdStack[context->opdStackLen - index - 1];

        if(wa_operand_slot_is_unknown(slot))
        {
            opd.type = WA_TYPE_UNKNOWN;
        }
        else
        {
            opd.index = slot->index;
            opd.type = context->regs[slot->index].type;
        }
    }
    else if(block->polymorphic)
    {
        opd = (wa_operand){
            .type = WA_TYPE_UNKNOWN,
        };
    }
    return (opd);
}

wa_operand wa_operand_stack_pop(wa_build_context* context)
{
    wa_operand opd = wa_operand_stack_lookup(context, 0);

    wa_block* block = wa_control_stack_top(context);
    OC_ASSERT(block);

    if(context->opdStackLen > block->scopeBase)
    {
        context->opdStackLen--;
        if(opd.type != WA_TYPE_UNKNOWN)
        {
            wa_release_register(context, opd.index);
        }
    }
    return opd;
}

u32 wa_operand_stack_scope_size(wa_build_context* context)
{
    wa_block* block = wa_control_stack_top(context);
    return context->opdStackLen - block->scopeBase;
}

void wa_operand_stack_pop_slots(wa_build_context* context, u64 count)
{
    wa_block* block = wa_control_stack_top(context);
    OC_ASSERT(block);
    u64 scopeBase = block->scopeBase;

    u64 opdCount = oc_min(context->opdStackLen - scopeBase, count);

    for(u64 i = 0; i < opdCount; i++)
    {
        wa_operand_slot* opd = &context->opdStack[context->opdStackLen - i - 1];
        wa_release_register(context, opd->index);
    }

    context->opdStackLen -= opdCount;
}

void wa_operand_stack_pop_scope(wa_build_context* context, wa_block* block)
{
    OC_ASSERT(context->opdStackLen >= block->scopeBase);

    u64 opdCount = context->opdStackLen - block->scopeBase;
    for(u64 i = 0; i < opdCount; i++)
    {
        wa_operand_stack_pop(context);
    }
    OC_ASSERT(context->opdStackLen == block->scopeBase);
}

bool wa_check_operand_type(wa_value_type t1, wa_value_type t2)
{
    return (t1 == t2
            || t1 == WA_TYPE_UNKNOWN
            || t2 == WA_TYPE_UNKNOWN
            || t1 == WA_TYPE_ANY
            || t2 == WA_TYPE_ANY
            || (t1 == WA_TYPE_REF && (t2 == WA_TYPE_FUNC_REF || t2 == WA_TYPE_EXTERN_REF))
            || (t2 == WA_TYPE_REF && (t1 == WA_TYPE_FUNC_REF || t1 == WA_TYPE_EXTERN_REF))
            || (t1 == WA_TYPE_NUM_OR_VEC && (wa_is_value_type_numeric(t2) || t2 == WA_TYPE_V128))
            || (t2 == WA_TYPE_NUM_OR_VEC && (wa_is_value_type_numeric(t1) || t1 == WA_TYPE_V128)));
}

wa_operand* wa_operand_stack_get_operands(oc_arena* arena,
                                          wa_build_context* context,
                                          wa_instr* instr,
                                          u32 count,
                                          wa_value_type* types,
                                          bool popOpds)
{
    wa_operand* opds = oc_arena_push_array(arena, wa_operand, count);
    for(u32 i = 0; i < count; i++)
    {
        u32 opdIndex = count - 1 - i;

        if(popOpds)
        {
            opds[opdIndex] = wa_operand_stack_pop(context);
        }
        else
        {
            opds[opdIndex] = wa_operand_stack_lookup(context, i);
        }

        if(wa_operand_is_nil(&opds[opdIndex]))
        {
            wa_compile_error(context, instr->ast, "unbalanced stack\n");
            break;
        }
        else
        {
            if(!wa_check_operand_type(opds[opdIndex].type, types[opdIndex]))
            {
                wa_compile_error(context, instr->ast, "operand types mismatch\n");
                //TODO: here we can give a better error message since we have all the operands popped so far
                break;
            }
        }
    }
    return (opds);
}

void wa_block_set_polymorphic(wa_build_context* context)
{
    wa_block* block = wa_control_stack_top(context);
    block->polymorphic = true;
    wa_operand_stack_pop_scope(context, block);
}

void wa_emit(wa_build_context* context, wa_code* code)
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
    memcpy(&context->code[context->codeLen], code, sizeof(wa_code));
    context->codeLen++;
}

wa_code* wa_push_code(wa_build_context* context)
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
    wa_code* code = &context->code[context->codeLen];
    memset(code, 0, sizeof(wa_code));
    context->codeLen++;
    return (code);
}

void wa_emit_opcode(wa_build_context* context, wa_instr_op op)
{
    u32 index = context->codeLen;
    wa_code* code = wa_push_code(context);
    code->opcode = op;

    if(context->currentFunction)
    {
        wa_module* module = context->module;
        wa_warm_to_wasm_loc_push(module, context->currentFunction - module->functions, index, context->currentInstr);
    }
    context->currentInstr->codeIndex = index;
}

void wa_emit_index(wa_build_context* context, u32 index)
{
    wa_code* code = wa_push_code(context);
    code->index = index;
}

void wa_emit_i64(wa_build_context* context, i64 val)
{
    wa_code* code = wa_push_code(context);
    code->valI64 = val;
}

bool wa_operand_slot_is_local(wa_build_context* context, wa_operand_slot* opd)
{
    u32 localCount = context->currentFunction ? context->currentFunction->localCount : 0;
    return (opd->index < localCount);
}

void wa_move_register_if_used_in_stack_range(wa_build_context* context,
                                             u32 slotIndex,
                                             u32 opdCount)
{
    OC_DEBUG_ASSERT(slotIndex < WA_MAX_REG);
    OC_DEBUG_ASSERT(opdCount < WA_TYPE_STACK_MAX_LEN);

    wa_block* block = wa_control_stack_top(context);
    u32 scopeBase = block->scopeBase;

    u32 newReg = UINT32_MAX;
    for(u32 opdIndex = 0; opdIndex < opdCount; opdIndex++)
    {
        //WARN: we compute these instead of directly computing stackIndex,
        //      because stackIndex could underflow if opdCount is bigger than
        //      the stack
        u32 indexFromStackTop = opdCount - 1 - opdIndex;
        u32 scopeLen = context->opdStackLen - scopeBase;

        if(indexFromStackTop < scopeLen)
        {
            u32 stackIndex = context->opdStackLen - opdCount + opdIndex;
            wa_operand_slot* opd = &context->opdStack[stackIndex];

            if(opd->index == slotIndex)
            {
                if(newReg == UINT32_MAX)
                {
                    OC_DEBUG_ASSERT(opd->index >= 0 && opd->index < context->regCount);
                    newReg = wa_allocate_register(context, context->regs[opd->index].type);
                }

                wa_retain_register(context, newReg);

                //TODO: move that below with count
                wa_release_register(context, slotIndex);

                opd->index = newReg;
            }
        }
    }
    if(newReg != UINT32_MAX)
    {
        wa_emit_opcode(context, WA_INSTR_move);
        wa_emit_index(context, slotIndex);
        wa_emit_index(context, newReg);
    }
}

void wa_move_local_if_used(wa_build_context* context, u32 slotIndex)
{
    OC_DEBUG_ASSERT(slotIndex < context->currentFunction->localCount);
    wa_block* block = wa_control_stack_top(context);
    wa_move_register_if_used_in_stack_range(context, slotIndex, context->opdStackLen - block->scopeBase);
}

void wa_move_locals_to_registers(wa_build_context* context)
{
    for(u32 stackIndex = 0; stackIndex < context->opdStackLen; stackIndex++)
    {
        wa_operand_slot* opd = &context->opdStack[stackIndex];
        if(wa_operand_slot_is_local(context, opd))
        {
            wa_move_local_if_used(context, opd->index);
        }
    }
}

void wa_push_block_inputs(wa_build_context* context, wa_func_type* type)
{
    OC_ASSERT(context->controlStackLen > 1);
    wa_block* prevBlock = &context->controlStack[context->controlStackLen - 2];

    u64 paramStart = context->opdStackLen - (type->paramCount + type->returnCount);
    u64 inputEnd = context->opdStackLen - type->returnCount;
    u64 copyStart = type->paramCount - (inputEnd - prevBlock->scopeBase);

    for(u64 inIndex = 0; inIndex < type->paramCount; inIndex++)
    {
        if(inIndex < copyStart)
        {
            //NOTE: if there wasn't enough operands on the stack,
            //      push fake ones with the correct type for that new block.
            //      This means this block is either unreachable, or will already have
            //      triggered a validation error.
            wa_operand_stack_push_unknown(context);
        }
        else
        {
            //NOTE copy block inputs on top of the stack
            wa_operand_stack_push_copy(context, paramStart + inIndex);
        }
    }
}

void wa_block_begin(wa_build_context* context, wa_instr* instr)
{
    oc_arena_scope scratch = oc_scratch_begin();
    wa_func_type* type = instr->blockType;

    //NOTE: check block type input (but we don't pop or use them here)
    wa_operand_stack_get_operands(scratch.arena,
                                  context,
                                  instr,
                                  type->paramCount,
                                  type->params,
                                  false);

    //NOTE allocate block results and push them on the stack
    for(u64 outIndex = 0; outIndex < type->returnCount; outIndex++)
    {
        wa_operand_stack_push_reg(context, type->returns[outIndex], instr);
        //TODO immediately put them in the freelist so they can be used in the branches (this might complicate copying results a bit...)
        // wa_free_slot(context, index);
    }

    wa_control_stack_push(context, instr);
    wa_push_block_inputs(context, type);

    oc_scratch_end(scratch);
}

void wa_block_move_results_to_input_slots(wa_build_context* context, wa_block* block, wa_instr* instr)
{
    oc_arena_scope scratch = oc_scratch_begin();
    wa_func_type* type = block->begin->blockType;

    wa_operand* opds = wa_operand_stack_get_operands(scratch.arena,
                                                     context,
                                                     instr,
                                                     type->paramCount,
                                                     type->params,
                                                     false);

    if(!block->polymorphic && !block->prevPolymorphic)
    {
        for(u64 paramIndex = 0; paramIndex < type->paramCount; paramIndex++)
        {
            wa_operand_slot* dst = &context->opdStack[block->scopeBase - type->returnCount - type->paramCount + paramIndex];
            wa_emit_opcode(context, WA_INSTR_move);
            wa_emit_index(context, opds[paramIndex].index);
            wa_emit_index(context, dst->index);
        }
    }
    oc_scratch_end(scratch);
}

void wa_block_move_results_to_output_slots(wa_build_context* context, wa_block* block, wa_instr* instr)
{
    oc_arena_scope scratch = oc_scratch_begin();
    wa_func_type* type = block->begin->blockType;

    wa_operand* opds = wa_operand_stack_get_operands(scratch.arena,
                                                     context,
                                                     instr,
                                                     type->returnCount,
                                                     type->returns,
                                                     false);

    if(!block->polymorphic && !block->prevPolymorphic)
    {
        for(u32 returnIndex = 0; returnIndex < type->returnCount; returnIndex++)
        {
            wa_operand_slot* dst = &context->opdStack[block->scopeBase - type->returnCount + returnIndex];
            wa_emit_opcode(context, WA_INSTR_move);
            wa_emit_index(context, opds[returnIndex].index);
            wa_emit_index(context, dst->index);
        }
    }
}

void wa_block_end(wa_build_context* context, wa_block* block, wa_instr* instr)
{
    if(block->begin->op == WA_INSTR_if && !block->begin->elseBranch)
    {
        //TODO: coalesce with case WA_INSTR_else in compile proc

        //NOTE: if there was no else branch, we must still generate a fake else branch to copy the block inputs to
        //      the output.
        wa_func_type* type = block->begin->blockType;

        wa_block_move_results_to_output_slots(context, block, instr);
        wa_operand_stack_pop_scope(context, block);
        wa_push_block_inputs(context, type);

        wa_emit_opcode(context, WA_INSTR_jump);
        wa_emit_i64(context, 0);

        block->polymorphic = false;
        block->begin->elseBranch = instr;
        block->elseOffset = context->codeLen;
    }

    wa_block_move_results_to_output_slots(context, block, instr);

    wa_func_type* type = block->begin->blockType;

    //NOTE - pop slots until scope is empty,
    //     - pop block scope
    //     - pop reserved outputs and saved params,
    //     - push result slots on top of stack

    wa_operand_stack_pop_scope(context, block);
    wa_control_stack_pop(context);

    //NOTE: here we keep return slots allocated by just decrementing stack len,
    //      and then we pop (and recycle) input slots. Then we just copy return
    //      slots to the top of the stack.
    //WARN: If we later decide to reuse reserved return slots inside block, we
    //      must re-mark them as allocated here too...

    context->opdStackLen -= type->returnCount;
    u64 returnSlotStart = context->opdStackLen;

    for(u64 index = 0; index < type->paramCount; index++)
    {
        wa_operand_stack_pop(context);
    }

    memcpy(&context->opdStack[context->opdStackLen],
           &context->opdStack[returnSlotStart],
           type->returnCount * sizeof(wa_operand_slot));

    context->opdStackLen += type->returnCount;

    if(block->begin->op == WA_INSTR_if)
    {
        OC_ASSERT(block->begin->elseBranch);

        //NOTE: patch conditional jump to else branch
        context->code[block->beginOffset + 1].valI64 = block->elseOffset - (block->beginOffset + 1);

        //NOTE: patch jump from end of if branch to end of else branch
        context->code[block->elseOffset - 1].valI64 = context->codeLen - (block->elseOffset - 1);
    }
}

void wa_patch_jump_targets(wa_build_context* context, wa_block* block)
{
    oc_list_for(block->jumpTargets, target, wa_jump_target, listElt)
    {
        context->code[target->offset].valI64 = context->codeLen - target->offset;
    }
}

int wa_compile_return(wa_build_context* context, wa_func_type* type, wa_instr* instr)
{
    oc_arena_scope scratch = oc_scratch_begin();

    //NOTE: typecheck the return operands, but don't use the result because we will potentally
    //      rewrite their register slots below
    wa_operand_stack_get_operands(scratch.arena,
                                  context,
                                  instr,
                                  type->returnCount,
                                  type->returns,
                                  false);

    //NOTE: move return operands to the beginning of the stack frame
    for(u32 retIndex = 0; retIndex < type->returnCount; retIndex++)
    {
        wa_operand opd = wa_operand_stack_lookup(context, type->returnCount - retIndex - 1);
        if(opd.index != retIndex)
        {
            //NOTE:
            //  if return operand isn't already stored at retIndex, we will move it there, so we need
            //  to save register retIndex to a new reg first if it's used in other return operands
            wa_move_register_if_used_in_stack_range(context, retIndex, type->returnCount - retIndex);

            wa_emit_opcode(context, WA_INSTR_move);
            wa_emit_index(context, opd.index);
            wa_emit_index(context, retIndex);
        }
    }
    wa_emit_opcode(context, WA_INSTR_return);

    //WARN: wa_compile_return() is also used by conditional or table branches when they target the top-level scope,
    //      so we don't set the stack polymorphic here. Instead we do it in the callers when return is unconditional
    oc_scratch_end(scratch);
    return 0;
}

void wa_compile_branch(wa_build_context* context, wa_instr* instr, u32 label)
{
    if(label + 1 == context->controlStackLen)
    {
        //Do a return
        wa_compile_return(context, context->exprType, instr);
    }
    else
    {
        wa_block* block = wa_control_stack_lookup(context, label);
        if(!block)
        {
            //TODO here we should pass the ast of the _immediate_, not the instruction
            wa_compile_error(context,
                             instr->ast,
                             "block level %u not found\n",
                             label);
            return;
        }

        if(block->begin->op == WA_INSTR_block
           || block->begin->op == WA_INSTR_if)
        {
            wa_block_move_results_to_output_slots(context, block, instr);

            //jump to end
            wa_emit_opcode(context, WA_INSTR_jump);

            //NOTE: emit a jump target operand for end of block
            wa_jump_target* target = oc_arena_push_type(&context->checkArena, wa_jump_target);
            target->offset = context->codeLen;
            oc_list_push_back(&block->jumpTargets, &target->listElt);

            wa_emit_i64(context, 0);
        }
        else if(block->begin->op == WA_INSTR_loop)
        {
            wa_block_move_results_to_input_slots(context, block, instr);

            //jump to begin
            wa_emit_opcode(context, WA_INSTR_jump);
            wa_emit_i64(context, block->beginOffset - context->codeLen);
        }
        else
        {
            OC_ASSERT(0, "unreachable");
        }
    }
}

void wa_build_context_clear(wa_build_context* context)
{
    //TODO: see why we can't just memset all (ie can't we just reconstruct the arenas)
    oc_arena_clear(&context->checkArena);

    context->codeLen = 0;

    context->opdStackLen = 0;
    context->opdStackCap = 0;

    context->controlStackLen = 0;
    context->controlStackCap = 0;

    context->regCount = 0;
    context->firstFreeReg = -1;

    context->currentFunction = 0;

    memset(context->registerMap, 0, sizeof(oc_list) * WA_MAX_REG);
    memset(context->registerMapCounts, 0, sizeof(u32) * WA_MAX_REG);
}

bool wa_validate_immediates(wa_build_context* context, wa_func* func, wa_instr* instr, const wa_instr_info* info)
{
    wa_module* module = context->module;

    bool check = true;
    //NOTE: validate immediates
    for(u32 immIndex = 0; immIndex < instr->immCount; immIndex++)
    {
        wa_immediate_type type;

        if(instr->op == WA_INSTR_select_t)
        {
            type = WA_IMM_VALUE_TYPE;
        }
        if(instr->op == WA_INSTR_br_table)
        {
            type = WA_IMM_LABEL;
        }
        else
        {
            type = info->imm[immIndex];
        }

        wa_code* imm = &instr->imm[immIndex];
        switch(type)
        {
            case WA_IMM_ZERO:
            {
                if(imm->valI32 != 0)
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "non zero value in 0x%x in zero immediate\n",
                                     (u32)imm->valI32);
                    check = false;
                }
            }
            break;

            case WA_IMM_VALUE_TYPE:
            {
                if(!wa_is_value_type(imm->valueType))
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "invalid value type 0x%x\n",
                                     (u32)imm->valueType);
                    check = false;
                }
            }
            break;
            case WA_IMM_REF_TYPE:
            {
                if(imm->valueType != WA_TYPE_FUNC_REF && imm->valueType != WA_TYPE_EXTERN_REF)
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "invalid reference type %x\n",
                                     (u32)imm->valueType);
                    check = false;
                }
            }
            break;
            case WA_IMM_LOCAL_INDEX:
            {
                if(imm->index >= func->localCount)
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "invalid local index %u (localCount: %u)\n",
                                     imm->index,
                                     func->localCount);
                    check = false;
                }
            }
            break;
            case WA_IMM_FUNC_INDEX:
            {
                if(imm->index >= module->functionCount)
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "invalid function index %u (function count: %u)\n",
                                     imm->index,
                                     module->functionCount);
                    check = false;
                }
            }
            break;
            case WA_IMM_GLOBAL_INDEX:
            {
                if(imm->index >= module->globalCount)
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "invalid global index %u (global count: %u)\n",
                                     imm->index,
                                     module->globalCount);
                    check = false;
                }
            }
            break;
            case WA_IMM_TYPE_INDEX:
            {
                if(imm->index >= module->typeCount)
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "invalid type index %u (type count: %u)\n",
                                     imm->index,
                                     module->typeCount);
                    check = false;
                }
            }
            break;

            case WA_IMM_TABLE_INDEX:
            {
                if(imm->index >= module->tableCount)
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "invalid table index %u (table count: %u)\n",
                                     imm->index,
                                     module->tableCount);
                    check = false;
                }
            }
            break;
            case WA_IMM_ELEM_INDEX:
            {
                if(imm->index >= module->elementCount)
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "invalid element index %u (element count: %u)\n",
                                     imm->index,
                                     module->elementCount);
                    check = false;
                }
            }
            break;
            case WA_IMM_DATA_INDEX:
            {
                if(imm->index >= module->dataCount)
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "invalid data index %u (data count: %u)\n",
                                     imm->index,
                                     module->dataCount);
                    check = false;
                }
            }
            break;

            default:
                break;
        }
    }
    return check;
}

void wa_compile_expression(wa_build_context* context, wa_func_type* type, wa_func* func, oc_list instructions)
{
    wa_module* module = context->module;

    context->exprType = type;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //TODO: remove the need to pass instr -- this will break else checks if first instr is an "if"...
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    wa_control_stack_push(context, oc_list_first_entry(instructions, wa_instr, listElt));

    oc_arena_scope scratch = oc_scratch_begin();

    oc_list_for(instructions, instr, wa_instr, listElt)
    {
        context->currentInstr = instr;

        oc_scratch_end(scratch);

        const wa_instr_info* info = &wa_instr_infos[instr->op];

        if(!wa_validate_immediates(context, func, instr, info))
        {
            //NOTE: skip validating the rest of the instruction to avoid using
            //      invalid indices.
            continue;
        }

        //NOTE: special case handling of control instructions
        if(instr->op == WA_INSTR_block || instr->op == WA_INSTR_loop)
        {
            wa_move_locals_to_registers(context);
            wa_block_begin(context, instr);
        }
        else if(instr->op == WA_INSTR_if)
        {
            wa_move_locals_to_registers(context);

            wa_operand* opd = wa_operand_stack_get_operands(scratch.arena,
                                                            context,
                                                            instr,
                                                            1,
                                                            (wa_value_type[]){ WA_TYPE_I32 },
                                                            true);
            wa_block_begin(context, instr);

            wa_emit_opcode(context, WA_INSTR_jump_if_zero);
            wa_emit_i64(context, 0);
            wa_emit_index(context, opd->index);
        }
        else if(instr->op == WA_INSTR_else)
        {
            wa_block* ifBlock = wa_control_stack_top(context);
            OC_ASSERT(ifBlock);

            if(ifBlock->begin->op != WA_INSTR_if
               || ifBlock->begin->elseBranch)
            {
                //TODO: should this be validated at parse stage?
                wa_compile_error(context,
                                 instr->ast,
                                 "unexpected else block\n");
            }
            else
            {
                wa_func_type* type = ifBlock->begin->blockType;

                wa_block_move_results_to_output_slots(context, ifBlock, instr);
                wa_operand_stack_pop_scope(context, ifBlock);
                wa_push_block_inputs(context, type);

                wa_emit_opcode(context, WA_INSTR_jump);
                wa_emit_i64(context, 0);

                ifBlock->polymorphic = false;
                ifBlock->begin->elseBranch = instr;
                ifBlock->elseOffset = context->codeLen;
            }
        }
        else if(instr->op == WA_INSTR_br)
        {
            u32 label = instr->imm[0].index;
            wa_compile_branch(context, instr, label);
            wa_block_set_polymorphic(context);
        }
        else if(instr->op == WA_INSTR_br_if)
        {
            wa_operand* opd = wa_operand_stack_get_operands(scratch.arena,
                                                            context,
                                                            instr,
                                                            1,
                                                            (wa_value_type[]){ WA_TYPE_I32 },
                                                            true);

            wa_emit_opcode(context, WA_INSTR_jump_if_zero);
            u64 jumpOffset = context->codeLen;
            wa_emit_i64(context, 0);
            wa_emit_index(context, opd->index);

            u32 label = instr->imm[0].index;
            wa_compile_branch(context, instr, label);

            context->code[jumpOffset].valI64 = context->codeLen - jumpOffset;
        }
        else if(instr->op == WA_INSTR_br_table)
        {
            wa_operand* opd = wa_operand_stack_get_operands(scratch.arena,
                                                            context,
                                                            instr,
                                                            1,
                                                            (wa_value_type[]){ WA_TYPE_I32 },
                                                            true);

            wa_emit_opcode(context, WA_INSTR_jump_table);
            u64 baseOffset = context->codeLen;

            wa_emit_index(context, instr->immCount);
            wa_emit_index(context, opd->index);

            u64* patchOffsets = oc_arena_push_array(scratch.arena, u64, instr->immCount);

            // reserve room for the table entries
            for(u32 i = 0; i < instr->immCount; i++)
            {
                patchOffsets[i] = context->codeLen;
                wa_emit_i64(context, 0);
            }

            // each entry jumps to a block that moves the results to the correct slots
            // and jumps to the actual destination
            //TODO: we can avoid this trampoline for branches that don't need result values
            for(u32 i = 0; i < instr->immCount; i++)
            {
                context->code[patchOffsets[i]].valI64 = context->codeLen - baseOffset;
                u32 label = instr->imm[i].index;
                wa_compile_branch(context, instr, label);
            }
            wa_block_set_polymorphic(context);
        }
        else if(instr->op == WA_INSTR_end)
        {
            wa_block* block = wa_control_stack_top(context);
            OC_ASSERT(block, "Unbalanced control stack.");

            if(context->controlStackLen == 1)
            {
                //TODO: is this sufficient to elide all previous returns?
                wa_instr* prev = oc_list_prev_entry(instr, wa_instr, listElt);
                if(!prev || prev->op != WA_INSTR_return)
                {
                    wa_compile_return(context, type, instr);
                }
                wa_patch_jump_targets(context, block);
                wa_control_stack_pop(context);

                OC_ASSERT(oc_list_last(instructions) == &instr->listElt);
            }
            else
            {
                wa_block_end(context, block, instr);
                wa_patch_jump_targets(context, block);
            }

            instr->codeIndex = context->codeLen;
        }
        else if(instr->op == WA_INSTR_call || instr->op == WA_INSTR_call_indirect)
        {
            //NOTE: compute max used slot
            //TODO: we could probably be more clever here, eg in some case just move the index operand
            //      past the arguments?
            i64 maxUsedSlot = func->localCount;

            for(u32 stackIndex = 0; stackIndex < context->opdStackLen; stackIndex++)
            {
                wa_operand_slot* opd = &context->opdStack[stackIndex];
                maxUsedSlot = oc_max((i64)opd->index, maxUsedSlot);
            }

            //NOTE: get callee type, and indirect index for call indirect
            wa_operand* indirectOpd = 0;
            wa_func* callee = 0;
            wa_func_type* type = 0;

            if(instr->op == WA_INSTR_call)
            {
                callee = &module->functions[instr->imm[0].index];
                type = callee->type;
            }
            else
            {
                indirectOpd = wa_operand_stack_get_operands(scratch.arena,
                                                            context,
                                                            instr,
                                                            1,
                                                            (wa_value_type[]){ WA_TYPE_I32 },
                                                            true);

                type = &module->types[instr->imm[0].index];
            }
            u32 paramCount = type->paramCount;

            //NOTE: put call args at the end of the stack
            //TODO: first check if args are already in order at the end of the frame?

            wa_operand* argOpds = wa_operand_stack_get_operands(scratch.arena,
                                                                context,
                                                                instr,
                                                                type->paramCount,
                                                                type->params,
                                                                true);

            for(u32 argIndex = 0; argIndex < paramCount; argIndex++)
            {
                wa_emit_opcode(context, WA_INSTR_move);
                wa_emit_index(context, argOpds[argIndex].index);
                wa_emit_index(context, maxUsedSlot + 1 + argIndex);
            }

            if(instr->op == WA_INSTR_call)
            {
                wa_emit_opcode(context, WA_INSTR_call);
                wa_emit_index(context, instr->imm[0].index);
                wa_emit_i64(context, maxUsedSlot + 1);
            }
            else
            {
                wa_emit_opcode(context, WA_INSTR_call_indirect);
                wa_emit_index(context, instr->imm[0].index);
                wa_emit_index(context, instr->imm[1].index);
                wa_emit_i64(context, maxUsedSlot + 1);
                wa_emit_index(context, indirectOpd->index);
            }

            wa_operand_stack_push_return_slots(context, maxUsedSlot, type->returnCount, type->returns);
        }
        else if(instr->op == WA_INSTR_return)
        {
            wa_compile_return(context, type, instr);

            wa_block* block = wa_control_stack_top(context);
            block->polymorphic = true;
            wa_operand_stack_pop_scope(context, block);
        }
        else
        {
            //NOTE: common codepath for all other instructions

            u32 immCount = instr->immCount;
            wa_code* imm = instr->imm;

            u32 inCount = info->inCount;
            wa_value_type* in = (wa_value_type*)info->in;

            u32 outCount = info->outCount;
            wa_value_type* out = (wa_value_type*)info->out;

            //NOTE: inputs/outputs types derived from immediates
            switch(instr->op)
            {
                case WA_INSTR_select_t:
                {
                    in = oc_arena_push_array(scratch.arena, wa_value_type, inCount);
                    in[0] = instr->imm[0].valueType;
                    in[1] = instr->imm[0].valueType;
                    in[2] = WA_TYPE_I32;
                }
                break;

                case WA_INSTR_if:
                {
                    inCount = instr->blockType->paramCount + 1;
                    in = oc_arena_push_array(scratch.arena, wa_value_type, inCount);
                    memcpy(in, instr->blockType->params, instr->blockType->paramCount * sizeof(wa_value_type));
                    in[inCount - 1] = WA_TYPE_I32;

                    wa_move_locals_to_registers(context);
                }
                break;

                case WA_INSTR_local_get:
                {
                    out = oc_arena_push_array(scratch.arena, wa_value_type, outCount);
                    u32 localIndex = imm[0].valU32;
                    out[0] = func->locals[localIndex];
                }
                break;
                case WA_INSTR_local_set:
                {
                    u32 localIndex = imm[0].valU32;
                    in = oc_arena_push_array(scratch.arena, wa_value_type, inCount);
                    in[0] = func->locals[localIndex];

                    //NOTE: check if the local was used in the stack and if so save it to a slot
                    //      this must be done before popping the stack to avoid saving the local
                    //      to the same reg as the operand (same for local_tee below).
                    wa_move_local_if_used(context, localIndex);
                }
                break;
                case WA_INSTR_local_tee:
                {
                    u32 localIndex = imm[0].valU32;
                    in = oc_arena_push_array(scratch.arena, wa_value_type, inCount);
                    in[0] = func->locals[localIndex];
                    out = oc_arena_push_array(scratch.arena, wa_value_type, outCount);
                    out[0] = func->locals[localIndex];

                    wa_move_local_if_used(context, localIndex);
                }
                break;
                case WA_INSTR_global_get:
                {
                    out = oc_arena_push_array(scratch.arena, wa_value_type, outCount);
                    u32 globalIndex = imm[0].valU32;
                    out[0] = module->globals[globalIndex].type;
                }
                break;
                case WA_INSTR_global_set:
                {
                    in = oc_arena_push_array(scratch.arena, wa_value_type, inCount);
                    u32 globalIndex = imm[0].valU32;
                    in[0] = module->globals[globalIndex].type;
                }
                break;
                case WA_INSTR_ref_null:
                {
                    immCount = 0;
                    out = oc_arena_push_array(scratch.arena, wa_value_type, outCount);
                    out[0] = imm[0].valueType;
                }
                break;
                case WA_INSTR_table_grow:
                {
                    u32 tableIndex = imm[0].valU32;
                    in = oc_arena_push_array(scratch.arena, wa_value_type, inCount);
                    in[0] = module->tables[tableIndex].type;
                    in[1] = WA_TYPE_I32;
                }
                break;
                case WA_INSTR_table_get:
                {
                    u32 tableIndex = imm[0].valU32;
                    out = oc_arena_push_array(scratch.arena, wa_value_type, outCount);
                    out[0] = module->tables[tableIndex].type;
                }
                break;
                case WA_INSTR_table_set:
                {
                    u32 tableIndex = imm[0].valU32;
                    in = oc_arena_push_array(scratch.arena, wa_value_type, inCount);
                    in[0] = WA_TYPE_I32;
                    in[1] = module->tables[tableIndex].type;
                }
                break;
                case WA_INSTR_table_fill:
                {
                    u32 tableIndex = imm[0].valU32;
                    in = oc_arena_push_array(scratch.arena, wa_value_type, inCount);
                    in[0] = WA_TYPE_I32;
                    in[1] = module->tables[tableIndex].type;
                    in[2] = WA_TYPE_I32;
                }
                break;

                case WA_INSTR_memory_size:
                case WA_INSTR_memory_grow:
                case WA_INSTR_memory_copy:
                case WA_INSTR_memory_fill:
                {
                    immCount = 0;
                }
                break;

                case WA_INSTR_memory_init:
                {
                    immCount = 1;
                }
                break;

                default:
                    break;
            }

            //NOTE: get inputs
            wa_operand* inOpds = wa_operand_stack_get_operands(scratch.arena,
                                                               context,
                                                               instr,
                                                               inCount,
                                                               in,
                                                               true);

            //NOTE: additional input checks
            if((instr->op >= WA_INSTR_i32_load && instr->op <= WA_INSTR_memory_grow)
               || instr->op == WA_INSTR_memory_init
               || instr->op == WA_INSTR_memory_copy
               || instr->op == WA_INSTR_memory_fill)
            {
                if(module->memoryCount == 0)
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "found memory instruction, but the module has no declared memory.\n");
                }
            }

            //NOTE: custom checks and emit
            if(instr->op == WA_INSTR_unreachable)
            {
                wa_emit_opcode(context, WA_INSTR_unreachable);
                wa_block_set_polymorphic(context);
            }
            else if(instr->op == WA_INSTR_drop
                    || instr->op == WA_INSTR_nop)
            {
                instr->codeIndex = context->codeLen;
                // do nothing
            }
            else if(instr->op == WA_INSTR_select
                    || instr->op == WA_INSTR_select_t)
            {
                if(!wa_check_operand_type(inOpds[0].type, inOpds[1].type))
                {
                    wa_compile_error(context, instr->ast, "select operands must be of same type\n");
                }
                out = oc_arena_push_array(scratch.arena, wa_value_type, outCount);
                out[0] = inOpds[0].type;

                u32 outIndex = wa_operand_stack_push_reg(context, out[0], instr);

                wa_emit_opcode(context, WA_INSTR_select);
                wa_emit_index(context, inOpds[0].index);
                wa_emit_index(context, inOpds[1].index);
                wa_emit_index(context, inOpds[2].index);
                wa_emit_index(context, outIndex);
            }
            else if(instr->op == WA_INSTR_local_get)
            {
                u32 localIndex = instr->imm[0].index;

                wa_operand_stack_push_local(context, localIndex);

                instr->codeIndex = context->codeLen;
            }
            else if(instr->op == WA_INSTR_local_set || instr->op == WA_INSTR_local_tee)
            {
                u32 localIndex = instr->imm[0].valU32;

                //TODO: check if the local was written to since the value was pushed, and if not, change
                //      the output operand of the value's origin instruction rather than issuing a move.
                //WARN:  this can't be used after a branch, since the branch might use the original slot index
                //      so we'd need to add a "touched" bit and set it for operands used in a branch?
                wa_emit_opcode(context, WA_INSTR_move);
                wa_emit_index(context, inOpds[0].index);
                wa_emit_index(context, localIndex);

                if(instr->op == WA_INSTR_local_tee)
                {
                    wa_operand_stack_push_local(context, localIndex);
                }
            }
            else if(instr->op == WA_INSTR_global_get)
            {
                u32 globalIndex = instr->imm[0].valU32;

                u32 regIndex = wa_operand_stack_push_reg(context,
                                                         module->globals[globalIndex].type,
                                                         instr);

                wa_emit_opcode(context, WA_INSTR_global_get);
                wa_emit_index(context, globalIndex);
                wa_emit_index(context, regIndex);
            }
            else if(instr->op == WA_INSTR_global_set)
            {
                u32 globalIndex = instr->imm[0].valU32;
                wa_emit_opcode(context, WA_INSTR_global_set);
                wa_emit_index(context, globalIndex);
                wa_emit_index(context, inOpds[0].index);
            }
            else
            {
                //NOTE generic emit code
                wa_emit_opcode(context, instr->op);

                for(int immIndex = 0; immIndex < immCount; immIndex++)
                {
                    wa_emit(context, &instr->imm[immIndex]);
                }

                for(u32 i = 0; i < inCount; i++)
                {
                    wa_emit_index(context, inOpds[i].index);
                }

                for(int opdIndex = 0; opdIndex < outCount; opdIndex++)
                {
                    u32 outIndex = wa_operand_stack_push_reg(context, out[opdIndex], instr);
                    wa_emit_index(context, outIndex);
                }
            }
        }
    }

    //NOTE: clear remaining tmp data if we exited the loop early
    oc_scratch_end(scratch);
}

void wa_compile_code(oc_arena* arena, wa_module* module)
{
    module->debugInfo.registerMaps = oc_arena_push_array(module->arena, wa_register_map*, module->functionCount);
    memset(module->debugInfo.registerMaps, 0, sizeof(wa_register_map) * module->functionCount);

    wa_build_context context = {
        .arena = arena,
        .module = module,
    };
    oc_arena_init(&context.codeArena);
    oc_arena_init(&context.checkArena);

    context.codeCap = 4;
    context.code = oc_arena_push_array(&context.codeArena, wa_code, 4);

    for(u32 funcIndex = module->functionImportCount; funcIndex < module->functionCount; funcIndex++)
    {
        wa_func* func = &module->functions[funcIndex];

        wa_build_context_clear(&context);

        context.regCount = func->localCount;
        for(u32 localIndex = 0; localIndex < func->localCount; localIndex++)
        {
            context.regs[localIndex].refCount = 0;
            context.regs[localIndex].type = func->locals[localIndex];
        }

        context.currentFunction = func;

        wa_compile_expression(&context, func->type, func, func->instructions);

        func->codeLen = context.codeLen;
        func->code = oc_arena_push_array(arena, wa_code, context.codeLen);
        memcpy(func->code, context.code, context.codeLen * sizeof(wa_code));

        if(context.regCount >= WA_MAX_SLOT_COUNT)
        {
            wa_compile_error(&context, 0, "too many register slots (%i, max is %i).", context.regCount, WA_MAX_SLOT_COUNT);
        }
        func->maxRegCount = context.regCount;

        //NOTE: emit wasm to warm mappings
        {
            u64 codeIndex = func->codeLen - 1;
            oc_list_for_reverse(func->instructions, instr, wa_instr, listElt)
            {
                if(instr->codeIndex)
                {
                    codeIndex = instr->codeIndex;
                }
                wa_wasm_to_warm_loc_push(module, funcIndex, codeIndex, instr);
            }
        }

        //NOTE: add register mappings for locals
        for(u32 localIndex = 0; localIndex < func->localCount; localIndex++)
        {
            wa_register_mapping_push(&context, localIndex, 0, context.codeLen, func->locals[localIndex]);
        }

        //NOTE: collect register mappings
        module->debugInfo.registerMaps[funcIndex] = oc_arena_push_array(module->arena, wa_register_map, func->maxRegCount);
        for(u32 regIndex = 0; regIndex < func->maxRegCount; regIndex++)
        {
            wa_register_map* map = &module->debugInfo.registerMaps[funcIndex][regIndex];
            map->count = context.registerMapCounts[regIndex];
            map->ranges = oc_arena_push_array(module->arena, wa_register_range, map->count);

            oc_list_for_indexed(context.registerMap[regIndex], it, wa_register_range_elt, listElt)
            {
                map->ranges[it.index] = it.elt->range;
            }
        }
    }

    for(u32 globalIndex = module->globalImportCount; globalIndex < module->globalCount; globalIndex++)
    {
        wa_global_desc* global = &module->globals[globalIndex];

        wa_build_context_clear(&context);
        context.regCount = 0;

        i64 t = 0x7f - (i64)global->type + 1;
        wa_func_type* exprType = (wa_func_type*)&WA_BLOCK_VALUE_TYPES[t];

        wa_compile_expression(&context, exprType, 0, global->init);

        global->codeLen = context.codeLen;
        global->code = oc_arena_push_array(arena, wa_code, context.codeLen);
        memcpy(global->code, context.code, context.codeLen * sizeof(wa_code));
    }

    for(u32 eltIndex = 0; eltIndex < module->elementCount; eltIndex++)
    {
        wa_element* element = &module->elements[eltIndex];

        if(!oc_list_empty(element->tableOffset))
        {
            ///////////////////////////////////////////////////////////////////////////
            //TODO: this should go in wa_compile_expression to avoid forgetting it?
            ///////////////////////////////////////////////////////////////////////////
            wa_build_context_clear(&context);
            context.regCount = 0;

            wa_compile_expression(&context, (wa_func_type*)&WA_BLOCK_VALUE_TYPES[1], 0, element->tableOffset);
            element->tableOffsetCode = oc_arena_push_array(arena, wa_code, context.codeLen);
            memcpy(element->tableOffsetCode, context.code, context.codeLen * sizeof(wa_code));
        }

        if(element->initCount)
        {
            element->code = oc_arena_push_array(arena, wa_code*, element->initCount);
            for(u32 exprIndex = 0; exprIndex < element->initCount; exprIndex++)
            {
                wa_build_context_clear(&context);
                context.regCount = 0;

                i64 t = 0x7f - (i64)element->type + 1;
                wa_func_type* exprType = (wa_func_type*)&WA_BLOCK_VALUE_TYPES[t];

                OC_ASSERT(!oc_list_empty(element->initInstr[exprIndex]));

                wa_compile_expression(&context, exprType, 0, element->initInstr[exprIndex]);

                element->code[exprIndex] = oc_arena_push_array(arena, wa_code, context.codeLen);
                memcpy(element->code[exprIndex], context.code, context.codeLen * sizeof(wa_code));
            }
        }
    }

    for(u32 dataIndex = 0; dataIndex < module->dataCount; dataIndex++)
    {
        wa_data_segment* seg = &module->data[dataIndex];

        if(!oc_list_empty(seg->memoryOffset))
        {
            wa_build_context_clear(&context);
            context.regCount = 0;

            wa_compile_expression(&context, (wa_func_type*)&WA_BLOCK_VALUE_TYPES[1], 0, seg->memoryOffset);
            seg->memoryOffsetCode = oc_arena_push_array(arena, wa_code, context.codeLen);
            memcpy(seg->memoryOffsetCode, context.code, context.codeLen * sizeof(wa_code));
        }
    }

    oc_arena_cleanup(&context.codeArena);
    oc_arena_cleanup(&context.checkArena);
}
