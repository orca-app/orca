/************************************************************//**
*
*	@file: orca_memory.c
*	@author: Martin Fouilleul
*	@date: 17/04/2023
*
*****************************************************************/

#include"platform_memory.h"

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
