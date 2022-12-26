/************************************************************//**
*
*	@file: graphics.m
*	@author: Martin Fouilleul
*	@date: 12/07/2020
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

static const u32 MP_METAL_MAX_DRAWABLES_IN_FLIGHT = 3;

typedef struct mg_metal_surface
{
	mg_surface_info interface;

	// permanent metal resources
	NSView* view;
	id<MTLDevice> device;
	CAMetalLayer* metalLayer;
	id<MTLCommandQueue> commandQueue;

	// transient metal resources
	id<CAMetalDrawable>  drawable;
	id<MTLCommandBuffer> commandBuffer;

	dispatch_semaphore_t drawableSemaphore;

} mg_metal_surface;

void mg_metal_surface_acquire_drawable_and_command_buffer(mg_metal_surface* surface)
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

	surface->drawable = [surface->metalLayer nextDrawable];
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

void mg_metal_surface_prepare(mg_surface_info* interface)
{
	mg_metal_surface* surface = (mg_metal_surface*)interface;
	mg_metal_surface_acquire_drawable_and_command_buffer(surface);

	@autoreleasepool
	{
		MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
		renderPassDescriptor.colorAttachments[0].texture = surface->drawable.texture;
		renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
		renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.0,0.0,0.0,0.0);

		id<MTLRenderCommandEncoder> renderEncoder = [surface->commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
		[renderEncoder endEncoding];
	}
}

void mg_metal_surface_present(mg_surface_info* interface)
{
	mg_metal_surface* surface = (mg_metal_surface*)interface;
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

void mg_metal_surface_destroy(mg_surface_info* interface)
{
	mg_metal_surface* surface = (mg_metal_surface*)interface;

	@autoreleasepool
	{
		[surface->commandQueue release];
		[surface->metalLayer release];
		[surface->device release];
	}
}

void mg_metal_surface_resize(mg_surface_info* interface, u32 width, u32 height)
{
	mg_metal_surface* surface = (mg_metal_surface*)interface;

	@autoreleasepool
	{
		//TODO(martin): actually detect scaling
		CGRect frame = {{0, 0}, {width, height}};
		[surface->view setFrame: frame];
		f32 scale = surface->metalLayer.contentsScale;
		CGSize drawableSize = (CGSize){.width = width * scale, .height = height * scale};
		surface->metalLayer.drawableSize = drawableSize;
	}
}

void mg_metal_surface_set_hidden(mg_surface_info* interface, bool hidden)
{
	mg_metal_surface* surface = (mg_metal_surface*)interface;
	@autoreleasepool
	{
		[CATransaction begin];
		[CATransaction setDisableActions:YES];
		[surface->metalLayer setHidden:hidden];
		[CATransaction commit];
	}
}

vec2 mg_metal_surface_get_size(mg_surface_info* interface)
{
	mg_metal_surface* surface = (mg_metal_surface*)interface;

	@autoreleasepool
	{
		//TODO(martin): actually detect scaling
		CGRect frame = surface->view.frame;
		return((vec2){frame.size.width, frame.size.height});
	}
}

void* mg_metal_surface_get_os_resource(mg_surface_info* interface)
{
	mg_metal_surface* surface = (mg_metal_surface*)interface;
	return((void*)surface->metalLayer);
}

static const f32 MG_METAL_SURFACE_CONTENTS_SCALING = 2;

mg_surface mg_metal_surface_create_for_window(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(!windowData)
	{
		return(mg_surface_nil());
	}
	else
	{
		mg_metal_surface* surface = (mg_metal_surface*)malloc(sizeof(mg_metal_surface));

		//NOTE(martin): setup interface functions
		surface->interface.backend = MG_BACKEND_METAL;
		surface->interface.destroy = mg_metal_surface_destroy;
		surface->interface.prepare = mg_metal_surface_prepare;
		surface->interface.present = mg_metal_surface_present;
		surface->interface.resize = mg_metal_surface_resize;
		surface->interface.setHidden = mg_metal_surface_set_hidden;
		surface->interface.getSize = mg_metal_surface_get_size;
		surface->interface.getOSResource = mg_metal_surface_get_os_resource;

		@autoreleasepool
		{
			NSRect frame = [[windowData->osx.nsWindow contentView] frame];
			surface->view = [[NSView alloc] initWithFrame: frame];
			[surface->view setWantsLayer:YES];

			[[windowData->osx.nsWindow contentView] addSubview: surface->view];

			surface->drawableSemaphore = dispatch_semaphore_create(MP_METAL_MAX_DRAWABLES_IN_FLIGHT);

			//-----------------------------------------------------------
			//NOTE(martin): create a metal device and a metal layer and
			//-----------------------------------------------------------
			surface->device = MTLCreateSystemDefaultDevice();
			[surface->device retain];
			surface->metalLayer = [CAMetalLayer layer];
			[surface->metalLayer retain];
			[surface->metalLayer setOpaque:NO];

			surface->metalLayer.device = surface->device;

			//-----------------------------------------------------------
			//NOTE(martin): set the size and scaling
			//-----------------------------------------------------------
			CGSize size = surface->view.bounds.size;
			size.width *= MG_METAL_SURFACE_CONTENTS_SCALING;
			size.height *= MG_METAL_SURFACE_CONTENTS_SCALING;
			surface->metalLayer.drawableSize = size;
			surface->metalLayer.contentsScale = MG_METAL_SURFACE_CONTENTS_SCALING;

			surface->metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;

			surface->view.wantsLayer = YES;
			surface->view.layer = surface->metalLayer;

			//NOTE(martin): handling resizing
			surface->view.layerContentsRedrawPolicy = NSViewLayerContentsRedrawDuringViewResize;
			surface->metalLayer.autoresizingMask = kCALayerHeightSizable | kCALayerWidthSizable;
			surface->metalLayer.needsDisplayOnBoundsChange = YES;

			//-----------------------------------------------------------
			//NOTE(martin): create a command queue
			//-----------------------------------------------------------
			surface->commandQueue = [surface->device newCommandQueue];
			[surface->commandQueue retain];

			//NOTE(martin): command buffer and drawable are set on demand and at the end of each present() call
			surface->drawable = nil;
			surface->commandBuffer = nil;
		}

		mg_surface handle = mg_surface_alloc_handle((mg_surface_info*)surface);
		return(handle);
	}
}

void* mg_metal_surface_layer(mg_surface surface)
{
	mg_surface_info* surfaceData = mg_surface_ptr_from_handle(surface);
	if(surfaceData && surfaceData->backend == MG_BACKEND_METAL)
	{
		mg_metal_surface* metalSurface = (mg_metal_surface*)surfaceData;
		return(metalSurface->metalLayer);
	}
	else
	{
		return(nil);
	}
}

void* mg_metal_surface_drawable(mg_surface surface)
{
	mg_surface_info* surfaceData = mg_surface_ptr_from_handle(surface);
	if(surfaceData && surfaceData->backend == MG_BACKEND_METAL)
	{
		mg_metal_surface* metalSurface = (mg_metal_surface*)surfaceData;
		return(metalSurface->drawable);
	}
	else
	{
		return(nil);
	}
}

void* mg_metal_surface_command_buffer(mg_surface surface)
{
	mg_surface_info* surfaceData = mg_surface_ptr_from_handle(surface);
	if(surfaceData && surfaceData->backend == MG_BACKEND_METAL)
	{
		mg_metal_surface* metalSurface = (mg_metal_surface*)surfaceData;
		return(metalSurface->commandBuffer);
	}
	else
	{
		return(nil);
	}
}


#undef LOG_SUBSYSTEM
