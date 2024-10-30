/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "orca.h"
#include "system.h"

#include <dirent.h>

oc_sys_err_def oc_sys_err;

oc_str8 oc_sys_getcwd(oc_arena* a)
{
    char* cwd = getcwd(0, 0);
    oc_str8 res = oc_str8_push_cstring(a, cwd);
    free(cwd);
    return (res);
}

bool oc_sys_exists(oc_str8 path)
{
    struct stat st;

    oc_arena_scope scratch = oc_scratch_begin();
    const char* cpath = oc_str8_to_cstring(scratch.arena, path);
    int result = stat(cpath, &st);
    oc_scratch_end(scratch);

    if(result)
    {
        OC_ASSERT(errno == ENOENT);
        return false;
    }

    return true;
}

bool oc_sys_isdir(oc_str8 path)
{
    struct stat st;

    oc_arena_scope scratch = oc_scratch_begin();
    const char* cpath = oc_str8_to_cstring(scratch.arena, path);
    int result = stat(cpath, &st);
    oc_scratch_end(scratch);

    if(result)
    {
        OC_ASSERT(errno == ENOENT);
        return false;
    }

    return (st.st_mode & S_IFDIR) != 0;
}

bool oc_sys_mkdirs(oc_str8 path)
{
    oc_arena_scope scratch = oc_scratch_begin();
    const char* cpath = oc_str8_to_cstring(scratch.arena, path);

    int result = mkdir(cpath, S_IRWXU | S_IRWXG | S_IRWXO);

    oc_scratch_end(scratch);

    if(result)
    {
        snprintf(oc_sys_err.msg, OC_SYS_MAX_ERROR,
                 "failed to create directories \"%.*s\"", oc_str8_ip(path));
        oc_sys_err.code = result;
        return false;
    }

    return true;
}

bool oc_sys_rmdir(oc_str8 path)
{
    oc_unimplemented();
    return false;
}

//TODO(pld): can't we just reuse the platform_io helpers?
//TODO(pld): openat, etc
bool oc_sys_copy(oc_str8 src, oc_str8 dst)
{
    int srcFd = -1, dstFd = -1;

    if(oc_sys_isdir(src))
    {
        snprintf(oc_sys_err.msg, OC_SYS_MAX_ERROR,
                 "failed to copy file: \"%.*s\" is a directory, oc_sys_copy can only copy files; use oc_sys_copytree for directories", oc_str8_ip(src));
        return false;
    }

    oc_arena_scope scratch = oc_scratch_begin();

    if(oc_sys_isdir(dst))
    {
        dst = oc_path_append(scratch.arena, dst, oc_path_slice_filename(src));
    }

    const char* csrc = oc_str8_to_cstring(scratch.arena, src);
    const char* cdst = oc_str8_to_cstring(scratch.arena, dst);

    srcFd = open(csrc, O_RDONLY);
    if(srcFd == -1)  goto RET_ERR;
    dstFd = open(cdst, O_WRONLY | O_CREAT);
    if(dstFd == -1)  goto RET_ERR;

    struct stat st;
    int ok = fstat(srcFd, &st);
    if(ok == -1)  goto RET_ERR;
    size_t srcLen = st.st_size;

    ssize_t copied = copy_file_range(srcFd, NULL, dstFd, NULL, srcLen, 0);
    if(copied == -1)  goto RET_ERR;
    OC_ASSERT(copied == srcLen);

    close(dstFd);
    close(srcFd);
    oc_scratch_end(scratch);

    return true;

RET_ERR:
    snprintf(oc_sys_err.msg, OC_SYS_MAX_ERROR,
             "failed to copy file \"%.*s\" to \"%.*s\"",
             oc_str8_ip(src), oc_str8_ip(dst));
    oc_sys_err.code = result;

    if(dstFd >= 0)  close(dstFd);
    if(srcFd >= 0)  close(srcFd);
    oc_scratch_end(scratch);
    return false;
}

bool oc_sys_copytree(oc_str8 src, oc_str8 dst)
{
    oc_unimplemented();
    return false;
}

bool oc_sys_move(oc_str8 src, oc_str8 dst)
{
    bool result = true;

    oc_arena_scope scratch = oc_scratch_begin();
    const char* csrc = oc_str8_to_cstring(scratch.arena, src);
    const char* cdst = oc_str8_to_cstring(scratch.arena, dst);

    oc_str8 src_dir = oc_path_slice_directory(src);
    oc_str8 dst_dir = oc_path_slice_directory(dst);

    if(oc_str8_cmp(src_dir, dst_dir) == 0)
    {
        if(rename(csrc, cdst) != 0)
        {
            result = false;
        }
    }
    else
    {
        if(oc_sys_isdir(src))
        {
            if(!oc_sys_copytree(src, dst))
            {
                result = false;
            }
            if(result && !oc_sys_rmdir(src))
            {
                result = false;
            }
        }
        else
        {
            if(!oc_sys_copy(src, dst))
            {
                result = false;
            }
            if(result && remove(csrc) != 0)
            {
                result = false;
            }
        }
    }

    if(!result)
    {
        snprintf(oc_sys_err.msg, OC_SYS_MAX_ERROR,
                 "failed to move \"%.*s\" to \"%.*s\"",
                 oc_str8_ip(src), oc_str8_ip(dst));
    }

    oc_scratch_end(scratch);
    return result;
}

oc_list oc_sys_read_dir(oc_arena* a, oc_str8 path)
{
    oc_list entries = {0};

    char* path_cstr = oc_str8_to_cstring(a, path);
    DIR* dir = opendir(path_cstr);
    if (!dir) {
        return entries;
    }

    oc_str8 base = oc_str8_push_copy(a, path);

    struct dirent* d = NULL;

    while((d = readdir(dir)) != NULL)
    {
        if(0 == strcmp(d->d_name, ".") || 0 == strcmp(d->d_name, ".."))
        {
            continue;
        }

        oc_sys_dir_entry* entry = oc_arena_push_type(a, oc_sys_dir_entry);
        entry->base = base;
        entry->name = oc_str8_push_cstring(a, d->d_name);
        entry->is_dir = d->d_type & DT_DIR;
        oc_list_push_back(&entries, &entry->node);
    }

    return entries;
}
