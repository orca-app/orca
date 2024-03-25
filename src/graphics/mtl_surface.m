/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <QuartzCore/QuartzCore.h>
#include <simd/simd.h>

#include "app/osx_app.h"
#include "surface.h"
#include "util/macros.h"
#include "platform/platform_memory.h" //TODO: maybe move oc_malloc_type out of platform?

typedef struct oc_mtl_surface
{
    oc_surface_base base;
    CAMetalLayer* mtlLayer;

} oc_mtl_surface;

void oc_mtl_surface_destroy(oc_surface_base* base)
{
    oc_mtl_surface* surface = (oc_mtl_surface*)base;

    @autoreleasepool
    {
        [surface->mtlLayer removeFromSuperlayer];
        [surface->mtlLayer release];
    }

    //TODO: surface-api-rework: call base cleanup when?
}

//TODO fix that according to real scaling, depending on the monitor settings
static const f32 OC_MTL_SURFACE_CONTENTS_SCALING = 2;

//NOTE: max frames in flight (n frames being queued on the GPU while we're working on the n+1 frame).
//      for triple buffering, there's 2 frames being queued on the GPU while we're working on the 3rd frame
static const int OC_MTL_MAX_FRAMES_IN_FLIGHT = 2;

oc_surface oc_mtl_surface_create_for_window(oc_window window)
{
    oc_mtl_surface* surface = 0;
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        surface = (oc_mtl_surface*)oc_malloc_type(oc_mtl_surface);

        oc_surface_base_init_for_window((oc_surface_base*)surface, windowData);

        surface->base.api = OC_SURFACE_METAL;
        surface->base.destroy = oc_mtl_surface_destroy;

        @autoreleasepool
        {
            //NOTE(martin): create metal layer

            surface->mtlLayer = [CAMetalLayer layer];
            [surface->mtlLayer retain];

            //TODO: surface-api-rework: we have to do this on user side now. surface->mtlLayer.device = surface->device;

            [surface->mtlLayer setOpaque:NO];
            surface->mtlLayer.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
            [surface->base.view.layer addSublayer:(CALayer*)surface->mtlLayer];

            //-----------------------------------------------------------
            //NOTE(martin): set the size and scaling
            //-----------------------------------------------------------
            NSRect frame = [[windowData->osx.nsWindow contentView] frame];
            CGSize size = frame.size;
            surface->mtlLayer.frame = (CGRect){ { 0, 0 }, size };
            size.width *= OC_MTL_SURFACE_CONTENTS_SCALING;
            size.height *= OC_MTL_SURFACE_CONTENTS_SCALING;
            surface->mtlLayer.drawableSize = size;
            surface->mtlLayer.contentsScale = OC_MTL_SURFACE_CONTENTS_SCALING;

            surface->mtlLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        }
    }
    oc_surface handle = oc_surface_nil();
    if(surface)
    {
        handle = oc_surface_handle_alloc((oc_surface_base*)surface);
    }
    return (handle);
}

void* oc_mtl_surface_layer(oc_surface handle)
{
    oc_surface_base* base = oc_surface_from_handle(handle);
    if(base && base->api == OC_SURFACE_METAL)
    {
        oc_mtl_surface* surface = (oc_mtl_surface*)base;
        return (surface->mtlLayer);
    }
    else
    {
        return (nil);
    }
}
