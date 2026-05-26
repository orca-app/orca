/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "platform.h"
#include "util/typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------------------
//NOTE(martin): platform base memory allocator
//--------------------------------------------------------------------------------
typedef struct oc_platform_memory oc_platform_memory;

typedef void* (*oc_mem_reserve_proc)(oc_platform_memory* context, u64 size);
typedef void (*oc_mem_modify_proc)(oc_platform_memory* context, void* ptr, u64 size);

typedef struct oc_platform_memory
{
    oc_mem_reserve_proc reserve;
    oc_mem_modify_proc commit;
    oc_mem_modify_proc decommit;
    oc_mem_modify_proc release;

} oc_platform_memory;

ORCA_API oc_platform_memory* oc_platform_memory_default(void);

#define oc_platform_memory_reserve(mem, size) mem->reserve(mem, size)
#define oc_platform_memory_commit(mem, ptr, size) mem->commit(mem, ptr, size)
#define oc_platform_memory_decommit(mem, ptr, size) mem->decommit(mem, ptr, size)
#define oc_platform_memory_release(mem, ptr, size) mem->release(mem, ptr, size)

//--------------------------------------------------------------------------------
//NOTE(martin): malloc/free
//--------------------------------------------------------------------------------
#include <stdlib.h>

#define oc_malloc_type(type) ((type*)malloc(sizeof(type)))
#define oc_malloc_array(type, count) ((type*)malloc(sizeof(type) * count))

//--------------------------------------------------------------------------------
//NOTE(martin): memset / memcpy
//--------------------------------------------------------------------------------

#include <string.h>

#ifdef __cplusplus
} // extern "C"
#endif
