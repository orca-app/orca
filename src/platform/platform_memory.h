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
//NOTE(martin): base allocator
//--------------------------------------------------------------------------------
typedef struct oc_base_allocator oc_base_allocator;

typedef void* (*oc_mem_reserve_proc)(oc_base_allocator* context, u64 size);
typedef void (*oc_mem_modify_proc)(oc_base_allocator* context, void* ptr, u64 size);

typedef struct oc_base_allocator
{
    oc_mem_reserve_proc reserve;
    oc_mem_modify_proc commit;
    oc_mem_modify_proc decommit;
    oc_mem_modify_proc release;

} oc_base_allocator;

ORCA_API oc_base_allocator* oc_base_allocator_default(void);

#define oc_base_reserve(base, size) base->reserve(base, size)
#define oc_base_commit(base, ptr, size) base->commit(base, ptr, size)
#define oc_base_decommit(base, ptr, size) base->decommit(base, ptr, size)
#define oc_base_release(base, ptr, size) base->release(base, ptr, size)

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
