/************************************************************/ /**
*
*	@file: main.cpp
*	@author: Martin Fouilleul
*	@date: 30/07/2022
*	@revision:
*
*****************************************************************/
#include <stdlib.h>
#include <string.h>

#define _USE_MATH_DEFINES //NOTE: necessary for MSVC
#include <math.h>

#include "orca.h"
#include "graphics/mtl_surface.h"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include "vertex.h"

static const my_vertex triangle[3] = { { { 250, -250 }, { 1, 0, 0, 1 } },
                                       { { -250, -250 }, { 0, 1, 0, 1 } },
                                       { { 0, 250 }, { 0, 0, 1, 1 } } };

int main()
{
    oc_init();

    oc_rect rect = { .x = 100, .y = 100, .w = 800, .h = 600 };
    oc_window window = oc_window_create(rect, OC_STR8("test"), 0);

    //NOTE: create surface
    oc_surface surface = oc_surface_create_for_window(window, OC_METAL);

    //NOTE(martin): load the library
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();

    oc_str8 shaderPath = oc_path_executable_relative(oc_scratch(), OC_STR8("triangle_shader.metallib"));
    const char* shaderPathCString = oc_str8_to_cstring(oc_scratch(), shaderPath);
    NSString* metalFileName = [[NSString alloc] initWithCString:shaderPathCString encoding:NSUTF8StringEncoding];
    NSError* err = 0;
    id<MTLLibrary> library = [device newLibraryWithFile:metalFileName error:&err];
    if(err != nil)
    {
        const char* errStr = [[err localizedDescription] UTF8String];
        printf("error : %s\n", errStr);
        return (-1);
    }
    id<MTLFunction> vertexFunction = [library newFunctionWithName:@"VertexShader"];
    id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"FragmentShader"];

    //NOTE(martin): create a render pipeline
    MTLRenderPipelineDescriptor* pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.label = @"My simple pipeline";
    pipelineStateDescriptor.vertexFunction = vertexFunction;
    pipelineStateDescriptor.fragmentFunction = fragmentFunction;

    CAMetalLayer* layer = oc_mtl_surface_layer(surface);
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = layer.pixelFormat;

    id<MTLRenderPipelineState> pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&err];
    if(err != nil)
    {
        const char* errStr = [[err localizedDescription] UTF8String];
        printf("error : %s\n", errStr);
        return (-1);
    }

    // start app

    oc_window_bring_to_front(window);
    oc_window_focus(window);

    while(!oc_should_quit())
    {
        oc_pump_events(0);
        oc_event* event = 0;
        while((event = oc_next_event(oc_scratch())) != 0)
        {
            switch(event->type)
            {
                case OC_EVENT_WINDOW_CLOSE:
                {
                    oc_request_quit();
                }
                break;

                default:
                    break;
            }

            oc_arena_clear(oc_scratch());
        }

        vector_uint2 viewportSize;
        viewportSize.x = 800;
        viewportSize.y = 600;

        oc_surface_select(surface);
        id<CAMetalDrawable> drawable = oc_mtl_surface_drawable(surface);
        id<MTLCommandBuffer> commandBuffer = oc_mtl_surface_command_buffer(surface);

        MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
        renderPassDescriptor.colorAttachments[0].texture = drawable.texture;
        renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
        id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];

        //Set the pipeline state
        [encoder setRenderPipelineState:pipelineState];

        //Send data to the shader and add draw call
        [encoder setVertexBytes:triangle length:sizeof(triangle) atIndex:vertexInputIndexVertices];
        [encoder setVertexBytes:&viewportSize length:sizeof(viewportSize) atIndex:vertexInputIndexViewportSize];
        [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
        [encoder endEncoding];

        oc_surface_present(surface);
    }

    oc_terminate();

    return (0);
}
