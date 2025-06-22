/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "platform_memory.h"
#include <sys/mman.h>

void* oc_base_reserve_mmap(oc_base_allocator* context UNUSED, u64 size)
{
    void* ptr = mmap(NULL, size, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if(ptr == MAP_FAILED)  return NULL;
    return ptr;
}

void oc_base_commit_mmap(oc_base_allocator* content UNUSED, void* ptr, u64 size)
{
    int err = mprotect(ptr, size, PROT_READ | PROT_WRITE);
    OC_ASSERT(!err);
}

void oc_base_decommit_mmap(oc_base_allocator* content UNUSED, void* ptr, u64 size)
{
    int err = mprotect(ptr, size, PROT_NONE);
    OC_ASSERT(!err);
    err = madvise(ptr, size, MADV_DONTNEED);
    OC_ASSERT(!err);
}

void oc_base_release_mmap(oc_base_allocator* context UNUSED, void* ptr, u64 size)
{
    int err = munmap(ptr, size);
    OC_ASSERT(!err);
}

static oc_base_allocator base_allocator = {
    .reserve = oc_base_reserve_mmap,
    .commit = oc_base_commit_mmap,
    .decommit = oc_base_decommit_mmap,
    .release = oc_base_release_mmap,
};

oc_base_allocator* oc_base_allocator_default()
{
    return (&base_allocator);
}
