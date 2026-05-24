/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "platform_memory.h"
#include <sys/mman.h>

/*NOTE(martin):
	Linux and MacOS don't make a distinction between reserved and committed memory, contrary to Windows
*/
void oc_base_nop(oc_platform_memory* context, void* ptr, u64 size) {}

void* oc_platform_memory_reserve_mmap(oc_platform_memory* context, u64 size)
{
    return (mmap(0, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0));
}

void oc_platform_memory_release_mmap(oc_platform_memory* context, void* ptr, u64 size)
{
    munmap(ptr, size);
}

oc_platform_memory* oc_platform_memory_default()
{
    static oc_platform_memory base = {};
    if(base.reserve == 0)
    {
        base.reserve = oc_platform_memory_reserve_mmap;
        base.commit = oc_base_nop;
        base.decommit = oc_base_nop;
        base.release = oc_platform_memory_release_mmap;
    }
    return (&base);
}
