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
#include "graphics_surface.h"
#include "util/macros.h"

typedef struct oc_mtl_surface
{
    oc_surface_data interface;

    // permanent mtl resources
    id<MTLDevice> device;
    CAMetalLayer* mtlLayer;
    id<MTLCommandQueue> commandQueue;

    // transient metal resources
    id<CAMetalDrawable> drawable;
    id<MTLCommandBuffer> commandBuffer;

} oc_mtl_surface;

void oc_mtl_surface_destroy(oc_surface_data* interface)
{
    oc_mtl_surface* surface = (oc_mtl_surface*)interface;

    @autoreleasepool
    {
        //NOTE: when GPU frame capture is enabled, if we release resources before all work is completed,
        //      libMetalCapture crashes... the following hack avoids this crash by enqueuing a last (empty)
        //      command buffer and waiting for it to complete, ensuring all previous buffers have completed
        id<MTLCommandBuffer> endBuffer = [surface->commandQueue commandBuffer];
        [endBuffer commit];
        [endBuffer waitUntilCompleted];

        [surface->commandQueue release];
        [surface->mtlLayer removeFromSuperlayer];
        [surface->mtlLayer release];
        [surface->device release];
    }
    //NOTE: we don't use oc_layer_cleanup here, because the CAMetalLayer is taken care off by the surface itself
}

void oc_mtl_surface_acquire_command_buffer(oc_mtl_surface* surface)
{
    @autoreleasepool
    {
        if(surface->commandBuffer == nil)
        {
            surface->commandBuffer = [surface->commandQueue commandBuffer];
            [surface->commandBuffer retain];
        }
    }
}

void oc_mtl_surface_acquire_drawable(oc_mtl_surface* surface)
{
    @autoreleasepool
    {
        /*WARN(martin):
		//TODO: we should stop trying to render if we detect that the app is in the background
		or occluded

		//TODO: We should set a reasonable timeout and skip the frame and log an error in case we are stalled
		for too long.
	*/

        //NOTE: returned drawable could be nil if we stall for more than 1s, although that never seem to happen in practice?
        if(surface->drawable == nil)
        {
            surface->drawable = [surface->mtlLayer nextDrawable];
            if(surface->drawable)
            {
                [surface->drawable retain];
            }
        }
    }
}

void oc_mtl_surface_prepare(oc_surface_data* interface)
{
    oc_mtl_surface* surface = (oc_mtl_surface*)interface;
    oc_mtl_surface_acquire_command_buffer(surface);
}

void oc_mtl_surface_present(oc_surface_data* interface)
{
    oc_mtl_surface* surface = (oc_mtl_surface*)interface;
    @autoreleasepool
    {
        if(surface->commandBuffer != nil)
        {
            if(surface->drawable != nil)
            {

                [surface->commandBuffer commit];
                [surface->commandBuffer waitUntilScheduled];
                [surface->drawable present];

                [surface->drawable release];
                surface->drawable = nil;
            }
            [surface->commandBuffer release];
            surface->commandBuffer = nil;
        }
    }
}

void oc_mtl_surface_swap_interval(oc_surface_data* interface, int swap)
{
    oc_mtl_surface* surface = (oc_mtl_surface*)interface;
    @autoreleasepool
    {
        [surface->mtlLayer setDisplaySyncEnabled:(swap ? YES : NO)];
    }
}

/*
void oc_mtl_surface_set_frame(oc_surface_data* interface, oc_rect frame)
{
	oc_mtl_surface* surface = (oc_mtl_surface*)interface;
	oc_osx_surface_set_frame(interface, frame);
	oc_vec2 scale = oc_osx_surface_contents_scaling(interface);

	CGRect cgFrame = {{frame.x, frame.y}, {frame.w, frame.h}};

//	dispatch_async(dispatch_get_main_queue(),
//		^{
			[CATransaction begin];
			[CATransaction setDisableActions:YES];
			[surface->mtlLayer setFrame: cgFrame];
			[CATransaction commit];
//		 });

	CGSize drawableSize = (CGSize){.width = frame.w * scale.x, .height = frame.h * scale.y};
	surface->mtlLayer.drawableSize = drawableSize;
}
*/

//TODO fix that according to real scaling, depending on the monitor settings
static const f32 OC_MTL_SURFACE_CONTENTS_SCALING = 2;

//NOTE: max frames in flight (n frames being queued on the GPU while we're working on the n+1 frame).
//      for triple buffering, there's 2 frames being queued on the GPU while we're working on the 3rd frame
static const int OC_MTL_MAX_FRAMES_IN_FLIGHT = 2;

oc_surface_data* oc_mtl_surface_create_for_window(oc_window window)
{
    oc_mtl_surface* surface = 0;
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        surface = (oc_mtl_surface*)malloc(sizeof(oc_mtl_surface));

        oc_surface_init_for_window((oc_surface_data*)surface, windowData);

        //NOTE(martin): setup interface functions
        surface->interface.api = OC_METAL;
        surface->interface.destroy = oc_mtl_surface_destroy;
        surface->interface.prepare = oc_mtl_surface_prepare;
        surface->interface.deselect = 0;
        surface->interface.present = oc_mtl_surface_present;
        surface->interface.swapInterval = oc_mtl_surface_swap_interval;

        @autoreleasepool
        {
            //-----------------------------------------------------------
            //NOTE(martin): create a mtl device and a mtl layer and
            //-----------------------------------------------------------

            /*WARN(martin):
				Calling MTLCreateDefaultSystemDevice(), as advised by the doc, hangs Discord's screen sharing...
				The workaround I found, which doesn't make sense, is to set the mtlLayer.device to mtlLayer.preferredDevice,
				even if mtlLayer.preferredDevice is the same value as returned by MTLCreateDefaultSystemDevice().
				This works for now and allows screen sharing while using orca, but we'll have to revisit this when we want
				more control over what GPU gets used.
			*/
            surface->device = nil;

            //Select a discrete GPU, if possible
            NSArray<id<MTLDevice>>* devices = MTLCopyAllDevices();
            for(id<MTLDevice> device in devices)
            {
                if(!device.isRemovable && !device.isLowPower)
                {
                    surface->device = device;
                    break;
                }
            }
            if(surface->device == nil)
            {
                oc_log_warning("Couldn't select a discrete GPU, using first available device\n");
                surface->device = devices[0];
            }

            //surface->device = MTLCreateSystemDefaultDevice();

            surface->mtlLayer = [CAMetalLayer layer];
            [surface->mtlLayer retain];
            surface->mtlLayer.device = surface->device;

            [surface->mtlLayer setOpaque:NO];
            surface->mtlLayer.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
            surface->mtlLayer.presentsWithTransaction = YES;
            [surface->interface.layer.caLayer addSublayer:(CALayer*)surface->mtlLayer];

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

            //-----------------------------------------------------------
            //NOTE(martin): create a command queue
            //-----------------------------------------------------------
            surface->commandQueue = [surface->device newCommandQueue];
            [surface->commandQueue retain];

            //NOTE(martin): command buffer and drawable are set on demand and at the end of each present() call
            surface->drawable = nil;
            surface->commandBuffer = nil;
        }
    }
    return ((oc_surface_data*)surface);
}

void* oc_mtl_surface_layer(oc_surface surface)
{
    oc_surface_data* surfaceData = oc_surface_data_from_handle(surface);
    if(surfaceData && surfaceData->api == OC_METAL)
    {
        oc_mtl_surface* mtlSurface = (oc_mtl_surface*)surfaceData;
        return (mtlSurface->mtlLayer);
    }
    else
    {
        return (nil);
    }
}

void* oc_mtl_surface_drawable(oc_surface surface)
{
    oc_surface_data* surfaceData = oc_surface_data_from_handle(surface);
    if(surfaceData && surfaceData->api == OC_METAL)
    {
        oc_mtl_surface* mtlSurface = (oc_mtl_surface*)surfaceData;
        oc_mtl_surface_acquire_drawable(mtlSurface);
        return (mtlSurface->drawable);
    }
    else
    {
        return (nil);
    }
}

void* oc_mtl_surface_command_buffer(oc_surface surface)
{
    oc_surface_data* surfaceData = oc_surface_data_from_handle(surface);
    if(surfaceData && surfaceData->api == OC_METAL)
    {
        oc_mtl_surface* mtlSurface = (oc_mtl_surface*)surfaceData;
        oc_mtl_surface_acquire_command_buffer(mtlSurface);
        return (mtlSurface->commandBuffer);
    }
    else
    {
        return (nil);
    }
}
