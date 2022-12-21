/************************************************************//**
*
*	@file: memory.c
*	@author: Martin Fouilleul
*	@date: 24/10/2019
*	@revision:
*
*****************************************************************/
#include<string.h> // memset

#include"memory.h"
#include"platform_base_allocator.h"
#include"macro_helpers.h"

static const u32 MEM_ARENA_DEFAULT_RESERVE_SIZE = 1<<30;
static const u32 MEM_ARENA_COMMIT_ALIGNMENT = 1<<20;

//--------------------------------------------------------------------------------
//NOTE(martin): memory arena
//--------------------------------------------------------------------------------
void mem_arena_init(mem_arena* arena)
{
	mem_arena_init_with_options(arena, &(mem_arena_options){0});
}

void mem_arena_init_with_options(mem_arena* arena, mem_arena_options* options)
{
	arena->base = options->base ? options->base : mem_base_allocator_default();
	arena->cap = options->reserve ? options->reserve : MEM_ARENA_DEFAULT_RESERVE_SIZE;

	arena->ptr = mem_base_reserve(arena->base, arena->cap);
	arena->committed = 0;
	arena->offset = 0;
}

void mem_arena_release(mem_arena* arena)
{
	mem_base_release(arena->base, arena->ptr, arena->cap);
	memset(arena, 0, sizeof(mem_arena));
}

void* mem_arena_alloc(mem_arena* arena, u64 size)
{
	u64 nextOffset = arena->offset + size;
	ASSERT(nextOffset <= arena->cap);

	if(nextOffset > arena->committed)
	{
		u64 nextCommitted = AlignUpOnPow2(nextOffset, MEM_ARENA_COMMIT_ALIGNMENT);
		nextCommitted = ClampHighBound(nextCommitted, arena->cap);
		u64 commitSize = nextCommitted - arena->committed;
		mem_base_commit(arena->base, arena->ptr + arena->committed, commitSize);
		arena->committed = nextCommitted;
	}
	char* p = arena->ptr + arena->offset;
	arena->offset += size;

	return(p);
}

void mem_arena_clear(mem_arena* arena)
{
	arena->offset = 0;
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
	ListInit(&pool->freeList);
}

void mem_pool_release(mem_pool* pool)
{
	mem_arena_release(&pool->arena);
	memset(pool, 0, sizeof(mem_pool));
}

void* mem_pool_alloc(mem_pool* pool)
{
	if(ListEmpty(&pool->freeList))
	{
		return(mem_arena_alloc(&pool->arena, pool->blockSize));
	}
	else
	{
		return(ListPop(&pool->freeList));
	}
}

void mem_pool_recycle(mem_pool* pool, void* ptr)
{
	ASSERT((((char*)ptr) >= pool->arena.ptr) && (((char*)ptr) < (pool->arena.ptr + pool->arena.offset)));
	ListPush(&pool->freeList, (list_elt*)ptr);
}

void mem_pool_clear(mem_pool* pool)
{
	mem_arena_clear(&pool->arena);
	ListInit(&pool->freeList);
}


//--------------------------------------------------------------------------------
//NOTE(martin): per-thread scratch arena
//--------------------------------------------------------------------------------

//TODO: move that somewhere in context cracking code
#ifdef _WIN32
	#define __thread __declspec(thread)
#endif

__thread mem_arena __scratchArena = {0};


mem_arena* mem_scratch()
{
	if(__scratchArena.ptr == 0)
	{
		mem_arena_init(&__scratchArena);
	}
	return(&__scratchArena);
}

void mem_scratch_clear()
{
	mem_arena_clear(mem_scratch());
}
