/************************************************************//**
*
*	@file: platform_io_internal.h
*	@author: Martin Fouilleul
*	@date: 10/06/2023
*
*****************************************************************/
#ifndef __PLATFORM_IO_INTERNAL_H_
#define __PLATFORM_IO_INTERNAL_H_

#include"platform_io.h"
#include"platform.h"

#if PLATFORM_MACOS || PLATFORM_LINUX
	#define PLATFORM_IO_NATIVE_MEMBER int fd
#elif PLATFORM_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include<windows>
	#define PLATFORM_IO_NATIVE_MEMBER HANDLE h
#endif

typedef struct file_slot
{
	u32 generation;
	io_error error;
	bool fatal;
	list_elt freeListElt;

	PLATFORM_IO_NATIVE_MEMBER;

} file_slot;

enum
{
	ORCA_MAX_FILE_SLOTS = 256,
};

typedef struct file_table
{
	file_slot slots[ORCA_MAX_FILE_SLOTS];
	u32 nextSlot;
	list_info freeList;
} file_table;

file_slot* file_slot_alloc(file_table* table);
void file_slot_recycle(file_table* table, file_slot* slot);
file_handle file_handle_from_slot(file_table* table, file_slot* slot);
file_slot* file_slot_from_handle(file_table* table, file_handle handle);

io_cmp io_wait_single_req_with_table(io_req* req, file_table* table);

#endif //__PLATFORM_IO_INTERNAL_H_
