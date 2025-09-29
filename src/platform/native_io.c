/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "platform/platform_path.h"
#include "platform_io.c"
#include "native_io.h"

//------------------------------------------------------------------------
// File table and handles
//------------------------------------------------------------------------
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

#include "util/wrapped_types.h"

/*-----------------------------------------------------------------------
//NOTE: Path resolution

    Resolve rootFd and path to a parent directory fd and a file name,
    expanding symlinks and '..', and making sure the path doesn't escape
    rootFd.
-----------------------------------------------------------------------*/
typedef struct oc_io_resolve_result
{
    oc_io_error error;
    oc_file_desc fd;
    oc_str8 path;
    oc_str8 name;
} oc_io_resolve_result;

oc_io_resolve_result oc_io_resolve(oc_arena* arena, oc_file_desc rootFd, oc_str8 path, oc_file_resolve_flags resolveFlags)
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
                oc_io_resolve_result r = oc_io_resolve(scratch.arena, rootFd, normPath, OC_FILE_RESOLVE_SYMLINK_DONT_FOLLOW);
                if(r.error != OC_IO_OK)
                {
                    result.error = r.error;
                    break;
                }
                oc_fd_close(fd);
                fd = r.fd;

                oc_str8_list_pop_back(&normElements);
            }
        }
        else
        {
            //NOTE: now what we need to do depends on the type of file
            oc_fd_stat_result r = oc_fd_stat_at(fd, elt->string);

            oc_file_status status = oc_catch(r)
            {
                if(r.error != OC_IO_ERR_NO_ENTRY || !oc_str8_list_empty(pathElements))
                {
                    //NOTE: only the last element is allowed to be non-existent
                    result.error = r.error;
                }
                else
                {
                    result.name = oc_str8_push_copy(arena, elt->string);
                }
                break;
            }

            if(status.type == OC_FILE_SYMLINK)
            {
                if(!oc_list_empty(pathElements.list) && (resolveFlags & OC_FILE_RESOLVE_SYMLINK_DONT_FOLLOW))
                {
                    result.error = OC_IO_ERR_SYMLINK;
                    break;
                }
                else if((resolveFlags & OC_FILE_RESOLVE_SYMLINK_DONT_FOLLOW) || (resolveFlags & OC_FILE_RESOLVE_SYMLINK_OPEN_LAST))
                {
                    result.name = oc_str8_push_copy(arena, elt->string);
                    break;
                }

                oc_fd_read_link_result r = oc_fd_read_link_at(scratch.arena, fd, elt->string);
                oc_str8 target = oc_catch(r)
                {
                    result.error = r.error;
                    break;
                }

                if(oc_path_is_absolute(target))
                {
                    result.error = OC_IO_ERR_WALKOUT;
                    break;
                }

                oc_str8_list linkElements = oc_path_split(scratch.arena, target);

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
                    oc_fd_result openResult = oc_fd_open_at(fd, elt->string, OC_FILE_ACCESS_NONE, OC_FILE_OPEN_DEFAULT);
                    oc_file_desc newFd = oc_catch(openResult)
                    {
                        result.error = openResult.error;
                        break;
                    }
                    oc_fd_close(fd);
                    fd = newFd;

                    oc_fd_stat_result statResult = oc_fd_stat(fd);

                    oc_file_status newStatus = oc_catch(statResult)
                    {
                        result.error = statResult.error;
                        break;
                    }

                    if(status.type != OC_FILE_DIRECTORY)
                    {
                        //NOTE: if we got anything other than a directory here, the file changed
                        // under our feet, so we could be walking out root dir.
                        result.error = OC_IO_ERR_WALKOUT;
                        break;
                    }

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

/*-----------------------------------------------------------------------
 Platform-agnostic IO handlers

    These handlers do permission checking and path resolution (making
    sure the path doesn't escape the root directory). They call into raw
    IO primitives (implemented in posix_io.c, win32_io.c, etc) to do the
    actual IO operations.
-----------------------------------------------------------------------*/

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

                oc_io_resolve_result resolve = oc_io_resolve(scratch.arena, rootFd, path, req->resolveFlags);
                if(resolve.error != OC_IO_OK)
                {
                    slot->error = resolve.error;
                }
                else
                {
                    //NOTE: here, we have an fd to the second-to-last element of the path. We can open the last element
                    // with the requested access rights and creation flags

                    oc_fd_result res = oc_fd_open_at(resolve.fd, resolve.name, req->open.rights, req->open.flags);
                    slot->fd = oc_catch(res)
                    {
                        slot->error = res.error;
                    }
                    oc_fd_close(resolve.fd);
                }
                oc_scratch_end(scratch);
            }
        }

        if(slot->error)
        {
            slot->fatal = true;
            cmp.error = slot->error;
        }
    }
    return (cmp);
}

oc_io_cmp oc_io_close(oc_file_slot* slot, oc_io_req* req, oc_file_table* table)
{
    oc_io_cmp cmp = { 0 };
    if(!oc_file_desc_is_nil(slot->fd))
    {
        cmp.error = oc_fd_close(slot->fd);
    }
    oc_file_slot_recycle(table, slot);
    return (cmp);
}

oc_io_cmp oc_io_get_error(oc_file_slot* slot, oc_io_req* req)
{
    oc_io_cmp cmp = { 0 };
    cmp.result = slot->error;
    return (cmp);
}

oc_io_cmp oc_io_fstat(oc_file_slot* slot, oc_io_req* req)
{
    oc_io_cmp cmp = { 0 };

    if(req->size < sizeof(oc_file_status))
    {
        cmp.error = OC_IO_ERR_ARG;
    }
    else
    {
        oc_fd_stat_result r = oc_fd_stat(slot->fd);
        if(oc_check(r))
        {
            oc_file_status status = r.value;
            memcpy(req->buffer, &status, sizeof(status));
        }
        else
        {
            cmp.error = r.error;
        }
    }
    return (cmp);
}

oc_io_cmp oc_io_seek(oc_file_slot* slot, oc_io_req* req)
{
    oc_io_cmp cmp = { 0 };
    oc_fd_seek_result r = oc_fd_seek(slot->fd, req->offset, req->whence);

    cmp.result = oc_catch(r)
    {
        slot->error = cmp.error = r.error;
    }
    return (cmp);
}

oc_io_cmp oc_io_read(oc_file_slot* slot, oc_io_req* req)
{
    oc_io_cmp cmp = { 0 };

    oc_fd_readwrite_result r = oc_fd_read(slot->fd, req->size, req->buffer);
    cmp.result = oc_catch(r)
    {
        slot->error = cmp.error = r.error;
    }

    return (cmp);
}

oc_io_cmp oc_io_write(oc_file_slot* slot, oc_io_req* req)
{
    oc_io_cmp cmp = { 0 };

    oc_fd_readwrite_result r = oc_fd_write(slot->fd, req->size, req->buffer);
    cmp.result = oc_catch(r)
    {
        slot->error = cmp.error = r.error;
    }

    return (cmp);
}

//TODO: remove
static oc_io_error oc_fd_convert_errno();

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
        slot->rights = OC_FILE_ACCESS_READ | OC_FILE_ACCESS_WRITE;
        cmp.handle = oc_file_from_slot(table, slot);

        oc_fd_result res = oc_fd_maketmp(req->makeTmpFlags);
        slot->fd = oc_catch(res)
        {
            slot->error = res.error;
        }
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
                    cmp.error = subCmp.error;
                }
            }
        }
        else
        {
            oc_io_resolve_result resolve = oc_io_resolve(scratch.arena, rootFd, path, req->resolveFlags);
            if(resolve.error)
            {
                cmp.error = resolve.error;
            }
            else
            {
                cmp.error = oc_fd_makedir_at(resolve.fd, resolve.name);
                if(cmp.error == OC_IO_ERR_EXISTS && (req->makeDirFlags & OC_FILE_MAKEDIR_IGNORE_EXISTING))
                {
                    cmp.error = OC_IO_OK;
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

        oc_io_resolve_result resolve = oc_io_resolve(scratch.arena, rootFd, path, OC_FILE_RESOLVE_SYMLINK_OPEN_LAST);
        if(resolve.error)
        {
            cmp.error = resolve.error;
        }
        else
        {
            cmp.error = oc_fd_remove(resolve.fd, resolve.name, req->removeFlags);
            close(resolve.fd);
        }

        oc_scratch_end(scratch);
    }
    return cmp;
}

//-----------------------------------------------------------------------
// IO Dispatch
//-----------------------------------------------------------------------

oc_file_list oc_file_listdir(oc_arena* arena, oc_file directory)
{
    return oc_file_listdir_for_table(arena, directory, &oc_globalFileTable);
}

oc_io_cmp oc_io_wait_single_req_for_table(oc_io_req* req, oc_file_table* table)
{
    oc_io_cmp cmp = { 0 };

    oc_file_slot* slot = oc_file_slot_from_handle(table, req->handle);
    if(!slot)
    {
        //TODO: clarify this. We need to skip open at here so that it
        //  returns a valid handle with an error on it...
        if(req->op != OC_IO_OPEN_AT && req->op != OC_IO_MAKE_TMP)
        {
            if(!oc_file_is_nil(req->handle)
               || (req->op != OC_IO_MAKE_DIR
                   && req->op != OC_IO_REMOVE))
            {
                cmp.error = OC_IO_ERR_HANDLE;
            }
        }
    }
    else if(slot->fatal
            && req->op != OC_IO_CLOSE
            && req->op != OC_OC_IO_ERROR)
    {
        cmp.error = OC_IO_ERR_PREV;
    }

    if(cmp.error == OC_IO_OK)
    {
        switch(req->op)
        {
            case OC_IO_OPEN_AT:
                cmp = oc_io_open_at(slot, req, table);
                break;

            case OC_IO_FSTAT:
                cmp = oc_io_fstat(slot, req);
                break;

            case OC_IO_CLOSE:
                cmp = oc_io_close(slot, req, table);
                break;

            case OC_IO_READ:
                cmp = oc_io_read(slot, req);
                break;

            case OC_IO_WRITE:
                cmp = oc_io_write(slot, req);
                break;

            case OC_IO_SEEK:
                cmp = oc_io_seek(slot, req);
                break;

            case OC_IO_MAKE_TMP:
                cmp = oc_io_maketmp(req, table);
                break;

            case OC_IO_MAKE_DIR:
                cmp = oc_io_makedir(slot, req);
                break;

            case OC_IO_REMOVE:
                cmp = oc_io_remove(slot, req);
                break;

            case OC_OC_IO_ERROR:
                cmp = oc_io_get_error(slot, req);
                break;

            default:
                cmp.error = OC_IO_ERR_OP;
                break;
        }
    }
    return (cmp);
}
