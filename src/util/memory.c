/************************************************************//**
*
*	@file: memory.c
*	@author: Martin Fouilleul
*	@date: 24/10/2019
*	@revision:
*
*****************************************************************/
#include"platform.h"
#include"memory.h"
#include"platform_memory.h"
#include"macro_helpers.h"

#if PLATFORM_ORCA
	static const u32 MEM_ARENA_DEFAULT_RESERVE_SIZE = 1<<20;
#else
	static const u32 MEM_ARENA_DEFAULT_RESERVE_SIZE = 1<<30;
#endif

static const u32 MEM_ARENA_COMMIT_ALIGNMENT = 4<<10;

//--------------------------------------------------------------------------------
//NOTE(martin): memory arena
//--------------------------------------------------------------------------------

mem_arena_chunk* mem_arena_chunk_alloc(mem_arena* arena, u64 reserveSize)
{
	reserveSize = AlignUpOnPow2(reserveSize, MEM_ARENA_COMMIT_ALIGNMENT);
	u64 commitSize = AlignUpOnPow2(sizeof(mem_arena_chunk), MEM_ARENA_COMMIT_ALIGNMENT);

	char* mem = mem_base_reserve(arena->base, reserveSize);
	mem_base_commit(arena->base, mem, commitSize);

	mem_arena_chunk* chunk = (mem_arena_chunk*)mem;

	chunk->ptr = mem;
	chunk->cap = reserveSize;
	chunk->offset = sizeof(mem_arena_chunk);
	chunk->committed = commitSize;

	list_push_back(&arena->chunks, &chunk->listElt);

	return(chunk);
}

void mem_arena_init(mem_arena* arena)
{
	mem_arena_init_with_options(arena, &(mem_arena_options){0});
}

void mem_arena_init_with_options(mem_arena* arena, mem_arena_options* options)
{
	memset(arena, 0, sizeof(mem_arena));

	arena->base = options->base ? options->base : mem_base_allocator_default();

	u64 reserveSize = options->reserve ? (options->reserve + sizeof(mem_arena_chunk)) : MEM_ARENA_DEFAULT_RESERVE_SIZE;

	arena->currentChunk = mem_arena_chunk_alloc(arena, reserveSize);
}

void mem_arena_release(mem_arena* arena)
{
	for_list_safe(&arena->chunks, chunk, mem_arena_chunk, listElt)
	{
		mem_base_release(arena->base, chunk, chunk->cap);
	}
	memset(arena, 0, sizeof(mem_arena));
}

void* mem_arena_alloc(mem_arena* arena, u64 size)
{
	mem_arena_chunk* chunk = arena->currentChunk;
	ASSERT(chunk);

	u64 nextOffset = chunk->offset + size;
	u64 lastCap = chunk->cap;
	while(chunk && nextOffset > chunk->cap)
	{
		chunk = list_next_entry(&arena->chunks, chunk, mem_arena_chunk, listElt);
		nextOffset = chunk->offset + size;
		lastCap = chunk->cap;
	}
	if(!chunk)
	{
		u64 reserveSize = maximum(lastCap * 1.5, size);

		chunk = mem_arena_chunk_alloc(arena, reserveSize);
		nextOffset = chunk->offset + size;
	}
	ASSERT(nextOffset <= chunk->cap);

	arena->currentChunk = chunk;

	if(nextOffset > chunk->committed)
	{
		u64 nextCommitted = AlignUpOnPow2(nextOffset, MEM_ARENA_COMMIT_ALIGNMENT);
		nextCommitted = ClampHighBound(nextCommitted, chunk->cap);
		u64 commitSize = nextCommitted - chunk->committed;
		mem_base_commit(arena->base, chunk->ptr + chunk->committed, commitSize);
		chunk->committed = nextCommitted;
	}
	char* p = chunk->ptr + chunk->offset;
	chunk->offset += size;

	return(p);
}

void mem_arena_clear(mem_arena* arena)
{
	for_list(&arena->chunks, chunk, mem_arena_chunk, listElt)
	{
		chunk->offset = sizeof(mem_arena_chunk);
	}
	arena->currentChunk = list_first_entry(&arena->chunks, mem_arena_chunk, listElt);
}

//--------------------------------------------------------------------------------
//NOTE(martin): memory pool
//--------------------------------------------------------------------------------
void mem_pool_init(mem_pool* pool, u64 blockSize)
{
	mem_pool_init_with_options(pool, blockSize, &(mem_pool_options){0});
}
void mem_pool_init_with_options(mem_pool* pool, u64 blockSize, mem_pool_options* options)
{
	mem_arena_init_with_options(&pool->arena, &(mem_arena_options){.base = options->base, .reserve = options->reserve});
	pool->blockSize = ClampLowBound(blockSize, sizeof(list_info));
	list_init(&pool->freeList);
}

void mem_pool_release(mem_pool* pool)
{
	mem_arena_release(&pool->arena);
	memset(pool, 0, sizeof(mem_pool));
}

void* mem_pool_alloc(mem_pool* pool)
{
	if(list_empty(&pool->freeList))
	{
		return(mem_arena_alloc(&pool->arena, pool->blockSize));
	}
	else
	{
		return(list_pop(&pool->freeList));
	}
}

void mem_pool_recycle(mem_pool* pool, void* ptr)
{
	list_push(&pool->freeList, (list_elt*)ptr);
}

void mem_pool_clear(mem_pool* pool)
{
	mem_arena_clear(&pool->arena);
	list_init(&pool->freeList);
}


//--------------------------------------------------------------------------------
//NOTE(martin): per-thread scratch arena
//--------------------------------------------------------------------------------

mp_thread_local mem_arena __scratchArena = {0};

mem_arena* mem_scratch()
{
	if(__scratchArena.base == 0)
	{
		mem_arena_init(&__scratchArena);
	}
	return(&__scratchArena);
}

void mem_scratch_clear()
{
	mem_arena_clear(mem_scratch());
}
