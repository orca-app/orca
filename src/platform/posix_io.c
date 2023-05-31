/************************************************************//**
*
*	@file: posix_io.c
*	@author: Martin Fouilleul
*	@date: 25/05/2023
*
*****************************************************************/

#include<errno.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<unistd.h>
#include<limits.h>

#include"platform_io_common.c"

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
		slot->fd = -1;
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

io_error io_convert_errno(int e)
{
	io_error error;
	switch(e)
	{
		case EPERM:
		case EACCES:
		case EROFS:
			error = IO_ERR_PERM;
			break;

		case ENOENT:
			error = IO_ERR_NO_ENTRY;
			break;

		case EINTR:
			error = IO_ERR_INTERRUPT;
			break;

		case EIO:
			error = IO_ERR_PHYSICAL;
			break;

		case ENXIO:
			error = IO_ERR_NO_DEVICE;
			break;

		case EBADF:
			// this should only happen when user tries to write/read to a file handle
			// opened with readonly/writeonly access
			error = IO_ERR_PERM;
			break;

		case ENOMEM:
			error = IO_ERR_MEM;
			break;

		case EFAULT:
		case EINVAL:
		case EDOM:
			error = IO_ERR_ARG;
			break;

		case EBUSY:
		case EAGAIN:
			error = IO_ERR_NOT_READY;
			break;

		case EEXIST:
			error = IO_ERR_EXISTS;
			break;

		case ENOTDIR:
			error = IO_ERR_NOT_DIR;
			break;

		case EISDIR:
			error = IO_ERR_DIR;
			break;

		case ENFILE:
		case EMFILE:
			error = IO_ERR_MAX_FILES;
			break;

		case EFBIG:
			error = IO_ERR_FILE_SIZE;
			break;

		case ENOSPC:
		case EDQUOT:
			error = IO_ERR_SPACE;
			break;

		case ELOOP:
			error = IO_ERR_MAX_LINKS;
			break;

		case ENAMETOOLONG:
			error = IO_ERR_PATH_LENGTH;
			break;

		case EOVERFLOW:
			error = IO_ERR_OVERFLOW;
			break;

		default:
			error = IO_ERR_UNKNOWN;
			break;
	}
	return(error);
}

int io_convert_open_flags(file_open_flags flags)
{
	int oflags = 0;

	if(flags & FILE_OPEN_READ)
	{
		if(flags & FILE_OPEN_WRITE)
		{
			oflags = O_RDWR;
		}
		else
		{
			oflags = O_RDONLY;
		}
	}
	else if(flags & FILE_OPEN_WRITE)
	{
		oflags = O_WRONLY;
	}

	if(flags & FILE_OPEN_TRUNCATE)
	{
		oflags |= O_TRUNC;
	}
	if(flags & FILE_OPEN_APPEND)
	{
		oflags |= O_APPEND;
	}
	if(flags & FILE_OPEN_CREATE)
	{
		oflags |= O_CREAT;
	}
	if(flags & FILE_OPEN_NO_FOLLOW)
	{
		oflags |= O_NOFOLLOW;
	}
	if(flags & FILE_OPEN_SYMLINK)
	{
		oflags |= O_SYMLINK;
	}
	return(oflags);
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

		int flags = io_convert_open_flags(req->openFlags);

		//TODO: allow specifying access
		mode_t mode = S_IRUSR
		            | S_IWUSR
		            | S_IRGRP
		            | S_IWGRP
		            | S_IROTH
		            | S_IWOTH;

		//NOTE: open
		mem_arena_scope scratch = mem_scratch_begin();

		str8 path = str8_from_buffer(req->size, req->buffer);
		char* pathCStr = str8_to_cstring(scratch.arena, path);

		//TODO: if FILE_OPEN_RESTRICT, do the file traversal to check that path is in the
		//      subtree rooted at atSlot->fd
		int fd = -1;
		if(atSlot)
		{
			fd = openat(atSlot->fd, pathCStr, flags, mode);
		}
		else
		{
			fd = open(pathCStr, flags, mode);
		}

		mem_scratch_end(scratch);

		if(fd >= 0)
		{
			slot->fd = fd;
		}
		else
		{
			slot->fd = -1;
			slot->fatal = true;
			slot->error = io_convert_errno(errno);
		}
		cmp.error = slot->error;
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
			type = MP_FILE_FIFO;
			break;

		case S_IFCHR:
			type = MP_FILE_CHARACTER;
			break;

		case S_IFDIR:
			type = MP_FILE_DIRECTORY;
			break;

		case S_IFBLK:
			type = MP_FILE_BLOCK;
			break;

		case S_IFREG:
			type = MP_FILE_REGULAR;
			break;

		case S_IFLNK:
			type = MP_FILE_SYMLINK;
			break;

		case S_IFSOCK:
			type = MP_FILE_SOCKET;
			break;

		default:
			type = MP_FILE_UNKNOWN;
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

io_cmp io_seek(file_slot* slot, io_req* req)
{
	io_cmp cmp = {0};

	int whence;
	switch(req->whence)
	{
		case FILE_SEEK_CURRENT:
			whence = SEEK_CUR;
			break;

		case FILE_SEEK_SET:
			whence = SEEK_SET;
			break;

		case FILE_SEEK_END:
			whence = SEEK_END;
	}
	cmp.result = lseek(slot->fd, req->offset, whence);

	if(cmp.result < 0)
	{
		slot->error = io_convert_errno(errno);
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
		slot->error = io_convert_errno(errno);
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
		slot->error = io_convert_errno(errno);
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

			case IO_OP_FSTAT:
				cmp = io_fstat(slot, req);
				break;

			case IO_OP_CLOSE:
				cmp = io_close(slot, req);
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
				break;
		}
	}
	return(cmp);
}
