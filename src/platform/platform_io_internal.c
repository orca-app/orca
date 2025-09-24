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

typedef struct oc_io_open_restrict_context
{
    oc_io_error error;
    u64 rootUID;
    oc_file_desc rootFd;
    oc_file_desc fd;

} oc_io_open_restrict_context;

oc_io_error oc_io_open_restrict_enter(oc_io_open_restrict_context* context, oc_str8 name, oc_file_access accessRights, oc_file_open_flags openFlags)
{
    oc_file_desc nextFd = oc_io_raw_open_at(context->fd, name, accessRights, openFlags);
    if(oc_file_desc_is_nil(nextFd))
    {
        context->error = oc_io_raw_last_error();
    }
    else
    {
        if(context->fd != context->rootFd)
        {
            oc_io_raw_close(context->fd);
        }
        context->fd = nextFd;
    }
    return (context->error);
}

typedef struct oc_io_open_restrict_result
{
    oc_io_error error;
    oc_file_desc fd;
} oc_io_open_restrict_result;

oc_io_open_restrict_result oc_io_open_restrict(oc_file_desc dirFd, oc_str8 path, oc_file_access accessRights, oc_file_open_flags openFlags)
{
    oc_arena_scope scratch = oc_scratch_begin();

    oc_str8_list pathElements = oc_path_split(scratch.arena, path);

    oc_io_open_restrict_context context = {
        .error = OC_IO_OK,
        .rootFd = dirFd,
        .fd = dirFd,
    };

    if(oc_file_desc_is_nil(dirFd))
    {
        context.error = OC_IO_ERR_HANDLE;
    }
    else
    {
        oc_file_status status;
        context.error = oc_io_raw_fstat(dirFd, &status);
        context.rootUID = status.uid;
    }

    if(context.error == OC_IO_OK)
    {
        oc_list_for(pathElements.list, elt, oc_str8_elt, listElt)
        {
            oc_str8 name = elt->string;
            oc_file_access eltAccessRights = OC_FILE_ACCESS_READ;
            oc_file_open_flags eltOpenFlags = 0;

            bool atLastElement = (&elt->listElt == oc_list_last(pathElements.list));
            if(atLastElement)
            {
                eltAccessRights = accessRights;
                eltOpenFlags = openFlags;
            }

            if(!oc_str8_cmp(name, OC_STR8("."))
               && !atLastElement)
            {
                //NOTE: if we're not at the last element we can just skip '.' elements
                continue;
            }
            else if(!oc_str8_cmp(name, OC_STR8("..")))
            {
                //NOTE: check that we don't escape root dir
                oc_file_status status;
                context.error = oc_io_raw_fstat(context.fd, &status);
                if(context.error)
                {
                    break;
                }
                else if(status.uid == context.rootUID)
                {
                    context.error = OC_IO_ERR_WALKOUT;
                    break;
                }
            }
            else if(!oc_io_raw_file_exists_at(context.fd, name, OC_FILE_OPEN_SYMLINK))
            {
                //NOTE: if the file doesn't exists, but we're at the last element and OC_FILE_OPEN_CREATE
                //      is set, we create the file. Otherwise it is a OC_IO_ERROR_NO_ENTRY error.
                if(!atLastElement
                   || !(openFlags & OC_FILE_OPEN_CREATE))
                {
                    context.error = OC_IO_ERR_NO_ENTRY;
                    break;
                }
            }
            else
            {
                //NOTE: if the file exists, we check the type of file
                oc_file_status status = { 0 };
                context.error = oc_io_raw_fstat_at(context.fd, name, OC_FILE_OPEN_SYMLINK, &status);
                if(context.error)
                {
                    break;
                }

                if(status.type == OC_FILE_REGULAR)
                {
                    if(!atLastElement)
                    {
                        context.error = OC_IO_ERR_NOT_DIR;
                        break;
                    }
                }
                else if(status.type == OC_FILE_SYMLINK)
                {
                    //TODO  - do we need a OC_FILE_OPEN_NO_FOLLOW that fails if last element is a symlink?
                    //      - do we need a OC_FILE_OPEN_NO_SYMLINKS that fails if _any_ element is a symlink?

                    if(!atLastElement
                       || !(openFlags & OC_FILE_OPEN_SYMLINK))
                    {
                        oc_io_raw_read_link_result link = oc_io_raw_read_link_at(scratch.arena, context.fd, name);
                        if(link.error)
                        {
                            context.error = link.error;
                            break;
                        }
                        if(link.target.len == 0)
                        {
                            //NOTE: treat an empty target as a '.'
                            link.target = OC_STR8(".");
                        }
                        else if(oc_path_is_absolute(link.target))
                        {
                            context.error = OC_IO_ERR_WALKOUT;
                            break;
                        }

                        oc_str8_list linkElements = oc_path_split(scratch.arena, link.target);

                        if(!oc_list_empty(linkElements.list))
                        {
                            //NOTE: insert linkElements into pathElements after elt
                            oc_list_elt* tmp = elt->listElt.next;
                            elt->listElt.next = linkElements.list.first;
                            linkElements.list.last->next = tmp;
                            if(!tmp)
                            {
                                pathElements.list.last = linkElements.list.last;
                            }
                        }
                        continue;
                    }
                }
                else if(status.type != OC_FILE_DIRECTORY)
                {
                    context.error = OC_IO_ERR_NOT_DIR;
                    break;
                }
            }

            //NOTE: if we arrive here, we have no errors and the correct flags are set,
            //      so we can enter the element
            OC_DEBUG_ASSERT(context.error == OC_IO_OK);
            oc_io_open_restrict_enter(&context, name, eltAccessRights, eltOpenFlags);
        }
    }

    if(context.error && !oc_file_desc_is_nil(context.fd))
    {
        if(context.fd != context.rootFd)
        {
            oc_io_raw_close(context.fd);
        }
        context.fd = oc_file_desc_nil();
    }

    oc_io_open_restrict_result result = {
        .error = context.error,
        .fd = context.fd
    };

    oc_scratch_end(scratch);
    return (result);
}

oc_io_cmp oc_io_open_at(oc_file_slot* atSlot, oc_io_req* req, oc_file_table* table)
{
    oc_io_cmp cmp = { 0 };

    oc_file_slot* slot = oc_file_slot_alloc(table);
    if(!slot)
    {
        cmp.error = OC_IO_ERR_MAX_FILES;
        cmp.result = 0;
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
            slot->error = OC_IO_ERR_HANDLE;
        }
        else
        {
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
                oc_file_desc dirFd = atSlot ? atSlot->fd : oc_file_desc_nil();

                if(req->open.flags & OC_FILE_OPEN_RESTRICT)
                {
                    oc_io_open_restrict_result res = oc_io_open_restrict(dirFd, path, slot->rights, req->open.flags);
                    slot->error = res.error;
                    slot->fd = res.fd;
                }
                else
                {
                    slot->fd = oc_io_raw_open_at(dirFd, path, slot->rights, req->open.flags);
                    if(oc_file_desc_is_nil(slot->fd))
                    {
                        slot->error = oc_io_raw_last_error();
                    }
                }
            }
        }

        if(slot->error == OC_IO_OK)
        {
            oc_file_status status;
            slot->error = oc_io_raw_fstat(slot->fd, &status);
            if(slot->error == OC_IO_OK)
            {
                slot->type = status.type;
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

#include <sys/stat.h>

oc_io_cmp oc_io_makedir(oc_file_slot* atSlot, oc_io_req* req)
{
    oc_io_cmp cmp = { 0 };

    oc_str8 path = oc_str8_from_buffer(req->size, req->buffer);

    if(!atSlot && !oc_file_is_nil(req->handle))
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

        //NOTE: by defaut the root fd is set to AT_FDCWD (if path is absolute, this will be ignored)
        // If the call has a root handle, set the root fd to it and make the path relative. Then
        // check that this relative path doesn't escape its parent
        int atFd = AT_FDCWD;
        if(atSlot)
        {
            atFd = atSlot->fd;
            if(path.len && path.ptr[0] == '/')
            {
                path = oc_str8_slice(path, 1, path.len);
            }
        }

        //NOTE: normalize the path and detect escapes from parent
        oc_str8_list pathElements = oc_path_split(scratch.arena, path);
        oc_str8_list normElements = { 0 };
        bool escape = false;

        oc_str8_list_for(pathElements, elt)
        {
            if(!oc_str8_cmp(elt->string, OC_STR8(".")))
            {
                // ignore
            }
            else if(!oc_str8_cmp(elt->string, OC_STR8("..")))
            {
                /*NOTE: .. goes up one directory.
                - If there's an element that isn't / or .., pop it.
                - If there's no last element, or the last element is a .., push ..
                */
                oc_str8_elt* last = oc_list_last_entry(normElements.list, oc_str8_elt, listElt);
                if(!last || !oc_str8_cmp(last->string, OC_STR8("..")) || !oc_str8_cmp(last->string, OC_STR8("/")))
                {
                    oc_str8_list_push(scratch.arena, &normElements, elt->string);
                    escape = true;
                }
                else
                {
                    oc_str8_list_pop_back(&normElements);
                }
            }
            else
            {
                oc_str8_list_push(scratch.arena, &normElements, elt->string);
            }
        }

        if(atSlot && escape)
        {
            cmp.error = OC_IO_ERR_WALKOUT;
        }
        else
        {
            path = oc_path_join(scratch.arena, normElements);

            if(req->makeDirFlags & OC_FILE_MAKEDIR_CREATE_PARENTS) //TODO maybe this should be handled in higher level function...
            {
                oc_str8 accPath = { 0 };
                oc_str8_list_for(normElements, elt)
                {
                    accPath = oc_path_append(scratch.arena, accPath, elt->string);

                    const char* accPathCStr = oc_str8_to_cstring(scratch.arena, accPath);
                    struct stat st;
                    int r = fstatat(atFd, accPathCStr, &st, 0);
                    if(r)
                    {
                        if(errno == ENOENT)
                        {
                            //NOTE: directory does not exist, create it.
                            mkdirat(atFd, accPathCStr, 0700);
                        }
                        else
                        {
                            cmp.error = oc_io_raw_last_error();
                            break;
                        }
                    }
                    else if(!(st.st_mode & S_IFDIR))
                    {
                        //NOTE: file exists but is not a directory
                        cmp.error = OC_IO_ERR_NOT_DIR;
                        break;
                    }
                    else if(&elt->listElt == normElements.list.last && !(req->makeDirFlags & OC_FILE_MAKEDIR_IGNORE_EXISTING))
                    {
                        cmp.error = OC_IO_ERR_EXISTS;
                        break;
                    }
                }
            }
            else
            {
                char* pathCStr = oc_str8_to_cstring(scratch.arena, path);

                int res = mkdirat(atFd, pathCStr, 0700); //TODO: pass mode
                if(res)
                {
                    if(errno != EEXIST || !(req->makeDirFlags & OC_FILE_MAKEDIR_IGNORE_EXISTING))
                    {
                        cmp.error = oc_io_raw_last_error();
                    }
                }
            }
        }
        oc_scratch_end(scratch);
    }
    return cmp;
}

oc_io_cmp oc_io_makedir(oc_file_slot* atSlot, oc_io_req* req)
{
}

oc_file_list oc_file_listdir(oc_arena* arena, oc_file directory)
{
    return oc_file_listdir_for_table(arena, directory, &oc_globalFileTable);
}
