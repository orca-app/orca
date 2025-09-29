/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#if OC_PLATFORM_MACOS && !defined(_DARWIN_FEATURE_ONLY_64_BIT_INODE)
    #define _DARWIN_USE_64_BIT_INODE
#endif

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "native_io.c"

//------------------------------------------------------------------------
// convertion helpers between orca and posix enums/constants etc
//------------------------------------------------------------------------
oc_file_desc oc_file_desc_nil()
{
    return (-1);
}

bool oc_file_desc_is_nil(oc_file_desc fd)
{
    return (fd < 0);
}

static oc_io_error oc_fd_convert_errno()
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

static int oc_fd_convert_access_rights(oc_file_access rights)
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

static int oc_fd_convert_open_flags(oc_file_open_flags flags)
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
    return (oflags);
}

static int oc_fd_update_dir_flags_at(int dirFd, char* path, int flags)
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

static oc_file_perm oc_fd_convert_perm_from_stat(u16 mode)
{
    oc_file_perm perm = mode & 07777;
    return (perm);
}

static oc_file_type oc_fd_convert_type_from_stat(u16 mode)
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

//------------------------------------------------------------------------
// raw IO primitives
//------------------------------------------------------------------------

oc_fd_result oc_fd_open_at(oc_file_desc dirFd, oc_str8 path, oc_file_access accessRights, oc_file_open_flags openFlags)
{
    if(oc_file_desc_is_nil(dirFd))
    {
        dirFd = AT_FDCWD;
    }

    oc_arena_scope scratch = oc_scratch_begin();
    char* pathCStr = oc_str8_to_cstring(scratch.arena, path);

    int flags = oc_fd_convert_access_rights(accessRights);
    flags |= oc_fd_convert_open_flags(openFlags);
    flags = oc_fd_update_dir_flags_at(dirFd, pathCStr, flags);
    flags |= O_SYMLINK; //NOTE: always open symlink, don't follow.

    mode_t mode = S_IRUSR
                | S_IWUSR
                | S_IRGRP
                | S_IWGRP
                | S_IROTH
                | S_IWOTH;

    int fd = openat(dirFd, pathCStr, flags, mode);

    oc_scratch_end(scratch);

    if(fd < 0)
    {
        return oc_wrap_error(oc_fd_result, oc_fd_convert_errno());
    }
    else
    {
        return oc_wrap_value(oc_fd_result, fd);
    }
}

oc_io_error oc_fd_close(oc_file_desc fd)
{
    int res = close(fd);
    if(res)
    {
        return oc_fd_convert_errno();
    }
    else
    {
        return OC_IO_OK;
    }
}

oc_fd_stat_result oc_fd_stat(oc_file_desc fd)
{
    oc_fd_stat_result result = { 0 };

    struct stat s;
    if(fstat(fd, &s))
    {
        result = oc_wrap_error(oc_fd_stat_result, oc_fd_convert_errno());
    }
    else
    {
        oc_file_status status = { 0 };
        status.uid = s.st_ino;
        status.perm = oc_fd_convert_perm_from_stat(s.st_mode);
        status.type = oc_fd_convert_type_from_stat(s.st_mode);
        status.size = s.st_size;
        status.creationDate = oc_datestamp_from_timespec(s.st_birthtimespec);
        status.accessDate = oc_datestamp_from_timespec(s.st_atimespec);
        status.modificationDate = oc_datestamp_from_timespec(s.st_mtimespec);

        result = oc_wrap_value(oc_fd_stat_result, status);
    }
    return (result);
}

oc_fd_stat_result oc_fd_stat_at(oc_file_desc dirFd, oc_str8 path)
{
    oc_fd_stat_result result = { 0 };

    if(oc_file_desc_is_nil(dirFd))
    {
        dirFd = AT_FDCWD;
    }

    oc_arena_scope scratch = oc_scratch_begin();
    char* pathCStr = oc_str8_to_cstring(scratch.arena, path);

    int statFlag = AT_SYMLINK_NOFOLLOW;

    struct stat s;
    if(fstatat(dirFd, pathCStr, &s, statFlag))
    {
        result = oc_wrap_error(oc_fd_stat_result, oc_fd_convert_errno());
    }
    else
    {
        oc_file_status status = { 0 };
        status.uid = s.st_ino;
        status.perm = oc_fd_convert_perm_from_stat(s.st_mode);
        status.type = oc_fd_convert_type_from_stat(s.st_mode);
        status.size = s.st_size;
        status.creationDate = oc_datestamp_from_timespec(s.st_birthtimespec);
        status.accessDate = oc_datestamp_from_timespec(s.st_atimespec);
        status.modificationDate = oc_datestamp_from_timespec(s.st_mtimespec);

        result = oc_wrap_value(oc_fd_stat_result, status);
    }

    oc_scratch_end(scratch);
    return (result);
}

oc_fd_read_link_result oc_fd_read_link_at(oc_arena* arena, oc_file_desc dirFd, oc_str8 path)
{
    oc_arena_scope scratch = oc_scratch_begin_next(arena);

    if(oc_file_desc_is_nil(dirFd))
    {
        dirFd = AT_FDCWD;
    }

    char* pathCStr = oc_str8_to_cstring(scratch.arena, path);

    oc_fd_read_link_result result = { 0 };

    char buffer[PATH_MAX];
    u64 bufferSize = PATH_MAX;
    i64 r = readlinkat(dirFd, pathCStr, buffer, bufferSize);

    if(r < 0)
    {
        result = oc_wrap_error(oc_fd_read_link_result, oc_fd_convert_errno());
    }
    else
    {
        oc_str8 path = oc_str8_push_buffer(arena, r, buffer);
        result = oc_wrap_value(oc_fd_read_link_result, path);
    }

    oc_scratch_end(scratch);
    return (result);
}

oc_fd_seek_result oc_fd_seek(oc_file_desc fd, u64 offset, oc_file_whence whence)
{
    oc_fd_seek_result result = oc_wrap_value(oc_fd_seek_result, 0);

    int posixWhence = 0;
    switch(whence)
    {
        case OC_FILE_SEEK_CURRENT:
            posixWhence = SEEK_CUR;
            break;

        case OC_FILE_SEEK_SET:
            posixWhence = SEEK_SET;
            break;

        case OC_FILE_SEEK_END:
            posixWhence = SEEK_END;
            break;

        default:
            result = oc_wrap_error(oc_fd_seek_result, OC_IO_ERR_ARG);
            break;
    }
    if(oc_check(result))
    {
        i64 r = lseek(fd, offset, posixWhence);
        if(r < 0)
        {
            result = oc_wrap_error(oc_fd_seek_result, oc_fd_convert_errno());
        }
        else
        {
            result = oc_wrap_value(oc_fd_seek_result, (u64)r);
        }
    }
    return result;
}

oc_fd_readwrite_result oc_fd_read(oc_file_desc fd, u64 size, char* buffer)
{
    ssize_t r = read(fd, buffer, size);
    if(r < 0)
    {
        return oc_wrap_error(oc_fd_readwrite_result, oc_fd_convert_errno());
    }
    else
    {
        return oc_wrap_value(oc_fd_readwrite_result, (u64)r);
    }
}

oc_fd_readwrite_result oc_fd_write(oc_file_desc fd, u64 size, char* buffer)
{
    ssize_t r = write(fd, buffer, size);
    if(r < 0)
    {
        return oc_wrap_error(oc_fd_readwrite_result, oc_fd_convert_errno());
    }
    else
    {
        return oc_wrap_value(oc_fd_readwrite_result, (u64)r);
    }
}

oc_fd_result oc_fd_maketmp(oc_file_maketmp_flags flags)
{
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 template = oc_str8_push_cstring(scratch.arena, "/tmp/orca.XXXXXX");

    oc_file_desc fd = oc_file_desc_nil();

    if(flags & OC_FILE_MAKETMP_DIRECTORY)
    {
        char* path = mkdtemp(template.ptr);
        if(path)
        {
            fd = open(path, O_DIRECTORY);
        }
    }
    else
    {
        fd = mkstemp(template.ptr);
    }
    oc_scratch_end(scratch);

    if(oc_file_desc_is_nil(fd))
    {
        return oc_wrap_error(oc_fd_result, oc_fd_convert_errno());
    }
    else
    {
        return oc_wrap_value(oc_fd_result, fd);
    }
}

oc_io_error oc_fd_makedir_at(oc_file_desc dirFd, oc_str8 path)
{
    oc_io_error error = OC_IO_OK;

    if(oc_file_desc_is_nil(dirFd))
    {
        dirFd = AT_FDCWD;
    }

    oc_arena_scope scratch = oc_scratch_begin();
    char* pathCStr = oc_str8_to_cstring(scratch.arena, path);

    int r = mkdirat(dirFd, pathCStr, 0700);
    if(r)
    {
        error = oc_fd_convert_errno();
    }
    oc_scratch_end(scratch);
    return error;
}

oc_io_error oc_fd_remove(oc_file_desc rootFd, oc_str8 path, oc_file_remove_flags flags)
{
    oc_fd_stat_result statResult = oc_fd_stat_at(rootFd, path);

    oc_io_error error = OC_IO_OK;
    if(!oc_check(statResult))
    {
        error = statResult.error;
    }
    else
    {
        oc_file_status status = statResult.value;

        if(status.type == OC_FILE_DIRECTORY && !(flags & OC_FILE_REMOVE_DIR))
        {
            error = OC_IO_ERR_DIR;
        }
        else
        {
            int flags = AT_SYMLINK_NOFOLLOW_ANY;
            if(status.type == OC_FILE_DIRECTORY)
            {
                flags |= AT_REMOVEDIR;
            }
            oc_arena_scope scratch = oc_scratch_begin();
            char* pathCStr = oc_str8_to_cstring(scratch.arena, path);

            int r = unlinkat(rootFd, pathCStr, flags);
            if(r)
            {
                error = oc_fd_convert_errno();
            }
            oc_scratch_end(scratch);
        }
    }
    return error;
}

////////////////////////////////////////////////////////////////////////////////
//TODO: move that in common code
////////////////////////////////////////////////////////////////////////////////

oc_file_list oc_file_listdir_for_table(oc_arena* arena, oc_file directory, oc_file_table* table)
{
    oc_file_list list = { 0 };

    oc_file_slot* slot = oc_file_slot_from_handle(table, directory);
    if(slot && !slot->fatal)
    {
        DIR* dir = fdopendir(slot->fd);
        if(dir)
        {
            struct dirent* entry = NULL;
            while((entry = readdir(dir)) != NULL)
            {
                // skip the current/previous directory entries
                if(!strcmp(".", entry->d_name) || !strcmp("..", entry->d_name))
                {
                    continue;
                }

                oc_file_listdir_elt* elt = oc_arena_push_type(arena, oc_file_listdir_elt);
                oc_list_push_back(&list.list, &elt->listElt);
                ++list.eltCount;

                oc_file_type type = OC_FILE_UNKNOWN;
                switch(entry->d_type)
                {
                    case DT_FIFO:
                        type = OC_FILE_FIFO;
                        break;
                    case DT_CHR:
                        type = OC_FILE_CHARACTER;
                        break;
                    case DT_DIR:
                        type = OC_FILE_DIRECTORY;
                        break;
                    case DT_BLK:
                        type = OC_FILE_BLOCK;
                        break;
                    case DT_REG:
                        type = OC_FILE_REGULAR;
                        break;
                    case DT_LNK:
                        type = OC_FILE_SYMLINK;
                        break;
                    case DT_SOCK:
                        type = OC_FILE_SOCKET;
                        break;
                    default:
                        type = OC_FILE_UNKNOWN;
                        break;
                }

                elt->basename = oc_str8_push_buffer(arena, entry->d_namlen, entry->d_name);
                elt->type = type;
            }

            closedir(dir);
        }
        else
        {
            slot->error = oc_fd_convert_errno();
        }
    }

    return list;
}
