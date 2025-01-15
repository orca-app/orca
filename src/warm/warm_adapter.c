/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "orca.h"
#include "wasm/wasm.h"
#include "warm.c"

u64 oc_wasm_mem_size(wa_instance* instance)
{
    return (instance->memories[0]->limits.min * WA_PAGE_SIZE);
}

oc_str8 oc_wasm_mem_get(wa_instance* instance)
{
    return (oc_str8){
        .ptr = instance->memories[0]->ptr,
        .len = instance->memories[0]->limits.min * WA_PAGE_SIZE,
    };
}

wa_status oc_wasm_mem_resize(wa_instance* instance, u32 n)
{
    wa_memory* mem = instance->memories[0];

    if(n <= mem->limits.max
       && (n >= mem->limits.min))
    {
        oc_base_allocator* allocator = oc_base_allocator_default();
        oc_base_commit(allocator, mem->ptr, n * WA_PAGE_SIZE);
        mem->limits.min = n;
        return WA_OK;
    }
    return WA_TRAP_MEMORY_OUT_OF_BOUNDS;
}

wa_value wa_global_get(wa_instance* instance, wa_global* global)
{
    wa_value value = { 0 };
    if(global)
    {
        value = global->value;
    }
    return value;
}

void wa_global_set(wa_instance* instance, wa_global* global, wa_value value)
{
    if(global)
    {
        global->value = value;
    }
}

/*
wa_global* wa_instance_find_global(wa_instance* instance, oc_str8 exportName)
{
    oc_wasm_global_pointer res = { 0 };
    wa_global* global = wa_instance_find_global(instance, exportName);
    if(global && global->type == WA_TYPE_I32)
    {
        res.handle = (oc_wasm_global_handle*)global;
        res.address = global->value.valI32;
    }
    return (res);
}
*/
