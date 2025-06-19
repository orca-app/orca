/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "platform/platform_memory.h"
#include "util/lists.h"
#include "util/typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------------------
//NOTE(martin): memory arena
//--------------------------------------------------------------------------------

typedef struct oc_arena_chunk
{
    oc_list_elt listElt;
    char* ptr;
    u64 offset;
    u64 committed;
    u64 cap;
} oc_arena_chunk;

typedef struct oc_arena
{
    oc_base_allocator* base;
    oc_list chunks;
    oc_arena_chunk* currentChunk;

} oc_arena;

typedef struct oc_arena_scope
{
    oc_arena* arena;
    oc_arena_chunk* chunk;
    u64 offset;
} oc_arena_scope;

typedef struct oc_arena_options
{
    oc_base_allocator* base;
    u64 reserve;
} oc_arena_options;

ORCA_API void oc_arena_init(oc_arena* arena);
ORCA_API void oc_arena_init_with_options(oc_arena* arena, oc_arena_options* options);
ORCA_API void oc_arena_cleanup(oc_arena* arena);

ORCA_API void* oc_arena_push(oc_arena* arena, u64 size);
ORCA_API void* oc_arena_push_aligned(oc_arena* arena, u64 size, u32 alignment);
ORCA_API void* oc_arena_push_uninitialized(oc_arena* arena, u64 size);
ORCA_API void* oc_arena_push_aligned_uninitialized(oc_arena* arena, u64 size, u32 alignment);

ORCA_API void oc_arena_clear(oc_arena* arena);

ORCA_API oc_arena_scope oc_arena_scope_begin(oc_arena* arena);
ORCA_API void oc_arena_scope_end(oc_arena_scope scope);

#define oc_arena_push_type(arena, type) ((type*)oc_arena_push_aligned(arena, sizeof(type), _Alignof(type)))
#define oc_arena_push_array(arena, type, count) ((type*)oc_arena_push_aligned(arena, sizeof(type) * (count), _Alignof(type)))
#define oc_arena_push_type_uninitialized(arena, type) ((type*)oc_arena_push_aligned_uninitialized(arena, sizeof(type), _Alignof(type)))
#define oc_arena_push_array_uninitialized(arena, type, count) ((type*)oc_arena_push_aligned_uninitialized(arena, sizeof(type) * (count), _Alignof(type)))

static inline void* oc_arena_push_zero(oc_arena* arena, u64 size)
{
    void* p = oc_arena_push(arena, size);
    memset(p, 0, size);
    return p;
}

static inline void* oc_arena_dup(oc_arena* arena, void* p, u64 size)
{
    void* p2 = oc_arena_push(arena, size);
    memcpy(p2, p, size);
    return p2;
}

//--------------------------------------------------------------------------------
//NOTE(martin): memory pool
//--------------------------------------------------------------------------------

//TODO: we could probably remove pool. Most of the time we construct pool on top of
//      arenas "manually" with different free lists per object types...

typedef struct oc_pool
{
    oc_arena arena;
    oc_list freeList;
    u64 blockSize;
} oc_pool;

typedef struct oc_pool_options
{
    oc_base_allocator* base;
    u64 reserve;
} oc_pool_options;

ORCA_API void oc_pool_init(oc_pool* pool, u64 blockSize);
ORCA_API void oc_pool_init_with_options(oc_pool* pool, u64 blockSize, oc_pool_options* options);
ORCA_API void oc_pool_cleanup(oc_pool* pool);

ORCA_API void* oc_pool_alloc(oc_pool* pool);
ORCA_API void oc_pool_recycle(oc_pool* pool, void* ptr);
ORCA_API void oc_pool_clear(oc_pool* pool);

#define oc_pool_alloc_type(arena, type) ((type*)oc_pool_alloc(arena))

//--------------------------------------------------------------------------------
//NOTE(martin): per-thread implicit scratch arena
//--------------------------------------------------------------------------------
ORCA_API oc_arena_scope oc_scratch_begin(void);
ORCA_API oc_arena_scope oc_scratch_begin_next(oc_arena* used);

#define oc_scratch_end(scope) oc_arena_scope_end(scope)

#ifdef __cplusplus
} // extern "C"
#endif
