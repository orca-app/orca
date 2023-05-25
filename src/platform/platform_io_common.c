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
	io_req req = {.op = IO_OP_OPEN,
	              .size = path.len,
	              .buffer = path.ptr,
	              .openFlags = flags};

	io_cmp cmp = io_wait_single_req(&req);

	//WARN: we always return a handle that can be queried for errors. Handles must be closed
	//      even if there was an error when opening
	file_handle handle = { cmp.result };
	return(handle);
}

void file_close(file_handle file)
{
	io_req req = {.op = IO_OP_CLOSE,
	              .handle = file};
	io_wait_single_req(&req);
}

off_t file_pos(file_handle file)
{
	io_req req = {.op = IO_OP_POS,
	              .handle = file};

	io_cmp cmp = io_wait_single_req(&req);
	return((size_t)cmp.result);
}

off_t file_seek(file_handle file, long offset, file_whence whence)
{
	io_req req = {.op = IO_OP_SEEK,
	              .handle = file,
	              .size = offset,
	              .whence = whence};

	io_cmp cmp = io_wait_single_req(&req);
	return((size_t)cmp.result);
}

size_t file_write(file_handle file, size_t size, char* buffer)
{
	io_req req = {.op = IO_OP_WRITE,
	              .handle = file,
	              .size = size,
	              .buffer = buffer};

	io_cmp cmp = io_wait_single_req(&req);
	return((size_t)cmp.result);
}

size_t file_read(file_handle file, size_t size, char* buffer)
{
	io_req req = {.op = IO_OP_READ,
	              .handle = file,
	              .size = size,
	              .buffer = buffer};

	io_cmp cmp = io_wait_single_req(&req);
	return((size_t)cmp.result);
}

io_error io_last_error(file_handle file)
{
	io_req req = {.op = IO_OP_ERROR,
	              .handle = file};

	io_cmp cmp = io_wait_single_req(&req);
	return((int)cmp.result);
}

file_status file_get_status(file_handle file)
{
	file_status status = {0};
	io_req req = {.op = IO_OP_FSTAT,
	              .handle = file,
	              .size = sizeof(file_status),
	              .buffer = (char*)status};

	io_cmp cmp = io_wait_single_req(&req);
	return(status);
}

size_t file_size(file_handle file)
{
	file_status status = file_get_status(file);
	return(status.size);
}
