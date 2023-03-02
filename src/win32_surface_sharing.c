/************************************************************//**
*
*	@file: win32_surface_sharing.c
*	@author: Martin Fouilleul
*	@date: 02/03/2023
*	@revision:
*
*****************************************************************/
#include"graphics_internal.h"

//------------------------------------------------------------------------------------------------
// Surface client
//------------------------------------------------------------------------------------------------

typedef struct mg_win32_surface_host
{
	mg_surface_data interface;
	mp_layer layer;

} mg_win32_surface_host;

void mg_win32_surface_host_prepare(mg_surface_data* interface)
{}

void mg_win32_surface_host_present(mg_surface_data* interface)
{}

void mg_win32_surface_host_swap_interval(mg_surface_data* interface, int swap)
{
	//TODO
}

vec2 mg_win32_surface_host_contents_scaling(mg_surface_data* interface)
{
	mg_win32_surface_host* surface = (mg_win32_surface_host*)interface;
	return(mp_layer_contents_scaling(&surface->layer));
}

mp_rect mg_win32_surface_host_get_frame(mg_surface_data* interface)
{
	mg_win32_surface_host* surface = (mg_win32_surface_host*)interface;
	return(mp_layer_get_frame(&surface->layer));
}

void mg_win32_surface_host_set_frame(mg_surface_data* interface, mp_rect frame)
{
	mg_win32_surface_host* surface = (mg_win32_surface_host*)interface;
	mp_layer_set_frame(&surface->layer, frame);
}

void mg_win32_surface_host_set_hidden(mg_surface_data* interface, bool hidden)
{
	mg_win32_surface_host* surface = (mg_win32_surface_host*)interface;
	mp_layer_set_hidden(&surface->layer, hidden);
}

bool mg_win32_surface_host_get_hidden(mg_surface_data* interface)
{
	mg_win32_surface_host* surface = (mg_win32_surface_host*)interface;
	return(mp_layer_get_hidden(&surface->layer));
}

void* mg_win32_surface_host_native_layer(mg_surface_data* interface)
{
	mg_win32_surface_host* surface = (mg_win32_surface_host*)interface;
	return(mp_layer_native_surface(&surface->layer));
}

void mg_win32_surface_host_destroy(mg_surface_data* interface)
{
	mg_win32_surface_host* surface = (mg_win32_surface_host*)interface;

	mp_layer_cleanup(&surface->layer);
	//TODO...

	free(surface);
}

MP_API void mg_win32_surface_host_connect(mg_surface_data* interface, mg_surface_id ID)
{
	mg_win32_surface_host* surface = (mg_win32_surface_host*)interface;

	//NOTE:Quick test
	HWND dstWnd = mp_layer_native_surface(&surface->layer);
	HWND srcWnd = (HWND)ID;

	RECT dstRect;
	GetClientRect(dstWnd, &dstRect);

	SetParent(srcWnd, dstWnd);
	ShowWindow(srcWnd, SW_NORMAL);

	SetWindowPos(srcWnd,
			        HWND_TOP,
			        0,
			        0,
			        dstRect.right - dstRect.left,
			        dstRect.bottom - dstRect.top,
			        SWP_NOACTIVATE | SWP_NOZORDER);
}

mg_surface_data* mg_win32_surface_create_host(mp_window window)
{
	mg_win32_surface_host* surface = 0;

	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		surface = malloc_type(mg_win32_surface_host);
		if(surface)
		{
			mp_layer_init_for_window(&surface->layer, windowData);

			surface->interface.backend = MG_BACKEND_REMOTE;
			surface->interface.destroy = mg_win32_surface_host_destroy;
			surface->interface.prepare = mg_win32_surface_host_prepare;
			surface->interface.present = mg_win32_surface_host_present;
			surface->interface.swapInterval = mg_win32_surface_host_swap_interval;
			surface->interface.contentsScaling = mg_win32_surface_host_contents_scaling;
			surface->interface.getFrame = mg_win32_surface_host_get_frame;
			surface->interface.setFrame = mg_win32_surface_host_set_frame;
			surface->interface.getHidden = mg_win32_surface_host_get_hidden;
			surface->interface.setHidden = mg_win32_surface_host_set_hidden;
			surface->interface.nativeLayer = mg_win32_surface_host_native_layer;
			surface->interface.connect = mg_win32_surface_host_connect;
			//TODO
		}
	}
	return((mg_surface_data*)surface);
}

mg_surface mg_surface_create_host(mp_window window)
{
	if(!__mgData.init)
	{
		mg_init();
	}
	mg_surface surfaceHandle = {0};
	mg_surface_data* surface = mg_win32_surface_create_host(window);
	if(surface)
	{
		surfaceHandle = mg_surface_handle_alloc(surface);
	}
	return(surfaceHandle);
}
