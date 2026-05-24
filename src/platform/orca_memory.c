/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "platform_memory.h"
#include "wasmbind/hostcalls.h"

void* orca_oc_platform_memory_reserve(oc_platform_memory* context, u64 size)
{
    return ((void*)oc_hostcall_mem_grow(size));
}

void orca_oc_base_nop(oc_platform_memory* context, void* ptr, u64 size) {}

oc_platform_memory* oc_platform_memory_default()
{
    static oc_platform_memory base = { 0 };
    if(base.reserve == 0)
    {
        base.reserve = orca_oc_platform_memory_reserve;
        base.commit = orca_oc_base_nop;
        base.decommit = orca_oc_base_nop;
        base.release = orca_oc_base_nop;
    }
    return (&base);
}
