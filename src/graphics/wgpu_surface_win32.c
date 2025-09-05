/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "platform/platform_memory.h"
#include "surface.h"
#include "wgpu_surface.h"

typedef struct oc_wgpu_surface
{
    oc_surface_base base;
    WGPUDevice device;
    WGPUSurface wgpuSurface;
    WGPUTexture currentTexture;
    oc_vec2 swapChainSize;

} oc_wgpu_surface;

i32 oc_wgpu_surface_destroy_callback(void* user)
{
    oc_surface_base* base = (oc_surface_base*)user;

    oc_wgpu_surface* surface = (oc_wgpu_surface*)base;

    wgpuSurfaceRelease(surface->wgpuSurface);
    if(surface->currentTexture)
    {
        wgpuTextureRelease(surface->currentTexture);
    }
    if(surface->device)
    {
        wgpuDeviceRelease(surface->device);
    }
    oc_surface_base_cleanup(base);
    free(surface);
    return 0;
}

void oc_wgpu_surface_destroy(oc_surface_base* base)
{
    oc_dispatch_on_main_thread_sync(oc_wgpu_surface_destroy_callback, base);
}

typedef struct oc_wgpu_surface_create_data
{
    WGPUInstance instance;
    oc_window window;
    oc_surface surface;
} oc_wgpu_surface_create_data;

i32 oc_wgpu_surface_create_callback(void* user)
{
    oc_wgpu_surface_create_data* data = (oc_wgpu_surface_create_data*)user;
    WGPUInstance instance = data->instance;
    oc_window window = data->window;

    oc_wgpu_surface* surface = 0;
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        surface = (oc_wgpu_surface*)oc_malloc_type(oc_wgpu_surface);
        memset(surface, 0, sizeof(oc_wgpu_surface));

        oc_surface_base_init_for_window((oc_surface_base*)surface, windowData);

        surface->base.api = OC_SURFACE_WEBGPU;
        surface->base.destroy = oc_wgpu_surface_destroy;

        WGPUSurfaceDescriptor desc = {
            .nextInChain = &((WGPUSurfaceDescriptorFromWindowsHWND){
                                 .chain.sType = WGPUSType_SurfaceDescriptorFromWindowsHWND,
                                 .hinstance = GetModuleHandle(NULL),
                                 .hwnd = surface->base.view.hWnd,
                             })
                                .chain,
        };
        surface->wgpuSurface = wgpuInstanceCreateSurface(instance, &desc);
    }
    oc_surface handle = oc_surface_nil();
    if(surface)
    {
        handle = oc_surface_handle_alloc((oc_surface_base*)surface);
    }
    data->surface = handle;
    return 0;
}

oc_surface oc_wgpu_surface_create_for_window(WGPUInstance instance, oc_window window)
{
    oc_wgpu_surface_create_data data = {
        .instance = instance,
        .window = window,
    };

    oc_dispatch_on_main_thread_sync(oc_wgpu_surface_create_callback, &data);

    return (data.surface);
}

WGPUTexture oc_wgpu_surface_get_current_texture(oc_surface handle, WGPUDevice device)
{
    WGPUTexture texture = 0;

    oc_surface_base* base = oc_surface_from_handle(handle);
    if(base && base->api == OC_SURFACE_WEBGPU)
    {
        oc_wgpu_surface* surface = (oc_wgpu_surface*)base;
        oc_vec2 size = base->getSize(base);
        oc_vec2 scale = base->contentsScaling(base);
        size.x *= scale.x;
        size.y *= scale.y;

        if(surface->device != device
           || surface->currentTexture == NULL)
        {
            if(surface->currentTexture)
            {
                wgpuTextureRelease(surface->currentTexture);
                surface->currentTexture = 0;
                wgpuSurfaceUnconfigure(surface->wgpuSurface);
            }
            if(surface->device)
            {
                wgpuDeviceRelease(surface->device);
            }
            surface->device = device;
            wgpuDeviceAddRef(surface->device);

            if(surface->swapChainSize.x != size.x
               || surface->swapChainSize.y != size.y)
            {
                surface->swapChainSize = size;

                WGPUSurfaceConfiguration config = {
                    .device = device,
                    .format = WGPUTextureFormat_BGRA8Unorm,
                    .usage = WGPUTextureUsage_RenderAttachment,
                    .alphaMode = WGPUCompositeAlphaMode_Premultiplied,
                    .width = size.x,
                    .height = size.y,
                    .presentMode = WGPUPresentMode_Fifo,
                };
                wgpuSurfaceConfigure(surface->wgpuSurface, &config);
            }
            WGPUSurfaceTexture surfaceTexture = { 0 };
            wgpuSurfaceGetCurrentTexture(surface->wgpuSurface, &surfaceTexture);
            surface->currentTexture = surfaceTexture.texture;
        }
        texture = surface->currentTexture;
    }
    return texture;
}

void oc_wgpu_surface_present(oc_surface handle)
{
    oc_surface_base* base = oc_surface_from_handle(handle);
    if(base && base->api == OC_SURFACE_WEBGPU)
    {
        oc_wgpu_surface* surface = (oc_wgpu_surface*)base;
        if(surface->currentTexture)
        {
            wgpuSurfacePresent(surface->wgpuSurface);
            wgpuTextureRelease(surface->currentTexture);
            surface->currentTexture = 0;
        }
    }
}
