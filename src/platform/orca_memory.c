/************************************************************//**
*
*	@file: orca_memory.c
*	@author: Martin Fouilleul
*	@date: 17/04/2023
*
*****************************************************************/

#include"platform_memory.h"
#include <stdlib.h>

void* ORCA_IMPORT(orca_mem_grow)(u64 size);

void* orca_mem_base_reserve(mem_base_allocator* context, u64 size)
{
	return(orca_mem_grow(size));
}

void orca_mem_base_nop(mem_base_allocator* context, void* ptr, u64 size) {}

mem_base_allocator* mem_base_allocator_default()
{
	static mem_base_allocator base = {0};
	if(base.reserve == 0)
	{
		base.reserve = orca_mem_base_reserve;
		base.commit = orca_mem_base_nop;
		base.decommit = orca_mem_base_nop;
		base.release = orca_mem_base_nop;
	}
	return(&base);
}

// malloc, free, realloc, etc. are defined in orca_malloc.c

void* memset(void* b, int c, size_t n)
{
	return __builtin_memset(b, c, n);
}

void* memcpy(void *restrict dst, const void *restrict src, size_t n)
{
	return __builtin_memcpy(dst, src, n);
}

void* memmove(void *dst, const void *src, size_t n)
{
	return __builtin_memmove(dst, src, n);
}

int memcmp(const void *s1, const void *s2, size_t n)
{
	return __builtin_memcmp(s1, s2, n);
}
