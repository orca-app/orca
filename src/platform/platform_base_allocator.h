/************************************************************//**
*
*	@file: platform_base_allocator.h
*	@author: Martin Fouilleul
*	@date: 10/09/2021
*	@revision:
*
*****************************************************************/
#ifndef __PLATFORM_BASE_ALLOCATOR_H_
#define __PLATFORM_BASE_ALLOCATOR_H_

#include"typedefs.h"
#include"memory.h"

#ifdef __cplusplus
extern "C" {
#endif

void* mem_base_reserve_mmap(void* context, u64 size);
void mem_base_release_mmap(void* context, void* ptr, u64 size);

mem_base_allocator* mem_base_allocator_default();

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__PLATFORM_BASE_ALLOCATOR_H_
