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
#include"platform_io_internal.c"

io_file_desc io_file_desc_nil()
{
	return(-1);
}

bool io_file_desc_is_nil(io_file_desc fd)
{
	return(fd < 0);
}

io_error io_raw_last_error()
{
	io_error error = IO_OK;
	switch(errno)
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

int io_convert_access_rights(file_access_rights rights)
{
	int oflags = 0;

	if(rights & FILE_ACCESS_READ)
	{
		if(rights & FILE_ACCESS_WRITE)
		{
			oflags = O_RDWR;
		}
		else
		{
			oflags = O_RDONLY;
		}
	}
	else if(rights & FILE_ACCESS_WRITE)
	{
		oflags = O_WRONLY;
	}
	return(oflags);
}

int io_convert_open_flags(file_open_flags flags)
{
	int oflags = 0;

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

int io_update_dir_flags_at(int dirFd, char* path, int flags)
{
	struct stat s;
	if(!fstatat(dirFd, path, &s, AT_SYMLINK_NOFOLLOW))
	{
		if((s.st_mode & S_IFMT) == S_IFDIR)
		{
			if(flags & O_WRONLY)
			{
				flags &= ~O_WRONLY;
			}
			else if(flags & O_RDWR)
			{
				flags &= ~O_RDWR;
				flags |= O_RDONLY;
			}
		}
	}
	return(flags);
}

io_file_desc io_raw_open_at(io_file_desc dirFd, str8 path, file_access_rights accessRights, file_open_flags openFlags)
{
	int flags = io_convert_access_rights(accessRights);
	flags |= io_convert_open_flags(openFlags);

	mode_t mode = S_IRUSR
	            | S_IWUSR
	            | S_IRGRP
	            | S_IWGRP
	            | S_IROTH
	            | S_IWOTH;

	mem_arena_scope scratch = mem_scratch_begin();

	io_file_desc fd = -1;
	if(dirFd >= 0)
	{
		if(path.len && path.ptr[0] == '/')
		{
			//NOTE: if path is absolute, change for a relative one, otherwise openat ignores fd.
			str8_list list = {0};
			str8_list_push(scratch.arena, &list, STR8("."));
			str8_list_push(scratch.arena, &list, path);
			path = str8_list_join(scratch.arena, list);
		}
	}
	else
	{
		dirFd = AT_FDCWD;
	}

	char* pathCStr = str8_to_cstring(scratch.arena, path);

	flags = io_update_dir_flags_at(dirFd, pathCStr, flags);

	fd = openat(dirFd, pathCStr, flags, mode);
	mem_scratch_end(scratch);

	return(fd);
}

void io_raw_close(io_file_desc fd)
{
	close(fd);
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

io_error io_raw_fstat(io_file_desc fd, file_status* status)
{
	io_error error = IO_OK;
	struct stat s;
	if(fstat(fd, &s))
	{
		error = io_raw_last_error();
	}
	else
	{
		status->uid = s.st_ino;
		status->perm = io_convert_perm_from_stat(s.st_mode);
		status->type = io_convert_type_from_stat(s.st_mode);
		status->size = s.st_size;
		//TODO: times
	}
	return(error);
}

io_error io_raw_fstat_at(io_file_desc dirFd, str8 path, file_open_flags flags, file_status* status)
{
	mem_arena_scope scratch = mem_scratch_begin();

	if(dirFd >= 0)
	{
		if(path.len && path.ptr[0] == '/')
		{
			str8_list list = {0};
			str8_list_push(scratch.arena, &list, STR8("."));
			str8_list_push(scratch.arena, &list, path);
			path = str8_list_join(scratch.arena, list);
		}
	}
	else
	{
		dirFd = AT_FDCWD;
	}

	char* pathCStr = str8_to_cstring(scratch.arena, path);

	int statFlag = (flags & FILE_OPEN_SYMLINK)? AT_SYMLINK_NOFOLLOW : 0;
	io_error error = IO_OK;
	struct stat s;
	if(fstatat(dirFd, pathCStr, &s, statFlag))
	{
		error = io_raw_last_error();
	}
	else
	{
		status->uid = s.st_ino;
		status->perm = io_convert_perm_from_stat(s.st_mode);
		status->type = io_convert_type_from_stat(s.st_mode);
		status->size = s.st_size;
		//TODO: times
	}

	mem_scratch_end(scratch);
	return(error);
}

io_raw_read_link_result io_raw_read_link_at(mem_arena* arena, io_file_desc dirFd, str8 path)
{
	mem_arena_scope scratch = mem_scratch_begin_next(arena);

	if(dirFd >= 0)
	{
		if(path.len && path.ptr[0] == '/')
		{
			str8_list list = {0};
			str8_list_push(scratch.arena, &list, STR8("."));
			str8_list_push(scratch.arena, &list, path);
			path = str8_list_join(scratch.arena, list);
		}
	}
	else
	{
		dirFd = AT_FDCWD;
	}

	char* pathCStr = str8_to_cstring(scratch.arena, path);

	io_raw_read_link_result result = {0};

	char buffer[PATH_MAX];
	u64 bufferSize = PATH_MAX;
	i64 r = readlinkat(dirFd, pathCStr, buffer, bufferSize);

	if(r<0)
	{
		result.error = io_raw_last_error();
	}
	else
	{
		result.target.len = r;
		result.target.ptr = mem_arena_alloc_array(arena, char, result.target.len);
		memcpy(result.target.ptr, buffer, result.target.len);
	}

	mem_scratch_end(scratch);
	return(result);
}

bool io_raw_file_exists_at(io_file_desc dirFd, str8 path, file_open_flags openFlags)
{
	mem_arena_scope scratch = mem_scratch_begin();

	if(dirFd >= 0)
	{
		if(path.len && path.ptr[0] == '/')
		{
			str8_list list = {0};
			str8_list_push(scratch.arena, &list, STR8("."));
			str8_list_push(scratch.arena, &list, path);
			path = str8_list_join(scratch.arena, list);
		}
	}
	else
	{
		dirFd = AT_FDCWD;
	}

	char* pathCStr = str8_to_cstring(scratch.arena, path);

	int flags = (openFlags & FILE_OPEN_SYMLINK)? AT_SYMLINK_NOFOLLOW : 0;
	int r = faccessat(dirFd, pathCStr, F_OK, flags);
	bool result = (r == 0);

	mem_scratch_end(scratch);
	return(result);
}

io_cmp io_close(file_slot* slot, io_req* req, file_table* table)
{
	io_cmp cmp = {0};
	if(slot->fd >= 0)
	{
		close(slot->fd);
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
		struct stat s;
		if(fstat(slot->fd, &s))
		{
			slot->error = io_raw_last_error();
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
		slot->error = io_raw_last_error();
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
		slot->error = io_raw_last_error();
		cmp.result = 0;
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
		slot->error = io_raw_last_error();
		cmp.result = 0;
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
	else if(  slot->fatal
	       && req->op != IO_OP_CLOSE
	       && req->op != IO_OP_ERROR)
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
				break;
		}
	}
	return(cmp);
}
