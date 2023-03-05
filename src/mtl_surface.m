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

static const u32 MP_MTL_MAX_DRAWABLES_IN_FLIGHT = 3;

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

	dispatch_semaphore_t drawableSemaphore;

} mg_mtl_surface;

void mg_mtl_surface_destroy(mg_surface_data* interface)
{
	mg_mtl_surface* surface = (mg_mtl_surface*)interface;

	@autoreleasepool
	{
		[surface->commandQueue release];
		[surface->mtlLayer removeFromSuperlayer];
		[surface->mtlLayer release];
		[surface->device release];
	}
	//NOTE: we don't use mp_layer_cleanup here, because the CAMetalLayer is taken care off by the surface itself
}

void mg_mtl_surface_acquire_drawable_and_command_buffer(mg_mtl_surface* surface)
{@autoreleasepool{
	/*WARN(martin): this is super important
		When the app is put in the background, it seems that if there are buffers in flight, the drawables to
		can be leaked. This causes the gpu to allocate more and more drawables, until the app crashes.
		(note: the drawable objects themselves are released once the app comes back to the forefront, but the
		memory allocated in the GPU is never freed...)

		In background the gpu seems to create drawable if none is available instead of actually
		blocking on nextDrawable. These drawable never get freed.
		This is not a problem if our shader is fast enough, since a previous drawable becomes
		available before we finish the frame. But we want to protect against it anyway

		The normal blocking mechanism of nextDrawable seems useless, so we implement our own scheme by
		counting the number of drawables available with a semaphore that gets decremented here and
		incremented in the presentedHandler of the drawable.
		Thus we ensure that we don't consume more drawables than we are able to draw.

		//TODO: we _also_ should stop trying to render if we detect that the app is in the background
		or occluded, but we can't count only on this because there is a potential race between the
		notification of background mode and the rendering.

		//TODO: We should set a reasonable timeout and skip the frame and log an error in case we are stalled
		for too long.
	*/
	dispatch_semaphore_wait(surface->drawableSemaphore, DISPATCH_TIME_FOREVER);

	surface->drawable = [surface->mtlLayer nextDrawable];
	ASSERT(surface->drawable != nil);

	//TODO: make this a weak reference if we use ARC
	dispatch_semaphore_t semaphore = surface->drawableSemaphore;
	[surface->drawable addPresentedHandler:^(id<MTLDrawable> drawable){
		dispatch_semaphore_signal(semaphore);
		}];

	//NOTE(martin): create a command buffer
	surface->commandBuffer = [surface->commandQueue commandBuffer];

	[surface->commandBuffer retain];
	[surface->drawable retain];
}}

void mg_mtl_surface_prepare(mg_surface_data* interface)
{
	mg_mtl_surface* surface = (mg_mtl_surface*)interface;
	mg_mtl_surface_acquire_drawable_and_command_buffer(surface);
}

void mg_mtl_surface_present(mg_surface_data* interface)
{
	mg_mtl_surface* surface = (mg_mtl_surface*)interface;
	@autoreleasepool
	{
		//NOTE(martin): present drawable and commit command buffer
		[surface->commandBuffer presentDrawable: surface->drawable];
		[surface->commandBuffer commit];
		[surface->commandBuffer waitUntilCompleted];

		//NOTE(martin): acquire next frame resources
		[surface->commandBuffer release];
		surface->commandBuffer = nil;
		[surface->drawable release];
		surface->drawable = nil;
	}
}

void mg_mtl_surface_swap_interval(mg_surface_data* interface, int swap)
{
	mg_mtl_surface* surface = (mg_mtl_surface*)interface;

	////////////////////////////////////////////////////////////////
	//TODO
	////////////////////////////////////////////////////////////////
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
			surface->drawableSemaphore = dispatch_semaphore_create(MP_MTL_MAX_DRAWABLES_IN_FLIGHT);

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
		return(mtlSurface->commandBuffer);
	}
	else
	{
		return(nil);
	}
}


#undef LOG_SUBSYSTEM
