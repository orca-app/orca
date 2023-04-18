/************************************************************//**
*
*	@file: memory.h
*	@author: Martin Fouilleul
*	@date: 24/10/2019
*	@revision:
*
*****************************************************************/
#ifndef __MEMORY_H_
#define __MEMORY_H_

#include"util/typedefs.h"
#include"util/lists.h"
#include"platform/platform_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------------------
//NOTE(martin): memory arena
//--------------------------------------------------------------------------------

typedef struct mem_arena_chunk
{
	list_elt listElt;
	char* ptr;
	u64 offset;
	u64 committed;
	u64 cap;
} mem_arena_chunk;

typedef struct mem_arena
{
	mem_base_allocator* base;
	list_info chunks;
	mem_arena_chunk* currentChunk;

} mem_arena;

typedef struct mem_arena_marker
{
	#if DEBUG
		mem_arena* arena;
	#endif
	mem_arena_chunk* chunk;
	u64 offset;
} mem_arena_marker;

typedef struct mem_arena_options
{
	mem_base_allocator* base;
	u64 reserve;
} mem_arena_options;

MP_API void mem_arena_init(mem_arena* arena);
MP_API void mem_arena_init_with_options(mem_arena* arena, mem_arena_options* options);
MP_API void mem_arena_release(mem_arena* arena);

MP_API void* mem_arena_alloc(mem_arena* arena, u64 size);
MP_API void mem_arena_clear(mem_arena* arena);
MP_API mem_arena_marker mem_arena_mark(mem_arena* arena);
MP_API void mem_arena_clear_to(mem_arena* arena, mem_arena_marker marker);

#define mem_arena_alloc_type(arena, type) ((type*)mem_arena_alloc(arena, sizeof(type)))
#define mem_arena_alloc_array(arena, type, count) ((type*)mem_arena_alloc(arena, sizeof(type)*(count)))

//--------------------------------------------------------------------------------
//NOTE(martin): memory pool
//--------------------------------------------------------------------------------
typedef struct mem_pool
{
	mem_arena arena;
	list_info freeList;
	u64 blockSize;
} mem_pool;

typedef struct mem_pool_options
{
	mem_base_allocator* base;
	u64 reserve;
} mem_pool_options;

MP_API void mem_pool_init(mem_pool* pool, u64 blockSize);
MP_API void mem_pool_init_with_options(mem_pool* pool, u64 blockSize, mem_pool_options* options);
MP_API void mem_pool_release(mem_pool* pool);

MP_API void* mem_pool_alloc(mem_pool* pool);
MP_API void mem_pool_recycle(mem_pool* pool, void* ptr);
MP_API void mem_pool_clear(mem_pool* pool);

#define mem_pool_alloc_type(arena, type) ((type*)mem_pool_alloc(arena))

//--------------------------------------------------------------------------------
//NOTE(martin): per-thread implicit scratch arena
//--------------------------------------------------------------------------------
MP_API void mem_scratch_clear();
MP_API mem_arena* mem_scratch();

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__MEMORY_H_
