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

oc_file oc_file_nil()
{
	return((oc_file){0});
}

bool oc_file_is_nil(oc_file handle)
{
	return(handle.h == 0);
}

oc_file oc_file_open(oc_str8 path, oc_file_access rights, oc_file_open_flags flags)
{
	oc_io_req req = {.op = OC_IO_OPEN_AT,
	              .size = path.len,
	              .buffer = path.ptr,
	              .open.rights = rights,
	              .open.flags = flags };

	oc_io_cmp cmp = oc_io_wait_single_req(&req);

	//WARN: we always return a handle that can be queried for errors. Handles must be closed
	//      even if there was an error when opening
	return(cmp.handle);
}

oc_file oc_file_open_at(oc_file dir, oc_str8 path, oc_file_access rights, oc_file_open_flags flags)
{
	oc_io_req req = {.op = OC_IO_OPEN_AT,
	              .handle = dir,
	              .size = path.len,
	              .buffer = path.ptr,
	              .open.rights = rights,
	              .open.flags = flags,};

	oc_io_cmp cmp = oc_io_wait_single_req(&req);
	return(cmp.handle);
}

void oc_file_close(oc_file file)
{
	oc_io_req req = {.op = OC_IO_CLOSE,
	              .handle = file};
	oc_io_wait_single_req(&req);
}

i64 oc_file_seek(oc_file file, i64 offset, oc_file_whence whence)
{
	oc_io_req req = {.op = OC_IO_SEEK,
	              .handle = file,
	              .offset = offset,
	              .whence = whence};

	oc_io_cmp cmp = oc_io_wait_single_req(&req);
	return(cmp.offset);
}

i64 oc_file_pos(oc_file file)
{
	return(oc_file_seek(file, 0, OC_FILE_SEEK_CURRENT));
}

u64 oc_file_write(oc_file file, u64 size, char* buffer)
{
	oc_io_req req = {.op = OC_IO_WRITE,
	              .handle = file,
	              .size = size,
	              .buffer = buffer};

	oc_io_cmp cmp = oc_io_wait_single_req(&req);
	return(cmp.size);
}

u64 oc_file_read(oc_file file, u64 size, char* buffer)
{
	oc_io_req req = {.op = OC_IO_READ,
	              .handle = file,
	              .size = size,
	              .buffer = buffer};

	oc_io_cmp cmp = oc_io_wait_single_req(&req);
	return(cmp.size);
}

oc_io_error oc_file_last_error(oc_file file)
{
	oc_io_req req = {.op = OC_OC_IO_ERROR,
	              .handle = file};

	oc_io_cmp cmp = oc_io_wait_single_req(&req);
	return((oc_io_error)cmp.result);
}

oc_file_status oc_file_get_status(oc_file file)
{
	oc_file_status status = {0};
	oc_io_req req = {.op = OC_IO_FSTAT,
	              .handle = file,
	              .size = sizeof(oc_file_status),
	              .buffer = (char*)&status};

	oc_io_cmp cmp = oc_io_wait_single_req(&req);
	return(status);
}

u64 oc_file_size(oc_file file)
{
	oc_file_status status = oc_file_get_status(file);
	return(status.size);
}
