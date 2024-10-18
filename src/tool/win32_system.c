/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include <processenv.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <io.h>

#include "orca.h"
#include "system.h"

oc_sys_err_def oc_sys_err;

// TODO: I guarantee I need to do more to escape paths EVERYWHERE.

oc_str8 oc_sys_getcwd(oc_arena* a)
{
    u64 len = GetCurrentDirectory(0, NULL);
    char* buf = oc_arena_push(a, len);
    GetCurrentDirectory(len, buf);
    return OC_STR8(buf);
}

bool oc_sys_exists(oc_str8 path)
{
    struct _stat stat;

    oc_arena_scope scratch = oc_scratch_begin();
    const char* cpath = oc_str8_to_cstring(scratch.arena, path);
    int result = _stat(cpath, &stat);
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
    struct _stat stat;

    oc_arena_scope scratch = oc_scratch_begin();
    const char* cpath = oc_str8_to_cstring(scratch.arena, path);
    int result = _stat(cpath, &stat);
    oc_scratch_end(scratch);

    if(result)
    {
        OC_ASSERT(errno == ENOENT);
        return false;
    }

    return (stat.st_mode & _S_IFDIR) != 0;
}

bool oc_sys_mkdirs(oc_str8 path)
{
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 full_path = oc_path_canonical(scratch.arena, path);
    oc_str8 cmd = oc_str8_pushf(scratch.arena,
                                "mkdir \"%.*s\"", oc_str8_ip(full_path));
    int result = system(cmd.ptr);
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
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 cmd = oc_str8_pushf(scratch.arena,
                                "rd /s /q \"%.*s\"",
                                oc_str8_ip(path));
    int result = system(cmd.ptr);
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
        oc_sys_err.code = 0;
        return false;
    }

    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 full_src = oc_path_canonical(scratch.arena, src);
    oc_str8 full_dst = { 0 };
    if(oc_sys_isdir(dst))
    {
        oc_str8 filename = oc_path_slice_filename(src);
        oc_str8 dst_with_filename = oc_path_append(scratch.arena, dst, filename);
        full_dst = oc_path_canonical(scratch.arena, dst_with_filename);
    }
    else
    {
        full_dst = oc_path_canonical(scratch.arena, dst);
    }

    // TODO(shaw): remove this once oc_win32_wide_to_utf8() is fixed to null terminate
    char* full_src_cstr = oc_str8_to_cstring(scratch.arena, full_src);
    char* full_dst_cstr = oc_str8_to_cstring(scratch.arena, full_dst);

    BOOL result = CopyFile(full_src_cstr, full_dst_cstr, false);

    oc_scratch_end(scratch);

    if(!result)
    {
        snprintf(oc_sys_err.msg, OC_SYS_MAX_ERROR,
                 "failed to copy file \"%.*s\" to \"%.*s\"",
                 oc_str8_ip(src), oc_str8_ip(dst));
        oc_sys_err.code = GetLastError();
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

    oc_str8 dst_dir = oc_path_slice_directory(dst);
    if(!oc_sys_exists(dst_dir) && !oc_sys_mkdirs(dst_dir))
    {
        return false;
    }

    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 full_src = oc_path_canonical(scratch.arena, src);
    oc_str8 full_dst = oc_path_canonical(scratch.arena, dst);
    oc_str8 cmd = oc_str8_pushf(scratch.arena,
                                "xcopy /s /e /y \"%.*s\" \"%.*s\"",
                                oc_str8_ip(full_src), oc_str8_ip(full_dst));
    int result = system(cmd.ptr);
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

// NOTE: if dst is a filename and it already exists, it will be overwritten
bool oc_sys_move(oc_str8 src, oc_str8 dst)
{
    if(!oc_sys_exists(src))
    {
        snprintf(oc_sys_err.msg, OC_SYS_MAX_ERROR,
                 "failed to move file or directory, source does not exist: \"%.*s\"",
                 oc_str8_ip(src));
        return false;
    }

    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 full_src = oc_path_canonical(scratch.arena, src);
    oc_str8 full_dst = oc_path_canonical(scratch.arena, dst);
    oc_str8 cmd = oc_str8_pushf(scratch.arena,
                                "move /Y \"%.*s\" \"%.*s\"",
                                oc_str8_ip(full_src), oc_str8_ip(full_dst));
    int result = system(cmd.ptr);
    oc_scratch_end(scratch);

    if(result)
    {
        snprintf(oc_sys_err.msg, OC_SYS_MAX_ERROR,
                 "failed to move \"%.*s\" to \"%.*s\"",
                 oc_str8_ip(src), oc_str8_ip(dst));
        oc_sys_err.code = result;
        return false;
    }

    return true;
}

// returns a list with element type oc_sys_dir_entry
oc_list oc_sys_read_dir(oc_arena *a, oc_str8 path)
{
    oc_list entries = {0};

    oc_str8 file_spec = oc_path_append(a, path, OC_STR8("*"));
    oc_str16 file_spec_wide = oc_win32_utf8_to_wide(a, file_spec);

    struct _wfinddata_t file_info;
    intptr_t handle = _wfindfirst(file_spec_wide.ptr, &file_info);
    if(handle == -1)
    {
        // no files
        return entries;
    }

    oc_str8 base = oc_str8_push_copy(a, path);

    do
    {
        if(0 == wcscmp(file_info.name, L".") || 0 == wcscmp(file_info.name, L".."))
        {
            continue;
        }
        oc_sys_dir_entry *entry = oc_arena_push_type(a, oc_sys_dir_entry);
        entry->base = base;
        oc_str16 name_wide = oc_str16_from_buffer(lstrlenW(file_info.name), file_info.name);
        entry->name = oc_win32_wide_to_utf8(a, name_wide);
        entry->is_dir = file_info.attrib & _A_SUBDIR;
        oc_list_push_back(&entries, &entry->node);
    } while (_wfindnext(handle, &file_info) == 0);

    return entries;
}
