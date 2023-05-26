/************************************************************//**
*
*	@file: platform_io.h
*	@author: Martin Fouilleul
*	@date: 25/05/2023
*
*****************************************************************/
#ifndef __PLATFORM_IO_H_
#define __PLATFORM_IO_H_

#include"util/typedefs.h"
#include"util/strings.h"

//----------------------------------------------------------------
// IO API
//----------------------------------------------------------------

typedef struct { u64 h; } file_handle;

typedef u32 file_open_flags;
enum {
	FILE_OPEN_READ     = 1<<0,
	FILE_OPEN_WRITE    = 1<<1,
	FILE_OPEN_APPEND   = 1<<2,
	FILE_OPEN_TRUNCATE = 1<<3,
	FILE_OPEN_CREATE   = 1<<4,

	FILE_OPEN_SYMLINK   = 1<<5,
	FILE_OPEN_NO_FOLLOW = 1<<6,
	FILE_OPEN_RESTRICT  = 1<<7,
	//...
};

typedef enum { FILE_SEEK_SET, FILE_SEEK_END, FILE_SEEK_CURRENT } file_whence;

typedef u64 io_req_id;

typedef u32 io_op;
enum
{
	IO_OP_OPEN_AT,
	IO_OP_CLOSE,

	IO_OP_FSTAT,

	IO_OP_POS,
	IO_OP_SEEK,
	IO_OP_READ,
	IO_OP_WRITE,

	IO_OP_ERROR,
	//...
};

typedef struct io_req
{
	io_req_id id;
	io_op op;
	file_handle handle;

	i64 offset;
	u64 size;
	union
	{
		char* buffer;
		u64 unused; // This is a horrible hack to get the same layout on wasm and on host
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
	io_req_id id;
	io_error error;

	union
	{
		i64 result;
		u64 size;
		i64 offset;
		file_handle handle;
		//...
	};
} io_cmp;

//----------------------------------------------------------------
//TODO: complete io queue api
//----------------------------------------------------------------
MP_API io_cmp io_wait_single_req(io_req* req);

//----------------------------------------------------------------
// File IO wrapper API
//----------------------------------------------------------------

MP_API file_handle file_open(str8 path, file_open_flags flags);
MP_API file_handle file_open_relative(file_handle base, str8 path, file_open_flags flags);
MP_API void file_close(file_handle file);

MP_API i64 file_pos(file_handle file);
MP_API i64 file_seek(file_handle file, i64 offset, file_whence whence);

MP_API u64 file_write(file_handle file, u64 size, char* buffer);
MP_API u64 file_read(file_handle file, u64 size, char* buffer);

MP_API io_error file_last_error(file_handle handle);

//----------------------------------------------------------------
// File System wrapper API
//----------------------------------------------------------------

typedef enum file_type
{
	MP_FILE_UNKNOWN,
	MP_FILE_REGULAR,
	MP_FILE_DIRECTORY,
	MP_FILE_SYMLINK,
	MP_FILE_BLOCK,
	MP_FILE_CHARACTER,
	MP_FILE_FIFO,
	MP_FILE_SOCKET,

} file_type;

typedef u16 file_perm;
enum file_perm
{
	MP_FILE_OTHER_EXEC  = 1<<0,
	MP_FILE_OTHER_WRITE = 1<<1,
	MP_FILE_OTHER_READ  = 1<<2,

	MP_FILE_GROUP_EXEC  = 1<<3,
	MP_FILE_GROUP_WRITE = 1<<4,
	MP_FILE_GROUP_READ  = 1<<5,

	MP_FILE_OWNER_EXEC  = 1<<6,
	MP_FILE_OWNER_WRITE = 1<<7,
	MP_FILE_OWNER_READ  = 1<<8,

	MP_FILE_STICKY_BIT  = 1<<9,
	MP_FILE_SET_GID     = 1<<10,
	MP_FILE_SET_UID     = 1<<11,
};

typedef struct file_status
{
	file_type type;
	file_perm perm;
	u64 size;

	//TODO times

} file_status;

MP_API file_status file_get_status(file_handle file);
MP_API u64 file_size(file_handle file);

//TODO: Complete as needed...


#endif //__PLATFORM_IO_H_
