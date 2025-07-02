/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "reader.h"
#include "dwarf.h"
#include "debug_info.h"
#include "warm.h"

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
    u64 index = hash % loc.module->debugInfo->warmToWasmMapLen;

    wa_instr* instr = 0;
    oc_list_for(loc.module->debugInfo->warmToWasmMap[index], mapping, wa_bytecode_mapping, listElt)
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
        result.offset = instr->loc.start;
    }
    OC_DEBUG_ASSERT(result.module);

    return (result);
}

/*NOTE:
    - for each emitted warm opcode, we have a corresponding wasm loc (see wa_emit_opcode) stored in warmToWasmMap
    - for each wasm instruction, we have a corresponding warm index (even if not an exact match) in wasmToWarmMap
*/

wa_warm_loc wa_warm_loc_from_wasm_loc(wa_wasm_loc loc)
{
    wa_warm_loc result = { 0 };

    u64 id = loc.offset;
    u64 hash = oc_hash_xx64_string((oc_str8){ .ptr = (char*)&id, .len = 8 });
    u64 index = hash % loc.module->debugInfo->wasmToWarmMapLen;

    wa_instr* instr = 0;
    oc_list_for(loc.module->debugInfo->wasmToWarmMap[index], mapping, wa_bytecode_mapping, listElt)
    {
        if(mapping->instr->loc.start == loc.offset)
        {
            result.module = loc.module;
            result.funcIndex = mapping->funcIndex;
            result.codeIndex = mapping->codeIndex;
            break;
        }
    }
    OC_DEBUG_ASSERT(result.module);

    return (result);
}

wa_line_loc wa_line_loc_from_warm_loc(wa_module* module, wa_warm_loc loc)
{
    wa_line_loc res = { 0 };
    wa_wasm_loc wasmLoc = wa_wasm_loc_from_warm_loc(loc);

    for(u64 entryIndex = 0; entryIndex < module->debugInfo->wasmToLineCount; entryIndex++)
    {
        wa_wasm_to_line_entry* entry = &module->debugInfo->wasmToLine[entryIndex];
        if(entry->wasmOffset > wasmLoc.offset)
        {
            if(entryIndex)
            {
                res = module->debugInfo->wasmToLine[entryIndex - 1].loc;
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

    for(u64 entryIndex = 0; entryIndex < module->debugInfo->wasmToLineCount; entryIndex++)
    {
        wa_wasm_to_line_entry* entry = &module->debugInfo->wasmToLine[entryIndex];

        if(loc.fileIndex == entry->loc.fileIndex
           && entry->loc.line >= loc.line
           && entry->loc.line < currentLine)
        {
            currentLine = entry->loc.line;
            wasmLoc.module = module;
            wasmLoc.offset = entry->wasmOffset;
        }
    }
    //NOTE: we always have a wasm loc here, even if not an _exact_ match

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

typedef enum wa_debug_loc_error
{
    WA_DEBUG_LOC_NO_FRAMEBASE,
    WA_DEBUG_LOC_UNKNOWN_OP,
    //...
} wa_debug_loc_error;

typedef oc_result(dw_stack_value, wa_debug_loc_error) dw_stack_value_result;

dw_stack_value_result wa_interpret_dwarf_expr(wa_interpreter* interpreter, wa_debug_function* funcInfo, dw_expr expr)
{
    u64 sp = 0;
    u64 pc = 0;

    const u64 DW_STACK_MAX = 1024;
    dw_stack_value stack[DW_STACK_MAX];

    while(pc < expr.codeLen)
    {
        dw_expr_instr* instr = &expr.code[pc];
        pc++;

        switch(instr->op)
        {
            case DW_OP_addr:
            {
                u32 opd = instr->operands[0].valU32;

                stack[sp] = (dw_stack_value){
                    .type = DW_STACK_VALUE_ADDRESS,
                    .valU32 = opd,
                };
                sp++;
            }
            break;

            case DW_OP_fbreg:
            {
                i64 offset = instr->operands[0].valI64;

                dw_loc* frameBaseLoc = oc_catch(funcInfo->frameBase)
                {
                    return oc_wrap_error(dw_stack_value_result, WA_DEBUG_LOC_NO_FRAMEBASE);
                }
                if(!frameBaseLoc->single || frameBaseLoc->entryCount != 1)
                {
                    return oc_wrap_error(dw_stack_value_result, WA_DEBUG_LOC_NO_FRAMEBASE);
                }

                dw_stack_value_result frameBaseResult = wa_interpret_dwarf_expr(interpreter, funcInfo, frameBaseLoc->entries[0].expr);
                dw_stack_value frameBase = oc_catch(frameBaseResult)
                {
                    return frameBaseResult;
                }

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
                u8 kind = instr->operands[0].valU8;
                switch(kind)
                {
                    case 0x00:
                    {
                        //NOTE: wasm local
                        u32 index = instr->operands[1].valU64;
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
                        u32 index = instr->operands[1].valU64;
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
                        u32 index = instr->operands[1].valU64;
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
                        u32 index = instr->operands[1].valU32;
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
                oc_log_error("unsupported dwarf op %s\n", dw_op_get_string(instr->op));
                return oc_wrap_error(dw_stack_value_result, WA_DEBUG_LOC_UNKNOWN_OP);
        }
    }

end:
    OC_ASSERT(sp > 0);
    return oc_wrap_value(dw_stack_value_result, stack[sp - 1]);
}

oc_str8 wa_debug_variable_get_value(oc_arena* arena, wa_interpreter* interpreter, wa_debug_function* funcInfo, wa_debug_variable* var)
{
    dw_loc* loc = oc_catch(var->loc)
    {
        return (oc_str8){ 0 };
    }

    wa_type* type = wa_type_strip(var->type);

    oc_str8 res = {
        .len = type->size,
        .ptr = oc_arena_push_aligned(arena, type->size, 8),
    };

    for(u64 entryIndex = 0; entryIndex < loc->entryCount; entryIndex++)
    {
        dw_loc_entry* entry = &loc->entries[entryIndex];

        dw_stack_value val = oc_catch(wa_interpret_dwarf_expr(interpreter, funcInfo, entry->expr))
        {
            return (oc_str8){ 0 };
        }

        switch(val.type)
        {
            case DW_STACK_VALUE_ADDRESS:
            {
                if(loc->single)
                {
                    if(val.valU32 + res.len > interpreter->instance->memories[0]->limits.min * WA_PAGE_SIZE
                       || val.valU32 + res.len < val.valU32)
                    {
                        return (oc_str8){ 0 };
                    }
                    else
                    {
                        memcpy(res.ptr, interpreter->instance->memories[0]->ptr + val.valU32, res.len);
                    }
                }
                else
                {

                    if(entry->end < entry->start)
                    {
                        //NOTE: error, faulty entry
                        return (oc_str8){ 0 };
                    }
                    else
                    {
                        u64 len = entry->end - entry->start;
                        if(entry->start + len > res.len || entry->start + len < entry->start)
                        {
                            //NOTE: error, write out of bounds of res.
                            return (oc_str8){ 0 };
                        }
                        else if(val.valU32 + len > interpreter->instance->memories[0]->limits.min * WA_PAGE_SIZE
                                || val.valU32 + len < val.valU32)
                        {
                            //NOTE: error, read out of bounds of memory
                            return (oc_str8){ 0 };
                        }
                        else
                        {
                            memcpy(res.ptr + entry->start, interpreter->instance->memories[0]->ptr + val.valU32, entry->end - entry->start);
                        }
                    }
                }
            }
            break;

            default:
                return (oc_str8){ 0 };
        }
    end:
        continue;
    }

    return res;
}

wa_debug_scope_ptr_option wa_debug_get_scope_for_warm_loc(wa_interpreter* interpreter, wa_warm_loc warmLoc)
{
    wa_wasm_loc loc = wa_wasm_loc_from_warm_loc(warmLoc);

    wa_debug_function* funcInfo = oc_catch(interpreter->instance->module->debugInfo->functions[warmLoc.funcIndex])
    {
        return oc_wrap_nil(wa_debug_scope_ptr_option);
    }

    wa_debug_scope* scope = &funcInfo->body;

    while(!oc_list_empty(scope->children))
    {
        wa_debug_scope* next = 0;
        oc_list_for(scope->children, child, wa_debug_scope, listElt)
        {
            for(u64 rangeIndex = 0; rangeIndex < child->rangeList.count; rangeIndex++)
            {
                wa_debug_range* range = &child->rangeList.ranges[rangeIndex];
                if(loc.offset >= range->low && loc.offset < range->high)
                {
                    next = child;
                    break;
                }
            }
        }
        if(!next)
        {
            break;
        }
        scope = next;
    }
    OC_DEBUG_ASSERT(scope);
    return oc_wrap_ptr(wa_debug_scope_ptr_option, scope);
}

wa_debug_unit_ptr_option wa_debug_get_unit_for_warm_loc(wa_interpreter* interpreter, wa_warm_loc warmLoc)
{
    wa_debug_info* debugInfo = interpreter->instance->module->debugInfo;
    wa_wasm_loc loc = wa_wasm_loc_from_warm_loc(warmLoc);

    wa_debug_unit* unit = 0;

    wa_debug_function_ptr_option funcOption = debugInfo->functions[warmLoc.funcIndex];
    if(oc_check(funcOption))
    {
        wa_debug_function* func = oc_unwrap(funcOption);
        unit = func->unit;
    }
    else
    {
        for(u64 unitIndex = 0; unitIndex < debugInfo->unitCount; unitIndex++)
        {
            wa_debug_unit* candidate = &debugInfo->units[unitIndex];
            for(u64 rangeIndex = 0; rangeIndex < candidate->rangeList.count; rangeIndex++)
            {
                wa_debug_range* range = &candidate->rangeList.ranges[rangeIndex];
                if(loc.offset >= range->low && loc.offset < range->high)
                {
                    unit = candidate;
                    break;
                }
            }
            if(unit)
            {
                break;
            }
        }
    }
    return oc_wrap_ptr(wa_debug_unit_ptr_option, unit);
}
