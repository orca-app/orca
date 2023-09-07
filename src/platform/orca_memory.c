/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "platform_memory.h"

void* ORCA_IMPORT(oc_mem_grow)(u64 size);

void* orca_oc_base_reserve(oc_base_allocator* context, u64 size)
{
    return (oc_mem_grow(size));
}

void orca_oc_base_nop(oc_base_allocator* context, void* ptr, u64 size) {}

oc_base_allocator* oc_base_allocator_default()
{
    static oc_base_allocator base = { 0 };
    if(base.reserve == 0)
    {
        base.reserve = orca_oc_base_reserve;
        base.commit = orca_oc_base_nop;
        base.decommit = orca_oc_base_nop;
        base.release = orca_oc_base_nop;
    }
    return (&base);
}
