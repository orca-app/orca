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
	typedef int io_file_desc;
#elif PLATFORM_WINDOWS
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
	#include<windows.h>
	typedef HANDLE io_file_desc;
#endif

typedef struct file_slot
{
	u32 generation;
	io_error error;
	bool fatal;
	list_elt freeListElt;

	file_type type;
	file_access_rights rights;
	io_file_desc fd;

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

MP_API io_cmp io_wait_single_req_with_table(io_req* req, file_table* table);


//-----------------------------------------------------------------------
// raw io primitives
//-----------------------------------------------------------------------

io_file_desc io_file_desc_nil();
bool io_file_desc_is_nil(io_file_desc fd);

/*WARN
	io_raw_xxx_at functions are similar to posix openat() regarding path resolution,
	but with some important differences:
		- If dirFd is a non-nil fd, path is considered relative to dirFd _even if it is an absolute path_
		- If dirFd is a nil fd, it is _ignored_ (i.e., path can be absolute, or relative to the current directory)

	This means that:
		- dirFd behaves more like the _root_ of path, ie dirFd won't be ignore if we pass an absolute path,
		- we don't need a special handle value to use a path relative to the current working directory
		  (we just pass a nil dirFd with a relative path)
*/
io_file_desc io_raw_open_at(io_file_desc dirFd, str8 path, file_access_rights accessRights, file_open_flags openFlags);
void io_raw_close(io_file_desc fd);
io_error io_raw_last_error();
bool io_raw_file_exists_at(io_file_desc dirFd, str8 path, file_open_flags openFlags);
io_error io_raw_fstat(io_file_desc fd, file_status* status);
io_error io_raw_fstat_at(io_file_desc dirFd, str8 path, file_open_flags openFlags, file_status* status);

typedef struct io_raw_read_link_result
{
	io_error error;
	str8 target;
} io_raw_read_link_result;

io_raw_read_link_result io_raw_read_link_at(mem_arena* arena, io_file_desc dirFd, str8 path);

#endif //__PLATFORM_IO_INTERNAL_H_
