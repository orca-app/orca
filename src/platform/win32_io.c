/************************************************************//**
*
*	@file: win32_io.c
*	@author: Martin Fouilleul
*	@date: 25/05/2023
*
*****************************************************************/

#include<errno.h>
#include<limits.h>

#include"platform_io_common.c"

typedef struct file_slot
{
	u32 generation;
	HANDLE h;
	io_error error;
	bool fatal;
	list_elt freeListElt;

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

file_table __globalFileTable = {0};

file_slot* file_slot_alloc(file_table* table)
{
	file_slot* slot = list_pop_entry(&table->freeList, file_slot, freeListElt);
	if(!slot && table->nextSlot < ORCA_MAX_FILE_SLOTS)
	{
		slot = &table->slots[table->nextSlot];
		slot->generation = 1;
		slot->h = NULL;
		table->nextSlot++;
	}
	return(slot);
}

void file_slot_recycle(file_table* table, file_slot* slot)
{
	slot->generation++;
	list_push(&table->freeList, &slot->freeListElt);
}

file_handle file_handle_from_slot(file_table* table, file_slot* slot)
{
	u64 index = slot - table->slots;
	u64 generation = slot->generation;
	file_handle handle = {.h = (generation<<32) | index };
	return(handle);
}

file_slot* file_slot_from_handle(file_table* table, file_handle handle)
{
	file_slot* slot = 0;

	u64 index = handle.h & 0xffffffff;
	u64 generation = handle.h>>32;

	if(index < table->nextSlot)
	{
		file_slot* candidate = &table->slots[index];
		if(candidate->generation == generation)
		{
			slot = candidate;
		}
	}
	return(slot);
}

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

io_cmp io_open_at(file_slot* atSlot, io_req* req)
{
	io_cmp cmp = {0};

	file_slot* slot = file_slot_alloc(&__globalFileTable);
	if(!slot)
	{
		cmp.error = IO_ERR_MAX_FILES;
		cmp.result = 0;
	}
	else
	{
		cmp.handle = file_handle_from_slot(&__globalFileTable, slot);

		//NOTE: open
		mem_arena_scope scratch = mem_scratch_begin();

		int sizeWide = 1 + MultiByteToWideChar(CP_UTF8, 0, req->buffer, req->size, NULL, 0);
		LPWSTR pathWide = mem_arena_alloc_array(scratch.arena, wchar_t, sizeWide);
		MultiByteToWideChar(CP_UTF8, 0, req->buffer, req->size, pathWide, sizeWide);
		pathWide[sizeWide-1] = '\0';

		DWORD accessFlags = 0;
		DWORD createFlags = 0;
		DWORD attributesFlags = FILE_ATTRIBUTE_NORMAL;

		if(req->openFlags & FILE_OPEN_READ)
		{
			accessFlags |= GENERIC_READ;
		}
		if(req->openFlags & FILE_OPEN_WRITE)
		{
			if(req->openFlags & FILE_OPEN_APPEND)
			{
				accessFlags |= FILE_APPEND_DATA;
			}
			else
			{
				accessFlags |= GENERIC_WRITE;
			}
		}

		if(req->openFlags & FILE_OPEN_TRUNCATE)
		{
			if(req->openFlags & FILE_OPEN_CREATE)
			{
				createFlags |= CREATE_ALWAYS;
			}
			else
			{
				createFlags |= TRUNCATE_EXISTING;
			}
		}
		if(req->openFlags & FILE_OPEN_CREATE)
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

		if(req->openFlags & FILE_OPEN_SYMLINK)
		{
			attributesFlags |= FILE_FLAG_OPEN_REPARSE_POINT;
		}

		if(req->openFlags & FILE_OPEN_RESTRICT)
		{
			//TODO: if FILE_OPEN_RESTRICT, do the file traversal to check that path is in the
			//      subtree rooted at atSlot->fd
		}

		//TODO: open at

		//TODO: take care of share mode and security attributes, make it consistent with posix impl
		slot->h = CreateFileW(pathWide, accessFlags, 0, NULL, createFlags, attributesFlags, NULL);

		mem_scratch_end(scratch);

		if(slot->h == INVALID_HANDLE_VALUE)
		{
			slot->fatal = true;
			slot->error = io_convert_win32_error(GetLastError());
		}
		cmp.error = slot->error;
	}

	return(cmp);
}

io_cmp io_close(file_slot* slot, io_req* req)
{
	io_cmp cmp = {0};
	if(slot->h)
	{
		CloseHandle(slot->h);
	}
	file_slot_recycle(&__globalFileTable, slot);
	return(cmp);
}

/*
file_perm io_convert_perm_from_stat(u16 mode)
{
	file_perm perm = mode & 07777;
	return(perm);
}

file_type io_convert_type_from_stat(u16 mode)
{
	file_type type;
	switch(mode & S_IFMT)
	{
		case S_IFIFO:
			type = FILE_FIFO;
			break;

		case S_IFCHR:
			type = FILE_CHARACTER;
			break;

		case S_IFDIR:
			type = FILE_DIRECTORY;
			break;

		case S_IFBLK:
			type = FILE_BLOCK;
			break;

		case S_IFREG:
			type = FILE_REGULAR;
			break;

		case S_IFLNK:
			type = FILE_SYMLINK;
			break;

		case S_IFSOCK:
			type = FILE_SOCKET;
			break;

		default:
			type = FILE_UNKNOWN;
			break;
	}
	return(type);
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
		struct stat s;
		if(fstat(slot->fd, &s))
		{
			slot->error = io_convert_errno(errno);
			cmp.error = slot->error;
		}
		else
		{
			file_status* status = (file_status*)req->buffer;
			status->perm = io_convert_perm_from_stat(s.st_mode);
			status->type = io_convert_type_from_stat(s.st_mode);
			status->size = s.st_size;
			//TODO: times
		}
	}
	return(cmp);
}
*/

io_cmp io_pos(file_slot* slot, io_req* req)
{
	io_cmp cmp = {0};
	LARGE_INTEGER off = {.QuadPart = req->offset};
	LARGE_INTEGER newPos = {0};

	if(!SetFilePointerEx(slot->h, off, &newPos, FILE_CURRENT))
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
		cmp.result = -1;
		slot->error = io_convert_win32_error(GetLastError());
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
		cmp.result = -1;
		slot->error = io_convert_win32_error(GetLastError());
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

io_cmp io_wait_single_req(io_req* req)
{
	io_cmp cmp = {0};

	file_slot* slot = file_slot_from_handle(&__globalFileTable, req->handle);
	if(!slot)
	{
		if(req->op != IO_OP_OPEN_AT)
		{
			cmp.error = IO_ERR_HANDLE;
		}
	}
	else if(slot->fatal && req->op != IO_OP_CLOSE)
	{
		cmp.error = IO_ERR_PREV;
	}

	if(cmp.error == IO_OK)
	{
		switch(req->op)
		{
			case IO_OP_OPEN_AT:
				cmp = io_open_at(slot, req);
				break;
/*
			case IO_OP_FSTAT:
				cmp = io_fstat(slot, req);
				break;
*/
			case IO_OP_CLOSE:
				cmp = io_close(slot, req);
				break;

			case IO_OP_READ:
				cmp = io_read(slot, req);
				break;

			case IO_OP_WRITE:
				cmp = io_write(slot, req);
				break;
/*
			case IO_OP_POS:
				cmp = io_pos(slot, req);
				break;

			case IO_OP_SEEK:
				cmp = io_seek(slot, req);
				break;
*/
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
