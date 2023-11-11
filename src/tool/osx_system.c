/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <ftw.h>
#include <copyfile.h>

#include "system.h"

#include "platform/platform_path.h"
#include "util/debug.h"
#include "util/memory.h"
#include "util/strings.h"

oc_sys_err_def oc_sys_err;

// TODO: I guarantee I need to do more to escape paths EVERYWHERE.

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
        oc_sys_err = (oc_sys_err_def){
            .msg = OC_STR8("failed to create directories"),
            .code = result,
        };
        return false;
    }

    return true;
}

int remove_callback(const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf)
{
    int result = remove(fpath);

    if(result)
    {
        perror(fpath);
    }
    return result;
}

int remove_recursive(char* path)
{
    return nftw(path, remove_callback, 64, FTW_DEPTH | FTW_PHYS);
}

bool oc_sys_rmdir(oc_str8 path)
{
    oc_arena_scope scratch = oc_scratch_begin();
    char* cpath = oc_str8_to_cstring(scratch.arena, path);

    int result = remove_recursive(cpath);

    oc_scratch_end(scratch);

    if(result)
    {
        oc_sys_err = (oc_sys_err_def){
            .msg = OC_STR8("failed to remove directory"),
            .code = result,
        };
        return false;
    }

    return true;
}

bool oc_sys_copy(oc_str8 src, oc_str8 dst)
{
    if(oc_sys_isdir(src))
    {
        oc_sys_err = (oc_sys_err_def){
            .msg = OC_STR8("can only copy files, not directories; use oc_sys_copytree for directories"),
        };
        return false;
    }

    oc_arena_scope scratch = oc_scratch_begin();

    if(oc_sys_isdir(dst))
    {
        dst = oc_path_append(scratch.arena, dst, oc_path_slice_filename(src));
    }

    const char* csrc = oc_str8_to_cstring(scratch.arena, src);
    const char* cdst = oc_str8_to_cstring(scratch.arena, dst);

    int result = copyfile(csrc, cdst, 0, COPYFILE_ALL);

    oc_scratch_end(scratch);

    if(result)
    {
        oc_sys_err = (oc_sys_err_def){
            .msg = OC_STR8("failed to copy file"),
            .code = result,
        };
        return false;
    }

    return true;
}

bool oc_sys_copytree(oc_str8 src, oc_str8 dst)
{
    if(!oc_sys_isdir(src))
    {
        oc_sys_err = (oc_sys_err_def){
            .msg = OC_STR8("can only copy directories, not files; use oc_sys_copy for files"),
        };
        return false;
    }

    oc_arena_scope scratch = oc_scratch_begin();
    const char* csrc = oc_str8_to_cstring(scratch.arena, src);
    const char* cdst = oc_str8_to_cstring(scratch.arena, dst);

    int result = copyfile(csrc, cdst, 0, COPYFILE_ALL | COPYFILE_RECURSIVE);

    oc_scratch_end(scratch);

    if(result)
    {
        oc_sys_err = (oc_sys_err_def){
            .msg = OC_STR8("failed to copy tree"),
            .code = result,
        };
        return false;
    }

    return true;
}
