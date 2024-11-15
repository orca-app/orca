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
    oc_arena_scope scratch = oc_scratch_begin();
    char *cpath = oc_str8_to_cstring(scratch.arena, path);
    int rootFd = open(cpath, O_RDWR | O_DIRECTORY);
    if(rootFd == -1 && errno == ENOENT)
    {
        return true;
    }
    else if(rootFd == -1)
    {
        goto RET_ERR;
    }
    DIR* root = fdopendir(rootFd);
    OC_ASSERT(root);

    struct
    {
        int fd;
        int parentFd;
        DIR* dir;
        char* name;
    } stack[128] = {0};
    u32 stackLen = 0;

    stack[stackLen++].fd = rootFd;
    stack[stackLen++].parentFd = -1;
    stack[stackLen++].dir = root;
    stack[stackLen++].name = "";

    while(stackLen > 0)
    {
        int fd = stack[stackLen - 1].fd;
        int parentFd = stack[stackLen - 1].parentFd;
        DIR* dir = stack[stackLen - 1].dir;
        char* name = stack[stackLen - 1].name;

        errno = 0;
        struct dirent* entry = readdir(dir);
        if(!entry && errno)
        {
            goto RET_ERR;
        }

        if(!entry)
        {
            int ok = close(fd);
            OC_ASSERT(ok == 0);
            ok = closedir(dir);
            OC_ASSERT(ok == 0);
            ok = unlinkat(parentFd, name, AT_REMOVEDIR);
            if(ok == -1)
            {
                goto RET_ERR;
            }
            stackLen--;
        }
        else if(entry->d_type == DT_DIR)
        {
            char* entryName = oc_arena_dup(scratch.arena, entry->d_name, strlen(entry->d_name) + 1);
            int entryFd = openat(fd, entryName, O_RDWR | O_DIRECTORY);
            if(entryFd == -1)
            {
                goto RET_ERR;
            }
            int entryFd2 = dup(entryFd);
            OC_ASSERT(entryFd2 != -1);
            DIR* entryDir = fdopendir(entryFd2);
            OC_ASSERT(entryDir);

            OC_ASSERT(stackLen < oc_array_size(stack));
            stack[stackLen].fd = entryFd;
            stack[stackLen].parentFd = fd;
            stack[stackLen].dir = entryDir;
            stack[stackLen].name = entryName;
            stackLen++;
        }
        else
        {
            int ok = unlinkat(fd, entry->d_name, 0);
            if(ok == -1)
            {
                goto RET_ERR;
            }
        }
    }

    oc_scratch_end(scratch);
    return true;

RET_ERR:
    //TODO(pld): make sure all entries in the stack are closed
    oc_scratch_end(scratch);
    oc_sys_err.code = errno;
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
    oc_sys_err.code = -1;

    if(dstFd >= 0)  close(dstFd);
    if(srcFd >= 0)  close(srcFd);
    oc_scratch_end(scratch);
    return false;
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
    char *full_csrc = oc_str8_to_cstring(scratch.arena, full_src);

    DIR* dir = opendir(full_csrc);
    oc_str8 rel = {0};
    struct dirent* d = NULL;
    while((d = readdir(dir)))
    {
        rel = oc_path_append(scratch.arena, rel, OC_STR8(d->d_name));
        if(d->d_type == DT_DIR) {
            //TODO(pld): recurse
        } else {
            oc_str8 file_src = oc_path_append(scratch.arena, full_src, rel);
            oc_str8 file_dst = oc_path_append(scratch.arena, full_dst, rel);
            oc_sys_copy(file_src, file_dst);
        }
    }

    oc_scratch_end(scratch);

    if(0)
    {
        snprintf(oc_sys_err.msg, OC_SYS_MAX_ERROR,
                 "failed to copy directory tree from \"%.*s\" to \"%.*s\"",
                 oc_str8_ip(src), oc_str8_ip(dst));
        oc_sys_err.code = -1;
        return false;
    }

    return true;


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
