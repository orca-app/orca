/************************************************************//**
*
*	@file: io.c
*	@author: Martin Fouilleul
*	@date: 09/05/2023
*
*****************************************************************/
#include"io.h"
#include"io_stubs.c"

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

size_t file_size(file_handle file)
{
	io_req req = {.op = IO_OP_SIZE,
	              .handle = file};

	io_cmp cmp = io_wait_single_req(&req);
	return((size_t)cmp.result);
}

size_t file_pos(file_handle file)
{
	io_req req = {.op = IO_OP_POS,
	              .handle = file};

	io_cmp cmp = io_wait_single_req(&req);
	return((size_t)cmp.result);
}

size_t file_seek(file_handle file, long offset, file_whence whence)
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

int file_error(file_handle file)
{
	io_req req = {.op = IO_OP_ERROR,
	              .handle = file};

	io_cmp cmp = io_wait_single_req(&req);
	return((int)cmp.result);
}
