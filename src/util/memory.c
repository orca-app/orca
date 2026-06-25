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

//--------------------------------------------------------------------------------
//NOTE(martin): allocator interface
//--------------------------------------------------------------------------------

void* oc_allocator_push_aligned_uninitialized(oc_allocator* allocator, u64 size, u64 align)
{
    return allocator->push(allocator, size, align);
}

void* oc_allocator_push_aligned(oc_allocator* allocator, u64 size, u64 align)
{
    void* p = allocator->push(allocator, size, align);
    if(size && p)
    {
        memset(p, 0, size);
    }
    return p;
}

void* oc_allocator_push_uninitialized(oc_allocator* allocator, u64 size)
{
    return oc_allocator_push_aligned_uninitialized(allocator, size, 1);
}

void* oc_allocator_push(oc_allocator* allocator, u64 size)
{
    return oc_allocator_push_aligned(allocator, size, 1);
}

//--------------------------------------------------------------------------------
//NOTE(martin): memory arena
//--------------------------------------------------------------------------------

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
        chunk = oc_list_next_elt(chunk, oc_arena_chunk, listElt);
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
    arena->currentChunk = oc_list_first_elt(arena->chunks, oc_arena_chunk, listElt);
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
        chunk = oc_list_prev_elt(chunk, oc_arena_chunk, listElt))
    {
        chunk->offset = sizeof(oc_arena_chunk);
    }
    scope.arena->currentChunk = scope.chunk;
    scope.arena->currentChunk->offset = scope.offset;
}

//--------------------------------------------------------------------------------
//NOTE(martin): arena-based heap
//--------------------------------------------------------------------------------
enum
{
    OC_HEAP_CHUNK_USED = 1,
    OC_HEAP_CHUNK_MIN_SIZE = 8,
    OC_HEAP_CHUNK_MAX_SMALL_SIZE = 504,
    OC_HEAP_CHUNK_MAX_LARGE_SIZE = 16 << 20,
    OC_HEAP_CHUNK_OVERHEAD = sizeof(oc_heap_chunk),
};

oc_heap_chunk_list* oc_heap_get_bin_for_chunk(oc_heap* heap, oc_heap_chunk* chunk)
{
    oc_heap_chunk_list* bin = 0;
    if(chunk->sizeAndStatus <= OC_HEAP_CHUNK_MAX_SMALL_SIZE)
    {
        u64 binIndex = chunk->sizeAndStatus / 8 - 2;
        bin = &heap->smallBins[binIndex];
    }
    else
    {
        u64 binIndex = 63 - __builtin_clzl(chunk->sizeAndStatus) - 9;
        bin = &heap->largeBins[binIndex];
    }
    return bin;
}

void oc_heap_bin_chunk(oc_heap* heap, oc_heap_chunk* chunk)
{
    oc_heap_chunk_list* bin = oc_heap_get_bin_for_chunk(heap, chunk);

    bool found = false;
    oc_typed_list_for(*bin, it)
    {
        if(it->sizeAndStatus >= chunk->sizeAndStatus)
        {
            oc_typed_list_insert_before(bin, it, chunk);
            found = true;
            break;
        }
    }
    if(!found)
    {
        oc_typed_list_push_back(bin, chunk);
    }
}

oc_heap_chunk* oc_heap_new_region(oc_heap* heap, u64 size)
{
    heap->nextChunkSize = oc_max(heap->nextChunkSize, size);

    size = sizeof(oc_heap_region) + 2 * sizeof(oc_heap_chunk) + heap->nextChunkSize;
    oc_heap_region* region = (oc_heap_region*)oc_platform_memory_reserve(heap->base, size);

    if(!region)
    {
        return 0;
    }

    oc_platform_memory_commit(heap->base, (void*)region, size);

    region->size = size;
    oc_typed_list_push_back(&heap->regions, region);

    oc_heap_chunk* chunk = (oc_heap_chunk*)region->mem;
    chunk->prevSize = 0;
    chunk->sizeAndStatus = heap->nextChunkSize;

    oc_heap_chunk* sentinel = (oc_heap_chunk*)(region->mem + sizeof(oc_heap_chunk) + heap->nextChunkSize);
    sentinel->prevSize = heap->nextChunkSize;
    sentinel->sizeAndStatus = OC_HEAP_CHUNK_USED;

    heap->nextChunkSize *= 2;
    return chunk;
}

void oc_heap_init(oc_heap* heap)
{
    memset(heap, 0, sizeof(oc_heap));

    heap->base = oc_platform_memory_default();
    heap->nextChunkSize = 1 << 10;

    oc_heap_chunk* chunk = oc_heap_new_region(heap, 0);
    oc_heap_bin_chunk(heap, chunk);
}

void oc_heap_cleanup(oc_heap* heap)
{
    oc_typed_list_for_safe(heap->regions, region)
    {
        oc_platform_memory_release(heap->base, region, region->size);
    }
    memset(heap, 0, sizeof(oc_heap));
}

void* oc_heap_alloc(oc_heap* heap, u64 size)
{
    if(!size)
    {
        return 0;
    }
    u64 sizeUp8 = oc_align_up_pow2(oc_max(size, 16), 8);

    oc_heap_chunk* chunk = 0;
    u64 largeStartIndex = 0;

    if(size <= OC_HEAP_CHUNK_MAX_SMALL_SIZE)
    {
        //NOTE: try to find suitable chunk in small bins
        u64 startIndex = sizeUp8 / 8 - 2;

        for(u64 binIndex = startIndex;
            binIndex < OC_HEAP_SMALL_BIN_COUNT;
            binIndex++)
        {
            oc_heap_chunk* front = oc_typed_list_pop_front(&heap->smallBins[binIndex]);
            if(front)
            {
                chunk = front;
                break;
            }
        }
    }
    else
    {
        largeStartIndex = 63 - __builtin_clzl(size) - 9;
        OC_ASSERT(largeStartIndex < OC_HEAP_LARGE_BIN_COUNT);
    }

    if(!chunk)
    {
        //NOTE: try to find suitable chunk in first large bin
        oc_typed_list_for(heap->largeBins[largeStartIndex], candidate)
        {
            if(candidate->sizeAndStatus >= size)
            {
                oc_typed_list_remove(&heap->largeBins[largeStartIndex], candidate);
                chunk = candidate;
                break;
            }
        }

        if(!chunk)
        {
            //NOTE: try to find suitable chunk in larger bins
            for(u64 binIndex = largeStartIndex + 1;
                chunk == 0 && binIndex < OC_HEAP_LARGE_BIN_COUNT;
                binIndex++)
            {
                oc_heap_chunk_list* bin = &heap->largeBins[binIndex];
                chunk = oc_typed_list_pop_front(bin);
            }
        }

        if(!chunk)
        {
            //NOTE: allocate new region and large chunk
            chunk = oc_heap_new_region(heap, sizeUp8);
        }
    }
    if(!chunk)
    {
        //NOTE: exhaustion
        return 0;
    }

    if(chunk->sizeAndStatus > sizeUp8 + OC_HEAP_CHUNK_MIN_SIZE + OC_HEAP_CHUNK_OVERHEAD)
    {
        //split chunk
        oc_heap_chunk* newChunk = (oc_heap_chunk*)((char*)chunk + OC_HEAP_CHUNK_OVERHEAD + sizeUp8);
        newChunk->prevSize = sizeUp8;
        newChunk->sizeAndStatus = chunk->sizeAndStatus - sizeUp8 - OC_HEAP_CHUNK_OVERHEAD;
        chunk->sizeAndStatus = sizeUp8;

        oc_heap_chunk* nextChunk = (oc_heap_chunk*)((char*)newChunk + sizeof(oc_heap_chunk) + newChunk->sizeAndStatus);
        nextChunk->prevSize = newChunk->sizeAndStatus;

        oc_heap_bin_chunk(heap, newChunk);
    }

    chunk->sizeAndStatus |= OC_HEAP_CHUNK_USED;
    return &chunk->mem;
}

void oc_heap_free(oc_heap* heap, void* p)
{
    if(!p)
    {
        return;
    }
    oc_heap_chunk* chunk = (oc_heap_chunk*)((char*)p - sizeof(oc_heap_chunk));
    chunk->sizeAndStatus &= ~OC_HEAP_CHUNK_USED;

    //NOTE: coalesce backward
    while(chunk->prevSize)
    {
        oc_heap_chunk* prevChunk = (oc_heap_chunk*)((char*)chunk - chunk->prevSize - sizeof(oc_heap_chunk));
        if(prevChunk->sizeAndStatus & OC_HEAP_CHUNK_USED)
        {
            break;
        }
        else
        {
            oc_heap_chunk_list* bin = oc_heap_get_bin_for_chunk(heap, prevChunk);
            oc_typed_list_remove(bin, prevChunk);

            prevChunk->sizeAndStatus += sizeof(oc_heap_chunk) + chunk->sizeAndStatus;
            chunk = prevChunk;
        }
    }

    //NOTE: coalesce forward
    oc_heap_chunk* nextChunk = (oc_heap_chunk*)((char*)chunk + sizeof(oc_heap_chunk) + chunk->sizeAndStatus);
    while(!(nextChunk->sizeAndStatus & OC_HEAP_CHUNK_USED))
    {
        oc_heap_chunk_list* bin = oc_heap_get_bin_for_chunk(heap, nextChunk);
        oc_typed_list_remove(bin, nextChunk);

        chunk->sizeAndStatus += sizeof(oc_heap_chunk) + nextChunk->sizeAndStatus;
        nextChunk = (oc_heap_chunk*)((char*)chunk + sizeof(oc_heap_chunk) + chunk->sizeAndStatus);
    }
    nextChunk->prevSize = chunk->sizeAndStatus;

    oc_heap_bin_chunk(heap, chunk);
}

void oc_heap_clear(oc_heap* heap)
{
    //TODO
}

void oc_heap_debug_print(oc_heap* heap)
{
    printf("Heap:\n");
    oc_typed_list_for(heap->regions, region)
    {
        printf("* region %p - %p\n", (char*)region->mem, (char*)region->mem + region->size);
        oc_heap_chunk* chunk = (oc_heap_chunk*)(region->mem);
        while(chunk->sizeAndStatus)
        {
            u64 size = chunk->sizeAndStatus & (~OC_HEAP_CHUNK_USED);
            printf("\t* %p -> %p", (char*)chunk, (char*)chunk + sizeof(oc_heap_chunk) + size);

            if(chunk->sizeAndStatus == OC_HEAP_CHUNK_USED)
            {
                printf(" (sentinel)");
            }
            printf("\n");

            printf("\t\tprevSize: %llu\n", chunk->prevSize);
            printf("\t\tsizeAndStatus: %llu %s\n", size, (chunk->sizeAndStatus & OC_HEAP_CHUNK_USED) ? "Used" : "Free");

            if(chunk->sizeAndStatus == OC_HEAP_CHUNK_USED)
            {
                break;
            }
            chunk = (oc_heap_chunk*)((char*)chunk + sizeof(oc_heap_chunk) + size);
        }
    }
    printf("\n");
}

int oc_heap_debug_check_consistency(oc_heap* heap)
{
    //NOTE: check regions

    if(oc_typed_list_empty(heap->regions))
    {
        oc_log_error("no regions");
        return -1;
    }

    oc_typed_list_for(heap->regions, region)
    {
        u64 prevSize = 0;

        oc_heap_chunk* chunk = (oc_heap_chunk*)(region->mem);
        while(chunk->sizeAndStatus)
        {
            if((char*)chunk < region->mem)
            {
                oc_log_error("chunk not in region memory (chunk = %p, region->mem = %p)\n", chunk, region->mem);
                return -1;
            }
            if((char*)chunk >= (char*)region + region->size)
            {
                oc_log_error("chunk outside region (chunk = %p, region start = %p, region end = %p)\n", chunk, region, (char*)region + region->size);
                oc_log_info("might be missing sentinel chunk?\n");
                return -1;
            }

            if((intptr_t)chunk % 8)
            {
                oc_log_error("chunk not aligned on 8 byte boundary (%p)\n", chunk);
                return -1;
            }

            if((char*)chunk + sizeof(oc_heap_chunk) > (char*)region + region->size)
            {
                oc_log_error("chunk header overflows region (chunk start = %p, chunk end = %p, region start = %p, region end = %p)\n",
                             chunk, (char*)chunk + sizeof(oc_heap_chunk),
                             region,
                             (char*)region + region->size);
                return -1;
            }

            u64 size = chunk->sizeAndStatus & (~OC_HEAP_CHUNK_USED);

            if(chunk->sizeAndStatus != OC_HEAP_CHUNK_USED && size < OC_HEAP_CHUNK_MIN_SIZE)
            {
                oc_log_error("chunk size less than minimum size (%llu <  %llu)\n", size, OC_HEAP_CHUNK_MIN_SIZE);
                return -1;
            }
            if(size % 8)
            {
                oc_log_error("chunk size not a multiple of 8 (%llu)\n", size);
                return -1;
            }
            if((char*)chunk + sizeof(oc_heap_chunk) + size > (char*)region + region->size)
            {
                oc_log_error("chunk overflows region (chunk start = %p, chunk end = %p, region start = %p, region end = %p)",
                             chunk, (char*)chunk + sizeof(oc_heap_chunk) + size,
                             region,
                             (char*)region + region->size);
                return -1;
            }

            if(chunk->prevSize != prevSize)
            {
                oc_log_error("inconsistent prevSize for chunk %p (expected %llu, got %llu)\n", chunk, prevSize, chunk->prevSize);
                return -1;
            }

            if(!(chunk->sizeAndStatus & OC_HEAP_CHUNK_USED))
            {
                if(chunk->prevSize)
                {
                    oc_heap_chunk* prev = (oc_heap_chunk*)((char*)chunk - chunk->prevSize - sizeof(oc_heap_chunk));
                    if(!(prev->sizeAndStatus & OC_HEAP_CHUNK_USED))
                    {
                        oc_log_error("contiguous free chunks %p and %p\n", prev, chunk);
                        return -1;
                    }
                }

                oc_heap_chunk_list* bin = oc_heap_get_bin_for_chunk(heap, chunk);
                bool found = false;
                oc_typed_list_for(*bin, it)
                {
                    if(it == chunk)
                    {
                        found = true;
                        break;
                    }
                }
                if(!found)
                {
                    oc_log_error("free chunk %p not in any bin\n", chunk);
                    return -1;
                }
            }

            if(chunk->sizeAndStatus == OC_HEAP_CHUNK_USED)
            {
                break;
            }
            prevSize = size;
            chunk = (oc_heap_chunk*)((char*)chunk + sizeof(oc_heap_chunk) + size);
        }
    }

    // check all bins
    for(int i = 0; i < OC_HEAP_SMALL_BIN_COUNT; i++)
    {
        oc_heap_chunk_list* bin = &heap->smallBins[i];
        oc_typed_list_for(*bin, chunk)
        {
            if(chunk->sizeAndStatus & OC_HEAP_CHUNK_USED)
            {
                oc_log_error("used chunk in free bin (%p)\n", chunk);
                return -1;
            }
            oc_heap_chunk_list* expectedBin = oc_heap_get_bin_for_chunk(heap, chunk);
            if(bin != expectedBin)
            {
                oc_log_error("chunk %p in wrong bin (expected %p, got %p)\n", chunk, expectedBin, bin);
                return -1;
            }
        }
    }

    for(int i = 0; i < OC_HEAP_LARGE_BIN_COUNT; i++)
    {
        oc_heap_chunk_list* bin = &heap->largeBins[i];
        oc_typed_list_for(*bin, chunk)
        {
            if(chunk->sizeAndStatus & OC_HEAP_CHUNK_USED)
            {
                oc_log_error("used chunk in free bin (%p)\n", chunk);
                return -1;
            }
            oc_heap_chunk_list* expectedBin = oc_heap_get_bin_for_chunk(heap, chunk);
            if(bin != expectedBin)
            {
                oc_log_error("chunk %p in wrong bin (expected %p, got %p)\n", chunk, expectedBin, bin);
                return -1;
            }
        }
    }

    return 0;
}

bool oc_heap_debug_is_allocated(oc_heap* heap, void* p, u64 minSize)
{
    oc_typed_list_for(heap->regions, region)
    {
        oc_heap_chunk* chunk = (oc_heap_chunk*)(region->mem);
        while(chunk->sizeAndStatus && chunk->sizeAndStatus != OC_HEAP_CHUNK_USED)
        {
            u64 size = chunk->sizeAndStatus & (~OC_HEAP_CHUNK_USED);

            if((chunk->sizeAndStatus & OC_HEAP_CHUNK_USED)
               && chunk->mem == (char*)p
               && size >= minSize)
            {
                return true;
            }

            chunk = (oc_heap_chunk*)((char*)chunk + sizeof(oc_heap_chunk) + size);
        }
    }
    return false;
}
