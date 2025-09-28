/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "platform_io.h"
#include "platform_path.h"

//------------------------------------------------------------------------------
// File stream read/write API
//------------------------------------------------------------------------------

oc_file oc_file_nil()
{
    return ((oc_file){ 0 });
}

bool oc_file_is_nil(oc_file handle)
{
    return (handle.h == 0);
}

oc_file oc_file_open(oc_str8 path, oc_file_access rights, oc_file_open_flags flags)
{
    oc_io_req req = { .op = OC_IO_OPEN_AT,
                      .size = path.len,
                      .buffer = path.ptr,
                      .open.rights = rights,
                      .open.flags = flags };

    oc_io_cmp cmp = oc_io_wait_single_req(&req);

    //WARN: we always return a handle that can be queried for errors. Handles must be closed
    //      even if there was an error when opening
    return (cmp.handle);
}

oc_file oc_file_open_at(oc_file dir, oc_str8 path, oc_file_access rights, oc_file_open_flags flags)
{
    oc_io_req req = {
        .op = OC_IO_OPEN_AT,
        .handle = dir,
        .size = path.len,
        .buffer = path.ptr,
        .open.rights = rights,
        .open.flags = flags,
    };

    oc_io_cmp cmp = oc_io_wait_single_req(&req);
    return (cmp.handle);
}

void oc_file_close(oc_file file)
{
    oc_io_req req = { .op = OC_IO_CLOSE,
                      .handle = file };
    oc_io_wait_single_req(&req);
}

i64 oc_file_seek(oc_file file, i64 offset, oc_file_whence whence)
{
    oc_io_req req = { .op = OC_IO_SEEK,
                      .handle = file,
                      .offset = offset,
                      .whence = whence };

    oc_io_cmp cmp = oc_io_wait_single_req(&req);
    return (cmp.offset);
}

i64 oc_file_pos(oc_file file)
{
    return (oc_file_seek(file, 0, OC_FILE_SEEK_CURRENT));
}

u64 oc_file_write(oc_file file, u64 size, char* buffer)
{
    oc_io_req req = { .op = OC_IO_WRITE,
                      .handle = file,
                      .size = size,
                      .buffer = buffer };

    oc_io_cmp cmp = oc_io_wait_single_req(&req);
    return (cmp.size);
}

u64 oc_file_read(oc_file file, u64 size, char* buffer)
{
    oc_io_req req = { .op = OC_IO_READ,
                      .handle = file,
                      .size = size,
                      .buffer = buffer };

    oc_io_cmp cmp = oc_io_wait_single_req(&req);
    return (cmp.size);
}

oc_io_error oc_file_last_error(oc_file file)
{
    oc_io_req req = { .op = OC_OC_IO_ERROR,
                      .handle = file };

    oc_io_cmp cmp = oc_io_wait_single_req(&req);
    return ((oc_io_error)cmp.result);
}

oc_file_status oc_file_get_status(oc_file file)
{
    oc_file_status status = { 0 };
    oc_io_req req = { .op = OC_IO_FSTAT,
                      .handle = file,
                      .size = sizeof(oc_file_status),
                      .buffer = (char*)&status };

    oc_io_cmp cmp = oc_io_wait_single_req(&req);
    return (status);
}

u64 oc_file_size(oc_file file)
{
    oc_file_status status = oc_file_get_status(file);
    return (status.size);
}

oc_file oc_file_maketmp(oc_file_maketmp_flags flags)
{
    oc_io_req req = {
        .op = OC_IO_MAKE_TMP,
        .makeTmpFlags = flags,
    };

    oc_io_cmp cmp = oc_io_wait_single_req(&req);
    return cmp.handle;
}

oc_io_error oc_file_makedir(oc_str8 path, oc_file_makedir_options* optionsPtr)
{
    oc_file_makedir_options options = optionsPtr
                                        ? *optionsPtr
                                        : (oc_file_makedir_options){ 0 };
    oc_io_req req = {
        .op = OC_IO_MAKE_DIR,
        .handle = options.root,
        .size = path.len,
        .buffer = path.ptr,
        .makeDirFlags = options.flags,
    };

    oc_io_cmp cmp = oc_io_wait_single_req(&req);
    return cmp.error;
}

oc_io_error oc_file_remove_recursive(oc_file root, oc_str8 path)
{
    oc_io_error error = OC_IO_OK;
    oc_file file = oc_file_open_at(root, path, OC_FILE_ACCESS_READ | OC_FILE_ACCESS_WRITE, OC_FILE_SYMLINK_OPEN_LAST);
    //TODO: err
    oc_file_status status = oc_file_get_status(file);
    if(status.type == OC_FILE_DIRECTORY)
    {
        oc_arena_scope scratch = oc_scratch_begin();
        oc_file_list list = oc_file_listdir(scratch.arena, file);
        oc_file_list_for(list, elt)
        {
            oc_str8 childPath = oc_path_append(scratch.arena, path, elt->basename);
            error = oc_file_remove_recursive(root, childPath);
            if(error)
            {
                break;
            }
        }
        oc_scratch_end(scratch);
    }
    oc_file_close(file);

    if(error == OC_IO_OK)
    {
        error = oc_file_remove(path, &(oc_file_remove_options){ .flags = OC_FILE_REMOVE_DIR });
    }
    return error;
}

oc_io_error oc_file_remove(oc_str8 path, oc_file_remove_options* optionsPtr)
{
    oc_file_remove_options options = optionsPtr
                                       ? *optionsPtr
                                       : (oc_file_remove_options){ 0 };

    if(options.flags & OC_FILE_REMOVE_RECURSIVE)
    {
        return oc_file_remove_recursive(options.root, path);
    }
    else
    {
        oc_io_req req = {
            .op = OC_IO_REMOVE,
            .handle = options.root,
            .size = path.len,
            .buffer = path.ptr,
            .removeFlags = options.flags,
        };

        oc_io_cmp cmp = oc_io_wait_single_req(&req);
        return cmp.error;
    }
}
