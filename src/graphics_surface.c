/************************************************************//**
*
*	@file: graphics_surface.c
*	@author: Martin Fouilleul
*	@date: 25/04/2023
*
*****************************************************************/

#include"graphics_surface.h"

//---------------------------------------------------------------
// typed handles functions
//---------------------------------------------------------------

mg_surface mg_surface_handle_alloc(mg_surface_data* surface)
{
	mg_surface handle = {.h = mg_handle_alloc(MG_HANDLE_SURFACE, (void*)surface) };
	return(handle);
}

mg_surface_data* mg_surface_data_from_handle(mg_surface handle)
{
	mg_surface_data* data = mg_data_from_handle(MG_HANDLE_SURFACE, handle.h);
	return(data);
}

mg_image mg_image_handle_alloc(mg_image_data* image)
{
	mg_image handle = {.h = mg_handle_alloc(MG_HANDLE_IMAGE, (void*)image) };
	return(handle);
}

mg_image_data* mg_image_data_from_handle(mg_image handle)
{
	mg_image_data* data = mg_data_from_handle(MG_HANDLE_IMAGE, handle.h);
	return(data);
}

//---------------------------------------------------------------
// surface API
//---------------------------------------------------------------

#if MG_COMPILE_GL
	#if PLATFORM_WINDOWS
		#include"wgl_surface.h"
		#define gl_surface_create_for_window mg_wgl_surface_create_for_window
	#endif
#endif

#if MG_COMPILE_GLES
	#include"egl_surface.h"
#endif

#if MG_COMPILE_METAL
	#include"mtl_surface.h"
#endif

#if MG_COMPILE_CANVAS
	#if PLATFORM_MACOS
		mg_surface_data* mtl_canvas_surface_create_for_window(mp_window window);
	#elif PLATFORM_WINDOWS
		//TODO
	#endif
#endif

bool mg_is_surface_backend_available(mg_surface_api api)
{
	bool result = false;
	switch(api)
	{
		#if MG_COMPILE_METAL
			case MG_METAL:
		#endif

		#if MG_COMPILE_GL
			case MG_GL:
		#endif

		#if MG_COMPILE_GLES
			case MG_GLES:
		#endif

		#if MG_COMPILE_CANVAS
			case MG_CANVAS:
		#endif
			result = true;
			break;

		default:
			break;
	}
	return(result);
}

mg_surface mg_surface_nil() { return((mg_surface){.h = 0}); }
bool mg_surface_is_nil(mg_surface surface) { return(surface.h == 0); }

mg_surface mg_surface_create_for_window(mp_window window, mg_surface_api api)
{
	if(__mgData.init)
	{
		mg_init();
	}
	mg_surface surfaceHandle = mg_surface_nil();
	mg_surface_data* surface = 0;

	switch(api)
	{
	#if MG_COMPILE_GL
		case MG_GL:
			surface = gl_surface_create_for_window(window);
			break;
	#endif

	#if MG_COMPILE_GLES
		case MG_GLES:
			surface = mg_egl_surface_create_for_window(window);
			break;
	#endif

	#if MG_COMPILE_METAL
		case MG_METAL:
			surface = mg_mtl_surface_create_for_window(window);
			break;
	#endif

	#if MG_COMPILE_CANVAS
		case MG_CANVAS:

		#if PLATFORM_MACOS
			surface = mtl_canvas_surface_create_for_window(window);
		#elif PLATFORM_WINDOWS
			surface = gl_canvas_surface_create_for_window(window);
		#endif
			break;
	#endif

		default:
			break;
	}
	if(surface)
	{
		surfaceHandle = mg_surface_handle_alloc(surface);
	}
	return(surfaceHandle);
}

mg_surface mg_surface_create_remote(u32 width, u32 height, mg_surface_api api)
{
	if(__mgData.init)
	{
		mg_init();
	}
	mg_surface surfaceHandle = mg_surface_nil();
	mg_surface_data* surface = 0;

	switch(api)
	{
	#if MG_COMPILE_GLES
		case MG_GLES:
			surface = mg_egl_surface_create_remote(width, height);
			break;
	#endif

		default:
			break;
	}
	if(surface)
	{
		surfaceHandle = mg_surface_handle_alloc(surface);
	}
	return(surfaceHandle);
}

mg_surface mg_surface_create_host(mp_window window)
{
	if(__mgData.init)
	{
		mg_init();
	}
	mg_surface handle = mg_surface_nil();
	mg_surface_data* surface = 0;
	#if PLATFORM_MACOS
		surface = mg_osx_surface_create_host(window);
	#elif PLATFORM_WINDOWS
		surface = mg_win32_surface_create_host(window);
	#endif

	if(surface)
	{
		handle = mg_surface_handle_alloc(surface);
	}
	return(handle);
}

void mg_surface_destroy(mg_surface handle)
{
	DEBUG_ASSERT(__mgData.init);
	mg_surface_data* surface = mg_surface_data_from_handle(handle);
	if(surface)
	{
		if(surface->backend && surface->backend->destroy)
		{
			surface->backend->destroy(surface->backend);
		}
		surface->destroy(surface);
		mg_handle_recycle(handle.h);
	}
}

void mg_surface_prepare(mg_surface surface)
{
	DEBUG_ASSERT(__mgData.init);
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->prepare)
	{
		surfaceData->prepare(surfaceData);
	}
}

void mg_surface_present(mg_surface surface)
{
	DEBUG_ASSERT(__mgData.init);
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->present)
	{
		surfaceData->present(surfaceData);
	}
}

void mg_surface_swap_interval(mg_surface surface, int swap)
{
	DEBUG_ASSERT(__mgData.init);
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->swapInterval)
	{
		surfaceData->swapInterval(surfaceData, swap);
	}
}

vec2 mg_surface_contents_scaling(mg_surface surface)
{
	DEBUG_ASSERT(__mgData.init);
	vec2 scaling = {1, 1};
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->contentsScaling)
	{
		scaling = surfaceData->contentsScaling(surfaceData);
	}
	return(scaling);
}


void mg_surface_set_frame(mg_surface surface, mp_rect frame)
{
	DEBUG_ASSERT(__mgData.init);
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->setFrame)
	{
		surfaceData->setFrame(surfaceData, frame);
	}
}

mp_rect mg_surface_get_frame(mg_surface surface)
{
	DEBUG_ASSERT(__mgData.init);
	mp_rect res = {0};
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->getFrame)
	{
		res = surfaceData->getFrame(surfaceData);
	}
	return(res);
}

void mg_surface_set_hidden(mg_surface surface, bool hidden)
{
	DEBUG_ASSERT(__mgData.init);
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->setHidden)
	{
		surfaceData->setHidden(surfaceData, hidden);
	}
}

bool mg_surface_get_hidden(mg_surface surface)
{
	DEBUG_ASSERT(__mgData.init);
	bool res = false;
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->getHidden)
	{
		res = surfaceData->getHidden(surfaceData);
	}
	return(res);
}

void* mg_surface_native_layer(mg_surface surface)
{
	void* res = 0;
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->nativeLayer)
	{
		res = surfaceData->nativeLayer(surfaceData);
	}
	return(res);
}

mg_surface_id mg_surface_remote_id(mg_surface handle)
{
	mg_surface_id remoteId = 0;
	mg_surface_data* surface = mg_surface_data_from_handle(handle);
	if(surface && surface->remoteID)
	{
		remoteId = surface->remoteID(surface);
	}
	return(remoteId);
}

void mg_surface_host_connect(mg_surface handle, mg_surface_id remoteID)
{
	mg_surface_data* surface = mg_surface_data_from_handle(handle);
	if(surface && surface->hostConnect)
	{
		surface->hostConnect(surface, remoteID);
	}
}

void mg_surface_render_commands(mg_surface surface,
                                mg_color clearColor,
                                u32 primitiveCount,
                                mg_primitive* primitives,
                                u32 eltCount,
                                mg_path_elt* elements)
{
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->backend)
	{
		surfaceData->backend->render(surfaceData->backend,
		                             clearColor,
		                             primitiveCount,
		                             primitives,
		                             eltCount,
		                             elements);
	}
}

//------------------------------------------------------------------------------------------
//NOTE(martin): images
//------------------------------------------------------------------------------------------

vec2 mg_image_size(mg_image image)
{
	vec2 res = {0};
	mg_image_data* imageData = mg_image_data_from_handle(image);
	if(imageData)
	{
		res = imageData->size;
	}
	return(res);
}

mg_image mg_image_create(mg_surface surface, u32 width, u32 height)
{
	mg_image image = mg_image_nil();
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->backend)
	{
		DEBUG_ASSERT(surfaceData->api == MG_CANVAS);

		mg_image_data* imageData = surfaceData->backend->imageCreate(surfaceData->backend, (vec2){width, height});
		if(imageData)
		{
			imageData->surface = surface;
			image = mg_image_handle_alloc(imageData);
		}
	}
	return(image);
}

void mg_image_destroy(mg_image image)
{
	mg_image_data* imageData = mg_image_data_from_handle(image);
	if(imageData)
	{
		mg_surface_data* surface = mg_surface_data_from_handle(imageData->surface);
		if(surface && surface->backend)
		{
			surface->backend->imageDestroy(surface->backend, imageData);
			mg_handle_recycle(image.h);
		}
	}
}

void mg_image_upload_region_rgba8(mg_image image, mp_rect region, u8* pixels)
{
	mg_image_data* imageData = mg_image_data_from_handle(image);
	if(imageData)
	{
		mg_surface_data* surfaceData = mg_surface_data_from_handle(imageData->surface);
		if(surfaceData)
		{
			DEBUG_ASSERT(surfaceData->backend);
			surfaceData->backend->imageUploadRegion(surfaceData->backend, imageData, region, pixels);
		}
	}
}
