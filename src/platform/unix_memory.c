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
void oc_base_nop(oc_base_allocator* context UNUSED, void* ptr UNUSED, u64 size UNUSED) {}

void* oc_base_reserve_mmap(oc_base_allocator* context UNUSED, u64 size)
{
    return (mmap(0, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0));
}

void oc_base_release_mmap(oc_base_allocator* context UNUSED, void* ptr, u64 size)
{
    munmap(ptr, size);
}

oc_base_allocator* oc_base_allocator_default()
{
    static oc_base_allocator base = {};
    if(base.reserve == 0)
    {
        base.reserve = oc_base_reserve_mmap;
        base.commit = oc_base_nop;
        base.decommit = oc_base_nop;
        base.release = oc_base_release_mmap;
    }
    return (&base);
}
