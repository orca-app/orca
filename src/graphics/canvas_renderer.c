/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "canvas_renderer.h"

//---------------------------------------------------------------
// typed handles
//---------------------------------------------------------------

oc_canvas_renderer oc_canvas_renderer_handle_alloc(oc_canvas_renderer_base* renderer)
{
    oc_canvas_renderer handle = { .h = oc_graphics_handle_alloc(OC_GRAPHICS_HANDLE_CANVAS_RENDERER, (void*)renderer) };
    return (handle);
}

oc_canvas_renderer_base* oc_canvas_renderer_from_handle(oc_canvas_renderer handle)
{
    oc_canvas_renderer_base* data = oc_graphics_data_from_handle(OC_GRAPHICS_HANDLE_CANVAS_RENDERER, handle.h);
    return (data);
}

oc_image oc_image_handle_alloc(oc_image_base* image)
{
    oc_image handle = { .h = oc_graphics_handle_alloc(OC_GRAPHICS_HANDLE_IMAGE, (void*)image) };
    return (handle);
}

oc_image_base* oc_image_from_handle(oc_image handle)
{
    oc_image_base* data = oc_graphics_data_from_handle(OC_GRAPHICS_HANDLE_IMAGE, handle.h);
    return (data);
}

//---------------------------------------------------------------
// renderer
//---------------------------------------------------------------

void oc_canvas_renderer_destroy(oc_canvas_renderer handle)
{
    oc_canvas_renderer_base* renderer = oc_canvas_renderer_from_handle(handle);
    if(renderer)
    {
        if(renderer->destroy)
        {
            renderer->destroy(renderer);
        }
        oc_graphics_handle_recycle(handle.h);
    }
}

void oc_canvas_renderer_submit(oc_canvas_renderer rendererHandle,
                               oc_surface surfaceHandle,
                               u32 msaaSampleCount,
                               oc_color clearColor,
                               u32 primitiveCount,
                               oc_primitive* primitives,
                               u32 eltCount,
                               oc_path_elt* elements)
{
    oc_canvas_renderer_base* renderer = oc_canvas_renderer_from_handle(rendererHandle);

    if(renderer && renderer->submit)
    {
        renderer->submit(renderer,
                         surfaceHandle,
                         msaaSampleCount,
                         clearColor,
                         primitiveCount,
                         primitives,
                         eltCount,
                         elements);
    }
}

void oc_canvas_present(oc_canvas_renderer rendererHandle, oc_surface surfaceHandle)
{
    oc_canvas_renderer_base* renderer = oc_canvas_renderer_from_handle(rendererHandle);
    if(renderer && renderer->present)
    {
        renderer->present(renderer, surfaceHandle);
    }
}

//------------------------------------------------------------------------------------------
// canvas surface
//------------------------------------------------------------------------------------------
oc_surface oc_canvas_surface_create_for_window(oc_canvas_renderer rendererHandle, oc_window window)
{
    oc_surface surface = oc_surface_nil();
    oc_canvas_renderer_base* renderer = oc_canvas_renderer_from_handle(rendererHandle);

    if(renderer && renderer->createSurfaceForWindow)
    {
        surface = renderer->createSurfaceForWindow(renderer, window);
    }
    return (surface);
}

//---------------------------------------------------------------
// image
//---------------------------------------------------------------

oc_image oc_image_create(oc_canvas_renderer handle, u32 width, u32 height)
{
    oc_image image = oc_image_nil();
    oc_canvas_renderer_base* renderer = oc_canvas_renderer_from_handle(handle);
    if(renderer && renderer->imageCreate)
    {
        oc_image_base* imageData = renderer->imageCreate(renderer, (oc_vec2){ width, height });
        if(imageData)
        {
            imageData->renderer = handle;
            image = oc_image_handle_alloc(imageData);
        }
    }
    return (image);
}

void oc_image_destroy(oc_image image)
{
    oc_image_base* imageData = oc_image_from_handle(image);

    if(imageData)
    {
        oc_canvas_renderer_base* renderer = oc_canvas_renderer_from_handle(imageData->renderer);
        if(renderer && renderer->imageDestroy)
        {
            renderer->imageDestroy(renderer, imageData);
        }
        oc_graphics_handle_recycle(image.h);
    }
}

void oc_image_upload_region_rgba8(oc_image image, oc_rect region, u8* pixels)
{
    oc_image_base* imageData = oc_image_from_handle(image);

    if(imageData)
    {
        oc_canvas_renderer_base* renderer = oc_canvas_renderer_from_handle(imageData->renderer);
        if(renderer && renderer->imageUploadRegion)
        {
            renderer->imageUploadRegion(renderer, imageData, region, pixels);
        }
    }
}

oc_vec2 oc_image_size(oc_image image)
{
    oc_vec2 res = { 0 };
    oc_image_base* imageData = oc_image_from_handle(image);
    if(imageData)
    {
        res = imageData->size;
    }
    return (res);
}
