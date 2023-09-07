/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>

#include "platform_io_common.c"
#include "platform_io_internal.c"

oc_file_desc oc_file_desc_nil()
{
    return (-1);
}

bool oc_file_desc_is_nil(oc_file_desc fd)
{
    return (fd < 0);
}

oc_io_error oc_io_raw_last_error()
{
    oc_io_error error = OC_IO_OK;
    switch(errno)
    {
        case EPERM:
        case EACCES:
        case EROFS:
            error = OC_IO_ERR_PERM;
            break;

        case ENOENT:
            error = OC_IO_ERR_NO_ENTRY;
            break;

        case EINTR:
            error = OC_IO_ERR_INTERRUPT;
            break;

        case EIO:
            error = OC_IO_ERR_PHYSICAL;
            break;

        case ENXIO:
            error = OC_IO_ERR_NO_DEVICE;
            break;

        case EBADF:
            // this should only happen when user tries to write/read to a file handle
            // opened with readonly/writeonly access
            error = OC_IO_ERR_PERM;
            break;

        case ENOMEM:
            error = OC_IO_ERR_MEM;
            break;

        case EFAULT:
        case EINVAL:
        case EDOM:
            error = OC_IO_ERR_ARG;
            break;

        case EBUSY:
        case EAGAIN:
            error = OC_IO_ERR_NOT_READY;
            break;

        case EEXIST:
            error = OC_IO_ERR_EXISTS;
            break;

        case ENOTDIR:
            error = OC_IO_ERR_NOT_DIR;
            break;

        case EISDIR:
            error = OC_IO_ERR_DIR;
            break;

        case ENFILE:
        case EMFILE:
            error = OC_IO_ERR_MAX_FILES;
            break;

        case EFBIG:
            error = OC_IO_ERR_FILE_SIZE;
            break;

        case ENOSPC:
        case EDQUOT:
            error = OC_IO_ERR_SPACE;
            break;

        case ELOOP:
            error = OC_IO_ERR_MAX_LINKS;
            break;

        case ENAMETOOLONG:
            error = OC_IO_ERR_PATH_LENGTH;
            break;

        case EOVERFLOW:
            error = OC_IO_ERR_OVERFLOW;
            break;

        default:
            error = OC_IO_ERR_UNKNOWN;
            break;
    }
    return (error);
}

static int oc_io_convert_access_rights(oc_file_access rights)
{
    int oflags = 0;

    if(rights & OC_FILE_ACCESS_READ)
    {
        if(rights & OC_FILE_ACCESS_WRITE)
        {
            oflags = O_RDWR;
        }
        else
        {
            oflags = O_RDONLY;
        }
    }
    else if(rights & OC_FILE_ACCESS_WRITE)
    {
        oflags = O_WRONLY;
    }
    return (oflags);
}

static int oc_io_convert_open_flags(oc_file_open_flags flags)
{
    int oflags = 0;

    if(flags & OC_FILE_OPEN_TRUNCATE)
    {
        oflags |= O_TRUNC;
    }
    if(flags & OC_FILE_OPEN_APPEND)
    {
        oflags |= O_APPEND;
    }
    if(flags & OC_FILE_OPEN_CREATE)
    {
        oflags |= O_CREAT;
    }
    if(flags & OC_FILE_OPEN_NO_FOLLOW)
    {
        oflags |= O_NOFOLLOW;
    }
    if(flags & OC_FILE_OPEN_SYMLINK)
    {
        oflags |= O_SYMLINK;
    }
    return (oflags);
}

static int oc_io_update_dir_flags_at(int dirFd, char* path, int flags)
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
    return (flags);
}

oc_file_desc oc_io_raw_open_at(oc_file_desc dirFd, oc_str8 path, oc_file_access accessRights, oc_file_open_flags openFlags)
{
    int flags = oc_io_convert_access_rights(accessRights);
    flags |= oc_io_convert_open_flags(openFlags);

    mode_t mode = S_IRUSR
                | S_IWUSR
                | S_IRGRP
                | S_IWGRP
                | S_IROTH
                | S_IWOTH;

    oc_arena_scope scratch = oc_scratch_begin();

    oc_file_desc fd = -1;
    if(dirFd >= 0)
    {
        if(path.len && path.ptr[0] == '/')
        {
            //NOTE: if path is absolute, change for a relative one, otherwise openat ignores fd.
            oc_str8_list list = { 0 };
            oc_str8_list_push(scratch.arena, &list, OC_STR8("."));
            oc_str8_list_push(scratch.arena, &list, path);
            path = oc_str8_list_join(scratch.arena, list);
        }
    }
    else
    {
        dirFd = AT_FDCWD;
    }

    char* pathCStr = oc_str8_to_cstring(scratch.arena, path);

    flags = oc_io_update_dir_flags_at(dirFd, pathCStr, flags);

    fd = openat(dirFd, pathCStr, flags, mode);
    oc_scratch_end(scratch);

    return (fd);
}

void oc_io_raw_close(oc_file_desc fd)
{
    close(fd);
}

static oc_file_perm oc_io_convert_perm_from_stat(u16 mode)
{
    oc_file_perm perm = mode & 07777;
    return (perm);
}

static oc_file_type oc_io_convert_type_from_stat(u16 mode)
{
    oc_file_type type;
    switch(mode & S_IFMT)
    {
        case S_IFIFO:
            type = OC_FILE_FIFO;
            break;

        case S_IFCHR:
            type = OC_FILE_CHARACTER;
            break;

        case S_IFDIR:
            type = OC_FILE_DIRECTORY;
            break;

        case S_IFBLK:
            type = OC_FILE_BLOCK;
            break;

        case S_IFREG:
            type = OC_FILE_REGULAR;
            break;

        case S_IFLNK:
            type = OC_FILE_SYMLINK;
            break;

        case S_IFSOCK:
            type = OC_FILE_SOCKET;
            break;

        default:
            type = OC_FILE_UNKNOWN;
            break;
    }
    return (type);
}

static const u64 OC_NTP_JAN_1970 = 2208988800ULL; // seconds from january 1900 to january 1970

oc_datestamp oc_datestamp_from_timespec(struct timespec ts)
{
    oc_datestamp d = { 0 };
    d.seconds = ts.tv_sec + OC_NTP_JAN_1970;
    d.fraction = (ts.tv_nsec * (1ULL << 32)) / 1000000000;

    return (d);
}

oc_io_error oc_io_raw_fstat(oc_file_desc fd, oc_file_status* status)
{
    oc_io_error error = OC_IO_OK;
    struct stat s;
    if(fstat(fd, &s))
    {
        error = oc_io_raw_last_error();
    }
    else
    {
        status->uid = s.st_ino;
        status->perm = oc_io_convert_perm_from_stat(s.st_mode);
        status->type = oc_io_convert_type_from_stat(s.st_mode);
        status->size = s.st_size;

        status->creationDate = oc_datestamp_from_timespec(s.st_birthtimespec);
        status->accessDate = oc_datestamp_from_timespec(s.st_atimespec);
        status->modificationDate = oc_datestamp_from_timespec(s.st_mtimespec);
    }
    return (error);
}

oc_io_error oc_io_raw_fstat_at(oc_file_desc dirFd, oc_str8 path, oc_file_open_flags flags, oc_file_status* status)
{
    oc_arena_scope scratch = oc_scratch_begin();

    if(dirFd >= 0)
    {
        if(path.len && path.ptr[0] == '/')
        {
            oc_str8_list list = { 0 };
            oc_str8_list_push(scratch.arena, &list, OC_STR8("."));
            oc_str8_list_push(scratch.arena, &list, path);
            path = oc_str8_list_join(scratch.arena, list);
        }
    }
    else
    {
        dirFd = AT_FDCWD;
    }

    char* pathCStr = oc_str8_to_cstring(scratch.arena, path);

    int statFlag = (flags & OC_FILE_OPEN_SYMLINK) ? AT_SYMLINK_NOFOLLOW : 0;
    oc_io_error error = OC_IO_OK;
    struct stat s;
    if(fstatat(dirFd, pathCStr, &s, statFlag))
    {
        error = oc_io_raw_last_error();
    }
    else
    {
        status->uid = s.st_ino;
        status->perm = oc_io_convert_perm_from_stat(s.st_mode);
        status->type = oc_io_convert_type_from_stat(s.st_mode);
        status->size = s.st_size;
        //TODO: times
    }

    oc_scratch_end(scratch);
    return (error);
}

oc_io_raw_read_link_result oc_io_raw_read_link_at(oc_arena* arena, oc_file_desc dirFd, oc_str8 path)
{
    oc_arena_scope scratch = oc_scratch_begin_next(arena);

    if(dirFd >= 0)
    {
        if(path.len && path.ptr[0] == '/')
        {
            oc_str8_list list = { 0 };
            oc_str8_list_push(scratch.arena, &list, OC_STR8("."));
            oc_str8_list_push(scratch.arena, &list, path);
            path = oc_str8_list_join(scratch.arena, list);
        }
    }
    else
    {
        dirFd = AT_FDCWD;
    }

    char* pathCStr = oc_str8_to_cstring(scratch.arena, path);

    oc_io_raw_read_link_result result = { 0 };

    char buffer[PATH_MAX];
    u64 bufferSize = PATH_MAX;
    i64 r = readlinkat(dirFd, pathCStr, buffer, bufferSize);

    if(r < 0)
    {
        result.error = oc_io_raw_last_error();
    }
    else
    {
        result.target.len = r;
        result.target.ptr = oc_arena_push_array(arena, char, result.target.len);
        memcpy(result.target.ptr, buffer, result.target.len);
    }

    oc_scratch_end(scratch);
    return (result);
}

bool oc_io_raw_file_exists_at(oc_file_desc dirFd, oc_str8 path, oc_file_open_flags openFlags)
{
    oc_arena_scope scratch = oc_scratch_begin();

    if(dirFd >= 0)
    {
        if(path.len && path.ptr[0] == '/')
        {
            oc_str8_list list = { 0 };
            oc_str8_list_push(scratch.arena, &list, OC_STR8("."));
            oc_str8_list_push(scratch.arena, &list, path);
            path = oc_str8_list_join(scratch.arena, list);
        }
    }
    else
    {
        dirFd = AT_FDCWD;
    }

    char* pathCStr = oc_str8_to_cstring(scratch.arena, path);

    int flags = (openFlags & OC_FILE_OPEN_SYMLINK) ? AT_SYMLINK_NOFOLLOW : 0;
    int r = faccessat(dirFd, pathCStr, F_OK, flags);
    bool result = (r == 0);

    oc_scratch_end(scratch);
    return (result);
}

oc_io_cmp oc_io_close(oc_file_slot* slot, oc_io_req* req, oc_file_table* table)
{
    oc_io_cmp cmp = { 0 };
    if(slot->fd >= 0)
    {
        close(slot->fd);
    }
    oc_file_slot_recycle(table, slot);
    return (cmp);
}

oc_io_cmp oc_io_fstat(oc_file_slot* slot, oc_io_req* req)
{
    oc_io_cmp cmp = { 0 };

    if(req->size < sizeof(oc_file_status))
    {
        cmp.error = OC_IO_ERR_ARG;
    }
    else
    {
        struct stat s;
        if(fstat(slot->fd, &s))
        {
            slot->error = oc_io_raw_last_error();
            cmp.error = slot->error;
        }
        else
        {
            oc_file_status* status = (oc_file_status*)req->buffer;
            status->perm = oc_io_convert_perm_from_stat(s.st_mode);
            status->type = oc_io_convert_type_from_stat(s.st_mode);
            status->size = s.st_size;
            //TODO: times
        }
    }
    return (cmp);
}

oc_io_cmp oc_io_seek(oc_file_slot* slot, oc_io_req* req)
{
    oc_io_cmp cmp = { 0 };

    int whence;
    switch(req->whence)
    {
        case OC_FILE_SEEK_CURRENT:
            whence = SEEK_CUR;
            break;

        case OC_FILE_SEEK_SET:
            whence = SEEK_SET;
            break;

        case OC_FILE_SEEK_END:
            whence = SEEK_END;
    }
    cmp.result = lseek(slot->fd, req->offset, whence);

    if(cmp.result < 0)
    {
        slot->error = oc_io_raw_last_error();
        cmp.error = slot->error;
    }

    return (cmp);
}

oc_io_cmp oc_io_read(oc_file_slot* slot, oc_io_req* req)
{
    oc_io_cmp cmp = { 0 };

    cmp.result = read(slot->fd, req->buffer, req->size);

    if(cmp.result < 0)
    {
        slot->error = oc_io_raw_last_error();
        cmp.result = 0;
        cmp.error = slot->error;
    }

    return (cmp);
}

oc_io_cmp oc_io_write(oc_file_slot* slot, oc_io_req* req)
{
    oc_io_cmp cmp = { 0 };

    cmp.result = write(slot->fd, req->buffer, req->size);

    if(cmp.result < 0)
    {
        slot->error = oc_io_raw_last_error();
        cmp.result = 0;
        cmp.error = slot->error;
    }

    return (cmp);
}

oc_io_cmp oc_io_get_error(oc_file_slot* slot, oc_io_req* req)
{
    oc_io_cmp cmp = { 0 };
    cmp.result = slot->error;
    return (cmp);
}

oc_io_cmp oc_io_wait_single_req_for_table(oc_io_req* req, oc_file_table* table)
{
    oc_io_cmp cmp = { 0 };

    oc_file_slot* slot = oc_file_slot_from_handle(table, req->handle);
    if(!slot)
    {
        if(req->op != OC_IO_OPEN_AT)
        {
            cmp.error = OC_IO_ERR_HANDLE;
        }
    }
    else if(slot->fatal
            && req->op != OC_IO_CLOSE
            && req->op != OC_OC_IO_ERROR)
    {
        cmp.error = OC_IO_ERR_PREV;
    }

    if(cmp.error == OC_IO_OK)
    {
        switch(req->op)
        {
            case OC_IO_OPEN_AT:
                cmp = oc_io_open_at(slot, req, table);
                break;

            case OC_IO_FSTAT:
                cmp = oc_io_fstat(slot, req);
                break;

            case OC_IO_CLOSE:
                cmp = oc_io_close(slot, req, table);
                break;

            case OC_IO_READ:
                cmp = oc_io_read(slot, req);
                break;

            case OC_IO_WRITE:
                cmp = oc_io_write(slot, req);
                break;

            case OC_IO_SEEK:
                cmp = oc_io_seek(slot, req);
                break;

            case OC_OC_IO_ERROR:
                cmp = oc_io_get_error(slot, req);
                break;

            default:
                cmp.error = OC_IO_ERR_OP;
                break;
        }
    }
    return (cmp);
}
