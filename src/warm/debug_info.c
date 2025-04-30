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

wa_debug_info* wa_debug_info_create(wa_module* module, oc_str8 contents)
{
    wa_debug_info* info = oc_arena_push_type(module->arena, wa_debug_info);

    //NOTE: alloc warm to wasm maps
    //TODO: tune size
    info->warmToWasmMapLen = 4096;
    info->warmToWasmMap = oc_arena_push_array(module->arena, oc_list, 4096);

    info->wasmToWarmMapLen = 4096;
    info->wasmToWarmMap = oc_arena_push_array(module->arena, oc_list, 4096);

    //NOTE: parse and process dwarf info from contents

    //TODO: pass debug info to parse dwarf?
    module->debugInfo = info;

    wa_import_dwarf(module, contents);
    wa_import_debug_locals(module);

    return info;
}

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
    return (result);
}

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

dw_stack_value wa_interpret_dwarf_expr(wa_interpreter* interpreter, wa_debug_function* funcInfo, oc_str8 expr)
{
    u64 sp = 0;

    const u64 DW_STACK_MAX = 1024;
    dw_stack_value stack[DW_STACK_MAX];

    wa_reader reader = {
        .contents = expr,
    };

    while(wa_reader_has_more(&reader))
    {
        dw_op op = wa_read_u8(&reader);

        switch(op)
        {
            case DW_OP_addr:
            {
                u32 opd = wa_read_u32(&reader);

                stack[sp] = (dw_stack_value){
                    .type = DW_STACK_VALUE_ADDRESS,
                    .valU32 = opd,
                };
                sp++;
            }
            break;

            case DW_OP_fbreg:
            {
                i64 offset = wa_read_leb128_i64(&reader);

                OC_ASSERT(funcInfo->frameBase->single && funcInfo->frameBase->entryCount == 1);
                dw_stack_value frameBase = wa_interpret_dwarf_expr(interpreter, funcInfo, funcInfo->frameBase->entries[0].desc);

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
                u8 kind = wa_read_u8(&reader);
                switch(kind)
                {
                    case 0x00:
                    {
                        //NOTE: wasm local
                        u32 index = wa_read_leb128_u32(&reader);
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
                        u32 index = wa_read_leb128_u32(&reader);
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
                        u32 index = wa_read_leb128_u32(&reader);
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
                        u32 index = wa_read_u32(&reader);
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
                oc_log_error("unsupported dwarf op %s\n", dw_op_get_string(op));
                goto end;
        }
    }

end:
    OC_ASSERT(sp > 0);
    return (stack[sp - 1]);
}

oc_str8 wa_debug_variable_get_value(oc_arena* arena, wa_interpreter* interpreter, wa_debug_function* funcInfo, wa_debug_variable* var)
{
    if(!var->loc)
    {
        return (oc_str8){ 0 };
    }
    wa_debug_type* type = wa_debug_type_strip(var->type);

    oc_str8 res = {
        .len = type->size,
        .ptr = oc_arena_push_aligned(arena, type->size, 8),
    };

    dw_loc* loc = var->loc;

    for(u64 entryIndex = 0; entryIndex < loc->entryCount; entryIndex++)
    {
        dw_loc_entry* entry = &loc->entries[entryIndex];

        dw_stack_value val = wa_interpret_dwarf_expr(interpreter, funcInfo, entry->desc);

        switch(val.type)
        {
            case DW_STACK_VALUE_ADDRESS:
            {
                //TODO: bounds check
                if(loc->single)
                {
                    memcpy(res.ptr, interpreter->instance->memories[0]->ptr + val.valU32, res.len);
                }
                else
                {
                    memcpy(res.ptr + entry->start, interpreter->instance->memories[0]->ptr + val.valU32, entry->end - entry->start);
                }
            }
            break;

            default:
                break;
        }
    end:
        continue;
    }

    return res;
}
