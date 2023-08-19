/************************************************************/ /**
*
*	@file: win32_memory.c
*	@author: Martin Fouilleul
*	@date: 17/12/2022
*	@revision:
*
*****************************************************************/
#define WIN32_LEAN_AND_MEAN
#include "platform_memory.h"
#include <windows.h>

void* oc_base_reserve_win32(oc_base_allocator* context, u64 size)
{
    void* result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
    return (result);
}

void oc_base_commit_win32(oc_base_allocator* context, void* ptr, u64 size)
{
    VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
}

void oc_base_release_win32(oc_base_allocator* context, void* ptr, u64 size)
{
    VirtualFree(ptr, size, MEM_RELEASE);
}

void oc_base_decommit_win32(oc_base_allocator* context, void* ptr, u64 size)
{
    VirtualFree(ptr, size, MEM_DECOMMIT);
}

oc_base_allocator* oc_base_allocator_default()
{
    static oc_base_allocator base = { 0 };
    if(base.reserve == 0)
    {
        base.reserve = oc_base_reserve_win32;
        base.commit = oc_base_commit_win32;
        base.decommit = oc_base_decommit_win32;
        base.release = oc_base_release_win32;
    }
    return (&base);
}
