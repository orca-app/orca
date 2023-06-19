/************************************************************//**
*
*	@file: memory.c
*	@author: Martin Fouilleul
*	@date: 24/10/2019
*	@revision:
*
*****************************************************************/
#include"platform/platform.h"
#include"memory.h"
#include"platform/platform_memory.h"
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
	while(nextOffset > chunk->cap)
	{
		chunk = list_next_entry(&arena->chunks, chunk, mem_arena_chunk, listElt);
		if(chunk)
		{
			nextOffset = chunk->offset + size;
			lastCap = chunk->cap;
		}
		else
		{
			break;
		}
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

mem_arena_scope mem_arena_scope_begin(mem_arena* arena)
{
	mem_arena_scope scope = {.arena = arena,
	                         .chunk = arena->currentChunk,
	                         .offset = arena->currentChunk->offset};
	return(scope);
}

void mem_arena_scope_end(mem_arena_scope scope)
{
	scope.arena->currentChunk = scope.chunk;
	scope.arena->currentChunk->offset = scope.offset;
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

enum
{
	MEM_SCRATCH_POOL_SIZE = 8,
	MEM_SCRATCH_DEFAULT_SIZE = 4096,
};

mp_thread_local mem_arena __scratchPool[MEM_SCRATCH_POOL_SIZE] = {0};


static mem_arena* mem_scratch_at_index(int index)
{
	mem_arena* scratch = 0;

	if(index >= 0 && index < MEM_SCRATCH_POOL_SIZE)
	{
		if(__scratchPool[index].base == 0)
		{
			mem_arena_options options = {.reserve = MEM_SCRATCH_DEFAULT_SIZE};
			mem_arena_init_with_options(&__scratchPool[index], &options);
		}
		scratch = &__scratchPool[index];
	}
	return(scratch);
}

mem_arena* mem_scratch()
{
	return(mem_scratch_at_index(0));
}

MP_API mem_arena* mem_scratch_next(mem_arena* used)
{
	mem_arena* res = 0;
	if( (used >= __scratchPool)
	  &&(used - __scratchPool < MEM_SCRATCH_POOL_SIZE))
	{
		u64 index = used - __scratchPool;
		if(index + 1 < MEM_SCRATCH_POOL_SIZE)
		{
			res = mem_scratch_at_index(index+1);
		}
	}
	else
	{
		res = mem_scratch_at_index(0);
	}
	return(res);
}

MP_API mem_arena_scope mem_scratch_begin()
{
	mem_arena* scratch = mem_scratch();
	mem_arena_scope scope = mem_arena_scope_begin(scratch);
	return(scope);
}

MP_API mem_arena_scope mem_scratch_begin_next(mem_arena* used)
{
	mem_arena* scratch = mem_scratch_next(used);
	mem_arena_scope scope = mem_arena_scope_begin(scratch);
	return(scope);
}
