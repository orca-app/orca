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

typedef enum { FILE_SET, FILE_END, FILE_CURR } file_whence;

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
	IO_ERR_INVALID, // invalid argument or argument combination
	IO_ERR_PERM,    // access denied
	IO_ERR_PATH,    // path does not exist
	IO_ERR_EXISTS,  // file already exists
	IO_ERR_HANDLE,  // invalid handle
	IO_ERR_MAX_FILES,
	//...
};

typedef struct io_cmp
{
	io_req id;
	io_error error;
	u64 result;
} io_cmp;

#endif //__IO_COMMON_H_
