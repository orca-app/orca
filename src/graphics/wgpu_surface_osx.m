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
    CAMetalLayer* mtlLayer;

    WGPUDevice device;
    WGPUSurface wgpuSurface;
    WGPUTexture currentTexture;
    oc_vec2 swapChainSize;

} oc_wgpu_surface;

void oc_wgpu_surface_destroy(oc_surface_base* base)
{
    @autoreleasepool
    {
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

        [surface->mtlLayer release];
        oc_surface_base_cleanup(base);
        free(surface);
    }
}

oc_surface oc_wgpu_surface_create_for_window(WGPUInstance instance, oc_window window)
{
    @autoreleasepool
    {
        oc_wgpu_surface* surface = 0;
        oc_window_data* windowData = oc_window_ptr_from_handle(window);
        if(windowData)
        {
            surface = (oc_wgpu_surface*)oc_malloc_type(oc_wgpu_surface);
            memset(surface, 0, sizeof(oc_wgpu_surface));

            oc_surface_base_init_for_window((oc_surface_base*)surface, windowData);

            surface->base.api = OC_SURFACE_WEBGPU;
            surface->base.destroy = oc_wgpu_surface_destroy;

            surface->mtlLayer = [CAMetalLayer layer];

            [surface->mtlLayer setOpaque:NO];
            surface->mtlLayer.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
            [surface->base.view.layer addSublayer:(CALayer*)surface->mtlLayer];

            NSRect frame = [[windowData->osx.nsWindow contentView] frame];
            CGSize size = frame.size;
            surface->mtlLayer.frame = (CGRect){ { 0, 0 }, size };

            /////////////////////////////////////////////////////////////////
            //TODO: get actual scaling from window
            /////////////////////////////////////////////////////////////////
            const int OC_OSX_SURFACE_CONTENTS_SCALING = 2;

            size.width *= OC_OSX_SURFACE_CONTENTS_SCALING;
            size.height *= OC_OSX_SURFACE_CONTENTS_SCALING;
            surface->mtlLayer.drawableSize = size;
            surface->mtlLayer.contentsScale = OC_OSX_SURFACE_CONTENTS_SCALING;
            surface->mtlLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;

            WGPUSurfaceDescriptor desc = {
                .nextInChain = &((WGPUSurfaceDescriptorFromMetalLayer){
                                     .chain.sType = WGPUSType_SurfaceDescriptorFromMetalLayer,
                                     .layer = surface->mtlLayer,
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
        return (handle);
    }
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
