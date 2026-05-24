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
//NOTE(martin): allocator interface
//--------------------------------------------------------------------------------

typedef struct oc_allocator oc_allocator;
typedef void* (*oc_allocator_push_proc)(oc_allocator* allocator, u64 size, u64 align);

typedef struct oc_allocator
{
    oc_allocator_push_proc push;
} oc_allocator;

#define oc_allocator_push_aligned_uninitialized(allocator, size, align) (allocator)->push(allocator, size, align)

#define oc_allocator_push_aligned(allocator, size, align)      \
    ({                                                         \
        void* __p = (allocator)->push(allocator, size, align); \
        if(size && __p)                                        \
        {                                                      \
            memset(__p, 0, size);                              \
        }                                                      \
        __p;                                                   \
    })

#define oc_allocator_push(allocator, size) oc_allocator_push_aligned(allocator, size, 1)
#define oc_allocator_push_uninitialized(allocator, size) oc_allocator_push_aligned_uninitialized(allocator, size, 1)

#define oc_allocator_push_type(allocator, type) oc_allocator_push_aligned(allocator, sizeof(type), _Alignof(type))
#define oc_allocator_push_array(allocator, type, count) oc_allocator_push_aligned(allocator, sizeof(type) * count, _Alignof(type))

#define oc_allocator_push_type_uninitialized(allocator, type) oc_allocator_push_aligned_uninitialized(allocator, sizeof(type), _Alignof(type))
#define oc_allocator_push_array_uninitialized(allocator, type, count) oc_allocator_push_aligned_uninitialized(allocator, sizeof(type) * count, _Alignof(type))

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
    oc_allocator_push_proc push;
    oc_allocator* allocator;

    oc_platform_memory* base;
    oc_list chunks;
    oc_arena_chunk* currentChunk;

} oc_arena;

typedef struct oc_arena_options
{
    oc_platform_memory* base;
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

#define oc_arena_push_type(arena, type) ((type*)oc_arena_push_aligned(arena, sizeof(type), _Alignof(type)))
#define oc_arena_push_array(arena, type, count) ((type*)oc_arena_push_aligned(arena, sizeof(type) * (count), _Alignof(type)))

#define oc_arena_push_type_uninitialized(arena, type) ((type*)oc_arena_push_aligned_uninitialized(arena, sizeof(type), _Alignof(type)))
#define oc_arena_push_array_uninitialized(arena, type, count) ((type*)oc_arena_push_aligned_uninitialized(arena, sizeof(type) * (count), _Alignof(type)))

//--------------------------------------------------------------------------------
//NOTE(martin): arena-based scratch allocator
//--------------------------------------------------------------------------------

typedef struct oc_scratch
{
    oc_allocator* allocator;
    oc_arena* arena;
    oc_arena_chunk* chunk;
    u64 offset;
} oc_scratch;

ORCA_API oc_scratch oc_scratch_begin_on_arena(oc_arena* arena);
ORCA_API oc_scratch oc_scratch_begin(void);

ORCA_API oc_scratch oc_scratch_begin_next_allocator(oc_allocator* allocator);
ORCA_API oc_scratch oc_scratch_begin_next_arena(oc_arena* used);

ORCA_API void oc_scratch_end(oc_scratch scratch);

#ifdef __cplusplus
} // extern "C"
#endif
