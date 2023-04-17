/************************************************************//**
*
*	@file: orca_memory.c
*	@author: Martin Fouilleul
*	@date: 17/04/2023
*
*****************************************************************/

#include"platform_memory.h"

extern void* orca_mem_grow(u64 size);

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


//TODO: implement malloc/realloc/free/memset/etc here...


void* memset(void* b, int c, size_t n)
{
	for(size_t i = 0; i<n; i++)
	{
		((char*)b)[i] = (u8)c;
	}
	return(b);
}

void* memcpy(void *restrict dst, const void *restrict src, size_t n)
{
	for(size_t i = 0; i<n; i++)
	{
		((char*)dst)[i] = ((char*)src)[i];
	}
	return(dst);
}

void* memmove(void *dst, const void *src, size_t n)
{
	if(src < dst)
	{
		for(size_t i = n-1; i>=0; i--)
		{
			((char*)dst)[i] = ((char*)src)[i];
		}
	}
	else if(src > dst)
	{
		for(size_t i = 0; i<n; i++)
		{
			((char*)dst)[i] = ((char*)src)[i];
		}
	}
	return(dst);
}

int memcmp(const void *s1, const void *s2, size_t n)
{
	const unsigned char* c1 = (const unsigned char*)s1;
	const unsigned char* c2 = (const unsigned char*)s2;

	for(size_t i = 0; i<n; i++)
	{
		if(c1[i] != c2[i])
		{
			return(c1 - c2);
		}
	}
	return(0);
}
