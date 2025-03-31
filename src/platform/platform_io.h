/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "util/strings.h"
#include "util/typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------
// IO API
//----------------------------------------------------------------

typedef struct
{
    u64 h;
} oc_file;

typedef u16 oc_file_open_flags;

enum oc_file_open_flags_enum
{
    OC_FILE_OPEN_NONE = 0,
    OC_FILE_OPEN_APPEND = 1 << 0,
    OC_FILE_OPEN_TRUNCATE = 1 << 1,
    OC_FILE_OPEN_CREATE = 1 << 2,

    OC_FILE_OPEN_SYMLINK = 1 << 3,
    OC_FILE_OPEN_NO_FOLLOW = 1 << 4,
    OC_FILE_OPEN_RESTRICT = 1 << 5,
    //...
};

typedef u16 oc_file_access;

enum oc_file_access_enum
{
    OC_FILE_ACCESS_NONE = 0,
    OC_FILE_ACCESS_READ = 1 << 0,
    OC_FILE_ACCESS_WRITE = 1 << 1,
};

typedef enum
{
    OC_FILE_SEEK_SET,
    OC_FILE_SEEK_END,
    OC_FILE_SEEK_CURRENT
} oc_file_whence;

typedef u64 oc_io_req_id;

typedef u32 oc_io_op;

enum oc_io_op_enum
{
    OC_IO_OPEN_AT = 0,
    OC_IO_CLOSE,

    OC_IO_FSTAT,

    OC_IO_SEEK,
    OC_IO_READ,
    OC_IO_WRITE,

    OC_OC_IO_ERROR,
    //...
};

typedef struct oc_io_req
{
    oc_io_req_id id;
    oc_io_op op;
    oc_file handle;

    i64 offset;
    u64 size;

    union
    {
        char* buffer;
        u64 unused; // This is a horrible hack to get the same layout on wasm and on host
    };

    union
    {
        struct
        {
            oc_file_access rights;
            oc_file_open_flags flags;
        } open;

        oc_file_whence whence;
    };

} oc_io_req;

typedef i32 oc_io_error;

enum oc_io_error_enum
{
    OC_IO_OK = 0,
    OC_IO_ERR_UNKNOWN,
    OC_IO_ERR_OP,          // unsupported operation
    OC_IO_ERR_HANDLE,      // invalid handle
    OC_IO_ERR_PREV,        // previously had a fatal error (last error stored on handle)
    OC_IO_ERR_ARG,         // invalid argument or argument combination
    OC_IO_ERR_PERM,        // access denied
    OC_IO_ERR_SPACE,       // no space left
    OC_IO_ERR_NO_ENTRY,    // file or directory does not exist
    OC_IO_ERR_EXISTS,      // file already exists
    OC_IO_ERR_NOT_DIR,     // path element is not a directory
    OC_IO_ERR_DIR,         // attempted to write directory
    OC_IO_ERR_MAX_FILES,   // max open files reached
    OC_IO_ERR_MAX_LINKS,   // too many symbolic links in path
    OC_IO_ERR_PATH_LENGTH, // path too long
    OC_IO_ERR_FILE_SIZE,   // file too big
    OC_IO_ERR_OVERFLOW,    // offset too big
    OC_IO_ERR_NOT_READY,   // no data ready to be read/written
    OC_IO_ERR_MEM,         // failed to allocate memory
    OC_IO_ERR_INTERRUPT,   // operation interrupted by a signal
    OC_IO_ERR_PHYSICAL,    // physical IO error
    OC_IO_ERR_NO_DEVICE,   // device not found
    OC_IO_ERR_WALKOUT,     // attempted to walk out of root directory

    //...
};

typedef struct oc_io_cmp
{
    oc_io_req_id id;
    oc_io_error error;

    union
    {
        i64 result;
        u64 size;
        i64 offset;
        oc_file handle;
        //...
    };
} oc_io_cmp;

//----------------------------------------------------------------
//TODO: complete io queue api
//----------------------------------------------------------------
ORCA_API oc_io_cmp oc_io_wait_single_req(oc_io_req* req);

//----------------------------------------------------------------
// File IO wrapper API
//----------------------------------------------------------------

ORCA_API oc_file oc_file_nil(void);
ORCA_API bool oc_file_is_nil(oc_file handle);

ORCA_API oc_file oc_file_open(oc_str8 path, oc_file_access rights, oc_file_open_flags flags);
ORCA_API oc_file oc_file_open_at(oc_file dir, oc_str8 path, oc_file_access rights, oc_file_open_flags flags);
ORCA_API void oc_file_close(oc_file file);

ORCA_API i64 oc_file_pos(oc_file file);
ORCA_API i64 oc_file_seek(oc_file file, i64 offset, oc_file_whence whence);

ORCA_API u64 oc_file_write(oc_file file, u64 size, char* buffer);
ORCA_API u64 oc_file_read(oc_file file, u64 size, char* buffer);

ORCA_API oc_io_error oc_file_last_error(oc_file handle);

//----------------------------------------------------------------
// File System wrapper API
//----------------------------------------------------------------

typedef enum oc_file_type
{
    OC_FILE_UNKNOWN,
    OC_FILE_REGULAR,
    OC_FILE_DIRECTORY,
    OC_FILE_SYMLINK,
    OC_FILE_BLOCK,
    OC_FILE_CHARACTER,
    OC_FILE_FIFO,
    OC_FILE_SOCKET,

} oc_file_type;

typedef u16 oc_file_perm;

enum oc_file_perm_enum
{
    OC_FILE_OTHER_EXEC = 1 << 0,
    OC_FILE_OTHER_WRITE = 1 << 1,
    OC_FILE_OTHER_READ = 1 << 2,

    OC_FILE_GROUP_EXEC = 1 << 3,
    OC_FILE_GROUP_WRITE = 1 << 4,
    OC_FILE_GROUP_READ = 1 << 5,

    OC_FILE_OWNER_EXEC = 1 << 6,
    OC_FILE_OWNER_WRITE = 1 << 7,
    OC_FILE_OWNER_READ = 1 << 8,

    OC_FILE_STICKY_BIT = 1 << 9,
    OC_FILE_SET_GID = 1 << 10,
    OC_FILE_SET_UID = 1 << 11,
};

typedef struct oc_datestamp
{
    i64 seconds;  // seconds relative to NTP epoch.
    u64 fraction; // fraction of seconds elapsed since the time specified by seconds.

} oc_datestamp;

typedef struct oc_file_status
{
    u64 uid;
    oc_file_type type;
    oc_file_perm perm;
    u64 size;

    oc_datestamp creationDate;
    oc_datestamp accessDate;
    oc_datestamp modificationDate;

} oc_file_status;

ORCA_API oc_file_status oc_file_get_status(oc_file file);
ORCA_API u64 oc_file_size(oc_file file);

//TODO: Complete as needed...

//----------------------------------------------------------------
// Acquiring new file capabilities through user interaction
//----------------------------------------------------------------

ORCA_API oc_file oc_file_open_with_request(oc_str8 path, oc_file_access rights, oc_file_open_flags flags);

#ifdef __cplusplus
}
#endif
