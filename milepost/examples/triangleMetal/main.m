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
#include"mtl_surface.h"

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
	mg_surface surface = mg_surface_create_for_window(window, MG_BACKEND_METAL);

	//NOTE(martin): load the library
	id<MTLDevice> device = MTLCreateSystemDefaultDevice();

	str8 shaderPath = path_executable_relative(mem_scratch(), STR8("triangle_shader.metallib"));
	const char* shaderPathCString = str8_to_cstring(mem_scratch(), shaderPath);
	NSString* metalFileName = [[NSString alloc] initWithCString: shaderPathCString encoding: NSUTF8StringEncoding];
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

	CAMetalLayer* layer = mg_mtl_surface_layer(surface);
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

				default:
					break;
			}
		}

		vector_uint2 viewportSize;
		viewportSize.x = 800;
		viewportSize.y = 600;

		mg_surface_prepare(surface);
			id<CAMetalDrawable> drawable = mg_mtl_surface_drawable(surface);
			id<MTLCommandBuffer> commandBuffer = mg_mtl_surface_command_buffer(surface);

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
