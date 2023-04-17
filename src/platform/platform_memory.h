/************************************************************//**
*
*	@file: platform_memory.h
*	@author: Martin Fouilleul
*	@date: 10/09/2021
*	@revision:
*
*****************************************************************/
#ifndef __PLATFORM_MEMORY_H_
#define __PLATFORM_MEMORY_H_

#include"typedefs.h"
#include"platform.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------------------
//NOTE(martin): base allocator
//--------------------------------------------------------------------------------
typedef struct mem_base_allocator mem_base_allocator;

typedef void*(*mem_reserve_function)(mem_base_allocator* context, u64 size);
typedef void(*mem_modify_function)(mem_base_allocator* context, void* ptr, u64 size);

typedef struct mem_base_allocator
{
	mem_reserve_function reserve;
	mem_modify_function commit;
	mem_modify_function decommit;
	mem_modify_function release;

} mem_base_allocator;

MP_API mem_base_allocator* mem_base_allocator_default();

#define mem_base_reserve(base, size) base->reserve(base, size)
#define mem_base_commit(base, ptr, size) base->commit(base, ptr, size)
#define mem_base_decommit(base, ptr, size) base->decommit(base, ptr, size)
#define mem_base_release(base, ptr, size) base->release(base, ptr, size)

//--------------------------------------------------------------------------------
//NOTE(martin): malloc/free
//--------------------------------------------------------------------------------
#if PLATFORM_ORCA
	void* malloc(size_t size);
	void* realloc(void* ptr, size_t size);
	void free(void* ptr);
#else
	#include<stdlib.h>
#endif

#define malloc_type(type) ((type*)malloc(sizeof(type)))
#define malloc_array(type, count) ((type*)malloc(sizeof(type)*count))

//--------------------------------------------------------------------------------
//NOTE(martin): memset / memcpy
//--------------------------------------------------------------------------------

#if PLATFORM_ORCA
	void* memset(void *b, int c, size_t len);
	void* memcpy(void *restrict dst, const void *restrict src, size_t n);
	void* memmove(void *dst, const void *src, size_t len);
	int memcmp(const void *s1, const void *s2, size_t n);
#else
	#include<string.h>
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__PLATFORM_MEMORY_H_
