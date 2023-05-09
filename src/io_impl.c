/************************************************************//**
*
*	@file: io_impl.c
*	@author: Martin Fouilleul
*	@date: 09/05/2023
*
*****************************************************************/

#include<fcntl.h>
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
		file_slot* slot = file_slot_alloc(&__globalFileTable);
		if(slot)
		{
			slot->fd = fd;
			file_handle handle = file_handle_from_slot(&__globalFileTable, slot);
			cmp.result = handle.h;
		}
		else
		{
			cmp.error = IO_ERR_MAX_FILES;
			close(fd);
		}
	}
	else
	{
		switch(errno)
		{
			case EACCES:
				cmp.error = IO_ERR_PERM;
				break;

			//TODO: convert open error codes to io_error
			default:
				cmp.error = IO_ERR_INVALID;
		}
	}

	mem_arena_clear_to(scratch, mark);

	return(cmp);
}

io_cmp io_close(io_req* req)
{
	io_cmp cmp = {0};
	file_slot* slot = file_slot_from_handle(&__globalFileTable, req->handle);
	if(slot)
	{
		close(slot->fd);
		file_slot_recycle(&__globalFileTable, slot);
	}
	else
	{
		cmp.error = IO_ERR_HANDLE;
	}
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

	u64 bufferIndex = (u64)req->buffer & 0xffffff;

	if(bufferIndex + req->size > memSize)
	{
		cmp.error = IO_ERR_INVALID;
	}
	else
	{
		req->buffer = memory + bufferIndex;

		switch(req->op)
		{
			case IO_OP_OPEN:
				cmp = io_open(req);
				break;

			case IO_OP_CLOSE:
				cmp = io_close(req);
				break;

			default:
				cmp.error = IO_ERR_INVALID;
				break;
		}
	}

	return(cmp);
}
