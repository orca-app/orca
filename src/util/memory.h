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

#include"typedefs.h"
#include"lists.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------------------
//NOTE(martin): base allocator
//--------------------------------------------------------------------------------
typedef void*(*mem_reserve_function)(void* context, u64 size);
typedef void(*mem_modify_function)(void* context, void* ptr, u64 size);

typedef struct mem_base_allocator
{
	mem_reserve_function reserve;
	mem_modify_function commit;
	mem_modify_function decommit;
	mem_modify_function release;
	void* context;

} mem_base_allocator;

MP_API mem_base_allocator* mem_base_allocator_default();

#define mem_base_reserve(base, size) base->reserve(base->context, size)
#define mem_base_commit(base, ptr, size) base->commit(base->context, ptr, size)
#define mem_base_decommit(base, ptr, size) base->decommit(base->context, ptr, size)
#define mem_base_release(base, ptr, size) base->release(base->context, ptr, size)

//--------------------------------------------------------------------------------
//NOTE(martin): memory arena
//--------------------------------------------------------------------------------
typedef struct mem_arena
{
	mem_base_allocator* base;
	char* ptr;
	u64 offset;
	u64 committed;
	u64 cap;
} mem_arena;

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
