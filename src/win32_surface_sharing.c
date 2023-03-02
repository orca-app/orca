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
// Surface server
//------------------------------------------------------------------------------------------------

typedef struct mg_surface_server_data
{
	int dummy;
} mg_surface_server_data;

mg_surface_server_data* mg_surface_server_data_from_handle(mg_surface_server handle)
{
	mg_surface_server_data* server = (mg_surface_server_data*)mg_data_from_handle(MG_HANDLE_SURFACE_SERVER, handle.h);
	return(server);
}

MP_API mg_surface_server mg_surface_server_create(void)
{
	mg_surface_server_data* server = malloc_type(mg_surface_server_data);

	//TODO

	mg_surface_server handle = (mg_surface_server){mg_handle_alloc(MG_HANDLE_SURFACE_SERVER, (void*)server)};
	return(handle);
}

MP_API void mg_surface_server_destroy(mg_surface_server handle)
{
	mg_surface_server_data* server = mg_surface_server_data_from_handle(handle);
	if(server)
	{
		//TODO

		free(server);
		mg_handle_recycle(handle.h);
	}
}

MP_API mg_surface_connection_id mg_surface_server_start(mg_surface_server handle, mg_surface surface)
{
	mg_surface_connection_id res = 0;

	mg_surface_server_data* server = mg_surface_server_data_from_handle(handle);
	if(server)
	{
		//NOTE: just a quick test
		res = (u64)mg_surface_native_layer(surface);
	}
	return(res);
}

MP_API void mg_surface_server_stop(mg_surface_server handle)
{
	mg_surface_server_data* server = mg_surface_server_data_from_handle(handle);
	if(server)
	{
		//TODO
	}
}

//------------------------------------------------------------------------------------------------
// Surface client
//------------------------------------------------------------------------------------------------

typedef struct mg_win32_surface_client
{
	mg_surface_data interface;
	mp_layer layer;

	HWND remoteWnd;

} mg_win32_surface_client;

void mg_win32_surface_client_prepare(mg_surface_data* interface)
{}

void mg_win32_surface_client_present(mg_surface_data* interface)
{
	mg_win32_surface_client* surface = (mg_win32_surface_client*)interface;

	HWND dstWindow = (HWND)mp_layer_native_surface(&surface->layer);
	RECT dstRect;
	GetClientRect(dstWindow, &dstRect);

	HDC dstDC = GetDC(dstWindow);
	HDC srcDC = GetDC(surface->remoteWnd);

	int res = BitBlt(dstDC,
	                 dstRect.left,
	                 dstRect.top,
	                 dstRect.right - dstRect.left,
	                 dstRect.bottom - dstRect.top,
	                 srcDC,
	                 0,
	                 0,
	                 SRCCOPY);
}

void mg_win32_surface_client_swap_interval(mg_surface_data* interface, int swap)
{
	//TODO
}

vec2 mg_win32_surface_client_contents_scaling(mg_surface_data* interface)
{
	mg_win32_surface_client* surface = (mg_win32_surface_client*)interface;
	return(mp_layer_contents_scaling(&surface->layer));
}

mp_rect mg_win32_surface_client_get_frame(mg_surface_data* interface)
{
	mg_win32_surface_client* surface = (mg_win32_surface_client*)interface;
	return(mp_layer_get_frame(&surface->layer));
}

void mg_win32_surface_client_set_frame(mg_surface_data* interface, mp_rect frame)
{
	mg_win32_surface_client* surface = (mg_win32_surface_client*)interface;
	mp_layer_set_frame(&surface->layer, frame);
}

void mg_win32_surface_client_set_hidden(mg_surface_data* interface, bool hidden)
{
	mg_win32_surface_client* surface = (mg_win32_surface_client*)interface;
	mp_layer_set_hidden(&surface->layer, hidden);
}

bool mg_win32_surface_client_get_hidden(mg_surface_data* interface)
{
	mg_win32_surface_client* surface = (mg_win32_surface_client*)interface;
	return(mp_layer_get_hidden(&surface->layer));
}

void* mg_win32_surface_client_native_layer(mg_surface_data* interface)
{
	mg_win32_surface_client* surface = (mg_win32_surface_client*)interface;
	return(mp_layer_native_surface(&surface->layer));
}

void mg_win32_surface_client_destroy(mg_surface_data* interface)
{
	mg_win32_surface_client* surface = (mg_win32_surface_client*)interface;

	mp_layer_cleanup(&surface->layer);
	//TODO...

	free(surface);
}

mg_surface_data* mg_win32_surface_client_create_for_window(mp_window window)
{
	mg_win32_surface_client* surface = 0;

	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		surface = malloc_type(mg_win32_surface_client);
		if(surface)
		{
			mp_layer_init_for_window(&surface->layer, windowData);

			surface->interface.backend = MG_BACKEND_REMOTE;
			surface->interface.destroy = mg_win32_surface_client_destroy;
			surface->interface.prepare = mg_win32_surface_client_prepare;
			surface->interface.present = mg_win32_surface_client_present;
			surface->interface.swapInterval = mg_win32_surface_client_swap_interval;
			surface->interface.contentsScaling = mg_win32_surface_client_contents_scaling;
			surface->interface.getFrame = mg_win32_surface_client_get_frame;
			surface->interface.setFrame = mg_win32_surface_client_set_frame;
			surface->interface.getHidden = mg_win32_surface_client_get_hidden;
			surface->interface.setHidden = mg_win32_surface_client_set_hidden;
			surface->interface.nativeLayer = mg_win32_surface_client_native_layer;

			//TODO
		}
	}
	return((mg_surface_data*)surface);
}

mg_surface mg_surface_client_create_for_window(mp_window window)
{
	if(!__mgData.init)
	{
		mg_init();
	}
	mg_surface surfaceHandle = {0};
	mg_surface_data* surface = mg_win32_surface_client_create_for_window(window);
	if(surface)
	{
		surfaceHandle = mg_surface_handle_alloc(surface);
	}
	return(surfaceHandle);
}

MP_API void mg_surface_client_connect(mg_surface handle, mg_surface_connection_id ID)
{
	mg_surface_data* interface = mg_surface_data_from_handle(handle);
	if(interface && interface->backend == MG_BACKEND_REMOTE)
	{
		mg_win32_surface_client* surface = (mg_win32_surface_client*)interface;

		//NOTE:Quick test
		surface->remoteWnd = (HWND)ID;
	}
}

MP_API void mg_surface_client_disconnect(mg_surface handle)
{
	mg_surface_data* interface = mg_surface_data_from_handle(handle);
	if(interface && interface->backend == MG_BACKEND_REMOTE)
	{
		//TODO
	}
}
