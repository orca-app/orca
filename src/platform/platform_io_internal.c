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
	str8_list_push(scratch.arena, &sep, STR8("\\"));
	str8_list pathElements = str8_split(scratch.arena, path, sep);

	io_open_restrict_context context = {
		.error = IO_OK,
		.rootFd = dirFd,
		.fd = dirFd,
	};

	if(io_file_desc_is_nil(dirFd))
	{
		context.error = IO_ERR_HANDLE;
	}
	else
	{
		file_status status;
		context.error = io_raw_fstat(dirFd, &status);
		context.rootUID = status.uid;
	}

	if(context.error == IO_OK)
	{
		for_list(&pathElements.list, elt, str8_elt, listElt)
		{
			str8 name = elt->string;
			file_access_rights eltAccessRights = FILE_ACCESS_READ;
			file_open_flags eltOpenFlags = 0;

			bool atLastElement = (&elt->listElt == list_last(&pathElements.list));
			if(atLastElement)
			{
				eltAccessRights = accessRights;
				eltOpenFlags = openFlags;
			}

			if(  !str8_cmp(name, STR8("."))
			  && !atLastElement)
			{
				//NOTE: if we're not at the last element we can just skip '.' elements
				continue;
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
			}
			else if(!io_raw_file_exists_at(context.fd, name, FILE_OPEN_SYMLINK))
			{
				//NOTE: if the file doesn't exists, but we're at the last element and FILE_OPEN_CREATE
				//      is set, we create the file. Otherwise it is a IO_ERROR_NO_ENTRY error.
				if(  !atLastElement
			  	|| !(openFlags & FILE_OPEN_CREATE))
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

				if(status.type == MP_FILE_REGULAR)
				{
					if(!atLastElement)
					{
						context.error = IO_ERR_NOT_DIR;
						break;
					}
				}
				else if(status.type == MP_FILE_SYMLINK)
				{
					//TODO  - do we need a FILE_OPEN_NO_FOLLOW that fails if last element is a symlink?
					//      - do we need a FILE_OPEN_NO_SYMLINKS that fails if _any_ element is a symlink?

					if(  !atLastElement
				  	|| !(openFlags & FILE_OPEN_SYMLINK))
					{
						io_raw_read_link_result link = io_raw_read_link_at(scratch.arena, context.fd, name);
						if(link.error)
						{
							context.error = link.error;
							break;
						}
						if(link.target.len == 0)
						{
							//NOTE: treat an empty target as a '.'
							link.target = STR8(".");
						}
						else if(path_is_absolute(link.target))
						{
							context.error = IO_ERR_WALKOUT;
							break;
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
						continue;
					}
				}
				else if(status.type != MP_FILE_DIRECTORY)
				{
					context.error = IO_ERR_NOT_DIR;
					break;
				}
			}

			//NOTE: if we arrive here, we have no errors and the correct flags are set,
			//      so we can enter the element
			DEBUG_ASSERT(context.error == IO_OK);
			io_open_restrict_enter(&context, name, eltAccessRights, eltOpenFlags);
		}
	}

	if(context.error && !io_file_desc_is_nil(context.fd))
	{
		if(context.fd != context.rootFd)
		{
			io_raw_close(context.fd);
		}
		context.fd = io_file_desc_nil();
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
		slot->fd = io_file_desc_nil();
		cmp.handle = file_handle_from_slot(table, slot);


		str8 path = str8_from_buffer(req->size, req->buffer);

		if(!path.len)
		{
			slot->error = IO_ERR_ARG;
		}
		else if(!atSlot && !file_handle_is_nil(req->handle))
		{
			slot->error = IO_ERR_HANDLE;
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
				slot->error = IO_ERR_PERM;
			}
			else
			{
				io_file_desc dirFd = atSlot ? atSlot->fd : io_file_desc_nil();

				if(req->open.flags & FILE_OPEN_RESTRICT)
				{
					io_open_restrict_result res = io_open_restrict(dirFd, path, slot->rights, req->open.flags);
					slot->error = res.error;
					slot->fd = res.fd;
				}
				else
				{
					slot->fd = io_raw_open_at(dirFd, path, slot->rights, req->open.flags);
					if(io_file_desc_is_nil(slot->fd))
					{
						slot->error = io_raw_last_error();
					}
				}
			}
		}

		if(slot->error == IO_OK)
		{
			file_status status;
			slot->error = io_raw_fstat(slot->fd, &status);
			if(slot->error == IO_OK)
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
	return(cmp);
}
