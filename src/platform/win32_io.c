/************************************************************//**
*
*	@file: win32_io.c
*	@author: Martin Fouilleul
*	@date: 25/05/2023
*
*****************************************************************/

#include<errno.h>
#include<limits.h>
#include<shlwapi.h>
#include<winioctl.h>

#include"platform_io_internal.c"
#include"platform_io_common.c"

io_error io_raw_last_error()
{
	io_error error = 0;

	int winError = GetLastError();
	switch(winError)
	{
		case ERROR_SUCCESS:
			error = IO_OK;
			break;

		case ERROR_ACCESS_DENIED:
			error = IO_ERR_PERM;
			break;

		case ERROR_FILE_NOT_FOUND:
		case ERROR_PATH_NOT_FOUND:
		case ERROR_INVALID_DRIVE:
		case ERROR_DIRECTORY:
			error = IO_ERR_NO_ENTRY;
			break;

		case ERROR_TOO_MANY_OPEN_FILES:
			error = IO_ERR_MAX_FILES;
			break;

		case ERROR_NOT_ENOUGH_MEMORY:
		case ERROR_OUTOFMEMORY:
			error = IO_ERR_MEM;
			break;

		case ERROR_DEV_NOT_EXIST:
			error = IO_ERR_NO_DEVICE;
			break;

		case ERROR_FILE_EXISTS:
		case ERROR_ALREADY_EXISTS:
			error = IO_ERR_EXISTS;
			break;

		case ERROR_BUFFER_OVERFLOW:
		case ERROR_FILENAME_EXCED_RANGE:
			error = IO_ERR_PATH_LENGTH;
			break;

		case ERROR_FILE_TOO_LARGE:
			error = IO_ERR_FILE_SIZE;
			break;

		//TODO: complete

		default:
			error = IO_ERR_UNKNOWN;
			break;
	}
	return(error);
}

str16 win32_utf8_to_wide_null_terminated(mem_arena* arena, str8 s)
{
	str16 res = {0};
	res.len = 1 + MultiByteToWideChar(CP_UTF8, 0, s.ptr, s.len, NULL, 0);
	res.ptr = mem_arena_alloc_array(arena, u16, res.len);
	MultiByteToWideChar(CP_UTF8, 0, s.ptr, s.len, res.ptr, res.len);
	res.ptr[res.len-1] = '\0';
	return(res);
}

str8 win32_wide_to_utf8(mem_arena* arena, str16 s)
{
	str8 res = {0};
	res.len = WideCharToMultiByte(CP_UTF8, 0, s.ptr, s.len, NULL, 0, NULL, NULL);
	res.ptr = mem_arena_alloc_array(arena, u8, res.len);
	WideCharToMultiByte(CP_UTF8, 0, s.ptr, s.len, res.ptr, res.len, NULL, NULL);
	return(res);
}

str16 win32_path_from_handle_null_terminated(mem_arena* arena, HANDLE handle)
{
	str16 res = {0};

	res.len = GetFinalPathNameByHandleW(handle, NULL, 0, FILE_NAME_NORMALIZED);
	if(res.len)
	{
		res.ptr = mem_arena_alloc_array(arena, u16, res.len);
		if(!GetFinalPathNameByHandleW(handle, res.ptr, res.len, FILE_NAME_NORMALIZED))
		{
			res.len = 0;
			res.ptr = 0;
		}
	}
	return(res);
}

typedef HANDLE io_file_desc;

io_file_desc io_file_desc_nil()
{
	return(INVALID_HANDLE_VALUE);
}

bool io_file_desc_is_nil(io_file_desc fd)
{
	return(fd == NULL || fd == INVALID_HANDLE_VALUE);
}

io_file_desc io_raw_open_at(io_file_desc dirFd, str8 path, file_access_rights accessRights, file_open_flags openFlags)
{
	HANDLE handle = INVALID_HANDLE_VALUE;

	// convert flags
	DWORD win32AccessFlags = 0;
	DWORD win32ShareMode = FILE_SHARE_READ|FILE_SHARE_WRITE;
	DWORD win32CreateFlags = 0;
	DWORD win32AttributeFlags = FILE_ATTRIBUTE_NORMAL
	                          | FILE_FLAG_BACKUP_SEMANTICS;

	if(accessRights & FILE_ACCESS_READ)
	{
		win32AccessFlags |= GENERIC_READ;
	}
	if(accessRights & FILE_ACCESS_WRITE)
	{
		if(accessRights & FILE_OPEN_APPEND)
		{
			win32AccessFlags |= FILE_APPEND_DATA;
		}
		else
		{
			win32AccessFlags |= GENERIC_WRITE;
		}
	}

	if(openFlags & FILE_OPEN_TRUNCATE)
	{
		if(openFlags & FILE_OPEN_CREATE)
		{
			win32CreateFlags |= CREATE_ALWAYS;
		}
		else
		{
			win32CreateFlags |= TRUNCATE_EXISTING;
		}
	}
	if(openFlags & FILE_OPEN_CREATE)
	{
		if(!(win32CreateFlags & CREATE_ALWAYS))
		{
			win32CreateFlags |= OPEN_ALWAYS;
		}
	}
	if(  !(win32CreateFlags & OPEN_ALWAYS)
		 && !(win32CreateFlags & CREATE_ALWAYS)
		 && !(win32CreateFlags & TRUNCATE_EXISTING))
	{
		win32CreateFlags |= OPEN_EXISTING;
	}

	if(openFlags & FILE_OPEN_SYMLINK)
	{
		win32AttributeFlags |= FILE_FLAG_OPEN_REPARSE_POINT;
	}

	mem_arena_scope scratch = mem_scratch_begin();
	str16 pathW = win32_utf8_to_wide_null_terminated(scratch.arena, path);

	if(dirFd == NULL || dirFd == INVALID_HANDLE_VALUE)
	{
		handle = CreateFileW(pathW.ptr, win32AccessFlags, win32ShareMode, NULL, win32CreateFlags, win32AttributeFlags, NULL);
	}
	else
	{
		str16 dirPathW = win32_path_from_handle_null_terminated(scratch.arena, dirFd);

		if(dirPathW.len && pathW.len)
		{
			u64 fullPathWSize = dirPathW.len + pathW.len;
			LPWSTR fullPathW = mem_arena_alloc_array(scratch.arena, u16, fullPathWSize);
			memcpy(fullPathW, dirPathW.ptr, (dirPathW.len-1)*sizeof(u16));
			fullPathW[dirPathW.len-1] = '\\';
			memcpy(fullPathW + dirPathW.len, pathW.ptr, pathW.len*sizeof(u16));

			LPWSTR canonical = mem_arena_alloc_array(scratch.arena, wchar_t, fullPathWSize);
			PathCanonicalizeW(canonical, fullPathW);

			handle = CreateFileW(canonical, win32AccessFlags, win32ShareMode, NULL, win32CreateFlags, win32AttributeFlags, NULL);
		}
	}
	mem_scratch_end(scratch);
	return(handle);
}

void io_raw_close(io_file_desc fd)
{
	CloseHandle(fd);
}

io_error io_raw_stat(io_file_desc fd, file_status* status)
{
	io_error error = IO_OK;

	BY_HANDLE_FILE_INFORMATION info;
	if(!GetFileInformationByHandle(fd, &info))
	{
		error = io_raw_last_error();
	}
	else
	{
		status->size = (((u64)info.nFileSizeHigh)<<32) | ((u64)info.nFileSizeLow);
		status->uid = ((u64)info.nFileIndexHigh<<32) | ((u64)info.nFileIndexLow);

		DWORD attrRegularSet = FILE_ATTRIBUTE_ARCHIVE
		                     | FILE_ATTRIBUTE_COMPRESSED
		                     | FILE_ATTRIBUTE_ENCRYPTED
		                     | FILE_ATTRIBUTE_HIDDEN
		                     | FILE_ATTRIBUTE_NORMAL
		                     | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED
		                     | FILE_ATTRIBUTE_OFFLINE
		                     | FILE_ATTRIBUTE_READONLY
		                     | FILE_ATTRIBUTE_SPARSE_FILE
		                     | FILE_ATTRIBUTE_SYSTEM
		                     | FILE_ATTRIBUTE_TEMPORARY;

		if((info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
		{
			FILE_ATTRIBUTE_TAG_INFO tagInfo;
			if(!GetFileInformationByHandleEx(fd, FileAttributeTagInfo, &tagInfo, sizeof(tagInfo)))
			{
				error = io_raw_last_error();
			}
			else if(tagInfo.ReparseTag == IO_REPARSE_TAG_SYMLINK)
			{
				status->type = MP_FILE_SYMLINK;
			}
			else
			{
				status->type = MP_FILE_UNKNOWN;
			}
		}
		else if(info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			status->type = MP_FILE_DIRECTORY;
		}
		else if(info.dwFileAttributes & attrRegularSet)
		{
			status->type = MP_FILE_REGULAR;
		}
		else
		{
			//TODO: might want to check for socket/block/character devices? (otoh MS STL impl. doesn't seem to do it)
			status->type = MP_FILE_UNKNOWN;
		}

		status->perm = MP_FILE_OWNER_READ | MP_FILE_GROUP_READ | MP_FILE_OTHER_READ;
		if(!(info.dwFileAttributes & FILE_ATTRIBUTE_READONLY))
		{
			status->perm = MP_FILE_OWNER_WRITE | MP_FILE_GROUP_WRITE | MP_FILE_OTHER_WRITE;
		}
		//TODO: times
	}
	return(error);
}

u64 io_raw_uid(io_file_desc fd)
{
	file_status status;
	io_raw_stat(fd, &status);
	return(status.uid);
}

io_error io_raw_stat_at(io_file_desc dirFd, str8 name, file_open_flags openFlags, file_status* status)
{
	io_error error = IO_OK;
	io_file_desc fd = io_raw_open_at(dirFd, name, FILE_ACCESS_READ, FILE_OPEN_SYMLINK);
	if(io_file_desc_is_nil(fd))
	{
		error = io_raw_last_error();
	}
	else
	{
		error = io_raw_stat(fd, status);
		io_raw_close(fd);
	}
	return(error);
}

typedef struct io_raw_read_link_result
{
	io_error error;
	str8 target;
} io_raw_read_link_result;

typedef struct
{
	ULONG ReparseTag;
	USHORT ReparseDataLength;
	USHORT Reserved;
	union
	{
		struct
		{
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			ULONG Flags;
			WCHAR PathBuffer[1];
		} SymbolicLinkReparseBuffer;

		struct
		{
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			WCHAR PathBuffer[1];
		} MountPointReparseBuffer;

		struct
		{
			UCHAR  DataBuffer[1];
		} GenericReparseBuffer;
	};
} REPARSE_DATA_BUFFER;

io_raw_read_link_result io_raw_read_link(mem_arena* arena, io_file_desc fd)
{
	io_raw_read_link_result result = {0};

	char buffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
	DWORD bytesReturned;

	if(!DeviceIoControl(fd, FSCTL_GET_REPARSE_POINT, NULL, 0, buffer, MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &bytesReturned, 0))
	{
		result.error = io_raw_last_error();
	}
	else
	{
		REPARSE_DATA_BUFFER* reparse = (REPARSE_DATA_BUFFER*)buffer;
		if(reparse->ReparseTag == IO_REPARSE_TAG_SYMLINK)
		{
			str16 nameW = {0};
			nameW.len = reparse->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(wchar_t);
			nameW.ptr = (u16*)((char*)reparse->SymbolicLinkReparseBuffer.PathBuffer + reparse->SymbolicLinkReparseBuffer.SubstituteNameOffset);
			result.target = win32_wide_to_utf8(arena, nameW);
		}
		else
		{
			result.error = IO_ERR_UNKNOWN;
		}
	}
	return(result);
}

io_raw_read_link_result io_raw_read_link_at(mem_arena* arena, io_file_desc dirFd, str8 name)
{
	io_file_desc fd = io_raw_open_at(dirFd, name, FILE_ACCESS_READ, FILE_OPEN_SYMLINK);
	io_raw_read_link_result result = io_raw_read_link(arena, fd);
	io_raw_close(fd);
	return(result);
}

typedef struct io_open_restrict_context
{
	io_error error;
	u64 rootUID;
	io_file_desc rootFd;
	io_file_desc fd;

} io_open_restrict_context;

io_error io_open_restrict_enter(io_open_restrict_context* context, str8 name, file_access_rights accessRights, file_open_flags openFlags)
{
	io_file_desc nextFd = io_raw_open_at(context->fd, name, accessRights, openFlags);
	if(io_file_desc_is_nil(nextFd))
	{
		context->error = io_raw_last_error();
	}
	else
	{
		if(context->fd != context->rootFd)
		{
			io_raw_close(context->fd);
		}
		context->fd = nextFd;
	}
	return(context->error);
}

typedef struct io_open_restrict_result
{
	io_error error;
	HANDLE h;
} io_open_restrict_result;


io_open_restrict_result io_open_path_restrict(io_file_desc dirFd, str8 path, file_access_rights accessRights, file_open_flags openFlags)
{
	mem_arena_scope scratch = mem_scratch_begin();

	str8_list sep = {0};
	str8_list_push(scratch.arena, &sep, STR8("/"));
	str8_list pathElements = str8_split(scratch.arena, path, sep);

	io_open_restrict_context context = {
		.error = IO_OK,
		.rootFd = dirFd,
		.rootUID = io_raw_uid(dirFd),
		.fd = dirFd,
	};

	for_list(&pathElements.list, elt, str8_elt, listElt)
	{
		str8 name = elt->string;

		if(!str8_cmp(name, STR8(".")))
		{
			//NOTE: skip;
			continue;
		}
		else if(!str8_cmp(name, STR8("..")))
		{
			//NOTE: check that we don't escape root dir
			if(io_raw_uid(context.fd) == context.rootUID)
			{
				context.error = IO_ERR_WALKOUT;
				break;
			}
			else
			{
				io_open_restrict_enter(&context, name, FILE_ACCESS_READ, 0);
			}
		}
		else
		{
			file_status status = {0};
			context.error = io_raw_stat_at(context.fd, name, FILE_OPEN_SYMLINK, &status);
			if(context.error)
			{
				break;
			}

			if(status.type == MP_FILE_SYMLINK)
			{
				//NOTE: read link target and add to file
				io_raw_read_link_result link = io_raw_read_link_at(scratch.arena, context.fd, name);

				if(link.error)
				{
					context.error = link.error;
					break;
				}

				if(link.target.len == 0)
				{
					// skip
				}
				else if(  link.target.ptr[0] == '/'
				       || link.target.ptr[0] == '\\')
				{
					context.error = IO_ERR_WALKOUT;
					break;
				}
				else
				{
					str8_list linkElements = str8_split(scratch.arena, link.target, sep);
					if(!list_empty(&linkElements.list))
					{
						//NOTE: insert linkElements into pathElements after elt
						list_elt* tmp = elt->listElt.next;
						elt->listElt.next = linkElements.list.first;
						linkElements.list.last->next = tmp;
						if(!tmp)
						{
							pathElements.list.last = linkElements.list.last;
						}
					}
				}
			}
			else if(status.type == MP_FILE_DIRECTORY)
			{
				//NOTE: descend in directory
				io_open_restrict_enter(&context, name, FILE_ACCESS_READ, 0);
			}
			else if(status.type == MP_FILE_REGULAR)
			{
				//NOTE: check that we're at the end of path and open that file
				if(&elt->listElt != list_last(&pathElements.list))
				{
					context.error = IO_ERR_NOT_DIR;
					break;
				}
				else
				{
					io_open_restrict_enter(&context, name, accessRights, openFlags);
				}
			}
			else
			{
				context.error = IO_ERR_NO_ENTRY;
			}
		}
	}

	if(context.error)
	{
		if(context.fd != context.rootFd)
		{
			io_raw_close(context.fd);
			context.fd = io_file_desc_nil();
		}
	}

	io_open_restrict_result result = {
		.error = context.error,
		.h = context.fd
	};

	mem_scratch_end(scratch);
	return(result);
}

io_cmp io_close(file_slot* slot, io_req* req, file_table* table)
{
	io_cmp cmp = {0};
	if(slot->fd)
	{
		CloseHandle(slot->fd);
	}
	file_slot_recycle(table, slot);
	return(cmp);
}

io_cmp io_fstat(file_slot* slot, io_req* req)
{
	io_cmp cmp = {0};

	if(req->size < sizeof(file_status))
	{
		cmp.error = IO_ERR_ARG;
	}
	else
	{
		slot->error = io_raw_stat(slot->fd, (file_status*)req->buffer);
		cmp.error = slot->error;
	}
	return(cmp);
}

io_cmp io_seek(file_slot* slot, io_req* req)
{
	io_cmp cmp = {0};

	DWORD whence;
	switch(req->whence)
	{
		case FILE_SEEK_CURRENT:
			whence = FILE_CURRENT;
			break;

		case FILE_SEEK_SET:
			whence = FILE_BEGIN;
			break;

		case FILE_SEEK_END:
			whence = FILE_END;
	}

	LARGE_INTEGER off = {.QuadPart = req->offset};
	LARGE_INTEGER newPos = {0};

	if(!SetFilePointerEx(slot->fd, off, &newPos, whence))
	{
		slot->error = io_raw_last_error();
		cmp.error = slot->error;
	}
	else
	{
		cmp.result = newPos.QuadPart;
	}

	return(cmp);
}

io_cmp io_read(file_slot* slot, io_req* req)
{
	io_cmp cmp = {0};

	DWORD bytesRead = 0;

	if(!ReadFile(slot->fd, req->buffer, req->size, &bytesRead, NULL))
	{
		slot->error = io_raw_last_error();
		cmp.result = 0;
		cmp.error = slot->error;
	}
	else
	{
		cmp.result = bytesRead;
	}
	return(cmp);
}

io_cmp io_write(file_slot* slot, io_req* req)
{
	io_cmp cmp = {0};

	DWORD bytesWritten = 0;

	if(!WriteFile(slot->fd, req->buffer, req->size, &bytesWritten, NULL))
	{
		slot->error = io_raw_last_error();
		cmp.result = 0;
		cmp.error = slot->error;
	}
	else
	{
		cmp.result = bytesWritten;
	}

	return(cmp);
}

io_cmp io_get_error(file_slot* slot, io_req* req)
{
	io_cmp cmp = {0};
	cmp.result = slot->error;
	return(cmp);
}

io_cmp io_wait_single_req_with_table(io_req* req, file_table* table)
{
	io_cmp cmp = {0};

	file_slot* slot = file_slot_from_handle(table, req->handle);
	if(!slot)
	{
		if(req->op != IO_OP_OPEN_AT)
		{
			cmp.error = IO_ERR_HANDLE;
		}
	}
	else if(slot->fatal && req->op != IO_OP_CLOSE && req->op != IO_OP_ERROR)
	{
		cmp.error = IO_ERR_PREV;
	}

	if(cmp.error == IO_OK)
	{
		switch(req->op)
		{
			case IO_OP_OPEN_AT:
				cmp = io_open_at(slot, req, table);
				break;

			case IO_OP_FSTAT:
				cmp = io_fstat(slot, req);
				break;

			case IO_OP_CLOSE:
				cmp = io_close(slot, req, table);
				break;

			case IO_OP_READ:
				cmp = io_read(slot, req);
				break;

			case IO_OP_WRITE:
				cmp = io_write(slot, req);
				break;

			case IO_OP_SEEK:
				cmp = io_seek(slot, req);
				break;

			case IO_OP_ERROR:
				cmp = io_get_error(slot, req);
				break;

			default:
				cmp.error = IO_ERR_OP;
				if(slot)
				{
					slot->error = cmp.error;
				}
				break;
		}
	}
	return(cmp);
}
