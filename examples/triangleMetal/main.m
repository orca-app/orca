/************************************************************//**
*
*	@file: main.cpp
*	@author: Martin Fouilleul
*	@date: 30/07/2022
*	@revision:
*
*****************************************************************/
#include<stdlib.h>
#include<string.h>

#define _USE_MATH_DEFINES //NOTE: necessary for MSVC
#include<math.h>

#include"milepost.h"
#include"metal_surface.h"

#define LOG_SUBSYSTEM "Main"

#import<Metal/Metal.h>
#import<QuartzCore/CAMetalLayer.h>

#include"vertex.h"

static const my_vertex triangle[3] = {{{250, -250},{1, 0, 0, 1}},
                                      {{-250, -250},{0, 1, 0, 1}},
				                      {{0, 250},{0, 0, 1, 1}}};

int main()
{
	LogLevel(LOG_LEVEL_DEBUG);

	mp_init();

	mp_rect rect = {.x = 100, .y = 100, .w = 800, .h = 600};
	mp_window window = mp_window_create(rect, "test", 0);

	//NOTE: create surface
	mg_surface surface = mg_metal_surface_create_for_window(window);

	//NOTE(martin): load the library
	id<MTLDevice> device = MTLCreateSystemDefaultDevice();

	const char* shaderPath = "shader.metallib";
	NSString* metalFileName = [[NSString alloc] initWithCString: shaderPath encoding: NSUTF8StringEncoding];
	NSError* err = 0;
	id<MTLLibrary> library = [device newLibraryWithFile: metalFileName error:&err];
	if(err != nil)
	{
		const char* errStr = [[err localizedDescription] UTF8String];
		printf("error : %s\n", errStr);
		return(-1);
	}
	id<MTLFunction> vertexFunction = [library newFunctionWithName:@"VertexShader"];
	id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"FragmentShader"];

	//NOTE(martin): create a render pipeline
	MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
	pipelineStateDescriptor.label = @"My simple pipeline";
	pipelineStateDescriptor.vertexFunction = vertexFunction;
	pipelineStateDescriptor.fragmentFunction = fragmentFunction;

	CAMetalLayer* layer = mg_metal_surface_layer(surface);
	pipelineStateDescriptor.colorAttachments[0].pixelFormat = layer.pixelFormat;

	id<MTLRenderPipelineState> pipelineState = [device newRenderPipelineStateWithDescriptor: pipelineStateDescriptor error:&err];
	if(err != nil)
	{
		const char* errStr = [[err localizedDescription] UTF8String];
		printf("error : %s\n", errStr);
		return(-1);
	}

	// start app

	mp_window_bring_to_front(window);
	mp_window_focus(window);

	while(!mp_should_quit())
	{
		mp_pump_events(0);
		mp_event event = {0};
		while(mp_next_event(&event))
		{
			switch(event.type)
			{
				case MP_EVENT_WINDOW_CLOSE:
				{
					mp_request_quit();
				} break;

				case MP_EVENT_WINDOW_RESIZE:
				{
					printf("resized, rect = {%f, %f, %f, %f}\n",
					       event.frame.rect.x,
					       event.frame.rect.y,
					       event.frame.rect.w,
					       event.frame.rect.h);
				} break;

				case MP_EVENT_WINDOW_MOVE:
				{
					printf("moved, rect = {%f, %f, %f, %f}\n",
					       event.frame.rect.x,
					       event.frame.rect.y,
					       event.frame.rect.w,
					       event.frame.rect.h);
				} break;

				case MP_EVENT_MOUSE_MOVE:
				{
					printf("mouse moved, pos = {%f, %f}, delta = {%f, %f}\n",
					       event.move.x,
					       event.move.y,
					       event.move.deltaX,
					       event.move.deltaY);
				} break;

				case MP_EVENT_MOUSE_WHEEL:
				{
					printf("mouse wheel, delta = {%f, %f}\n",
					       event.move.deltaX,
					       event.move.deltaY);
				} break;

				case MP_EVENT_MOUSE_ENTER:
				{
					printf("mouse enter\n");
				} break;

				case MP_EVENT_MOUSE_LEAVE:
				{
					printf("mouse leave\n");
				} break;

				case MP_EVENT_MOUSE_BUTTON:
				{
					printf("mouse button %i: %i\n",
					       event.key.code,
					       event.key.action == MP_KEY_PRESS ? 1 : 0);
				} break;

				case MP_EVENT_KEYBOARD_KEY:
				{
					printf("key %i: %s\n",
					        event.key.code,
					        event.key.action == MP_KEY_PRESS ? "press" : (event.key.action == MP_KEY_RELEASE ? "release" : "repeat"));
				} break;

				case MP_EVENT_KEYBOARD_CHAR:
				{
					printf("entered char %s\n", event.character.sequence);
				} break;

				default:
					break;
			}
		}

		vector_uint2 viewportSize;
		viewportSize.x = 800;
		viewportSize.y = 600;

		mg_surface_prepare(surface);
			id<CAMetalDrawable> drawable = mg_metal_surface_drawable(surface);
			id<MTLCommandBuffer> commandBuffer = mg_metal_surface_command_buffer(surface);

			MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
			renderPassDescriptor.colorAttachments[0].texture = drawable.texture;
			renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
			id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];

			//Set the pipeline state
			[encoder setRenderPipelineState:pipelineState];

			//Send data to the shader and add draw call
			[encoder setVertexBytes: triangle length:sizeof(triangle) atIndex: vertexInputIndexVertices];
			[encoder setVertexBytes: &viewportSize length:sizeof(viewportSize) atIndex: vertexInputIndexViewportSize];
			[encoder drawPrimitives: MTLPrimitiveTypeTriangle vertexStart: 0 vertexCount: 3];
			[encoder endEncoding];

		mg_surface_present(surface);
	}

	mp_terminate();

	return(0);
}
