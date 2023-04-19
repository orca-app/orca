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
	if(ringbuffer_write_available(&__mpApp.eventQueue) < sizeof(mp_event))
	{
		log_error("event queue full\n");
	}
	else
	{
		u32 written = ringbuffer_write(&__mpApp.eventQueue, sizeof(mp_event), (u8*)event);
		DEBUG_ASSERT(written == sizeof(mp_event));
	}
}

bool mp_next_event(mp_event* event)
{
	//NOTE pop and return event from queue
	if(ringbuffer_read_available(&__mpApp.eventQueue) >= sizeof(mp_event))
	{
		u64 read = ringbuffer_read(&__mpApp.eventQueue, sizeof(mp_event), (u8*)event);
		DEBUG_ASSERT(read == sizeof(mp_event));
		return(true);
	}
	else
	{
		return(false);
	}
}
