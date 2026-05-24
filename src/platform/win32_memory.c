/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#define WIN32_LEAN_AND_MEAN
#include "platform_memory.h"
#include <windows.h>

void* oc_platform_memory_reserve_win32(oc_platform_memory* context, u64 size)
{
    void* result = 0;
    if(size)
    {
        result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
    }
    return (result);
}

void oc_platform_memory_commit_win32(oc_platform_memory* context, void* ptr, u64 size)
{
    if(size)
    {
        void* res = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
        OC_DEBUG_ASSERT(res);
    }
}

void oc_platform_memory_release_win32(oc_platform_memory* context, void* ptr, u64 size)
{
    BOOL res = VirtualFree(ptr, 0, MEM_RELEASE);
    OC_DEBUG_ASSERT(res);
}

void oc_platform_memory_decommit_win32(oc_platform_memory* context, void* ptr, u64 size)
{
    BOOL res = VirtualFree(ptr, size, MEM_DECOMMIT);
    OC_DEBUG_ASSERT(res);
}

oc_platform_memory* oc_platform_memory_default()
{
    static oc_platform_memory base = { 0 };
    if(base.reserve == 0)
    {
        base.reserve = oc_platform_memory_reserve_win32;
        base.commit = oc_platform_memory_commit_win32;
        base.decommit = oc_platform_memory_decommit_win32;
        base.release = oc_platform_memory_release_win32;
    }
    return (&base);
}
