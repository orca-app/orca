/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "memory.h"
#include "macros.h"
#include "platform/platform.h"
#include "platform/platform_memory.h"

#if OC_PLATFORM_ORCA
enum
{
    OC_ARENA_DEFAULT_RESERVE_SIZE = 1 << 20,
};
#else
enum
{
    OC_ARENA_DEFAULT_RESERVE_SIZE = 1 << 30,
};
#endif

enum
{
    OC_ARENA_COMMIT_ALIGNMENT = 4 << 10,
};

//--------------------------------------------------------------------------------
//NOTE(martin): memory arena
//--------------------------------------------------------------------------------

oc_arena_chunk* oc_arena_chunk_alloc(oc_arena* arena, u64 chunkMinSize)
{
    u64 reserveSize = oc_align_up_pow2(chunkMinSize + sizeof(oc_arena_chunk), OC_ARENA_COMMIT_ALIGNMENT);
    u64 commitSize = oc_align_up_pow2(sizeof(oc_arena_chunk), OC_ARENA_COMMIT_ALIGNMENT);

    char* mem = oc_platform_memory_reserve(arena->base, reserveSize);
    oc_platform_memory_commit(arena->base, mem, commitSize);

    oc_arena_chunk* chunk = (oc_arena_chunk*)mem;

    chunk->ptr = mem;
    chunk->cap = reserveSize;
    chunk->offset = sizeof(oc_arena_chunk);
    chunk->committed = commitSize;

    oc_list_push_back(&arena->chunks, &chunk->listElt);

    return (chunk);
}

void oc_arena_init(oc_arena* arena)
{
    oc_arena_init_with_options(arena, &(oc_arena_options){ 0 });
}

void* oc_arena_allocator_push(oc_allocator* allocator, u64 size, u64 align)
{
    oc_arena* arena = (oc_arena*)allocator;
    return oc_arena_push_aligned_uninitialized(arena, size, align);
}

void oc_arena_init_with_options(oc_arena* arena, oc_arena_options* options)
{
    memset(arena, 0, sizeof(oc_arena));

    arena->push = oc_arena_allocator_push;
    arena->allocator = (oc_allocator*)arena;

    arena->base = options->base ? options->base : oc_platform_memory_default();

    u64 reserveSize = options->reserve ? (options->reserve + sizeof(oc_arena_chunk)) : OC_ARENA_DEFAULT_RESERVE_SIZE;

    arena->currentChunk = oc_arena_chunk_alloc(arena, reserveSize);
}

void oc_arena_cleanup(oc_arena* arena)
{
    oc_list_for_safe(arena->chunks, chunk, oc_arena_chunk, listElt)
    {
        oc_platform_memory_release(arena->base, chunk, chunk->cap);
    }
    memset(arena, 0, sizeof(oc_arena));
}

void* oc_arena_push(oc_arena* arena, u64 size)
{
    return oc_arena_push_aligned(arena, size, 1);
}

void* oc_arena_push_aligned(oc_arena* arena, u64 size, u32 alignment)
{
    void* p = oc_arena_push_aligned_uninitialized(arena, size, alignment);
    if(size && p)
    {
        memset(p, 0, size);
    }
    return p;
}

void* oc_arena_push_uninitialized(oc_arena* arena, u64 size)
{
    return oc_arena_push_aligned_uninitialized(arena, size, 1);
}

void* oc_arena_push_aligned_uninitialized(oc_arena* arena, u64 size, u32 alignment)
{
    if(!size)
    {
        return (0);
    }

    oc_arena_chunk* chunk = arena->currentChunk;
    OC_ASSERT(chunk);

    u64 alignedOffset = oc_align_up_pow2(chunk->offset, alignment);
    u64 nextOffset = alignedOffset + size;
    u64 lastCap = chunk->cap;
    while(nextOffset > chunk->cap)
    {
        chunk = oc_list_next_entry(chunk, oc_arena_chunk, listElt);
        if(chunk)
        {
            alignedOffset = oc_align_up_pow2(chunk->offset, alignment);
            nextOffset = alignedOffset + size;
            lastCap = chunk->cap;
        }
        else
        {
            break;
        }
    }
    if(!chunk)
    {
        u64 chunkMinSize = oc_max(lastCap * 1.5, size + alignment);

        chunk = oc_arena_chunk_alloc(arena, chunkMinSize);
        alignedOffset = oc_align_up_pow2(chunk->offset, alignment);
        nextOffset = alignedOffset + size;
    }
    OC_ASSERT(nextOffset <= chunk->cap);

    arena->currentChunk = chunk;

    if(nextOffset > chunk->committed)
    {
        u64 nextCommitted = oc_align_up_pow2(nextOffset, OC_ARENA_COMMIT_ALIGNMENT);
        nextCommitted = oc_clamp_high(nextCommitted, chunk->cap);
        u64 commitSize = nextCommitted - chunk->committed;
        oc_platform_memory_commit(arena->base, chunk->ptr + chunk->committed, commitSize);
        chunk->committed = nextCommitted;
    }
    char* p = chunk->ptr + alignedOffset;
    chunk->offset = nextOffset;

    return (p);
}

void oc_arena_clear(oc_arena* arena)
{
    oc_list_for(arena->chunks, chunk, oc_arena_chunk, listElt)
    {
        chunk->offset = sizeof(oc_arena_chunk);
    }
    arena->currentChunk = oc_list_first_entry(arena->chunks, oc_arena_chunk, listElt);
}

//--------------------------------------------------------------------------------
//NOTE(martin): scratch arena
//--------------------------------------------------------------------------------

enum
{
    OC_SCRATCH_POOL_SIZE = 8,
    OC_SCRATCH_DEFAULT_SIZE = 4096,
};

oc_thread_local oc_arena __scratchPool[OC_SCRATCH_POOL_SIZE] = { 0 };

static oc_arena* oc_scratch_at_index(int index)
{
    oc_arena* scratch = 0;

    if(index >= 0 && index < OC_SCRATCH_POOL_SIZE)
    {
        if(__scratchPool[index].base == 0)
        {
            oc_arena_options options = { .reserve = OC_SCRATCH_DEFAULT_SIZE };
            oc_arena_init_with_options(&__scratchPool[index], &options);
        }
        scratch = &__scratchPool[index];
    }
    return (scratch);
}

oc_scratch oc_scratch_begin_on_arena(oc_arena* arena)
{
    oc_scratch scope = {
        .allocator = (oc_allocator*)arena,
        .arena = arena,
        .chunk = arena->currentChunk,
        .offset = arena->currentChunk->offset,
    };
    return (scope);
}

oc_scratch oc_scratch_begin(void)
{
    oc_arena* arena = oc_scratch_at_index(0);
    return oc_scratch_begin_on_arena(arena);
}

ORCA_API oc_scratch oc_scratch_begin_next_arena(oc_arena* used)
{
    oc_arena* arena = 0;
    if((used >= __scratchPool)
       && ((u64)(used - __scratchPool) < (u64)OC_SCRATCH_POOL_SIZE))
    {
        u64 index = (u64)(used - __scratchPool);
        if(index + 1 < (u64)OC_SCRATCH_POOL_SIZE)
        {
            arena = oc_scratch_at_index(index + 1);
        }
        else
        {
            OC_ABORT("no arenas left in scratch pool, used: %p, scratchPool: %p, index: %llu\n", used, __scratchPool, index);
        }
    }
    else
    {
        arena = oc_scratch_at_index(0);
    }

    OC_ASSERT(arena);

    return oc_scratch_begin_on_arena(arena);
}

ORCA_API oc_scratch oc_scratch_begin_next_allocator(oc_allocator* allocator)
{
    return oc_scratch_begin_next_arena((oc_arena*)allocator);
}

void oc_scratch_end(oc_scratch scope)
{
    for(oc_arena_chunk* chunk = scope.arena->currentChunk;
        chunk != 0 && chunk != scope.chunk;
        chunk = oc_list_prev_entry(chunk, oc_arena_chunk, listElt))
    {
        chunk->offset = sizeof(oc_arena_chunk);
    }
    scope.arena->currentChunk = scope.chunk;
    scope.arena->currentChunk->offset = scope.offset;
}
