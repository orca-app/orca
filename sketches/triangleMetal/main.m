/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <stdlib.h>
#include <string.h>
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

    //NOTE: create device and command queue
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    id<MTLCommandQueue> commandQueue = [device newCommandQueue];

    //NOTE: create surface
    oc_surface surface = oc_mtl_surface_create_for_window(window);
    CAMetalLayer* layer = oc_mtl_surface_layer(surface);
    layer.device = device;

    //NOTE(martin): load the library
    oc_arena_scope scratch = oc_scratch_begin();

    oc_str8 shaderPath = oc_path_executable_relative(scratch.arena, OC_STR8("triangle_shader.metallib"));
    const char* shaderPathCString = oc_str8_to_cstring(scratch.arena, shaderPath);
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

    pipelineStateDescriptor.colorAttachments[0].pixelFormat = layer.pixelFormat;

    id<MTLRenderPipelineState> pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&err];
    if(err != nil)
    {
        const char* errStr = [[err localizedDescription] UTF8String];
        printf("error : %s\n", errStr);
        return (-1);
    }

    oc_scratch_end(scratch);

    oc_window_bring_to_front(window);
    oc_window_focus(window);

    while(!oc_should_quit())
    {
        @autoreleasepool
        {

            scratch = oc_scratch_begin();
            oc_pump_events(0);
            oc_event* event = 0;
            while((event = oc_next_event(scratch.arena)) != 0)
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
            }

            vector_uint2 viewportSize;
            viewportSize.x = 800;
            viewportSize.y = 600;

            id<CAMetalDrawable> drawable = [layer nextDrawable];
            id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];

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

            [commandBuffer presentDrawable:drawable];
            [commandBuffer commit];

            oc_scratch_end(scratch);
        }
    }

    oc_terminate();

    return (0);
}
