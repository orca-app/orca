/************************************************************//**
*
*	@file: platform_io_internal.c
*	@author: Martin Fouilleul
*	@date: 11/06/2023
*
*****************************************************************/

#include"platform_io_internal.h"
#include"platform_path.h"

file_table __globalFileTable = {0};

file_slot* file_slot_alloc(file_table* table)
{
	file_slot* slot = list_pop_entry(&table->freeList, file_slot, freeListElt);
	if(!slot && table->nextSlot < ORCA_MAX_FILE_SLOTS)
	{
		slot = &table->slots[table->nextSlot];
		slot->generation = 1;
		table->nextSlot++;
	}

	u32 tmpGeneration = slot->generation;
	memset(slot, 0, sizeof(file_slot));
	slot->generation = tmpGeneration;

	return(slot);
}

void file_slot_recycle(file_table* table, file_slot* slot)
{
	slot->generation++;
	list_push(&table->freeList, &slot->freeListElt);
}

file_handle file_handle_from_slot(file_table* table, file_slot* slot)
{
	u64 index = slot - table->slots;
	u64 generation = slot->generation;
	file_handle handle = {.h = (generation<<32) | index };
	return(handle);
}

file_slot* file_slot_from_handle(file_table* table, file_handle handle)
{
	file_slot* slot = 0;

	u64 index = handle.h & 0xffffffff;
	u64 generation = handle.h>>32;

	if(index < table->nextSlot)
	{
		file_slot* candidate = &table->slots[index];
		if(candidate->generation == generation)
		{
			slot = candidate;
		}
	}
	return(slot);
}

io_cmp io_wait_single_req(io_req* req)
{
	return(io_wait_single_req_with_table(req, &__globalFileTable));
}

//-----------------------------------------------------------------------
// io common primitives
//-----------------------------------------------------------------------

typedef struct io_open_restrict_context
{
	io_error error;
	u64 rootUID;
	io_file_desc rootFd;
	io_file_desc fd;

} io_open_restrict_context;

io_error io_open_restrict_enter(io_open_restrict_context* context, str8 name, file_access_rights accessRights, file_open_flags openFlags)
{
	io_file_desc nextFd = io_raw_open_at(context->fd, name, accessRights, openFlags);
	if(io_file_desc_is_nil(nextFd))
	{
		context->error = io_raw_last_error();
	}
	else
	{
		if(context->fd != context->rootFd)
		{
			io_raw_close(context->fd);
		}
		context->fd = nextFd;
	}
	return(context->error);
}

typedef struct io_open_restrict_result
{
	io_error error;
	io_file_desc fd;
} io_open_restrict_result;


io_open_restrict_result io_open_restrict(io_file_desc dirFd, str8 path, file_access_rights accessRights, file_open_flags openFlags)
{
	mem_arena_scope scratch = mem_scratch_begin();

	str8_list sep = {0};
	str8_list_push(scratch.arena, &sep, STR8("/"));
	str8_list pathElements = str8_split(scratch.arena, path, sep);

	io_open_restrict_context context = {
		.error = IO_OK,
		.rootFd = dirFd,
		.fd = dirFd,
	};

	file_status status;
	context.error = io_raw_fstat(dirFd, &status);
	context.rootUID = status.uid;

	for_list(&pathElements.list, elt, str8_elt, listElt)
	{
		str8 name = elt->string;

		/*TODO:
			The last element should be treated a bit differently:

			- if it doesn't exists, and FILE_OPEN_CREATE is set, we can create it
			- it must be opened with the correct flags
		*/

		if(!str8_cmp(name, STR8(".")))
		{
			if(&elt->listElt == list_last(&pathElements.list))
			{
				//NOTE: if we're at the last element, re-enter current directory with correct flags
				io_open_restrict_enter(&context, name, accessRights, openFlags);
			}
			else
			{
				//NOTE: else we can just skip the element
			}
		}
		else if(!str8_cmp(name, STR8("..")))
		{
			//NOTE: check that we don't escape root dir
			file_status status;
			context.error = io_raw_fstat(context.fd, &status);
			if(context.error)
			{
				break;
			}
			else if(status.uid == context.rootUID)
			{
				context.error = IO_ERR_WALKOUT;
				break;
			}
			else if(&elt->listElt == list_last(&pathElements.list))
			{
				//NOTE: if we're at the last element, enter parent directory with correct flags
				io_open_restrict_enter(&context, name, accessRights, openFlags);
			}
			else
			{
				io_open_restrict_enter(&context, name, FILE_ACCESS_READ, 0);
			}
		}
		else
		{
			if(!io_raw_file_exists_at(context.fd, name, FILE_OPEN_SYMLINK))
			{
				//NOTE: if the file doesn't exists, but we're at the last element and FILE_OPEN_CREATE
				//      is set, we create the file. Otherwise it is a IO_ERROR_NO_ENTRY error.
				if( (&elt->listElt == list_last(&pathElements.list))
				  &&(openFlags & FILE_OPEN_CREATE))
				{
					io_open_restrict_enter(&context, name, accessRights, openFlags);
				}
				else
				{
					context.error = IO_ERR_NO_ENTRY;
					break;
				}
			}
			else
			{
				//NOTE: if the file exists, we check the type of file
				file_status status = {0};
				context.error = io_raw_fstat_at(context.fd, name, FILE_OPEN_SYMLINK, &status);
				if(context.error)
				{
					break;
				}

				if(status.type == MP_FILE_SYMLINK)
				{
					//TODO  - do we need a FILE_OPEN_NO_FOLLOW that fails if last element is a symlink?
					//      - do we need a FILE_OPEN_NO_SYMLINKS that fails if _any_ element is a symlink?

					if( (&elt->listElt == list_last(&pathElements.list))
					  &&(openFlags & FILE_OPEN_SYMLINK))
					{
						io_open_restrict_enter(&context, name, accessRights, openFlags);
					}
					else
					{
						io_raw_read_link_result link = io_raw_read_link_at(scratch.arena, context.fd, name);
						if(link.error)
						{
							context.error = link.error;
							break;
						}

						if(path_is_absolute(link.target))
						{
							context.error = IO_ERR_WALKOUT;
							break;
						}
						else
						{
							if(link.target.len == 0)
							{
								//NOTE: treat an empty target as a '.'
								link.target = STR8(".");
							}

							str8_list linkElements = str8_split(scratch.arena, link.target, sep);
							if(!list_empty(&linkElements.list))
							{
								//NOTE: insert linkElements into pathElements after elt
								list_elt* tmp = elt->listElt.next;
								elt->listElt.next = linkElements.list.first;
								linkElements.list.last->next = tmp;
								if(!tmp)
								{
									pathElements.list.last = linkElements.list.last;
								}
							}
						}
					}
				}
				else if(status.type == MP_FILE_DIRECTORY)
				{
					//NOTE: descend in directory
					if(&elt->listElt == list_last(&pathElements.list))
					{
						io_open_restrict_enter(&context, name, accessRights, openFlags);
					}
					else
					{
						io_open_restrict_enter(&context, name, FILE_ACCESS_READ, 0);
					}
				}
				else if(status.type == MP_FILE_REGULAR)
				{
					//NOTE: check that we're at the end of path and open that file
					if(&elt->listElt != list_last(&pathElements.list))
					{
						context.error = IO_ERR_NOT_DIR;
						break;
					}
					else
					{
						io_open_restrict_enter(&context, name, accessRights, openFlags);
					}
				}
				else
				{
					context.error = IO_ERR_NO_ENTRY;
				}
			}
		}
	}

	if(context.error)
	{
		if(context.fd != context.rootFd)
		{
			io_raw_close(context.fd);
			context.fd = io_file_desc_nil();
		}
	}

	io_open_restrict_result result = {
		.error = context.error,
		.fd = context.fd
	};

	mem_scratch_end(scratch);
	return(result);
}

io_cmp io_open_at(file_slot* atSlot, io_req* req, file_table* table)
{
	io_cmp cmp = {0};

	file_slot* slot = file_slot_alloc(table);
	if(!slot)
	{
		cmp.error = IO_ERR_MAX_FILES;
		cmp.result = 0;
	}
	else
	{
		slot->fd = -1;

		cmp.handle = file_handle_from_slot(table, slot);

		slot->rights = req->open.rights;
		if(atSlot)
		{
			slot->rights &= atSlot->rights;
		}

		if(slot->rights != req->open.rights)
		{
			slot->error = IO_ERR_PERM;
			slot->fatal = true;
		}
		else
		{
			str8 path = str8_from_buffer(req->size, req->buffer);

			io_file_desc dirFd = atSlot ? atSlot->fd : io_file_desc_nil();
			slot->fd = -1;

			if(req->open.flags & FILE_OPEN_RESTRICT)
			{
				io_open_restrict_result res = io_open_restrict(dirFd, path, slot->rights, req->open.flags);
				if(res.error == IO_OK)
				{
					slot->fd = res.fd;
				}
				else
				{
					slot->fatal = true;
					slot->error = res.error;
				}
			}
			else
			{
				slot->fd = io_raw_open_at(dirFd, path, slot->rights, req->open.flags);
				if(slot->fd < 0)
				{
					slot->fatal = true;
					slot->error = io_raw_last_error();
				}
			}
		}
		cmp.error = slot->error;
	}
	return(cmp);
}
