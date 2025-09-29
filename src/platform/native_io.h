/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "platform.h"
#include "platform_io.h"
#include "platform_io_dialog.h"
#include "util/wrapped_types.h"

#if OC_PLATFORM_MACOS || PLATFORM_LINUX
typedef int oc_file_desc;
    #define OC_FILE_AT_FDCWD AT_FDCWD
#elif OC_PLATFORM_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
typedef HANDLE oc_file_desc;
    #define OC_FILE_AT_FDCWD NULL
#endif

//-----------------------------------------------------------------------
// File table and file handle functions
//-----------------------------------------------------------------------

typedef struct oc_file_slot
{
    u32 generation;
    oc_io_error error;
    bool fatal;
    oc_list_elt freeListElt;

    oc_file_type type;
    oc_file_access rights;
    oc_file_desc fd;

} oc_file_slot;

enum
{
    OC_IO_MAX_FILE_SLOTS = 256,
};

typedef struct oc_file_table
{
    oc_file_slot slots[OC_IO_MAX_FILE_SLOTS];
    u32 nextSlot;
    oc_list freeList;
} oc_file_table;

ORCA_API oc_file_table* oc_file_table_get_global();

oc_file_slot* oc_file_slot_alloc(oc_file_table* table);
void oc_file_slot_recycle(oc_file_table* table, oc_file_slot* slot);
oc_file oc_file_from_slot(oc_file_table* table, oc_file_slot* slot);
oc_file_slot* oc_file_slot_from_handle(oc_file_table* table, oc_file handle);

oc_file_desc oc_file_desc_nil();
bool oc_file_desc_is_nil(oc_file_desc fd);

//-----------------------------------------------------------------------
// io dispatch functions
//-----------------------------------------------------------------------

ORCA_API oc_io_cmp oc_io_wait_single_req_for_table(oc_io_req* req, oc_file_table* table);
ORCA_API oc_file oc_file_open_with_request_for_table(oc_str8 path, oc_file_access rights, oc_file_open_flags flags, oc_file_table* table);
ORCA_API oc_file_open_with_dialog_result oc_file_open_with_dialog_for_table(oc_arena* arena,
                                                                            oc_file_access rights,
                                                                            oc_file_open_flags flags,
                                                                            oc_file_dialog_desc* desc,
                                                                            oc_file_table* table);
ORCA_API oc_file_list oc_file_listdir_for_table(oc_arena* arena, oc_file directory, oc_file_table* table);

//-----------------------------------------------------------------------
// io primitives
//-----------------------------------------------------------------------
/*NOTE:
    These primitives must be implemented per-platform and are used by platform-agnostic IO request handlers.

    Path resolution follow these requirments:
    - if rootFd is nil and path is relative, search relative to current working directory
    - if path is absolute, ignore rootFd.
    - If the file designated by rootFd & path is a symlink, always open the symlink, not its target.
      (i.e. always open with the equivalent of O_SYMLINK).

   IMPORTANT: These primitives don't ensure that path doesn't walks out of the tree rooted at rootFd!
   This is done in the IO request handlers by resolving rootFd and path to the parent directory fd and
   the base name of the file. The primitves can then be called, knowing that the path can't go outside
   of the parent directory (this is why it is important that these always open symlinks, and _not follow_ them).
*/

typedef oc_result(oc_file_desc, oc_io_error) oc_fd_result;

oc_fd_result oc_fd_open_at(oc_file_desc rootFd, oc_str8 path, oc_file_access accessRights, oc_file_open_flags openFlags);
oc_io_error oc_fd_close(oc_file_desc fd);

typedef oc_result(oc_file_status, oc_io_error) oc_fd_stat_result;

oc_fd_stat_result oc_fd_stat(oc_file_desc fd);
oc_fd_stat_result oc_fd_stat_at(oc_file_desc rootFd, oc_str8 path);

typedef oc_result(u64, oc_io_error) oc_fd_seek_result;

oc_fd_seek_result oc_fd_seek(oc_file_desc fd, u64 offset, oc_file_whence whence);

typedef oc_result(u64, oc_io_error) oc_fd_readwrite_result;

oc_fd_readwrite_result oc_fd_read(oc_file_desc fd, u64 size, char* buffer);
oc_fd_readwrite_result oc_fd_write(oc_file_desc fd, u64 size, char* buffer);

oc_fd_result oc_fd_maketmp(oc_file_maketmp_flags flags);
oc_io_error oc_fd_makedir_at(oc_file_desc fd, oc_str8 path);
oc_io_error oc_fd_remove(oc_file_desc rootFd, oc_str8 path, oc_file_remove_flags flags);

typedef oc_result(oc_str8, oc_io_error) oc_fd_read_link_result;

oc_fd_read_link_result oc_fd_read_link_at(oc_arena* arena, oc_file_desc rootFd, oc_str8 path);
