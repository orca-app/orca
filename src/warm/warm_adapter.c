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

void wa_module_destroy(wa_module* module)
{
    //NOTE: do nothing, everything is done when arena is cleared
}

void wa_instance_destroy(wa_instance* instance)
{
    //NOTE: do nothing, everything is done when arena is cleared
}

wa_memory wa_instance_get_memory(wa_instance* instance)
{
    return *instance->memories[0];
}

wa_status wa_instance_resize_memory(wa_instance* instance, u32 n)
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
