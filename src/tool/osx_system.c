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
#include <dirent.h>

#include "orca.h"
#include "system.h"

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
    bool success = true;

    if (path.len > 0) {
        oc_arena_scope scratch = oc_scratch_begin();

        // iterate through each path component to ensure we create the whole path
        for (const char* begin = path.ptr; begin;) {
            begin = strchr(*begin == '/' ? begin + 1 : begin, '/');

            oc_str8 slice = oc_str8_slice(path, 0, (begin == NULL) ? path.len : begin - path.ptr);

            if (!oc_sys_isdir(slice)) {
                const char* cpath = oc_str8_to_cstring(scratch.arena, slice);

                int result = mkdir(cpath, S_IRWXU | S_IRWXG | S_IRWXO);
                if(result)
                {
                    snprintf(oc_sys_err.msg, OC_SYS_MAX_ERROR,
                             "failed to create directories \"%.*s\"", oc_str8_ip(path));
                    oc_sys_err.code = result;
                    success = false;
                    break;
                }
            }
        }

        oc_scratch_end(scratch);
    }

    return success;
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
        snprintf(oc_sys_err.msg, OC_SYS_MAX_ERROR,
                 "failed to remove directory \"%.*s\"", oc_str8_ip(path));
        oc_sys_err.code = result;
        return false;
    }

    return true;
}

bool oc_sys_copy(oc_str8 src, oc_str8 dst)
{
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

    int result = copyfile(csrc, cdst, 0, COPYFILE_ALL);

    oc_scratch_end(scratch);

    if(result)
    {
        snprintf(oc_sys_err.msg, OC_SYS_MAX_ERROR,
                 "failed to copy file \"%.*s\" to \"%.*s\"",
                 oc_str8_ip(src), oc_str8_ip(dst));
        oc_sys_err.code = result;
        return false;
    }

    return true;
}

bool oc_sys_copytree(oc_str8 src, oc_str8 dst)
{
    if(!oc_sys_isdir(src))
    {
        snprintf(oc_sys_err.msg, OC_SYS_MAX_ERROR,
                 "can only copy directories, not files; use oc_sys_copy for files");
        oc_sys_err.code = 0;
        return false;
    }

    oc_arena_scope scratch = oc_scratch_begin();
    const char* csrc = oc_str8_to_cstring(scratch.arena, src);
    const char* cdst = oc_str8_to_cstring(scratch.arena, dst);

    int result = copyfile(csrc, cdst, 0, COPYFILE_ALL | COPYFILE_RECURSIVE);

    oc_scratch_end(scratch);

    if(result)
    {
        snprintf(oc_sys_err.msg, OC_SYS_MAX_ERROR,
                 "failed to copy directory tree from \"%.*s\" to \"%.*s\"",
                 oc_str8_ip(src), oc_str8_ip(dst));
        oc_sys_err.code = result;
        return false;
    }

    return true;
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

oc_list oc_sys_read_dir(oc_arena *a, oc_str8 path)
{
    oc_list entries = {0};

    char *path_cstr = oc_str8_to_cstring(a, path);
	DIR *dir = opendir(path_cstr);
	if (!dir) {
        return entries;
	}

    oc_str8 base = oc_str8_push_copy(a, path);

	struct dirent *d = NULL;

	while((d = readdir(dir)) != NULL)
    {
		if(0 == strcmp(d->d_name, ".") || 0 == strcmp(d->d_name, ".."))
        {
			continue;
		}

        oc_sys_dir_entry *entry = oc_arena_push_type(a, oc_sys_dir_entry);
        entry->base = base;
        entry->name = oc_str8_push_cstring(a, d->d_name);
        entry->is_dir = d->d_type & DT_DIR; 
        oc_list_push_back(&entries, &entry->node);
	}

    return entries;
}
