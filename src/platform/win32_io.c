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

#include"platform_io_internal.c"
#include"platform_io_common.c"

io_error io_convert_win32_error(int winError)
{
	io_error error = 0;
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

typedef struct io_open_restrict_result
{
	io_error error;
	HANDLE h;
} io_open_restrict_result;


str16 win32_utf8_to_wide_null_terminated(mem_arena* arena, str8 s)
{
	str16 res = {0};
	res.len = 1 + MultiByteToWideChar(CP_UTF8, 0, s.ptr, s.len, NULL, 0);
	res.ptr = mem_arena_alloc_array(arena, u16, res.len);
	MultiByteToWideChar(CP_UTF8, 0, s.ptr, s.len, res.ptr, res.len);
	res.ptr[res.len-1] = '\0';
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

HANDLE io_open_relative(HANDLE dirHandle, str8 path, DWORD accessFlags, DWORD createFlags, DWORD attributesFlags)
{
	HANDLE handle = INVALID_HANDLE_VALUE;

	mem_arena_scope scratch = mem_scratch_begin();

	str16 dirPathW = win32_path_from_handle_null_terminated(scratch.arena, dirHandle);
	str16 pathW = win32_utf8_to_wide_null_terminated(scratch.arena, path);

	if(dirPathW.len && pathW.len)
	{
		u64 fullPathWSize = dirPathW.len + pathW.len;
		LPWSTR fullPathW = mem_arena_alloc_array(scratch.arena, u16, fullPathWSize);
		memcpy(fullPathW, dirPathW.ptr, (dirPathW.len-1)*sizeof(u16));
		fullPathW[dirPathW.len-1] = '\\';
		memcpy(fullPathW + dirPathW.len, pathW.ptr, pathW.len*sizeof(u16));

		LPWSTR canonical = mem_arena_alloc_array(scratch.arena, wchar_t, fullPathWSize);
		PathCanonicalizeW(canonical, fullPathW);

		handle = CreateFileW(canonical, accessFlags, 0, NULL, createFlags, attributesFlags, NULL);
	}
	return(handle);
}

/*
io_open_restrict_result io_open_path_restrict(HANDLE dirHandle, str8 path, DWORD accessFlags, DWORD createFlags, DWORD attributesFalgs)
{
	io_open_restrict_result res = {0};

	mem_arena_scope scratch = mem_scratch_begin();

	str8_list sep = {0};
	str8_list_push(scratch.arena, &sep, STR8("/"));
	str8_list pathElements = str8_split(scratch.arena, path, sep);

	HANDLE handle = NULL;
	DuplicateHandle(GetCurrentProcess(), dirHandle, GetCurrentProcess(), &handle);

	for_list(&pathElements.list, elt, str8_elt, listElt)
	{
		str8 name = elt->string;

		if(!str8_cmp(name, STR8(".")))
		{
			//NOTE: skip;
			continue
		}
		else if(!str8_cmp(name, STR8("..")))
		{
			//NOTE: check that we don't escape root dir
			if(CompareObjectHandles(dirHandle, handle))
			{
				result.error = IO_ERR_WALKOUT;
				goto error;
			}
			else
			{
				// Open parent and continue

				HANDLE nextHandle = NULL;
				OBJECT_ATTRIBUTES attributes;
				//TODO initialize attributes

				IO_STATUS_BLOCK status;

				NtCreateFile(&nextHandle,
				             FILE_LIST_DIRECTORY|FILE_TRAVERSE,
				             &attributes,
				             &status,
				             0,
				             FILE_ATTRIBUTE_NORMAL,
				             FILE_SHARE_READ,
				             FILE_OPEN,
				             FILE_DIRECTORY_FILE,
				             NULL,
				             0);
			}
		}
		else
		{
			//NOTE: open that dir, check if dir/symlink/etc

		}
	}

	error:
		return(res);
}
*/

io_cmp io_open_at(file_slot* atSlot, io_req* req, file_table* table)
{
	io_cmp cmp = {0};

	file_slot* slot = file_slot_alloc(table);
	if(!slot)
	{
		cmp.error = IO_ERR_MAX_FILES;
		cmp.result = 0;
	}
	else
	{
		cmp.handle = file_handle_from_slot(table, slot);

		slot->rights = req->open.rights;
		if(atSlot)
		{
			slot->rights &= atSlot->rights;
		}

		if(slot->rights != req->open.rights)
		{
			slot->error = IO_ERR_PERM;
			slot->fatal = true;
		}
		else
		{
			//NOTE: open
			mem_arena_scope scratch = mem_scratch_begin();

			str8 path = str8_from_buffer(req->size, req->buffer);
			str16 pathW = win32_utf8_to_wide_null_terminated(scratch.arena, path);

			DWORD accessFlags = 0;
			DWORD createFlags = 0;
			DWORD attributesFlags = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS;

			if(req->open.rights & FILE_ACCESS_READ)
			{
				accessFlags |= GENERIC_READ;
			}
			if(req->open.rights & FILE_ACCESS_WRITE)
			{
				if(req->open.rights & FILE_OPEN_APPEND)
				{
					accessFlags |= FILE_APPEND_DATA;
				}
				else
				{
					accessFlags |= GENERIC_WRITE;
				}
			}

			if(req->open.flags & FILE_OPEN_TRUNCATE)
			{
				if(req->open.flags & FILE_OPEN_CREATE)
				{
					createFlags |= CREATE_ALWAYS;
				}
				else
				{
					createFlags |= TRUNCATE_EXISTING;
				}
			}
			if(req->open.flags & FILE_OPEN_CREATE)
			{
				if(!createFlags & CREATE_ALWAYS)
				{
					createFlags |= OPEN_ALWAYS;
				}
			}
			if(  !(createFlags & OPEN_ALWAYS)
		  	&& !(createFlags & CREATE_ALWAYS)
		  	&& !(createFlags & TRUNCATE_EXISTING))
			{
				createFlags |= OPEN_EXISTING;
			}

			if(req->open.flags & FILE_OPEN_SYMLINK)
			{
				attributesFlags |= FILE_FLAG_OPEN_REPARSE_POINT;
			}

			slot->h = INVALID_HANDLE_VALUE;
			if(atSlot)
			{
			/*
				if(req->open.flags & FILE_OPEN_RESTRICT)
				{
					//TODO: if FILE_OPEN_RESTRICT, do the full path traversal to check that path is in the
					//      subtree rooted at atSlot->fd
					io_open_restrict_result res = io_open_path_restrict(atSlot->h, path, accessFlags, createFlags, attributesFalgs);
					if(res.error)
					{
						slot->fatal = true;
						slot->error = res.error;
					}
				}
				else
			*/
				{
					slot->h = io_open_relative(atSlot->h, path, accessFlags, createFlags, attributesFlags);
					if(slot->h == INVALID_HANDLE_VALUE)
					{
						slot->fatal = true;
						slot->error = io_convert_win32_error(GetLastError());
					}
				}
			}
			else
			{
				//TODO: take care of share mode and security attributes, make it consistent with posix impl
				slot->h = CreateFileW(pathW.ptr, accessFlags, 0, NULL, createFlags, attributesFlags, NULL);
				if(slot->h == INVALID_HANDLE_VALUE)
				{
					slot->fatal = true;
					slot->error = io_convert_win32_error(GetLastError());
				}
			}

			mem_scratch_end(scratch);
		}
		cmp.error = slot->error;
	}

	return(cmp);
}

io_cmp io_close(file_slot* slot, io_req* req, file_table* table)
{
	io_cmp cmp = {0};
	if(slot->h)
	{
		CloseHandle(slot->h);
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
		BY_HANDLE_FILE_INFORMATION info;
		if(!GetFileInformationByHandle(slot->h, &info))
		{
			slot->error = io_convert_win32_error(GetLastError());
			cmp.error = slot->error;
		}
		else
		{
			file_status* status = (file_status*)req->buffer;

			status->size = (((u64)info.nFileSizeHigh)<<32) | ((u64)info.nFileSizeLow);

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

			if(info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				status->type = MP_FILE_DIRECTORY;
			}
			else if((info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
			{
				FILE_ATTRIBUTE_TAG_INFO tagInfo;
				if(!GetFileInformationByHandleEx(slot->h, FileAttributeTagInfo, &tagInfo, sizeof(tagInfo)))
				{
					slot->error = io_convert_win32_error(GetLastError());
					cmp.error = slot->error;
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

	if(!SetFilePointerEx(slot->h, off, &newPos, whence))
	{
		slot->error = io_convert_win32_error(GetLastError());
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

	if(!ReadFile(slot->h, req->buffer, req->size, &bytesRead, NULL))
	{
		slot->error = io_convert_win32_error(GetLastError());
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

	if(!WriteFile(slot->h, req->buffer, req->size, &bytesWritten, NULL))
	{
		slot->error = io_convert_win32_error(GetLastError());
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
