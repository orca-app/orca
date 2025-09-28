/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "platform/platform_io_internal.h"
#include "platform/platform_path.h"

oc_file_table oc_globalFileTable = { 0 };

oc_file_table* oc_file_table_get_global()
{
    return (&oc_globalFileTable);
}

oc_file_slot* oc_file_slot_alloc(oc_file_table* table)
{
    oc_file_slot* slot = oc_list_pop_front_entry(&table->freeList, oc_file_slot, freeListElt);
    if(!slot && table->nextSlot < OC_IO_MAX_FILE_SLOTS)
    {
        slot = &table->slots[table->nextSlot];
        slot->generation = 1;
        table->nextSlot++;
    }

    u32 tmpGeneration = slot->generation;
    memset(slot, 0, sizeof(oc_file_slot));
    slot->generation = tmpGeneration;

    return (slot);
}

void oc_file_slot_recycle(oc_file_table* table, oc_file_slot* slot)
{
    slot->generation++;
    oc_list_push_front(&table->freeList, &slot->freeListElt);
}

oc_file oc_file_from_slot(oc_file_table* table, oc_file_slot* slot)
{
    u64 index = slot - table->slots;
    u64 generation = slot->generation;
    oc_file handle = { .h = (generation << 32) | index };
    return (handle);
}

oc_file_slot* oc_file_slot_from_handle(oc_file_table* table, oc_file handle)
{
    oc_file_slot* slot = 0;

    u64 index = handle.h & 0xffffffff;
    u64 generation = handle.h >> 32;

    if(index < table->nextSlot)
    {
        oc_file_slot* candidate = &table->slots[index];
        if(candidate->generation == generation)
        {
            slot = candidate;
        }
    }
    return (slot);
}

oc_io_cmp oc_io_wait_single_req(oc_io_req* req)
{
    return (oc_io_wait_single_req_for_table(req, &oc_globalFileTable));
}

//-----------------------------------------------------------------------
// io common primitives
//-----------------------------------------------------------------------
#include "util/wrapped_types.h"

static int oc_io_convert_access_rights(oc_file_access rights);
static int oc_io_convert_open_flags(oc_file_open_flags flags);
static int oc_io_update_dir_flags_at(int dirFd, char* path, int flags);

typedef oc_result(oc_file_desc, oc_io_error) oc_io_open_no_follow_result;

oc_io_open_no_follow_result oc_io_open_no_follow(oc_file_desc rootFd, oc_str8 path, oc_file_access rights, oc_file_open_flags openFlags)
{
    /*NOTE: Open a file, never following symlinks.
        Leading symlinks in the path will error.
        If the file is a symlink, the symlink itself will be opened (not its target)

        (openat flags don't allow to error on leading symlinks while still opening the final
        symlink as a file. So we have to walk the path ourselves here.)
    */
    oc_arena_scope scratch = oc_scratch_begin();

    if(rootFd >= 0)
    {
        //NOTE: if path is absolute, change for a relative one, otherwise openat ignores fd.
        if(path.len && path.ptr[0] == '/')
        {
            u64 start = 0;
            while(start < path.len && path.ptr[start] == '/')
            {
                start++;
            }
            path = oc_str8_slice(path, start, path.len);
        }
    }
    else
    {
        rootFd = AT_FDCWD;
    }

    oc_file_desc fd = rootFd;
    oc_str8_list leadingElements = oc_path_split(scratch.arena, path);
    oc_str8 name = oc_str8_list_pop_back(&leadingElements);

    oc_io_error error = OC_IO_OK;

    oc_str8_list_for(leadingElements, elt)
    {
        //NOTE: open next element, and get its status, while holding to its fd
        //NOTE: for leading directories, we don't need access rights / flags
        char* eltCStr = oc_str8_to_cstring(scratch.arena, elt->string);
        oc_file_desc newFd = openat(fd, eltCStr, O_SYMLINK);
        close(fd);
        fd = newFd;

        if(oc_file_desc_is_nil(fd))
        {
            error = oc_io_raw_last_error();
        }
        else
        {
            struct stat st;
            int r = fstat(fd, &st);
            if(r)
            {
                error = oc_io_raw_last_error();
            }
            else if((st.st_mode & S_IFMT) != S_IFDIR)
            {
                //NOTE: leading path must contain only directories.
                error = OC_IO_ERR_NOT_DIR;
            }
        }
        if(error)
        {
            close(fd);
            break;
        }
    }

    if(error == OC_IO_OK)
    {
        //NOTE: here, we have an fd to the second-to-last element of the path. We can open the last element
        // with the requested access rights and creation flags
        char* nameCStr = oc_str8_to_cstring(scratch.arena, name);

        //TODO: convert flags & mode
        int flags = oc_io_convert_access_rights(rights);
        flags |= oc_io_convert_open_flags(openFlags);
        flags = oc_io_update_dir_flags_at(fd, nameCStr, flags);

        mode_t mode = S_IRUSR
                    | S_IWUSR
                    | S_IRGRP
                    | S_IWGRP
                    | S_IROTH
                    | S_IWOTH;

        oc_file_desc newFd = openat(fd, nameCStr, flags, mode);

        if(oc_file_desc_is_nil(newFd))
        {
            error = oc_io_raw_last_error();
        }
        close(fd);
        fd = newFd;
    }
    oc_scratch_end(scratch);

    if(error == OC_IO_OK)
    {
        return oc_wrap_value(oc_io_open_no_follow_result, fd);
    }
    else
    {
        return oc_wrap_error(oc_io_open_no_follow_result, error);
    }
}

typedef struct oc_io_resolve_result
{
    oc_io_error error;
    oc_file_desc fd;
    oc_str8 path;
    oc_str8 name;
} oc_io_resolve_result;

oc_io_resolve_result oc_io_resolve(oc_arena* arena, oc_file_desc rootFd, oc_str8 path, oc_file_open_flags resolveFlags)
{
    oc_io_resolve_result result = { 0 };
    oc_arena_scope scratch = oc_scratch_begin_next(arena);

    if(oc_file_desc_is_nil(rootFd))
    {
        //NOTE: if rootFd is nil, we set it to the current directory.
        // Absolute paths will ignore this.
        rootFd = OC_FILE_AT_FDCWD;
    }
    else if(path.len && path.ptr[0] == '/')
    {
        //NOTE: if rootFd is not null, we treat absolute paths as relative to rootFd
        path = oc_path_append(scratch.arena, OC_STR8("."), path);
    }

    oc_file_desc fd = rootFd == OC_FILE_AT_FDCWD ? rootFd : dup(rootFd);
    oc_str8_list normElements = { 0 };
    oc_str8_list pathElements = oc_path_split(scratch.arena, path);
    oc_str8_elt* elt = 0;

    while((elt = oc_list_pop_front_entry(&pathElements.list, oc_str8_elt, listElt)) != 0)
    {
        if(!oc_str8_cmp(elt->string, OC_STR8(".")))
        {
            // skip '.'
        }
        else if(!oc_str8_cmp(elt->string, OC_STR8("..")))
        {
            /*NOTE: .. goes up one directory.
                - If there's an element that isn't / or .., pop it.
                - If there's no last element, or the last element is .. or /, this is an error
            */
            oc_str8_elt* last = oc_list_last_entry(normElements.list, oc_str8_elt, listElt);
            if(!last || !oc_str8_cmp(last->string, OC_STR8("..")) || !oc_str8_cmp(last->string, OC_STR8("/")))
            {
                result.error = OC_IO_ERR_WALKOUT;
                break;
            }
            else
            {
                //NOTE: here we need to recompute fd from path. We can't just openat ".." because the directory
                // associated with fd could have been moved, and we could potentially escape the root
                oc_str8 normPath = oc_path_join(scratch.arena, normElements);
                oc_io_resolve_result r = oc_io_resolve(scratch.arena, rootFd, normPath, OC_FILE_SYMLINK_DONT_FOLLOW);
                if(r.error != OC_IO_OK)
                {
                    result.error = r.error;
                    break;
                }
                close(fd);
                fd = r.fd;

                oc_str8_list_pop_back(&normElements);
            }
        }
        else
        {
            //NOTE: now what we need to do depends on the type of file
            oc_file_status status = { 0 };
            oc_io_error error = oc_io_raw_fstat_at(fd, elt->string, OC_FILE_SYMLINK_OPEN_LAST, &status);

            if(error)
            {
                if(error != OC_IO_ERR_NO_ENTRY || !oc_str8_list_empty(pathElements))
                {
                    //NOTE: only the last element is allowed to be non-existent
                    result.error = error;
                }
                else
                {
                    result.name = oc_str8_push_copy(arena, elt->string);
                }
                break;
            }

            char* eltCStr = oc_str8_to_cstring(scratch.arena, elt->string);

            if(status.type == OC_FILE_SYMLINK)
            {
                if(!oc_list_empty(pathElements.list) && (resolveFlags & OC_FILE_SYMLINK_DONT_FOLLOW))
                {
                    result.error = OC_IO_ERR_SYMLINK;
                    break;
                }
                else if((resolveFlags & OC_FILE_SYMLINK_DONT_FOLLOW) || (resolveFlags & OC_FILE_SYMLINK_OPEN_LAST))
                {
                    result.name = oc_str8_push_copy(arena, elt->string);
                    break;
                }

                char* buff = oc_arena_push_array(scratch.arena, char, PATH_MAX);
                ssize_t sz = readlinkat(fd, eltCStr, buff, PATH_MAX);
                if(sz < 0)
                {
                    result.error = oc_io_raw_last_error();
                    break;
                }
                oc_str8 link = oc_str8_from_buffer(sz, buff);

                if(oc_path_is_absolute(link))
                {
                    result.error = OC_IO_ERR_WALKOUT;
                    break;
                }

                oc_str8_list linkElements = oc_path_split(scratch.arena, link);

                //NOTE: push linkElements in front of pathElements
                oc_list_for_reverse(linkElements.list, elt, oc_str8_elt, listElt)
                {
                    oc_str8_list_push_front(scratch.arena, &pathElements, elt->string);
                }
            }
            else if(status.type == OC_FILE_DIRECTORY || status.type == OC_FILE_REGULAR)
            {
                if(oc_str8_list_empty(pathElements))
                {
                    //NOTE: we're on the last element, set name and break.
                    result.name = oc_str8_push_copy(arena, elt->string);
                    break;
                }
                else if(status.type != OC_FILE_DIRECTORY)
                {
                    //NOTE: leading elements must be directories
                    result.error = OC_IO_ERR_NOT_DIR;
                    break;
                }
                else
                {
                    //NOTE: move fd and push directory to normalize elements
                    oc_file_desc newFd = openat(fd, eltCStr, O_DIRECTORY);
                    if(oc_file_desc_is_nil(newFd))
                    {
                        result.error = oc_io_raw_last_error();
                        break;
                    }
                    oc_io_raw_close(fd);
                    fd = newFd;

                    oc_str8_list_push(scratch.arena, &normElements, elt->string);
                }
            }
            else
            {
                result.error = OC_IO_ERR_NOT_DIR;
                break;
            }
        }
    }

    if(result.error)
    {
        close(fd);
    }
    else
    {
        result.fd = fd;

        if(result.name.len == 0)
        {
            //NOTE: the name could be empty because it ends with .. or symlinks.
            // In this case, we treat it as the current directory
            result.name = OC_STR8(".");
        }
        result.path = oc_path_join(scratch.arena, normElements);
        result.path = oc_path_append(arena, result.path, result.name);
    }

    oc_scratch_end(scratch);
    return result;
}

oc_io_cmp oc_io_open_at(oc_file_slot* atSlot, oc_io_req* req, oc_file_table* table)
{
    oc_io_cmp cmp = { 0 };

    oc_file_slot* slot = oc_file_slot_alloc(table);
    if(!slot)
    {
        cmp.error = OC_IO_ERR_MAX_FILES;
    }
    else
    {
        slot->fd = oc_file_desc_nil();
        cmp.handle = oc_file_from_slot(table, slot);

        oc_str8 path = oc_str8_from_buffer(req->size, req->buffer);

        if(!path.len)
        {
            slot->error = OC_IO_ERR_ARG;
        }
        else if(!atSlot && !oc_file_is_nil(req->handle))
        {
            //TODO: redundant with checks upstack?
            slot->error = OC_IO_ERR_HANDLE;
        }
        else
        {
            //NOTE: parent capability's rights must be greater or equal to requested rights
            slot->rights = req->open.rights;
            if(atSlot)
            {
                slot->rights &= atSlot->rights;
            }

            if(slot->rights != req->open.rights)
            {
                slot->error = OC_IO_ERR_PERM;
            }
            else
            {
                oc_arena_scope scratch = oc_scratch_begin();

                oc_file_desc rootFd = atSlot ? atSlot->fd : oc_file_desc_nil();

                oc_io_resolve_result resolve = oc_io_resolve(scratch.arena, rootFd, path, req->open.flags);
                if(resolve.error != OC_IO_OK)
                {
                    slot->error = resolve.error;
                }
                else
                {
                    //NOTE: here, we have an fd to the second-to-last element of the path. We can open the last element
                    // with the requested access rights and creation flags

                    int flags = oc_io_convert_access_rights(req->open.rights);
                    flags |= oc_io_convert_open_flags(req->open.flags);
                    flags = oc_io_update_dir_flags_at(resolve.fd, resolve.name.ptr, flags);

                    mode_t mode = S_IRUSR
                                | S_IWUSR
                                | S_IRGRP
                                | S_IWGRP
                                | S_IROTH
                                | S_IWOTH;

                    slot->fd = openat(resolve.fd, resolve.name.ptr, flags | O_SYMLINK, mode);

                    if(oc_file_desc_is_nil(slot->fd))
                    {
                        slot->error = oc_io_raw_last_error();
                    }
                    close(resolve.fd);
                }
                oc_scratch_end(scratch);
            }
        }

        /////////////////////////////////////////////////////////////////////////////
        //TODO: do the opposite: set cmp error above, and set slot to cmp here
        /////////////////////////////////////////////////////////////////////////////
        if(slot->error)
        {
            slot->fatal = true;
            cmp.error = slot->error;
        }
    }
    return (cmp);
}

oc_io_cmp oc_io_maketmp(oc_io_req* req, oc_file_table* table)
{
    oc_io_cmp cmp = { 0 };

    oc_file_slot* slot = oc_file_slot_alloc(table);
    if(!slot)
    {
        cmp.error = OC_IO_ERR_MAX_FILES;
    }
    else
    {
        oc_arena_scope scratch = oc_scratch_begin();
        oc_str8 template = oc_str8_push_cstring(scratch.arena, "/tmp/orca.XXXXXX");

        slot->fd = oc_file_desc_nil();
        cmp.handle = oc_file_from_slot(table, slot);

        slot->rights = OC_FILE_ACCESS_READ | OC_FILE_ACCESS_WRITE;
        if(req->makeTmpFlags & OC_FILE_MAKETMP_DIRECTORY)
        {
            char* path = mkdtemp(template.ptr);
            slot->fd = open(path, O_DIRECTORY);
            if(oc_file_desc_is_nil(slot->fd))
            {
                slot->error = oc_io_raw_last_error();
            }
        }
        else
        {
            slot->fd = mkstemp(template.ptr);
            if(oc_file_desc_is_nil(slot->fd))
            {
                slot->error = oc_io_raw_last_error();
            }
        }

        oc_scratch_end(scratch);
    }
    if(slot->error)
    {
        slot->fatal = true;
        cmp.error = slot->error;
    }
    return cmp;
}

#include <sys/stat.h>

oc_io_cmp oc_io_makedir(oc_file_slot* atSlot, oc_io_req* req)
{
    oc_io_cmp cmp = { 0 };

    oc_str8 path = oc_str8_from_buffer(req->size, req->buffer);

    if(!path.len)
    {
        cmp.error = OC_IO_ERR_ARG;
    }
    else if(!atSlot && !oc_file_is_nil(req->handle))
    {
        cmp.error = OC_IO_ERR_HANDLE;
    }
    else if(atSlot
            && (!(atSlot->rights & OC_FILE_ACCESS_READ) || !(atSlot->rights & OC_FILE_ACCESS_WRITE))) //TODO: should we allow write only?)
    {
        cmp.error = OC_IO_ERR_PERM;
    }
    else
    {
        oc_arena_scope scratch = oc_scratch_begin();

        oc_file_desc rootFd = atSlot ? atSlot->fd : oc_file_desc_nil();

        //////////////////////////////////////////////////////
        //TODO: here, if flags has OC_FILE_MAKEDIR_CREATE_PARENTS, we'd like to call
        // ourselves for each element with the OC_FILE_MAKEDIR_IGNORE_EXISTING flags set,
        // but the atSlot / req parameters make this a bit annoying...
        if(req->makeDirFlags & OC_FILE_MAKEDIR_CREATE_PARENTS)
        {
            oc_str8_list pathElements = oc_path_split(scratch.arena, path);
            oc_str8 prefixPath = { 0 };
            oc_str8_list_for(pathElements, elt)
            {
                prefixPath = oc_path_append(scratch.arena, prefixPath, elt->string);

                oc_io_req subReq = {
                    .op = OC_IO_MAKE_DIR,
                    .handle = req->handle,
                    .size = prefixPath.len,
                    .buffer = prefixPath.ptr,
                    .makeDirFlags = OC_FILE_MAKEDIR_IGNORE_EXISTING,
                };
                oc_io_cmp subCmp = oc_io_makedir(atSlot, &subReq);
                if(subCmp.error != OC_IO_OK)
                {
                    cmp.error = OC_IO_OK;
                    break;
                }
            }
        }
        else
        {
            oc_io_resolve_result resolve = oc_io_resolve(scratch.arena, rootFd, path, req->makeDirFlags);
            if(resolve.error)
            {
                cmp.error = resolve.error;
            }
            else
            {
                int r = mkdirat(resolve.fd, resolve.name.ptr, 0700);
                if(r)
                {
                    oc_io_error error = oc_io_raw_last_error();
                    if(error != OC_IO_ERR_EXISTS || !(req->makeDirFlags & OC_FILE_MAKEDIR_IGNORE_EXISTING))
                    {
                        cmp.error = error;
                    }
                }
            }
        }
        oc_scratch_end(scratch);
    }
    return cmp;
}

oc_io_cmp oc_io_remove(oc_file_slot* atSlot, oc_io_req* req)
{
    oc_io_cmp cmp = { 0 };
    oc_str8 path = oc_str8_from_buffer(req->size, req->buffer);

    if(!path.len)
    {
        cmp.error = OC_IO_ERR_ARG;
    }
    else if(!atSlot && !oc_file_is_nil(req->handle))
    {
        cmp.error = OC_IO_ERR_HANDLE;
    }
    else if(atSlot
            && (!(atSlot->rights & OC_FILE_ACCESS_READ) || !(atSlot->rights & OC_FILE_ACCESS_WRITE)))
    {
        cmp.error = OC_IO_ERR_PERM;
    }
    else
    {
        oc_arena_scope scratch = oc_scratch_begin();

        oc_file_desc rootFd = atSlot ? atSlot->fd : oc_file_desc_nil();

        oc_io_resolve_result resolve = oc_io_resolve(scratch.arena, rootFd, path, OC_FILE_OPEN_NONE);
        if(resolve.error)
        {
            cmp.error = resolve.error;
        }
        else
        {
            oc_file_status status = { 0 };
            cmp.error = oc_io_raw_fstat_at(resolve.fd, resolve.name, OC_FILE_SYMLINK_OPEN_LAST, &status);

            if(cmp.error == OC_IO_OK)
            {
                if(status.type == OC_FILE_DIRECTORY || status.type == OC_FILE_REGULAR)
                {
                    if(status.type == OC_FILE_DIRECTORY && !(req->removeFlags & OC_FILE_REMOVE_DIR))
                    {
                        cmp.error = OC_IO_ERR_DIR;
                    }
                    else
                    {
                        int flags = AT_SYMLINK_NOFOLLOW_ANY;
                        if(status.type == OC_FILE_DIRECTORY)
                        {
                            flags |= AT_REMOVEDIR;
                        }
                        int r = unlinkat(resolve.fd, resolve.name.ptr, flags);
                        if(r)
                        {
                            cmp.error = oc_io_raw_last_error();
                        }
                    }
                }
                else
                {
                    cmp.error = OC_IO_ERR_UNKNOWN; //TODO
                }
            }
            close(resolve.fd);
        }

        oc_scratch_end(scratch);
    }
    return cmp;
}

oc_file_list oc_file_listdir(oc_arena* arena, oc_file directory)
{
    return oc_file_listdir_for_table(arena, directory, &oc_globalFileTable);
}
