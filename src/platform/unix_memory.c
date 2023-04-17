/************************************************************//**
*
*	@file: unix_memory.c
*	@author: Martin Fouilleul
*	@date: 10/09/2021
*	@revision:
*
*****************************************************************/
#include<sys/mman.h>
#include"platform_memory.h"

/*NOTE(martin):
	Linux and MacOS don't make a distinction between reserved and committed memory, contrary to Windows
*/
void mem_base_nop(mem_base_allocator* context, void* ptr, u64 size) {}

void* mem_base_reserve_mmap(mem_base_allocator* context, u64 size)
{
	return(mmap(0, size, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, 0, 0));
}

void mem_base_release_mmap(mem_base_allocator* context, void* ptr, u64 size)
{
	munmap(ptr, size);
}

mem_base_allocator* mem_base_allocator_default()
{
	static mem_base_allocator base = {};
	if(base.reserve == 0)
	{
		base.reserve = mem_base_reserve_mmap;
		base.commit = mem_base_nop;
		base.decommit = mem_base_nop;
		base.release = mem_base_release_mmap;
	}
	return(&base);
}
