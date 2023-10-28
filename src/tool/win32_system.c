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

#include "system.h"

#include "platform/platform_path.h"
#include "util/debug.h"
#include "util/memory.h"
#include "util/strings.h"

oc_sys_err_def oc_sys_err;

// TODO: I guarantee I need to do more to escape paths EVERYWHERE.

oc_str8 oc_sys_getcwd(oc_arena* a)
{
    u64 len = GetCurrentDirectory(0, NULL);
    char* buf = oc_arena_push(a, len);
    GetCurrentDirectory(len, buf);
    return OC_STR8(buf);
}

bool oc_sys_path_exists(oc_str8 path)
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

bool oc_sys_mkdirs(oc_str8 path)
{
    char* abspath = _fullpath(NULL, path.ptr, path.len * 2);
    // TODO: remove these debugs
    printf("PATH: %.*s\n", oc_str8_printf(path));
    printf("ABSPATH: %s\n", abspath);
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 cmd = oc_str8_pushf(scratch.arena, "mkdir \"%s\"", abspath);
    int result = system(cmd.ptr);
    oc_scratch_end(scratch);
    free(abspath);

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

bool oc_sys_rmdir(oc_str8 path)
{
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 cmd = oc_str8_pushf(scratch.arena,
                                "rd /s /q \"%.*s\"",
                                oc_str8_printf(path));
    int result = system(cmd.ptr);
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
    char* csrc = _fullpath(NULL, src.ptr, src.len * 2);
    char* cdst = _fullpath(NULL, dst.ptr, dst.len * 2);
    // TODO: remove these debugs
    printf("SRC: %s\n", csrc);
    printf("DST: %s\n", cdst);
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 cmd = oc_str8_pushf(scratch.arena, "copy \"%s\" \"%s\"", csrc, cdst);
    int result = system(cmd.ptr);
    oc_scratch_end(scratch);
    free(csrc);
    free(cdst);

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
