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

void* oc_allocator_push_aligned_uninitialized(oc_allocator* allocator, u64 size, u64 align);
void* oc_allocator_push_aligned(oc_allocator* allocator, u64 size, u64 align);
void* oc_allocator_push_uninitialized(oc_allocator* allocator, u64 size);
void* oc_allocator_push(oc_allocator* allocator, u64 size);

#define oc_allocator_push_type(allocator, type) oc_allocator_push_aligned(allocator, sizeof(type), _Alignof(type))
#define oc_allocator_push_array(allocator, type, count) oc_allocator_push_aligned(allocator, sizeof(type) * count, _Alignof(type))
#define oc_allocator_push_type_uninitialized(allocator, type) oc_allocator_push_aligned_uninitialized(allocator, sizeof(type), _Alignof(type))
#define oc_allocator_push_array_uninitialized(allocator, type, count) oc_allocator_push_aligned_uninitialized(allocator, sizeof(type) * count, _Alignof(type))

//--------------------------------------------------------------------------------
//NOTE(martin): memory arena
//--------------------------------------------------------------------------------

typedef struct oc_arena_chunk
{
    oc_list_links listElt;
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

//--------------------------------------------------------------------------------
//NOTE(martin): arena-based heap
//--------------------------------------------------------------------------------

typedef struct oc_heap_region
{
    oc_list_links links;
    u64 size;
    char mem[];
} oc_heap_region;

typedef oc_typed_list(oc_heap_region, links) oc_heap_region_list;

typedef struct oc_heap_chunk
{
    u64 prevSize;
    u64 sizeAndStatus;
    oc_list_links links;
    char mem[];
} oc_heap_chunk;

typedef oc_typed_list(oc_heap_chunk, links) oc_heap_chunk_list;

enum
{
    OC_HEAP_SMALL_BIN_COUNT = 61,
    OC_HEAP_LARGE_BIN_COUNT = 16,
};

typedef struct oc_heap
{
    oc_platform_memory* base;
    oc_heap_region_list regions;
    oc_heap_chunk_list smallBins[OC_HEAP_SMALL_BIN_COUNT]; // fixed sizes from 16 to 504
    oc_heap_chunk_list largeBins[OC_HEAP_LARGE_BIN_COUNT]; // pow2 sizes from 512 (2^9) to 16M (2^24)
    u64 nextChunkSize;
} oc_heap;

ORCA_API void oc_heap_init(oc_heap* heap);
ORCA_API void oc_heap_cleanup(oc_heap* heap);

ORCA_API void* oc_heap_alloc(oc_heap* heap, u64 size);
ORCA_API void oc_heap_free(oc_heap* heap, void* p);
ORCA_API void oc_heap_clear(oc_heap* heap);

void oc_heap_debug_print(oc_heap* heap);

#ifdef __cplusplus
} // extern "C"
#endif
