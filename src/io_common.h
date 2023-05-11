/************************************************************//**
*
*	@file: io_common.h
*	@author: Martin Fouilleul
*	@date: 09/05/2023
*
*****************************************************************/
#ifndef __IO_COMMON_H_
#define __IO_COMMON_H_

#include"util/typedefs.h"

//------------------------------------------------------------------------------
// Host IO interface
//------------------------------------------------------------------------------

typedef struct { u64 h; } file_handle;

typedef u64 io_req_id;

typedef u32 io_op;
enum
{
	IO_OP_OPEN,
	IO_OP_CLOSE,
	IO_OP_SIZE,
	IO_OP_POS,
	IO_OP_SEEK,
	IO_OP_READ,
	IO_OP_WRITE,
	IO_OP_ERROR,
	//...
};

typedef enum { IO_SEEK_SET, IO_SEEK_END, IO_SEEK_CURRENT } file_whence;

typedef u32 file_open_flags;
enum {
	IO_OPEN_READ,
	IO_OPEN_WRITE,
	IO_OPEN_APPEND,
	IO_OPEN_TRUNCATE,
	IO_OPEN_CREATE,
	//...
};

typedef struct io_req
{
	io_req_id id;
	io_op op;
	file_handle handle;
	u64 offset;
	u64 size;

	union
	{
		char* buffer;
		u64 shadow; // This is a horrible hack to get the same layout on wasm and on host
	};

	union
	{
		file_open_flags openFlags;
		file_whence whence;
	};

} io_req;

typedef i32 io_error;
enum {
	IO_OK = 0,
	IO_ERR_UNKNOWN,
	IO_ERR_OP,          // unsupported operation
	IO_ERR_HANDLE,      // invalid handle
	IO_ERR_PREV,        // previously had a fatal error (last error stored on handle)
	IO_ERR_ARG,         // invalid argument or argument combination
	IO_ERR_PERM,        // access denied
	IO_ERR_SPACE,       // no space left
	IO_ERR_NO_ENTRY,    // file or directory does not exist
	IO_ERR_EXISTS,      // file already exists
	IO_ERR_NOT_DIR,     // path element is not a directory
	IO_ERR_DIR,         // attempted to write directory
	IO_ERR_MAX_FILES,   // max open files reached
	IO_ERR_MAX_LINKS,   // too many symbolic links in path
	IO_ERR_PATH_LENGTH, // path too long
	IO_ERR_FILE_SIZE,   // file too big
	IO_ERR_OVERFLOW,    // offset too big
	IO_ERR_NOT_READY,   // no data ready to be read/written
	IO_ERR_MEM,         // failed to allocate memory
	IO_ERR_INTERRUPT,   // operation interrupted by a signal
	IO_ERR_PHYSICAL,    // physical IO error
	IO_ERR_NO_DEVICE,   // device not found
	IO_ERR_WALKOUT,     // attempted to walk out of root directory

	//...
};

typedef struct io_cmp
{
	io_req id;
	io_error error;
	u64 result;
} io_cmp;

#endif //__IO_COMMON_H_
