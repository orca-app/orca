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

    char* mem = oc_base_reserve(arena->base, reserveSize);
    oc_base_commit(arena->base, mem, commitSize);

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

void oc_arena_init_with_options(oc_arena* arena, oc_arena_options* options)
{
    memset(arena, 0, sizeof(oc_arena));

    arena->base = options->base ? options->base : oc_base_allocator_default();

    u64 reserveSize = options->reserve ? (options->reserve + sizeof(oc_arena_chunk)) : OC_ARENA_DEFAULT_RESERVE_SIZE;

    arena->currentChunk = oc_arena_chunk_alloc(arena, reserveSize);
}

void oc_arena_cleanup(oc_arena* arena)
{
    oc_list_for_safe(arena->chunks, chunk, oc_arena_chunk, listElt)
    {
        oc_base_release(arena->base, chunk, chunk->cap);
    }
    memset(arena, 0, sizeof(oc_arena));
}

void* oc_arena_push(oc_arena* arena, u64 size)
{
    return oc_arena_push_aligned(arena, size, 1);
}

void* oc_arena_push_aligned(oc_arena* arena, u64 size, u32 alignment)
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
        chunk = oc_list_next_entry(arena->chunks, chunk, oc_arena_chunk, listElt);
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
        oc_base_commit(arena->base, chunk->ptr + chunk->committed, commitSize);
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

oc_arena_scope oc_arena_scope_begin(oc_arena* arena)
{
    oc_arena_scope scope = { .arena = arena,
                             .chunk = arena->currentChunk,
                             .offset = arena->currentChunk->offset };
    return (scope);
}

void oc_arena_scope_end(oc_arena_scope scope)
{
    scope.arena->currentChunk = scope.chunk;
    scope.arena->currentChunk->offset = scope.offset;
}

//--------------------------------------------------------------------------------
//NOTE(martin): memory pool
//--------------------------------------------------------------------------------
void oc_pool_init(oc_pool* pool, u64 blockSize)
{
    oc_pool_init_with_options(pool, blockSize, &(oc_pool_options){ 0 });
}

void oc_pool_init_with_options(oc_pool* pool, u64 blockSize, oc_pool_options* options)
{
    oc_arena_init_with_options(&pool->arena, &(oc_arena_options){ .base = options->base, .reserve = options->reserve });
    pool->blockSize = oc_clamp_low(blockSize, sizeof(oc_list));
    oc_list_init(&pool->freeList);
}

void oc_pool_cleanup(oc_pool* pool)
{
    oc_arena_cleanup(&pool->arena);
    memset(pool, 0, sizeof(oc_pool));
}

void* oc_pool_alloc(oc_pool* pool)
{
    if(oc_list_empty(pool->freeList))
    {
        return (oc_arena_push(&pool->arena, pool->blockSize));
    }
    else
    {
        return (oc_list_pop_front(&pool->freeList));
    }
}

void oc_pool_recycle(oc_pool* pool, void* ptr)
{
    oc_list_push_front(&pool->freeList, (oc_list_elt*)ptr);
}

void oc_pool_clear(oc_pool* pool)
{
    oc_arena_clear(&pool->arena);
    oc_list_init(&pool->freeList);
}

//--------------------------------------------------------------------------------
//NOTE(martin): per-thread scratch arena
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

ORCA_API oc_arena_scope oc_scratch_begin(void)
{
    oc_arena* scratch = oc_scratch_at_index(0);
    oc_arena_scope scope = oc_arena_scope_begin(scratch);
    return (scope);
}

ORCA_API oc_arena_scope oc_scratch_begin_next(oc_arena* used)
{
    oc_arena_scope scope = { 0 };
    oc_arena* arena = 0;
    if((used >= __scratchPool)
       && (used - __scratchPool < OC_SCRATCH_POOL_SIZE))
    {
        u64 index = used - __scratchPool;
        if(index + 1 < OC_SCRATCH_POOL_SIZE)
        {
            arena = oc_scratch_at_index(index + 1);
        }
    }
    else
    {
        arena = oc_scratch_at_index(0);
    }

    OC_ASSERT(arena);

    scope = oc_arena_scope_begin(arena);

    return (scope);
}
