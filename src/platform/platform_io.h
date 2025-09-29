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

typedef enum oc_file_access
{
    OC_FILE_ACCESS_NONE = 0,
    OC_FILE_ACCESS_READ = 1 << 0,
    OC_FILE_ACCESS_WRITE = 1 << 1,
} oc_file_access;

typedef enum oc_file_open_flags
{
    OC_FILE_OPEN_DEFAULT = 0,
    OC_FILE_OPEN_APPEND = 1 << 0,
    OC_FILE_OPEN_TRUNCATE = 1 << 1,
    OC_FILE_OPEN_CREATE = 1 << 2,
    //...
} oc_file_open_flags;

typedef enum oc_file_resolve_flags
{
    OC_FILE_RESOLVE_DEFAULT = 0,
    OC_FILE_RESOLVE_SYMLINK_OPEN_LAST = 1,
    OC_FILE_RESOLVE_SYMLINK_DONT_FOLLOW = 1 << 1,
} oc_file_resolve_flags;

typedef enum oc_file_whence
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
    OC_IO_MAKE_TMP,
    OC_IO_MAKE_DIR,
    OC_IO_REMOVE,
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

    u8 resolveFlags;

    union
    {
        struct
        {
            u16 rights;
            u16 flags;
        } open;

        u32 makeTmpFlags;
        u32 makeDirFlags;
        u32 removeFlags;
        u32 copyFlags;
        u8 whence;
    };

} oc_io_req;

typedef enum oc_io_error
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
    OC_IO_ERR_SYMLINK,     // encountered a symlink while following symlinks was disabled
    //...
} oc_io_error;

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

typedef struct oc_file_open_options
{
    oc_file root;
    oc_file_open_flags flags;
    oc_file_resolve_flags resolve;
} oc_file_open_options;

ORCA_API oc_file oc_file_open(oc_str8 path, oc_file_access rights, oc_file_open_options* options);
ORCA_API void oc_file_close(oc_file file);
ORCA_API i64 oc_file_pos(oc_file file);
ORCA_API i64 oc_file_seek(oc_file file, i64 offset, oc_file_whence whence);
ORCA_API u64 oc_file_write(oc_file file, u64 size, char* buffer);
ORCA_API u64 oc_file_read(oc_file file, u64 size, char* buffer);
ORCA_API oc_io_error oc_file_last_error(oc_file file);

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

typedef enum oc_file_maketmp_flags
{
    OC_FILE_MAKETMP_FILE = 0,
    OC_FILE_MAKETMP_DIRECTORY = 1,
} oc_file_maketmp_flags;

oc_file oc_file_maketmp(oc_file_maketmp_flags flags);

typedef enum oc_file_makedir_flags
{
    OC_FILE_MAKEDIR_DEFAULT = 0,
    OC_FILE_MAKEDIR_CREATE_PARENTS = 1,
    OC_FILE_MAKEDIR_IGNORE_EXISTING = 1 << 1,
} oc_file_makedir_flags;

typedef struct oc_file_makedir_options
{
    oc_file root;
    oc_file_makedir_flags flags;
    oc_file_resolve_flags resolve;
} oc_file_makedir_options;

ORCA_API oc_io_error oc_file_makedir(oc_str8 path, oc_file_makedir_options* options);

typedef enum oc_file_remove_flags
{
    OC_FILE_REMOVE_DEFAULT = 0,
    OC_FILE_REMOVE_DIR = 1,            // allow removing (empty) directory
    OC_FILE_REMOVE_RECURSIVE = 1 << 1, // allow removing full directory
} oc_file_remove_flags;

typedef struct oc_file_remove_options
{
    oc_file root;
    oc_file_remove_flags flags;
} oc_file_remove_options;

ORCA_API oc_io_error oc_file_remove(oc_str8 path, oc_file_remove_options* options);

/*
typedef enum oc_file_copy_flags
{
    OC_FILE_COPY_DEFAULT = 0,
    OC_FILE_COPY_INSIDE_DEST = 1,             // copy src inside dst directory
    OC_FILE_COPY_SRC_CONTENTS = 1 << 1,       // copy contents of src instead of src
    OC_FILE_COPY_CREATE_PARENTS = 1 << 2,     // create parents dirs of dst if they don't exist
    OC_FILE_COPY_FOLLOW_SYMLINKS = 1 << 3,    // copy target tree of symlinks instead of symlink file
    OC_FILE_COPY_OVERWRITE_EXISTING = 1 << 4, // if dst exists, overwrite existing files with new files
    OC_FILE_COPY_REPLACE_EXISTING = 1 << 5,   // if dst exists, completely remove its contents before copying
} oc_file_copy_flags;

typedef bool(oc_file_copy_ignore_proc*)(oc_str8 path, void* data);

typedef struct oc_file_copy_options
{
    oc_file_copy_flags flags;
    oc_file_copy_ignore_proc ignore;
    void* ignoreData;
} oc_file_copy_options;

ORCA_API oc_io_error oc_file_copy(oc_str8 src, oc_str8 dst, oc_file_copy_options* options);
*/
//----------------------------------------------------------------
// File Enumeration API
//----------------------------------------------------------------

typedef struct oc_file_listdir_elt
{
    oc_list_elt listElt;
    oc_str8 basename; // filename and extension, if any
    oc_file_type type;
} oc_file_listdir_elt;

typedef struct oc_file_list
{
    oc_list list;
    u64 eltCount;
} oc_file_list;

ORCA_API oc_file_list oc_file_listdir(oc_arena* arena, oc_file directory);

#define oc_file_list_for(filelist, elt) oc_list_for(filelist.list, elt, oc_file_listdir_elt, listElt)

//----------------------------------------------------------------
// Acquiring new file capabilities through user interaction
//----------------------------------------------------------------

ORCA_API oc_file oc_file_open_with_request(oc_str8 path, oc_file_access rights, oc_file_open_flags flags);

#ifdef __cplusplus
}
#endif
