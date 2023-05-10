/************************************************************//**
*
*	@file: io_impl.c
*	@author: Martin Fouilleul
*	@date: 09/05/2023
*
*****************************************************************/

#include<fcntl.h>
#include<sys/stat.h>
#include<unistd.h>
#include"io_common.h"

//TODO: - file_handle to FILE* association
//      - open / close handles
//      - read / write
//      - file positioning & size

typedef struct file_slot
{
	u32 generation;
	int fd;
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

	if(index < ORCA_MAX_FILE_SLOTS)
	{
		file_slot* candidate = &table->slots[index];
		if(candidate->generation == generation)
		{
			slot = candidate;
		}
	}
	return(slot);
}

io_cmp io_open(io_req* req)
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
		file_handle handle = file_handle_from_slot(&__globalFileTable, slot);
		cmp.result = handle.h;

		int flags = 0;
		if(req->openFlags & IO_OPEN_READ)
		{
			if(req->openFlags & IO_OPEN_WRITE)
			{
				flags = O_RDWR;
			}
			else
			{
				flags = O_RDONLY;
			}
		}
		else if(req->openFlags & IO_OPEN_WRITE)
		{
			flags = O_WRONLY;
		}

		if(req->openFlags & IO_OPEN_TRUNCATE)
		{
			flags |= O_TRUNC;
		}
		if(req->openFlags & IO_OPEN_APPEND)
		{
			flags |= O_APPEND;
		}
		if(req->openFlags & IO_OPEN_CREATE)
		{
			flags |= O_CREAT;
		}

		mode_t mode = S_IRUSR
	            	| S_IWUSR
	            	| S_IRGRP
	            	| S_IWGRP
	            	| S_IROTH
	            	| S_IWOTH;

		//NOTE: build path
		////////////////////////////////////////////////////////////////////////////////////
		//TODO: canonicalize directory path & check that it's inside local app folder
		////////////////////////////////////////////////////////////////////////////////////

		mem_arena* scratch = mem_scratch();
		mem_arena_marker mark = mem_arena_mark(scratch);

		str8_list list = {0};

		str8 execPath = mp_app_get_executable_path(scratch);
		str8 execDir = mp_path_directory(execPath);

		str8_list_push(scratch, &list, execDir);
		str8_list_push(scratch, &list, STR8("/../data/"));
		str8_list_push(scratch, &list, str8_from_buffer(req->size, req->buffer));

		str8 absPath = str8_list_join(scratch, list);
		char* absCString = str8_to_cstring(scratch, absPath);

		//NOTE: open
		int fd = open(absCString, flags, mode);

		if(fd >= 0)
		{
			slot->fd = fd;
			file_handle handle = file_handle_from_slot(&__globalFileTable, slot);
			cmp.result = handle.h;
		}
		else
		{
			slot->fd = -1;
			slot->fatal = true;

			switch(errno)
			{
				case EACCES:
				case EROFS:
					slot->error = IO_ERR_PERM;
					break;

				case EDQUOT:
				case ENOSPC:
					slot->error = IO_ERR_SPACE;
					break;

				case EEXIST:
					slot->error = IO_ERR_EXISTS;
					break;

				case EFAULT:
				case EINVAL:
				case EISDIR:
					slot->error = IO_ERR_ARG;
					break;

				case EMFILE:
				case ENFILE:
					slot->error = IO_ERR_MAX_FILES;
					break;

				case ENAMETOOLONG:
					slot->error = IO_ERR_PATH_LENGTH;
					break;

				case ENOENT:
				case ENOTDIR:
					slot->error = IO_ERR_NO_FILE;
					break;

				case EOVERFLOW:
					slot->error = IO_ERR_FILE_SIZE;
					break;

				default:
					slot->error = IO_ERR_UNKNOWN;
					break;
			}
			cmp.error = slot->error;
		}

		mem_arena_clear_to(scratch, mark);
	}

	return(cmp);
}

io_cmp io_close(file_slot* slot, io_req* req)
{
	io_cmp cmp = {0};
	if(slot->fd >= 0)
	{
		close(slot->fd);
	}
	file_slot_recycle(&__globalFileTable, slot);
	return(cmp);
}

io_cmp io_size(file_slot* slot, io_req* req)
{
	io_cmp cmp = {0};

	struct stat s;
	if(fstat(slot->fd, &s))
	{
		slot->error = IO_ERR_UNKNOWN;
		cmp.error = slot->error;
	}
	else
	{
		cmp.result = s.st_size;
	}

	return(cmp);
}

io_cmp io_pos(file_slot* slot, io_req* req)
{
	io_cmp cmp = {0};
	cmp.result = lseek(slot->fd, 0, SEEK_CUR);
	if(cmp.result < 0)
	{
		switch(errno)
		{
			case EINVAL:
				slot->error = IO_ERR_ARG;
				break;

			case EOVERFLOW:
				slot->error = IO_ERR_OVERFLOW;
				break;

			default:
				slot->error = IO_ERR_UNKNOWN;
				break;
		}
		cmp.error = slot->error;
	}

	return(cmp);
}

io_cmp io_seek(file_slot* slot, io_req* req)
{
	io_cmp cmp = {0};

	int whence;
	switch(req->whence)
	{
		case IO_SEEK_CURRENT:
			whence = SEEK_CUR;
			break;

		case IO_SEEK_SET:
			whence = SEEK_SET;
			break;

		case IO_SEEK_END:
			whence = SEEK_END;
	}
	cmp.result = lseek(slot->fd, req->size, whence);

	if(cmp.result < 0)
	{
		switch(errno)
		{
			case EINVAL:
				slot->error = IO_ERR_ARG;
				break;

			case EOVERFLOW:
				slot->error = IO_ERR_OVERFLOW;
				break;

			default:
				slot->error = IO_ERR_UNKNOWN;
				break;
		}
		cmp.error = slot->error;
	}

	return(cmp);
}

io_cmp io_read(file_slot* slot, io_req* req)
{
	io_cmp cmp = {0};

	cmp.result = read(slot->fd, req->buffer, req->size);

	if(cmp.result < 0)
	{
		switch(errno)
		{
			case EAGAIN:
				slot->error = IO_ERR_NOT_READY;
				break;

			case EFAULT:
			case EINVAL:
				slot->error = IO_ERR_ARG;
				break;

			case ENOBUFS:
			case ENOMEM:
				slot->error = IO_ERR_MEM;
				break;

			default:
				slot->error = IO_ERR_UNKNOWN;
				break;
		}
		cmp.error = slot->error;
	}

	return(cmp);
}

io_cmp io_write(file_slot* slot, io_req* req)
{
	io_cmp cmp = {0};

	cmp.result = write(slot->fd, req->buffer, req->size);

	if(cmp.result < 0)
	{
		switch(errno)
		{
			case EDQUOT:
			case ENOSPC:
				slot->error = IO_ERR_SPACE;
				break;

			case EFAULT:
			case EINVAL:
				slot->error = IO_ERR_ARG;
				break;

			case EAGAIN:
				slot->error = IO_ERR_NOT_READY;
				break;

			case EFBIG:
				slot->error = IO_ERR_FILE_SIZE;
				break;

			case ENOBUFS:
			case ENOMEM:
				slot->error = IO_ERR_MEM;
				break;

			default:
				slot->error = IO_ERR_UNKNOWN;
				break;
		}
		cmp.error = slot->error;
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
	mem_arena* scratch = mem_scratch();

	io_cmp cmp = {0};

	////////////////////////////////////////////////////////////////////////////////////////
	//TODO: we need to convert the req->buffer wasm pointer to a native pointer here
	////////////////////////////////////////////////////////////////////////////////////////
	//TODO: check that req->buffer lies in wasm memory with at least req->size space

	/////////////////////////////////////////////////
	//TODO: we can't read reqs from wasm memory since the pointers are not the same size
	//      (hence the layout won't be the same...)
	/////////////////////////////////////////////////

	//NOTE: for some reason, wasm3 memory doesn't start at the beginning of the block we give it.
	u32 memSize = 0;
	char* memory = (char*)m3_GetMemory(__orcaApp.runtime.m3Runtime, &memSize, 0);

	u64 bufferIndex = (u64)req->buffer & 0xffffffff;

	if(bufferIndex + req->size > memSize)
	{
		cmp.error = IO_ERR_ARG;
	}
	else
	{
		//TODO: avoid modifying req.
		req->buffer = memory + bufferIndex;

		file_slot* slot = 0;
		if(req->op != IO_OP_OPEN)
		{
			slot = file_slot_from_handle(&__globalFileTable, req->handle);
			if(!slot)
			{
				cmp.error = IO_ERR_HANDLE;
			}
			else if(slot->fatal && req->op != IO_OP_CLOSE)
			{
				cmp.error = IO_ERR_PREV;
			}
		}

		if(cmp.error == IO_OK)
		{
			switch(req->op)
			{
				case IO_OP_OPEN:
					cmp = io_open(req);
					break;

				case IO_OP_CLOSE:
					cmp = io_close(slot, req);
					break;

				case IO_OP_SIZE:
					cmp = io_size(slot, req);
					break;

				case IO_OP_READ:
					cmp = io_read(slot, req);
					break;

				case IO_OP_WRITE:
					cmp = io_write(slot, req);
					break;

				case IO_OP_POS:
					cmp = io_pos(slot, req);
					break;

				case IO_OP_SEEK:
					cmp = io_seek(slot, req);
					break;

				case IO_OP_ERROR:
					cmp = io_get_error(slot, req);
					break;

				default:
					cmp.error = IO_ERR_OP;
					break;
			}
		}
	}

	return(cmp);
}
