/************************************************************//**
*
*	@file: runtime_memory.c
*	@author: Martin Fouilleul
*	@date: 17/04/2023
*
*****************************************************************/

#include"runtime.h"

void* wasm_memory_resize_callback(void* p, unsigned long size, void* userData)
{
	wasm_memory* memory = (wasm_memory*)userData;

	if(memory->committed >= size)
	{
		return(memory->ptr);
	}
	else if(memory->committed < memory->reserved)
	{
		u32 commitSize = size - memory->committed;

		oc_base_allocator* allocator = oc_base_allocator_default();
		oc_base_commit(allocator, memory->ptr + memory->committed, commitSize);
		memory->committed += commitSize;
		return(memory->ptr);
	}
	else
	{
		OC_ABORT("Out of memory");
		return(0);
	}
}

void wasm_memory_free_callback(void* p, void* userData)
{
	wasm_memory* memory = (wasm_memory*)userData;

	oc_base_allocator* allocator = oc_base_allocator_default();
	oc_base_release(allocator, memory->ptr, memory->reserved);
	memset(memory, 0, sizeof(wasm_memory));
}

extern u32 oc_mem_grow(u64 size)
{
	oc_runtime_env* runtime = oc_runtime_env_get();
	wasm_memory* memory = &runtime->wasmMemory;

	size = oc_align_up_pow2(size, d_m3MemPageSize);
	u64 totalSize = size + m3_GetMemorySize(runtime->m3Runtime);

	u32 addr = memory->committed;

	//NOTE: call resize memory, which will call our custom resize callback... this is a bit involved because
	//      wasm3 doesn't allow resizing the memory directly
	M3Result res = ResizeMemory(runtime->m3Runtime, totalSize/d_m3MemPageSize);

	return(addr);
}

void* wasm_memory_offset_to_ptr(wasm_memory* memory, u32 offset)
{
	M3MemoryHeader* header = (M3MemoryHeader*)(memory->ptr);
	OC_DEBUG_ASSERT(offset < header->length, "Wasm offset exceeds memory length");
	return memory->ptr + sizeof(M3MemoryHeader) + offset;
}
