/************************************************************//**
*
*	@file: win32_app.c
*	@author: Martin Fouilleul
*	@date: 16/12/2022
*	@revision:
*
*****************************************************************/

#define WIN32_LEAN_AND_MEAN
#include<windows.h>

#include"win32_app.h"
#include"ringbuffer.h"

#define LOG_SUBSYSTEM "Application"


enum { MP_APP_MAX_WINDOWS = 128 };

typedef struct mp_app_data
{
	bool init;
	bool shouldQuit;

	mp_window_data windowPool[MP_APP_MAX_WINDOWS];
	list_info windowFreeList;

	ringbuffer eventQueue;

} mp_app_data;

mp_app_data __mpAppData = {0};

void mp_init_window_handles()
{
	ListInit(&__mpAppData.windowFreeList);
	for(int i=0; i<MP_APP_MAX_WINDOWS; i++)
	{
		__mpAppData.windowPool[i].generation = 1;
		ListAppend(&__mpAppData.windowFreeList, &__mpAppData.windowPool[i].freeListElt);
	}
}

mp_window_data* mp_window_alloc()
{
	return(ListPopEntry(&__mpAppData.windowFreeList, mp_window_data, freeListElt));
}

mp_window_data* mp_window_ptr_from_handle(mp_window handle)
{
	u32 index = handle.h>>32;
	u32 generation = handle.h & 0xffffffff;
	if(index >= MP_APP_MAX_WINDOWS)
	{
		return(0);
	}
	mp_window_data* window = &__mpAppData.windowPool[index];
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
	DEBUG_ASSERT(  (window - __mpAppData.windowPool) >= 0
	            && (window - __mpAppData.windowPool) < MP_APP_MAX_WINDOWS);

	u64 h = ((u64)(window - __mpAppData.windowPool))<<32
	      | ((u64)window->generation);

	return((mp_window){h});
}

void mp_window_recycle_ptr(mp_window_data* window)
{
	window->generation++;
	ListPush(&__mpAppData.windowFreeList, &window->freeListElt);
}

//////////////////////

void mp_queue_event(mp_event* event)
{
	if(ringbuffer_write_available(&__mpAppData.eventQueue) < sizeof(mp_event))
	{
		LOG_ERROR("event queue full\n");
	}
	else
	{
		u32 written = ringbuffer_write(&__mpAppData.eventQueue, sizeof(mp_event), (u8*)event);
		DEBUG_ASSERT(written == sizeof(mp_event));
	}
}

//////////////////////

void mp_init()
{
	if(!__mpAppData.init)
	{
		memset(&__mpAppData, 0, sizeof(__mpAppData));
		mp_init_window_handles();

		ringbuffer_init(&__mpAppData.eventQueue, 16);
	}
}

void mp_terminate()
{
	if(__mpAppData.init)
	{
		__mpAppData = (mp_app_data){0};
	}
}

LRESULT WinProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	mp_window_data* mpWindow = GetPropW(windowHandle, L"MilePost");
	//TODO: put messages in queue

	switch(message)
	{
		case WM_CLOSE:
		{
			mp_event event = {0};
			event.window = mp_window_handle_from_ptr(mpWindow);
			event.type = MP_EVENT_WINDOW_CLOSE;

			mp_queue_event(&event);

		} break;

		default:
		{
			result = DefWindowProc(windowHandle, message, wParam, lParam);
		} break;
	}

	return(result);
}

mp_window mp_window_create(mp_rect rect, const char* title, mp_window_style style)
{
	WNDCLASS windowClass = {.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
	                       .lpfnWndProc = WinProc,
	                       .hInstance = GetModuleHandleW(NULL),
	                       .lpszClassName = "ApplicationWindowClass",
	                       .hCursor = LoadCursor(0, IDC_ARROW)};

	if(!RegisterClass(&windowClass))
	{
		//TODO: error
		goto quit;
	}

	HWND windowHandle = CreateWindow("ApplicationWindowClass", "Test Window",
	                                 WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
	                                 800, 600,
	                                 0, 0, windowClass.hInstance, 0);

	if(!windowHandle)
	{
		//TODO: error
		goto quit;
	}

	UpdateWindow(windowHandle);

	//TODO: return wrapped window
	quit:;
	mp_window_data* window = mp_window_alloc();
	window->windowHandle = windowHandle;

	SetPropW(windowHandle, L"MilePost", window);

	return(mp_window_handle_from_ptr(window));
}

void mp_window_bring_to_front_and_focus(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		ShowWindow(windowData->windowHandle, 1);
	}
}

bool mp_should_quit()
{
	return(__mpAppData.shouldQuit);
}

void mp_do_quit()
{
	__mpAppData.shouldQuit = true;
}

void mp_pump_events(f64 timeout)
{
	MSG message;
	while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}

bool mp_next_event(mp_event* event)
{
	//NOTE pop and return event from queue
	if(ringbuffer_read_available(&__mpAppData.eventQueue) >= sizeof(mp_event))
	{
		u64 read = ringbuffer_read(&__mpAppData.eventQueue, sizeof(mp_event), (u8*)event);
		DEBUG_ASSERT(read == sizeof(mp_event));
		return(true);
	}
	else
	{
		return(false);
	}
}


#undef LOG_SUBSYSTEM
