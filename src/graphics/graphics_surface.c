/************************************************************/ /**
*
*	@file: graphics_surface.c
*	@author: Martin Fouilleul
*	@date: 25/04/2023
*
*****************************************************************/

#include "graphics_surface.h"

//---------------------------------------------------------------
// per-thread selected surface
//---------------------------------------------------------------

oc_thread_local oc_surface oc_selectedSurface = { 0 };

//---------------------------------------------------------------
// typed handles functions
//---------------------------------------------------------------

oc_surface oc_surface_handle_alloc(oc_surface_data* surface)
{
    oc_surface handle = { .h = oc_graphics_handle_alloc(OC_GRAPHICS_HANDLE_SURFACE, (void*)surface) };
    return (handle);
}

oc_surface_data* oc_surface_data_from_handle(oc_surface handle)
{
    oc_surface_data* data = oc_graphics_data_from_handle(OC_GRAPHICS_HANDLE_SURFACE, handle.h);
    return (data);
}

oc_image oc_image_handle_alloc(oc_image_data* image)
{
    oc_image handle = { .h = oc_graphics_handle_alloc(OC_GRAPHICS_HANDLE_IMAGE, (void*)image) };
    return (handle);
}

oc_image_data* oc_image_data_from_handle(oc_image handle)
{
    oc_image_data* data = oc_graphics_data_from_handle(OC_GRAPHICS_HANDLE_IMAGE, handle.h);
    return (data);
}

//---------------------------------------------------------------
// surface API
//---------------------------------------------------------------

#if OC_COMPILE_GL
    #if OC_PLATFORM_WINDOWS
        #include "wgl_surface.h"
        #define oc_gl_surface_create_for_window oc_wgl_surface_create_for_window
    #endif
#endif

#if OC_COMPILE_GLES
    #include "egl_surface.h"
#endif

#if OC_COMPILE_METAL
    #include "mtl_surface.h"
#endif

#if OC_COMPILE_CANVAS
    #if OC_PLATFORM_MACOS
oc_surface_data* oc_mtl_canvas_surface_create_for_window(oc_window window);
    #elif OC_PLATFORM_WINDOWS
oc_surface_data* oc_gl_canvas_surface_create_for_window(oc_window window);
    #endif
#endif

bool oc_is_surface_backend_available(oc_surface_api api)
{
    bool result = false;
    switch(api)
    {
#if OC_COMPILE_METAL
        case OC_METAL:
#endif

#if OC_COMPILE_GL
        case OC_GL:
#endif

#if OC_COMPILE_GLES
        case OC_GLES:
#endif

#if OC_COMPILE_CANVAS
        case OC_CANVAS:
#endif
            result = true;
            break;

        default:
            break;
    }
    return (result);
}

oc_surface oc_surface_nil() { return ((oc_surface){ .h = 0 }); }

bool oc_surface_is_nil(oc_surface surface) { return (surface.h == 0); }

oc_surface oc_surface_create_for_window(oc_window window, oc_surface_api api)
{
    if(oc_graphicsData.init)
    {
        oc_graphics_init();
    }
    oc_surface surfaceHandle = oc_surface_nil();
    oc_surface_data* surface = 0;

    switch(api)
    {
#if OC_COMPILE_GL
        case OC_GL:
            surface = oc_gl_surface_create_for_window(window);
            break;
#endif

#if OC_COMPILE_GLES
        case OC_GLES:
            surface = oc_egl_surface_create_for_window(window);
            break;
#endif

#if OC_COMPILE_METAL
        case OC_METAL:
            surface = oc_mtl_surface_create_for_window(window);
            break;
#endif

#if OC_COMPILE_CANVAS
        case OC_CANVAS:

    #if OC_PLATFORM_MACOS
            surface = oc_mtl_canvas_surface_create_for_window(window);
    #elif OC_PLATFORM_WINDOWS
            surface = oc_gl_canvas_surface_create_for_window(window);
    #endif
            break;
#endif

        default:
            break;
    }
    if(surface)
    {
        surfaceHandle = oc_surface_handle_alloc(surface);
        oc_surface_select(surfaceHandle);
    }
    return (surfaceHandle);
}

void oc_surface_destroy(oc_surface handle)
{
    OC_DEBUG_ASSERT(oc_graphicsData.init);
    oc_surface_data* surface = oc_surface_data_from_handle(handle);
    if(surface)
    {
        if(oc_selectedSurface.h == handle.h)
        {
            oc_surface_deselect();
        }

        if(surface->backend && surface->backend->destroy)
        {
            surface->backend->destroy(surface->backend);
        }
        surface->destroy(surface);
        oc_graphics_handle_recycle(handle.h);
    }
}

void oc_surface_deselect()
{
    OC_DEBUG_ASSERT(oc_graphicsData.init);

    oc_surface_data* prevSurface = oc_surface_data_from_handle(oc_selectedSurface);
    if(prevSurface && prevSurface->deselect)
    {
        prevSurface->deselect(prevSurface);
    }
    oc_selectedSurface = oc_surface_nil();
}

void oc_surface_select(oc_surface surface)
{
    OC_DEBUG_ASSERT(oc_graphicsData.init);

    if(surface.h != oc_selectedSurface.h)
    {
        oc_surface_deselect();
    }

    oc_surface_data* surfaceData = oc_surface_data_from_handle(surface);
    if(surfaceData && surfaceData->prepare)
    {
        surfaceData->prepare(surfaceData);
        oc_selectedSurface = surface;
    }
}

void oc_surface_present(oc_surface surface)
{
    OC_DEBUG_ASSERT(oc_graphicsData.init);
    oc_surface_data* surfaceData = oc_surface_data_from_handle(surface);
    if(surfaceData && surfaceData->present)
    {
        surfaceData->present(surfaceData);
    }
}

void oc_surface_swap_interval(oc_surface surface, int swap)
{
    OC_DEBUG_ASSERT(oc_graphicsData.init);
    oc_surface_data* surfaceData = oc_surface_data_from_handle(surface);
    if(surfaceData && surfaceData->swapInterval)
    {
        surfaceData->swapInterval(surfaceData, swap);
    }
}

oc_vec2 oc_surface_get_size(oc_surface surface)
{
    OC_DEBUG_ASSERT(oc_graphicsData.init);
    oc_vec2 size = { 0 };
    oc_surface_data* surfaceData = oc_surface_data_from_handle(surface);
    if(surfaceData && surfaceData->getSize)
    {
        size = surfaceData->getSize(surfaceData);
    }
    return (size);
}

oc_vec2 oc_surface_contents_scaling(oc_surface surface)
{
    OC_DEBUG_ASSERT(oc_graphicsData.init);
    oc_vec2 scaling = { 1, 1 };
    oc_surface_data* surfaceData = oc_surface_data_from_handle(surface);
    if(surfaceData && surfaceData->contentsScaling)
    {
        scaling = surfaceData->contentsScaling(surfaceData);
    }
    return (scaling);
}

void oc_surface_set_hidden(oc_surface surface, bool hidden)
{
    OC_DEBUG_ASSERT(oc_graphicsData.init);
    oc_surface_data* surfaceData = oc_surface_data_from_handle(surface);
    if(surfaceData && surfaceData->setHidden)
    {
        surfaceData->setHidden(surfaceData, hidden);
    }
}

bool oc_surface_get_hidden(oc_surface surface)
{
    OC_DEBUG_ASSERT(oc_graphicsData.init);
    bool res = false;
    oc_surface_data* surfaceData = oc_surface_data_from_handle(surface);
    if(surfaceData && surfaceData->getHidden)
    {
        res = surfaceData->getHidden(surfaceData);
    }
    return (res);
}

void* oc_surface_native_layer(oc_surface surface)
{
    void* res = 0;
    oc_surface_data* surfaceData = oc_surface_data_from_handle(surface);
    if(surfaceData && surfaceData->nativeLayer)
    {
        res = surfaceData->nativeLayer(surfaceData);
    }
    return (res);
}

void oc_surface_render_commands(oc_surface surface,
                                oc_color clearColor,
                                u32 primitiveCount,
                                oc_primitive* primitives,
                                u32 eltCount,
                                oc_path_elt* elements)
{
    oc_surface_data* surfaceData = oc_surface_data_from_handle(surface);

    if(surface.h != oc_selectedSurface.h)
    {
        oc_log_error("surface is not selected. Make sure to call oc_surface_select() before drawing onto a surface.\n");
    }
    else if(surfaceData && surfaceData->backend)
    {
        surfaceData->backend->render(surfaceData->backend,
                                     clearColor,
                                     primitiveCount,
                                     primitives,
                                     eltCount,
                                     elements);
    }
}

void oc_surface_bring_to_front(oc_surface handle)
{
    oc_surface_data* surface = oc_surface_data_from_handle(handle);
    if(surface && surface->bringToFront)
    {
        surface->bringToFront(surface);
    }
}

void oc_surface_send_to_back(oc_surface handle)
{
    oc_surface_data* surface = oc_surface_data_from_handle(handle);
    if(surface && surface->sendToBack)
    {
        surface->sendToBack(surface);
    }
}

//------------------------------------------------------------------------------------------
//NOTE(martin): images
//------------------------------------------------------------------------------------------

oc_vec2 oc_image_size(oc_image image)
{
    oc_vec2 res = { 0 };
    oc_image_data* imageData = oc_image_data_from_handle(image);
    if(imageData)
    {
        res = imageData->size;
    }
    return (res);
}

oc_image oc_image_create(oc_surface surface, u32 width, u32 height)
{
    oc_image image = oc_image_nil();
    oc_surface_data* surfaceData = oc_surface_data_from_handle(surface);

    if(surface.h != oc_selectedSurface.h)
    {
        oc_log_error("surface is not selected. Make sure to call oc_surface_select() before modifying graphics resources.\n");
    }
    else if(surfaceData && surfaceData->backend)
    {
        OC_DEBUG_ASSERT(surfaceData->api == OC_CANVAS);

        oc_image_data* imageData = surfaceData->backend->imageCreate(surfaceData->backend, (oc_vec2){ width, height });
        if(imageData)
        {
            imageData->surface = surface;
            image = oc_image_handle_alloc(imageData);
        }
    }
    return (image);
}

void oc_image_destroy(oc_image image)
{
    oc_image_data* imageData = oc_image_data_from_handle(image);

    if(imageData)
    {
        if(imageData->surface.h != oc_selectedSurface.h)
        {
            oc_log_error("surface is not selected. Make sure to call oc_surface_select() before modifying graphics resources.\n");
        }
        else
        {
            oc_surface_data* surface = oc_surface_data_from_handle(imageData->surface);
            if(surface && surface->backend)
            {
                surface->backend->imageDestroy(surface->backend, imageData);
                oc_graphics_handle_recycle(image.h);
            }
        }
    }
}

void oc_image_upload_region_rgba8(oc_image image, oc_rect region, u8* pixels)
{
    oc_image_data* imageData = oc_image_data_from_handle(image);

    if(imageData)
    {
        if(imageData->surface.h != oc_selectedSurface.h)
        {
            oc_log_error("surface is not selected. Make sure to call oc_surface_select() before modifying graphics resources.\n");
        }
        else
        {
            oc_surface_data* surfaceData = oc_surface_data_from_handle(imageData->surface);
            if(surfaceData)
            {
                OC_DEBUG_ASSERT(surfaceData->backend);
                surfaceData->backend->imageUploadRegion(surfaceData->backend, imageData, region, pixels);
            }
        }
    }
}
