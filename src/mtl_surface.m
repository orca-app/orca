/************************************************************//**
*
*	@file: mtl_surface.m
*	@author: Martin Fouilleul
*	@date: 12/07/2023
*	@revision:
*
*****************************************************************/
#import<Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#import<QuartzCore/CAMetalLayer.h>
#include<simd/simd.h>

#include"graphics_internal.h"
#include"macro_helpers.h"
#include"osx_app.h"

#define LOG_SUBSYSTEM "Graphics"

typedef struct mg_mtl_surface
{
	mg_surface_data interface;

	// permanent mtl resources
	id<MTLDevice> device;
	CAMetalLayer* mtlLayer;
	id<MTLCommandQueue> commandQueue;

	// transient metal resources
	id<CAMetalDrawable>  drawable;
	id<MTLCommandBuffer> commandBuffer;

} mg_mtl_surface;

void mg_mtl_surface_destroy(mg_surface_data* interface)
{
	mg_mtl_surface* surface = (mg_mtl_surface*)interface;

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
	//NOTE: we don't use mp_layer_cleanup here, because the CAMetalLayer is taken care off by the surface itself
}

void mg_mtl_surface_acquire_command_buffer(mg_mtl_surface* surface)
{
	if(surface->commandBuffer == nil)
	{
		surface->commandBuffer = [surface->commandQueue commandBuffer];
		[surface->commandBuffer retain];
	}
}

void mg_mtl_surface_acquire_drawable(mg_mtl_surface* surface)
{@autoreleasepool{
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
}}

void mg_mtl_surface_prepare(mg_surface_data* interface)
{
	mg_mtl_surface* surface = (mg_mtl_surface*)interface;
	mg_mtl_surface_acquire_command_buffer(surface);
}

void mg_mtl_surface_present(mg_surface_data* interface)
{
	mg_mtl_surface* surface = (mg_mtl_surface*)interface;
	@autoreleasepool
	{
		if(surface->commandBuffer != nil)
		{
			if(surface->drawable != nil)
			{
				[surface->commandBuffer presentDrawable: surface->drawable];
				[surface->drawable release];
				surface->drawable = nil;
			}
			[surface->commandBuffer commit];
			[surface->commandBuffer release];
			surface->commandBuffer = nil;
		}
	}
}

void mg_mtl_surface_swap_interval(mg_surface_data* interface, int swap)
{
	mg_mtl_surface* surface = (mg_mtl_surface*)interface;
	@autoreleasepool
	{
		[surface->mtlLayer setDisplaySyncEnabled: (swap ? YES : NO)];
	}
}

void mg_mtl_surface_set_frame(mg_surface_data* interface, mp_rect frame)
{
	mg_mtl_surface* surface = (mg_mtl_surface*)interface;
	mg_osx_surface_set_frame(interface, frame);
	vec2 scale = mg_osx_surface_contents_scaling(interface);

	CGSize drawableSize = (CGSize){.width = frame.w * scale.x, .height = frame.h * scale.y};
	surface->mtlLayer.drawableSize = drawableSize;
}


//TODO fix that according to real scaling, depending on the monitor settings
static const f32 MG_MTL_SURFACE_CONTENTS_SCALING = 2;

//NOTE: max frames in flight (n frames being queued on the GPU while we're working on the n+1 frame).
//      for triple buffering, there's 2 frames being queued on the GPU while we're working on the 3rd frame
static const int MG_MTL_MAX_FRAMES_IN_FLIGHT = 2;

mg_surface_data* mg_mtl_surface_create_for_window(mp_window window)
{
	mg_mtl_surface* surface = 0;
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		surface = (mg_mtl_surface*)malloc(sizeof(mg_mtl_surface));

		mg_surface_init_for_window((mg_surface_data*)surface, windowData);

		//NOTE(martin): setup interface functions
		surface->interface.backend = MG_BACKEND_METAL;
		surface->interface.destroy = mg_mtl_surface_destroy;
		surface->interface.prepare = mg_mtl_surface_prepare;
		surface->interface.present = mg_mtl_surface_present;
		surface->interface.swapInterval = mg_mtl_surface_swap_interval;

		surface->interface.setFrame = mg_mtl_surface_set_frame;

		@autoreleasepool
		{
			//-----------------------------------------------------------
			//NOTE(martin): create a mtl device and a mtl layer and
			//-----------------------------------------------------------

			surface->device = MTLCreateSystemDefaultDevice();
			[surface->device retain];
			surface->mtlLayer = [CAMetalLayer layer];
			[surface->mtlLayer retain];

			surface->mtlLayer.device = surface->device;
			[surface->mtlLayer setOpaque:NO];

			[surface->interface.layer.caLayer addSublayer: (CALayer*)surface->mtlLayer];

			//-----------------------------------------------------------
			//NOTE(martin): set the size and scaling
			//-----------------------------------------------------------
			NSRect frame = [[windowData->osx.nsWindow contentView] frame];
			CGSize size = frame.size;
			surface->mtlLayer.frame = (CGRect){{0, 0}, size};
			size.width *= MG_MTL_SURFACE_CONTENTS_SCALING;
			size.height *= MG_MTL_SURFACE_CONTENTS_SCALING;
			surface->mtlLayer.drawableSize = size;
			surface->mtlLayer.contentsScale = MG_MTL_SURFACE_CONTENTS_SCALING;

			surface->mtlLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;

			//NOTE(martin): handling resizing
			surface->mtlLayer.autoresizingMask = kCALayerHeightSizable | kCALayerWidthSizable;
			surface->mtlLayer.needsDisplayOnBoundsChange = YES;

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
	return((mg_surface_data*)surface);
}

void* mg_mtl_surface_layer(mg_surface surface)
{
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->backend == MG_BACKEND_METAL)
	{
		mg_mtl_surface* mtlSurface = (mg_mtl_surface*)surfaceData;
		return(mtlSurface->mtlLayer);
	}
	else
	{
		return(nil);
	}
}

void* mg_mtl_surface_drawable(mg_surface surface)
{
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->backend == MG_BACKEND_METAL)
	{
		mg_mtl_surface* mtlSurface = (mg_mtl_surface*)surfaceData;
		mg_mtl_surface_acquire_drawable(mtlSurface);
		return(mtlSurface->drawable);
	}
	else
	{
		return(nil);
	}
}

void* mg_mtl_surface_command_buffer(mg_surface surface)
{
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->backend == MG_BACKEND_METAL)
	{
		mg_mtl_surface* mtlSurface = (mg_mtl_surface*)surfaceData;
		mg_mtl_surface_acquire_command_buffer(mtlSurface);
		return(mtlSurface->commandBuffer);
	}
	else
	{
		return(nil);
	}
}


#undef LOG_SUBSYSTEM
