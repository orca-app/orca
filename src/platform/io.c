/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "io.h"
#include "path.h"

oc_str8 oc_io_error_string(oc_io_error error)
{
    switch(error)
    {
        case OC_IO_OK:
            return OC_STR8("no errors");
        case OC_IO_ERR_UNKNOWN:
            return OC_STR8("unknown IO error");
        case OC_IO_ERR_OP:
            return OC_STR8("unsupported operation");
        case OC_IO_ERR_HANDLE:
            return OC_STR8("invalid handle");
        case OC_IO_ERR_PREV:
            return OC_STR8("previously had a fatal error (last error stored on handle)");
        case OC_IO_ERR_ARG:
            return OC_STR8("invalid argument or argument combination");
        case OC_IO_ERR_PERM:
            return OC_STR8("access denied");
        case OC_IO_ERR_SPACE:
            return OC_STR8("no space left");
        case OC_IO_ERR_NO_ENTRY:
            return OC_STR8("file or directory does not exist");
        case OC_IO_ERR_EXISTS:
            return OC_STR8("file already exists");
        case OC_IO_ERR_NOT_DIR:
            return OC_STR8("path element is not a directory");
        case OC_IO_ERR_DIR:
            return OC_STR8("attempted to write directory");
        case OC_IO_ERR_NOT_EMPTY:
            return OC_STR8("attempted to remove a non-empty directory");
        case OC_IO_ERR_MAX_FILES:
            return OC_STR8("max open files reached");
        case OC_IO_ERR_MAX_LINKS:
            return OC_STR8("too many symbolic links in path");
        case OC_IO_ERR_PATH_LENGTH:
            return OC_STR8("path too long");
        case OC_IO_ERR_FILE_SIZE:
            return OC_STR8("file too big");
        case OC_IO_ERR_OVERFLOW:
            return OC_STR8("offset too big");
        case OC_IO_ERR_NOT_READY:
            return OC_STR8("no data ready to be read/written");
        case OC_IO_ERR_MEM:
            return OC_STR8("failed to allocate memory");
        case OC_IO_ERR_INTERRUPT:
            return OC_STR8("operation interrupted by a signal");
        case OC_IO_ERR_PHYSICAL:
            return OC_STR8("physical IO error");
        case OC_IO_ERR_NO_DEVICE:
            return OC_STR8("device not found");
        case OC_IO_ERR_WALKOUT:
            return OC_STR8("attempted to walk out of root directory");
        case OC_IO_ERR_SYMLINK:
            return OC_STR8("encountered a symlink while following symlinks was disabled");
    }
}

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

oc_file_result oc_file_open(oc_str8 path, oc_file_access rights, oc_file_open_options* optionsPtr)
{
    oc_file_open_options options = optionsPtr ? *optionsPtr : (oc_file_open_options){ 0 };

    oc_io_req req = {
        .op = OC_IO_OPEN,
        .handle = options.root,
        .size = path.len,
        .buffer = path.ptr,
        .resolveFlags = options.resolve,
        .open.rights = rights,
        .open.flags = options.flags,
    };

    oc_io_cmp cmp = oc_io_wait_single_req(&req);

    //WARN: we always return a handle that can be queried for errors. Handles must be closed
    //      even if there was an error when opening
    if(cmp.error != OC_IO_OK)
    {
        return oc_result_error(oc_file_result, cmp.error);
    }
    else
    {
        return oc_result_value(oc_file_result, cmp.handle);
    }
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
    oc_io_req req = { .op = OC_IO_STAT,
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

oc_file_name_result oc_file_name(oc_arena* arena, oc_file file)
{
    oc_file_name_result result = { 0 };

    oc_arena_scope scratch = oc_scratch_begin_next(arena);
    u64 size = 4096;
    char* buffer = oc_arena_push_array_uninitialized(scratch.arena, char, size);
    oc_io_req req = {
        .op = OC_IO_GETNAME,
        .handle = file,
        .size = size,
        .buffer = buffer,
    };

    oc_io_cmp cmp = oc_io_wait_single_req(&req);
    if(cmp.error)
    {
        result = oc_result_error(oc_file_name_result, cmp.error);
    }
    else
    {
        oc_str8 string = oc_str8_push_copy(arena, oc_str8_from_buffer(cmp.size, req.buffer));
        result = oc_result_value(oc_file_name_result, string);
    }

    oc_scratch_end(scratch);
    return result;
}

oc_file_result oc_file_maketmp(oc_file_maketmp_flags flags)
{
    oc_io_req req = {
        .op = OC_IO_MAKE_TMP,
        .makeTmpFlags = flags,
    };

    oc_io_cmp cmp = oc_io_wait_single_req(&req);
    if(cmp.error != OC_IO_OK)
    {
        return oc_result_error(oc_file_result, cmp.error);
    }
    else
    {
        return oc_result_value(oc_file_result, cmp.handle);
    }
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
        .resolveFlags = options.resolve,
    };

    oc_io_cmp cmp = oc_io_wait_single_req(&req);
    return cmp.error;
}

oc_io_error oc_file_remove_recursive(oc_file root, oc_str8 path)
{
    oc_io_error error = OC_IO_OK;
    oc_file_result openRes = oc_file_open(path,
                                          OC_FILE_ACCESS_READ | OC_FILE_ACCESS_WRITE,
                                          &(oc_file_open_options){
                                              .root = root,
                                              .resolve = OC_FILE_RESOLVE_SYMLINK_OPEN_LAST,
                                          });
    oc_file file = oc_catch(openRes)
    {
        return openRes.error;
    }
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
        error = oc_file_remove(path,
                               &(oc_file_remove_options){
                                   .root = root,
                                   .flags = OC_FILE_REMOVE_DIR,
                               });
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

oc_io_error oc_file_copy_recursive(oc_str8 srcPath, oc_str8 dstPath, oc_file_copy_options* options)
{
    oc_file src = oc_catch(oc_file_open(srcPath,
                                        OC_FILE_ACCESS_READ,
                                        &(oc_file_open_options){
                                            .root = options->srcRoot,
                                        }))
    {
        return oc_last_error();
    }
    oc_file_status srcStatus = oc_file_get_status(src);

    if(srcStatus.type == OC_FILE_DIRECTORY)
    {
        oc_arena_scope scratch = oc_scratch_begin();

        //NOTE: if src is a directory, we create dst as a directory if it doesn't exist,
        // and copy src/* into dst/*
        oc_io_error error = oc_file_makedir(dstPath,
                                            &(oc_file_makedir_options){
                                                .root = options->dstRoot,
                                                .resolve = options->dstResolve,
                                                .flags = OC_FILE_MAKEDIR_IGNORE_EXISTING,
                                            });
        if(error != OC_IO_OK)
        {
            oc_file_close(src);
            return error;
        }

        oc_file_list files = oc_file_listdir(scratch.arena, src);
        oc_file_close(src);

        oc_file_list_for(files, file)
        {
            oc_str8 srcChild = oc_path_append(scratch.arena, srcPath, file->basename);
            oc_str8 dstChild = oc_path_append(scratch.arena, dstPath, file->basename);
            error = oc_file_copy_recursive(srcChild, dstChild, options);
            if(error != OC_IO_OK)
            {
                break;
            }
        }
        oc_scratch_end(scratch);
        return error;
    }
    else
    {
        //NOTE: if src is not a directory, we copy file to file
        return oc_file_copy(srcPath, dstPath, options);
    }
}

oc_io_error oc_file_copy(oc_str8 src, oc_str8 dst, oc_file_copy_options* optionsPtr)
{
    oc_file_copy_options options = optionsPtr
                                     ? *optionsPtr
                                     : (oc_file_copy_options){ 0 };

    options.srcResolve = OC_FILE_RESOLVE_SYMLINK_OPEN_LAST;
    options.dstResolve = OC_FILE_RESOLVE_SYMLINK_OPEN_LAST;

    oc_file_result srcRes = oc_file_open(src,
                                         OC_FILE_ACCESS_READ,
                                         &(oc_file_open_options){
                                             .root = options.srcRoot,
                                             .resolve = options.srcResolve,
                                         });
    oc_file srcFile = oc_catch(srcRes)
    {
        return srcRes.error;
    }

    oc_file_status srcStat = oc_file_get_status(srcFile);

    if(srcStat.type == OC_FILE_REGULAR || srcStat.type == OC_FILE_SYMLINK)
    {
        //NOTE: if src is a file, we open dst, creating it if it doesn't exist
        oc_file dstFile = oc_catch(oc_file_open(dst,
                                                OC_FILE_ACCESS_WRITE,
                                                &(oc_file_open_options){
                                                    .root = options.dstRoot,
                                                    .resolve = options.dstResolve,
                                                    .flags = OC_FILE_OPEN_CREATE,
                                                }))
        {
            oc_file_close(srcFile);
            return oc_last_error();
        }
        oc_file_status dstStat = oc_file_get_status(dstFile);
        if(dstStat.type == OC_FILE_DIRECTORY)
        {
            //NOTE: if dst is a directory, copy file into directory with the same basename
            oc_str8 basename = oc_path_slice_filename(src);
            oc_file_result tmpRes = oc_file_open(basename,
                                                 OC_FILE_ACCESS_WRITE,
                                                 &(oc_file_open_options){
                                                     .root = dstFile,
                                                     .resolve = OC_FILE_RESOLVE_SYMLINK_OPEN_LAST,
                                                     .flags = OC_FILE_OPEN_CREATE,
                                                 });
            oc_file tmp = oc_catch(tmpRes)
            {
                oc_file_close(srcFile);
                oc_file_close(dstFile);
                return tmpRes.error;
            }
            oc_file_close(dstFile);
            dstFile = tmp;
        }

        oc_io_req req = {
            .op = OC_IO_COPY,
            .handle = srcFile,
            .copy.dst = dstFile,
            .copy.flags = options.flags,
        };

        oc_io_cmp cmp = oc_io_wait_single_req(&req);

        oc_file_close(srcFile);
        oc_file_close(dstFile);
        return cmp.error;
    }
    else if(srcStat.type == OC_FILE_DIRECTORY)
    {
        oc_file_close(srcFile);

        return oc_file_copy_recursive(src, dst, &options);
    }
    else
    {
        return OC_IO_ERR_UNKNOWN; //TODO
    }
}
