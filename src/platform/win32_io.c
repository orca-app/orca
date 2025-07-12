/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include <errno.h>
#include <limits.h>
#include <shlwapi.h>
#include <winioctl.h>

#include "platform_io_common.c"
#include "platform_io_internal.c"
#include "win32_string_helpers.h"

oc_io_error oc_io_raw_last_error()
{
    oc_io_error error = 0;

    int winError = GetLastError();
    switch(winError)
    {
        case ERROR_SUCCESS:
            error = OC_IO_OK;
            break;

        case ERROR_ACCESS_DENIED:
            error = OC_IO_ERR_PERM;
            break;

        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
        case ERROR_INVALID_DRIVE:
        case ERROR_DIRECTORY:
            error = OC_IO_ERR_NO_ENTRY;
            break;

        case ERROR_TOO_MANY_OPEN_FILES:
            error = OC_IO_ERR_MAX_FILES;
            break;

        case ERROR_NOT_ENOUGH_MEMORY:
        case ERROR_OUTOFMEMORY:
            error = OC_IO_ERR_MEM;
            break;

        case ERROR_DEV_NOT_EXIST:
            error = OC_IO_ERR_NO_DEVICE;
            break;

        case ERROR_FILE_EXISTS:
        case ERROR_ALREADY_EXISTS:
            error = OC_IO_ERR_EXISTS;
            break;

        case ERROR_BUFFER_OVERFLOW:
        case ERROR_FILENAME_EXCED_RANGE:
            error = OC_IO_ERR_PATH_LENGTH;
            break;

        case ERROR_FILE_TOO_LARGE:
            error = OC_IO_ERR_FILE_SIZE;
            break;

            //TODO: complete

        default:
            error = OC_IO_ERR_UNKNOWN;
            break;
    }
    return (error);
}

oc_str16 win32_path_from_handle_null_terminated(oc_arena* arena, HANDLE handle)
{
    oc_str16 res = { 0 };

    res.len = GetFinalPathNameByHandleW(handle, NULL, 0, FILE_NAME_NORMALIZED);
    if(res.len)
    {
        res.ptr = oc_arena_push_array(arena, u16, res.len);
        if(!GetFinalPathNameByHandleW(handle, res.ptr, res.len, FILE_NAME_NORMALIZED))
        {
            res.len = 0;
            res.ptr = 0;
        }
    }
    return (res);
}

typedef HANDLE oc_file_desc;

oc_file_desc oc_file_desc_nil()
{
    return (INVALID_HANDLE_VALUE);
}

bool oc_file_desc_is_nil(oc_file_desc fd)
{
    return (fd == NULL || fd == INVALID_HANDLE_VALUE);
}

static oc_str16 win32_get_path_at_null_terminated(oc_arena* arena, oc_file_desc dirFd, oc_str8 path)
{
    oc_str16 result = { 0 };
    oc_arena_scope scratch = oc_scratch_begin_next(arena);

    oc_str16 dirPathW = win32_path_from_handle_null_terminated(scratch.arena, dirFd);
    oc_str16 pathW = oc_win32_utf8_to_wide(scratch.arena, path);

    if(dirPathW.len && pathW.len)
    {
        u64 fullPathWSize = dirPathW.len + pathW.len;
        LPWSTR fullPathW = oc_arena_push_array(scratch.arena, u16, fullPathWSize);
        memcpy(fullPathW, dirPathW.ptr, (dirPathW.len - 1) * sizeof(u16));
        fullPathW[dirPathW.len - 1] = '\\';
        memcpy(fullPathW + dirPathW.len, pathW.ptr, pathW.len * sizeof(u16));

        result.len = fullPathWSize;
        result.ptr = oc_arena_push_array(arena, wchar_t, result.len);

        if(PathCanonicalizeW(result.ptr, fullPathW))
        {
            result.len = lstrlenW(result.ptr);
        }
        else
        {
            result.ptr = 0;
            result.len = 0;
        }
    }
    oc_scratch_end(scratch);

    return (result);
}

oc_file_desc oc_io_raw_open_at(oc_file_desc dirFd, oc_str8 path, oc_file_access accessRights, oc_file_open_flags openFlags)
{
    HANDLE handle = INVALID_HANDLE_VALUE;

    // convert flags
    DWORD win32AccessFlags = 0;
    DWORD win32ShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    DWORD win32CreateFlags = 0;
    DWORD win32AttributeFlags = FILE_ATTRIBUTE_NORMAL
                              | FILE_FLAG_BACKUP_SEMANTICS;

    if(accessRights & OC_FILE_ACCESS_READ)
    {
        win32AccessFlags |= GENERIC_READ;
    }
    if(accessRights & OC_FILE_ACCESS_WRITE)
    {
        if(openFlags & OC_FILE_OPEN_APPEND)
        {
            win32AccessFlags |= FILE_APPEND_DATA;
        }
        else
        {
            win32AccessFlags |= GENERIC_WRITE;
        }
    }

    if(openFlags & OC_FILE_OPEN_TRUNCATE)
    {
        if(openFlags & OC_FILE_OPEN_CREATE)
        {
            win32CreateFlags |= CREATE_ALWAYS;
        }
        else
        {
            win32CreateFlags |= TRUNCATE_EXISTING;
        }
    }
    if(openFlags & OC_FILE_OPEN_CREATE)
    {
        if(!(win32CreateFlags & CREATE_ALWAYS))
        {
            win32CreateFlags |= OPEN_ALWAYS;
        }
    }
    if(!(win32CreateFlags & OPEN_ALWAYS)
       && !(win32CreateFlags & CREATE_ALWAYS)
       && !(win32CreateFlags & TRUNCATE_EXISTING))
    {
        win32CreateFlags |= OPEN_EXISTING;
    }

    if(openFlags & OC_FILE_OPEN_SYMLINK)
    {
        win32AttributeFlags |= FILE_FLAG_OPEN_REPARSE_POINT;
    }

    oc_arena_scope scratch = oc_scratch_begin();
    oc_str16 pathW = oc_win32_utf8_to_wide(scratch.arena, path);

    if(dirFd == NULL || dirFd == INVALID_HANDLE_VALUE)
    {
        handle = CreateFileW(pathW.ptr, win32AccessFlags, win32ShareMode, NULL, win32CreateFlags, win32AttributeFlags, NULL);
    }
    else
    {
        oc_str16 fullPathW = win32_get_path_at_null_terminated(scratch.arena, dirFd, path);
        if(fullPathW.len)
        {
            handle = CreateFileW(fullPathW.ptr, win32AccessFlags, win32ShareMode, NULL, win32CreateFlags, win32AttributeFlags, NULL);
        }
    }
    oc_scratch_end(scratch);
    return (handle);
}

void oc_io_raw_close(oc_file_desc fd)
{
    CloseHandle(fd);
}

bool oc_io_raw_file_exists_at(oc_file_desc dirFd, oc_str8 path, oc_file_open_flags openFlags)
{
    bool result = false;
    oc_file_desc fd = oc_io_raw_open_at(dirFd, path, OC_FILE_ACCESS_NONE, (openFlags & OC_FILE_OPEN_SYMLINK));
    if(!oc_file_desc_is_nil(fd))
    {
        result = true;
        oc_io_raw_close(fd);
    }
    return (result);
}

enum
{
    OC_NTP_01_JAN_1601 = -9435484800LL,
    OC_WIN32_TICKS_PER_SECOND = 10000000LL
};

oc_datestamp oc_datestamp_from_win32_filetime(FILETIME ft)
{
    oc_datestamp d = { 0 };

    i64 win32Ticks = (((u64)ft.dwHighDateTime) << 32) | (u64)ft.dwLowDateTime;

    i64 win32Seconds = win32Ticks / OC_WIN32_TICKS_PER_SECOND;
    u64 win32Rem = 0;
    if(win32Ticks < 0)
    {
        win32Seconds -= OC_WIN32_TICKS_PER_SECOND;
        win32Rem = win32Ticks - win32Seconds * OC_WIN32_TICKS_PER_SECOND;
    }
    else
    {
        win32Rem = win32Ticks % OC_WIN32_TICKS_PER_SECOND;
    }

    d.seconds = win32Seconds + OC_NTP_01_JAN_1601;
    d.fraction = (win32Rem * (1ULL << 32)) / OC_WIN32_TICKS_PER_SECOND;

    return (d);
}

oc_io_error oc_io_raw_fstat(oc_file_desc fd, oc_file_status* status)
{
    oc_io_error error = OC_IO_OK;

    BY_HANDLE_FILE_INFORMATION info;
    if(!GetFileInformationByHandle(fd, &info))
    {
        error = oc_io_raw_last_error();
    }
    else
    {
        status->size = (((u64)info.nFileSizeHigh) << 32) | ((u64)info.nFileSizeLow);
        status->uid = ((u64)info.nFileIndexHigh << 32) | ((u64)info.nFileIndexLow);

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
                error = oc_io_raw_last_error();
            }
            else if(tagInfo.ReparseTag == IO_REPARSE_TAG_SYMLINK)
            {
                status->type = OC_FILE_SYMLINK;
            }
            else
            {
                status->type = OC_FILE_UNKNOWN;
            }
        }
        else if(info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            status->type = OC_FILE_DIRECTORY;
        }
        else if(info.dwFileAttributes & attrRegularSet)
        {
            status->type = OC_FILE_REGULAR;
        }
        else
        {
            //TODO: might want to check for socket/block/character devices? (otoh MS STL impl. doesn't seem to do it)
            status->type = OC_FILE_UNKNOWN;
        }

        status->perm = OC_FILE_OWNER_READ | OC_FILE_GROUP_READ | OC_FILE_OTHER_READ;
        if(!(info.dwFileAttributes & FILE_ATTRIBUTE_READONLY))
        {
            status->perm = OC_FILE_OWNER_WRITE | OC_FILE_GROUP_WRITE | OC_FILE_OTHER_WRITE;
        }

        FILETIME win32CreationDate;
        FILETIME win32AccessDate;
        FILETIME win32ModificationDate;

        GetFileTime(fd, &win32CreationDate, &win32AccessDate, &win32ModificationDate);

        status->creationDate = oc_datestamp_from_win32_filetime(win32CreationDate);
        status->accessDate = oc_datestamp_from_win32_filetime(win32AccessDate);
        status->modificationDate = oc_datestamp_from_win32_filetime(win32ModificationDate);
    }
    return (error);
}

oc_io_error oc_io_raw_fstat_at(oc_file_desc dirFd, oc_str8 name, oc_file_open_flags openFlags, oc_file_status* status)
{
    oc_io_error error = OC_IO_OK;
    oc_file_desc fd = oc_io_raw_open_at(dirFd, name, OC_FILE_ACCESS_NONE, OC_FILE_OPEN_SYMLINK);
    if(oc_file_desc_is_nil(fd))
    {
        error = oc_io_raw_last_error();
    }
    else
    {
        error = oc_io_raw_fstat(fd, status);
        oc_io_raw_close(fd);
    }
    return (error);
}

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
            UCHAR DataBuffer[1];
        } GenericReparseBuffer;
    };
} oc_win32_reparse_data_buffer;

oc_io_raw_read_link_result oc_io_raw_read_link(oc_arena* arena, oc_file_desc fd)
{
    oc_io_raw_read_link_result result = { 0 };

    char buffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
    DWORD bytesReturned;

    if(!DeviceIoControl(fd, FSCTL_GET_REPARSE_POINT, NULL, 0, buffer, MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &bytesReturned, 0))
    {
        result.error = oc_io_raw_last_error();
    }
    else
    {
        oc_win32_reparse_data_buffer* reparse = (oc_win32_reparse_data_buffer*)buffer;
        if(reparse->ReparseTag == IO_REPARSE_TAG_SYMLINK)
        {
            oc_str16 nameW = { 0 };
            nameW.len = reparse->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(wchar_t);
            nameW.ptr = (u16*)((char*)reparse->SymbolicLinkReparseBuffer.PathBuffer + reparse->SymbolicLinkReparseBuffer.SubstituteNameOffset);
            result.target = oc_win32_wide_to_utf8(arena, nameW);
        }
        else
        {
            result.error = OC_IO_ERR_UNKNOWN;
        }
    }
    oc_win32_path_normalize_slash_in_place(result.target);

    return (result);
}

oc_io_raw_read_link_result oc_io_raw_read_link_at(oc_arena* arena, oc_file_desc dirFd, oc_str8 name)
{
    oc_file_desc fd = oc_io_raw_open_at(dirFd, name, OC_FILE_ACCESS_READ, OC_FILE_OPEN_SYMLINK);
    oc_io_raw_read_link_result result = oc_io_raw_read_link(arena, fd);
    oc_io_raw_close(fd);
    return (result);
}

static oc_io_cmp oc_io_seek(oc_file_slot* slot, oc_io_req* req)
{
    oc_io_cmp cmp = { 0 };

    DWORD whence;
    switch(req->whence)
    {
        case OC_FILE_SEEK_CURRENT:
            whence = FILE_CURRENT;
            break;

        case OC_FILE_SEEK_SET:
            whence = FILE_BEGIN;
            break;

        case OC_FILE_SEEK_END:
            whence = FILE_END;
    }

    LARGE_INTEGER off = { .QuadPart = req->offset };
    LARGE_INTEGER newPos = { 0 };

    if(!SetFilePointerEx(slot->fd, off, &newPos, whence))
    {
        slot->error = oc_io_raw_last_error();
        cmp.error = slot->error;
    }
    else
    {
        cmp.result = newPos.QuadPart;
    }

    return (cmp);
}

static oc_io_cmp oc_io_read(oc_file_slot* slot, oc_io_req* req)
{
    oc_io_cmp cmp = { 0 };

    if(slot->type != OC_FILE_REGULAR)
    {
        slot->error = OC_IO_ERR_PERM;
        cmp.error = slot->error;
    }
    else
    {
        DWORD bytesRead = 0;

        if(!ReadFile(slot->fd, req->buffer, req->size, &bytesRead, NULL))
        {
            slot->error = oc_io_raw_last_error();
            cmp.result = 0;
            cmp.error = slot->error;
        }
        else
        {
            cmp.result = bytesRead;
        }
    }
    return (cmp);
}

static oc_io_cmp oc_io_write(oc_file_slot* slot, oc_io_req* req)
{
    oc_io_cmp cmp = { 0 };

    if(slot->type != OC_FILE_REGULAR)
    {
        slot->error = OC_IO_ERR_PERM;
        cmp.error = slot->error;
    }
    else
    {
        DWORD bytesWritten = 0;

        if(!WriteFile(slot->fd, req->buffer, req->size, &bytesWritten, NULL))
        {
            slot->error = oc_io_raw_last_error();
            cmp.result = 0;
            cmp.error = slot->error;
        }
        else
        {
            cmp.result = bytesWritten;
        }
    }
    return (cmp);
}

static oc_io_cmp oc_io_get_error(oc_file_slot* slot, oc_io_req* req)
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
    else if(slot->fatal && req->op != OC_IO_CLOSE && req->op != OC_OC_IO_ERROR)
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
                if(slot)
                {
                    slot->error = cmp.error;
                }
                break;
        }
    }
    return (cmp);
}
