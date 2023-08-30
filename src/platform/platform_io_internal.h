/************************************************************/ /**
*
*	@file: platform_io_internal.h
*	@author: Martin Fouilleul
*	@date: 10/06/2023
*
*****************************************************************/
#ifndef __PLATFORM_IO_INTERNAL_H_
#define __PLATFORM_IO_INTERNAL_H_

#include "platform.h"
#include "platform_io.h"
#include "platform_io_dialog.h"

#if OC_PLATFORM_MACOS || PLATFORM_LINUX
typedef int oc_file_desc;
#elif OC_PLATFORM_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
typedef HANDLE oc_file_desc;
#endif

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

ORCA_API oc_io_cmp oc_io_wait_single_req_for_table(oc_io_req* req, oc_file_table* table);

ORCA_API oc_file oc_file_open_with_request_for_table(oc_str8 path, oc_file_access rights, oc_file_open_flags flags, oc_file_table* table);

ORCA_API oc_file_open_with_dialog_result oc_file_open_with_dialog_for_table(oc_arena* arena,
                                                                            oc_file_access rights,
                                                                            oc_file_open_flags flags,
                                                                            oc_file_dialog_desc* desc,
                                                                            oc_file_table* table);

//-----------------------------------------------------------------------
// raw io primitives
//-----------------------------------------------------------------------

oc_file_desc oc_file_desc_nil();
bool oc_file_desc_is_nil(oc_file_desc fd);

/*WARN
	oc_io_raw_xxx_at functions are similar to posix openat() regarding path resolution,
	but with some important differences:
		- If dirFd is a non-nil fd, path is considered relative to dirFd _even if it is an absolute oc_path_
		- If dirFd is a nil fd, it is _ignored_ (i.e., path can be absolute, or relative to the current directory)

	This means that:
		- dirFd behaves more like the _root_ of path, ie dirFd won't be ignore if we pass an absolute path,
		- we don't need a special handle value to use a path relative to the current working directory
		  (we just pass a nil dirFd with a relative path)
*/
oc_file_desc oc_io_raw_open_at(oc_file_desc dirFd, oc_str8 path, oc_file_access accessRights, oc_file_open_flags openFlags);
void oc_io_raw_close(oc_file_desc fd);
oc_io_error oc_io_raw_last_error();
bool oc_io_raw_file_exists_at(oc_file_desc dirFd, oc_str8 path, oc_file_open_flags openFlags);
oc_io_error oc_io_raw_fstat(oc_file_desc fd, oc_file_status* status);
oc_io_error oc_io_raw_fstat_at(oc_file_desc dirFd, oc_str8 path, oc_file_open_flags openFlags, oc_file_status* status);

typedef struct oc_io_raw_read_link_result
{
    oc_io_error error;
    oc_str8 target;
} oc_io_raw_read_link_result;

oc_io_raw_read_link_result oc_io_raw_read_link_at(oc_arena* arena, oc_file_desc dirFd, oc_str8 path);

#endif //__PLATFORM_IO_INTERNAL_H_
