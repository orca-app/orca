/************************************************************//**
*
*	@file: platform_io_common.c
*	@author: Martin Fouilleul
*	@date: 25/05/2023
*
*****************************************************************/

#include"platform_io.h"
//------------------------------------------------------------------------------
// File stream read/write API
//------------------------------------------------------------------------------

file_handle file_open(str8 path, file_open_flags flags)
{
	io_req req = {.op = IO_OP_OPEN_AT,
	              .size = path.len,
	              .buffer = path.ptr,
	              .openFlags = flags};

	io_cmp cmp = io_wait_single_req(&req);

	//WARN: we always return a handle that can be queried for errors. Handles must be closed
	//      even if there was an error when opening
	return(cmp.handle);
}

void file_close(file_handle file)
{
	io_req req = {.op = IO_OP_CLOSE,
	              .handle = file};
	io_wait_single_req(&req);
}

i64 file_seek(file_handle file, i64 offset, file_whence whence)
{
	io_req req = {.op = IO_OP_SEEK,
	              .handle = file,
	              .offset = offset,
	              .whence = whence};

	io_cmp cmp = io_wait_single_req(&req);
	return(cmp.offset);
}

i64 file_pos(file_handle file)
{
	return(file_seek(file, 0, FILE_SEEK_CURRENT));
}

u64 file_write(file_handle file, u64 size, char* buffer)
{
	io_req req = {.op = IO_OP_WRITE,
	              .handle = file,
	              .size = size,
	              .buffer = buffer};

	io_cmp cmp = io_wait_single_req(&req);
	return(cmp.size);
}

u64 file_read(file_handle file, u64 size, char* buffer)
{
	io_req req = {.op = IO_OP_READ,
	              .handle = file,
	              .size = size,
	              .buffer = buffer};

	io_cmp cmp = io_wait_single_req(&req);
	return(cmp.size);
}

io_error file_last_error(file_handle file)
{
	io_req req = {.op = IO_OP_ERROR,
	              .handle = file};

	io_cmp cmp = io_wait_single_req(&req);
	return((io_error)cmp.result);
}

file_status file_get_status(file_handle file)
{
	file_status status = {0};
	io_req req = {.op = IO_OP_FSTAT,
	              .handle = file,
	              .size = sizeof(file_status),
	              .buffer = (char*)&status};

	io_cmp cmp = io_wait_single_req(&req);
	return(status);
}

u64 file_size(file_handle file)
{
	file_status status = file_get_status(file);
	return(status.size);
}
