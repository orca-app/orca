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

typedef struct io_open_restrict_result
{
	io_error error;
	int fd;

} io_open_restrict_result;

io_open_restrict_result io_open_restrict(int dirFd, str8 path, int flags, mode_t mode)
{
	io_open_restrict_result result = {.fd = -1};

	mem_arena_scope scratch = mem_scratch_begin();

	str8_list sep = {0};
	str8_list_push(scratch.arena, &sep, STR8("/"));
	str8_list pathElements = str8_split(scratch.arena, path, sep);

	result.fd = dirFd;
	if(result.fd < 0)
	{
		result.error = io_convert_errno(errno);
		goto error;
	}
	struct stat dirStat;
	if(fstat(result.fd, &dirStat))
	{
		result.error = io_convert_errno(errno);
		goto error;
	}
	ino_t baseInode = dirStat.st_ino;
	ino_t currentInode = baseInode;

	for_list(&pathElements.list, elt, str8_elt, listElt)
	{
		str8 name = elt->string;

		if(!str8_cmp(name, STR8(".")))
		{
			//NOTE: skip
			continue;
		}
		else if(!str8_cmp(name, STR8("..")))
		{
			//NOTE: check that we don't escape 'root' dir
			if(currentInode == baseInode)
			{
				result.error = IO_ERR_WALKOUT;
				goto error;
			}
			else
			{
				// open that dir and continue
				int nextFd = openat(result.fd, "..", O_RDONLY);
				if(!nextFd)
				{
					result.error = io_convert_errno(errno);
					goto error;
				}
				else
				{
					close(result.fd);
					result.fd = nextFd;
					if(fstat(result.fd, &dirStat))
					{
						result.error = io_convert_errno(errno);
						goto error;
					}
					currentInode = dirStat.st_ino;
				}
			}
		}
		else
		{
			char* nameCStr = str8_to_cstring(scratch.arena, name);

			if(faccessat(result.fd, nameCStr, F_OK, AT_SYMLINK_NOFOLLOW))
			{
				//NOTE: file does not exist. if we're not at the end of path, or we don't have create flag,
				//      return a IO_ERR_NO_ENTRY
				if(&elt->listElt != list_last(&pathElements.list) && !(flags & O_CREAT))
				{
					result.error = IO_ERR_NO_ENTRY;
					goto error;
				}
				else
				{
					//NOTE create the flag and return
					int nextFd = openat(result.fd, nameCStr, flags);
					if(!nextFd)
					{
						result.error = io_convert_errno(errno);
						goto error;
					}
					else
					{
						close(result.fd);
						result.fd = nextFd;
					}
					continue;
				}
			}

			struct stat st;
			if(fstatat(result.fd, nameCStr, &st, AT_SYMLINK_NOFOLLOW))
			{
				result.error = io_convert_errno(errno);
				goto error;
			}

			int type = st.st_mode & S_IFMT;

			if(type == S_IFLNK)
			{
				// symlink, check that it's relative, and insert it in elements
				char buff[PATH_MAX+1];
				int r = readlinkat(result.fd, nameCStr, buff, PATH_MAX);
				if(r<0)
				{
					result.error = io_convert_errno(errno);
					goto error;
				}
				else if(r == 0)
				{
					//NOTE: skip
				}
				else if(buff[0] == '/')
				{
					result.error = IO_ERR_WALKOUT;
					goto error;
				}
				else
				{
					buff[r] = '\0';

					str8_list linkElements = str8_split(scratch.arena, str8_from_buffer(r, buff), sep);
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
			else if(type == S_IFDIR)
			{
				// dir, open it and continue
				int nextFd = openat(result.fd, nameCStr, O_RDONLY);
				if(!nextFd)
				{
					result.error = io_convert_errno(errno);
					goto error;
				}
				else
				{
					close(result.fd);
					result.fd = nextFd;
					currentInode = st.st_ino;
				}
			}
			else if(type == S_IFREG)
			{
				// regular file, check that we're at the last element
				if(&elt->listElt != list_last(&pathElements.list))
				{
					result.error = IO_ERR_NOT_DIR;
					goto error;
				}
			}
			else
			{
				result.error = IO_ERR_NOT_DIR;
				goto error;
			}
		}
	}
	goto end;

	error:
	{
		close(result.fd);
		result.fd = -1;
	}
	end:
	mem_scratch_end(scratch);
	return(result);
}

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


		//TODO: if FILE_OPEN_RESTRICT, do the file traversal to check that path is in the
		//      subtree rooted at atSlot->fd
		slot->fd = -1;
		if(atSlot)
		{
			if(path.len && path.ptr[0] == '/')
			{
				//NOTE: if path is absolute, change for a relative one, otherwise openat ignores fd.
				str8_list list = {0};
				str8_list_push(scratch.arena, &list, STR8("."));
				str8_list_push(scratch.arena, &list, path);
				path = str8_list_join(scratch.arena, list);
			}
			char* pathCStr = str8_to_cstring(scratch.arena, path);

			if(req->openFlags & FILE_OPEN_RESTRICT)
			{
				io_open_restrict_result res = io_open_restrict(atSlot->fd, path, flags, mode);
				if(res.error == IO_OK)
				{
					slot->fd = res.fd;
				}
				else
				{
					slot->fatal = true;
					slot->error = res.error;
				}
			}
			else
			{
				slot->fd = openat(atSlot->fd, pathCStr, flags, mode);
				if(slot->fd < 0)
				{
					slot->fatal = true;
					slot->error = io_convert_errno(errno);
				}
			}
		}
		else
		{
			char* pathCStr = str8_to_cstring(scratch.arena, path);
			slot->fd = open(pathCStr, flags, mode);
			if(slot->fd < 0)
			{
				slot->fatal = true;
				slot->error = io_convert_errno(errno);
			}
		}

		mem_scratch_end(scratch);
		cmp.error = slot->error;
	}

	return(cmp);
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
