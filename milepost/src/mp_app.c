/************************************************************//**
*
*	@file: mp_app_internal.c
*	@author: Martin Fouilleul
*	@date: 23/12/2022
*	@revision:
*
*****************************************************************/
#include"platform_assert.h"
#include"mp_app_internal.h"

mp_app __mpApp = {0};

//---------------------------------------------------------------
// Window handles
//---------------------------------------------------------------

void mp_init_window_handles()
{
	list_init(&__mpApp.windowFreeList);
	for(int i=0; i<MP_APP_MAX_WINDOWS; i++)
	{
		__mpApp.windowPool[i].generation = 1;
		list_append(&__mpApp.windowFreeList, &__mpApp.windowPool[i].freeListElt);
	}
}

bool mp_window_handle_is_null(mp_window window)
{
	return(window.h == 0);
}

mp_window mp_window_null_handle()
{
	return((mp_window){.h = 0});
}

mp_window_data* mp_window_alloc()
{
	return(list_pop_entry(&__mpApp.windowFreeList, mp_window_data, freeListElt));
}

mp_window_data* mp_window_ptr_from_handle(mp_window handle)
{
	u32 index = handle.h>>32;
	u32 generation = handle.h & 0xffffffff;
	if(index >= MP_APP_MAX_WINDOWS)
	{
		return(0);
	}
	mp_window_data* window = &__mpApp.windowPool[index];
	if(window->generation != generation)
	{
		return(0);
	}
	else
	{
		return(window);
	}
}

mp_window mp_window_handle_from_ptr(mp_window_data* window)
{
	DEBUG_ASSERT(  (window - __mpApp.windowPool) >= 0
	            && (window - __mpApp.windowPool) < MP_APP_MAX_WINDOWS);

	u64 h = ((u64)(window - __mpApp.windowPool))<<32
	      | ((u64)window->generation);

	return((mp_window){h});
}

void mp_window_recycle_ptr(mp_window_data* window)
{
	window->generation++;
	list_push(&__mpApp.windowFreeList, &window->freeListElt);
}

//---------------------------------------------------------------
// Init
//---------------------------------------------------------------

static void mp_init_common()
{
	mp_init_window_handles();
	ringbuffer_init(&__mpApp.eventQueue, 16);
}

static void mp_terminate_common()
{
	ringbuffer_cleanup(&__mpApp.eventQueue);
}

//---------------------------------------------------------------
// Event handling
//---------------------------------------------------------------

void mp_queue_event(mp_event* event)
{
	ringbuffer* queue = &__mpApp.eventQueue;

	if(ringbuffer_write_available(queue) < sizeof(mp_event))
	{
		log_error("event queue full\n");
	}
	else
	{
		bool error = false;
		ringbuffer_reserve(queue, sizeof(mp_event), (u8*)event);

		if(event->type == MP_EVENT_PATHDROP)
		{
			for_list(&event->paths.list, elt, str8_elt, listElt)
			{
				str8* path = &elt->string;
				if(ringbuffer_write_available(queue) < (sizeof(u64) + path->len))
				{
					log_error("event queue full\n");
					error = true;
					break;
				}
				else
				{
					ringbuffer_reserve(queue, sizeof(u64), (u8*)&path->len);
					ringbuffer_reserve(queue, path->len, (u8*)path->ptr);
				}
			}
		}
		if(error)
		{
			ringbuffer_rewind(queue);
		}
		else
		{
			ringbuffer_commit(queue);
		}
	}
}

mp_event* mp_next_event(mem_arena* arena)
{
	//NOTE: pop and return event from queue
	mp_event* event = 0;
	ringbuffer* queue = &__mpApp.eventQueue;

	if(ringbuffer_read_available(queue) >= sizeof(mp_event))
	{
		event = mem_arena_alloc_type(arena, mp_event);
		u64 read = ringbuffer_read(queue, sizeof(mp_event), (u8*)event);
		DEBUG_ASSERT(read == sizeof(mp_event));

		if(event->type == MP_EVENT_PATHDROP)
		{
			u64 pathCount = event->paths.eltCount;
			event->paths = (str8_list){0};

			for(int i=0; i<pathCount; i++)
			{
				if(ringbuffer_read_available(queue) < sizeof(u64))
				{
					log_error("malformed path payload: no string size\n");
					break;
				}

				u64 len = 0;
				ringbuffer_read(queue, sizeof(u64), (u8*)&len);
				if(ringbuffer_read_available(queue) < len)
				{
					log_error("malformed path payload: string shorter than expected\n");
					break;
				}

				char* buffer = mem_arena_alloc_array(arena, char, len);
				ringbuffer_read(queue, len, (u8*)buffer);

				str8_list_push(arena, &event->paths, str8_from_buffer(len, buffer));
			}
		}
	}
	return(event);
}

//---------------------------------------------------------------
// window rects helpers
//---------------------------------------------------------------

void mp_window_set_content_position(mp_window window, vec2 position)
{
	mp_rect rect = mp_window_get_content_rect(window);
	rect.x = position.x;
	rect.y = position.y;
	mp_window_set_content_rect(window, rect);
}

void mp_window_set_content_size(mp_window window, vec2 size)
{
	mp_rect rect = mp_window_get_content_rect(window);
	rect.w = size.x;
	rect.h = size.y;
	mp_window_set_content_rect(window, rect);
}

void mp_window_set_frame_position(mp_window window, vec2 position)
{
	mp_rect frame = mp_window_get_frame_rect(window);
	frame.x = position.x;
	frame.y = position.y;
	mp_window_set_frame_rect(window, frame);
}

void mp_window_set_frame_size(mp_window window, vec2 size)
{
	mp_rect frame = mp_window_get_frame_rect(window);
	frame.w = size.x;
	frame.h = size.y;
	mp_window_set_frame_rect(window, frame);
}
