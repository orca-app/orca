/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <dlfcn.h>
#include <sys/stat.h>

#include "util/memory.h"
#include "reload.h"

int oc_reload_if_needed(oc_reload_info* info)
{
    oc_scratch scratch = oc_scratch_begin();
    struct stat dylibStat = { 0 };

    stat(info->path.ptr, &dylibStat);
    f64 newModTime = dylibStat.st_mtimespec.tv_sec + dylibStat.st_mtimespec.tv_nsec * 1e-9;

    if(newModTime > info->modTime)
    {
        oc_log_info("reload!");

        if(info->begin)
        {
            info->begin(info->userData);
        }

        oc_str8 tmpPath = oc_str8_push_copy(scratch.allocator, OC_STR8("/tmp/libXXXXXX"));
        mkstemp(tmpPath.ptr);
        oc_file_copy(info->path, tmpPath, 0);

        dlclose(info->module);
        info->module = dlopen(tmpPath.ptr, RTLD_LAZY | RTLD_LOCAL);

        for(int i = 0; i < OC_RELOAD_PROC_COUNT; i++)
        {
            if(info->symbolNames[i])
            {
                info->procs[i] = dlsym(info->module, info->symbolNames[i]);
            }
            else
            {
                info->procs[i] = 0;
            }
        }

        if(info->modTime != 0 && info->end)
        {
            info->end(info->userData);
        }
        info->modTime = newModTime;
    }
    oc_scratch_end(scratch);
    return 0;
}
