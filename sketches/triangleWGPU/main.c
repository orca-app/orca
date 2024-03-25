/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _USE_MATH_DEFINES //NOTE: necessary for MSVC
#include <math.h>

#include "orca.h"
#include "graphics/wgpu_surface.h"

static void OnDeviceError(WGPUErrorType type, const char* message, void* userdata)
{
    oc_log_error("%s\n", message);
    exit(-1);
}

static void OnAdapterRequestEnded(WGPURequestAdapterStatus status, WGPUAdapter adapter, const char* message, void* userdata)
{
    if(status != WGPURequestAdapterStatus_Success)
    {
        // cannot find adapter?
        oc_log_error("%s\n", message);
        exit(-1);
    }
    else
    {
        // use first adapter provided
        WGPUAdapter* result = userdata;
        if(*result == NULL)
        {
            *result = adapter;
        }
    }
}

void buffer_map_callback(WGPUBufferMapAsyncStatus status, void* userdata)
{
    WGPUBuffer buffer = (WGPUBuffer)userdata;
    if(wgpuBufferGetMapState(buffer) == WGPUBufferMapState_Mapped)
    {
        const u64* data = (const u64*)wgpuBufferGetConstMappedRange(buffer, 0, 2 * sizeof(u64));
        u64 start = data[0];
        u64 end = data[1];
        oc_log_info("pass duration = %llu\n", end - start);

        wgpuBufferUnmap(buffer);
    }
}

int main()
{
    oc_init();

    oc_rect rect = { .x = 100, .y = 100, .w = 800, .h = 600 };
    oc_window window = oc_window_create(rect, OC_STR8("test"), 0);

    //NOTE: create surface

    WGPUInstance wgpuInstance = wgpuCreateInstance(NULL);
    oc_surface surface = oc_wgpu_surface_create_for_window(wgpuInstance, window);

    struct Vertex
    {
        float position[2];
        float uv[2];
        float color[3];
    };

    const uint32_t kVertexStride = sizeof(struct Vertex);
    static const struct Vertex kVertexData[] = {
        { { -0.00f, +0.75f }, { 25.0f, 50.0f }, { 1, 0, 0 } },
        { { +0.75f, -0.50f }, { 0.0f, 0.0f }, { 0, 1, 0 } },
        { { -0.75f, -0.50f }, { 50.0f, 0.0f }, { 0, 0, 1 } },
    };
    const uint32_t kTextureWidth = 2;
    const uint32_t kTextureHeight = 2;
    // ============================================================================================
    const WGPUTextureFormat kSwapChainFormat = WGPUTextureFormat_BGRA8Unorm;

    WGPUAdapter adapter = NULL;
    {
        WGPURequestAdapterOptions options = {

            //            .compatibleSurface = surface,
            .powerPreference = WGPUPowerPreference_HighPerformance,
        };
        wgpuInstanceRequestAdapter(wgpuInstance, &options, &OnAdapterRequestEnded, &adapter);
        OC_ASSERT(adapter && "Failed to get WebGPU adapter");
        // can query extra details on what adapter supports:
        // wgpuAdapterEnumerateFeatures
        // wgpuAdapterGetLimits
        // wgpuAdapterGetProperties
        // wgpuAdapterHasFeature

        if(!wgpuAdapterHasFeature(adapter, WGPUFeatureName_TimestampQuery))
        {
            oc_log_error("adapter doesn't have feature TimestampQuery\n");
        }

        {
            WGPUAdapterProperties properties = { 0 };
            wgpuAdapterGetProperties(adapter, &properties);
            const char* adapter_types[] = {
                [WGPUAdapterType_DiscreteGPU] = "Discrete GPU",
                [WGPUAdapterType_IntegratedGPU] = "Integrated GPU",
                [WGPUAdapterType_CPU] = "CPU",
                [WGPUAdapterType_Unknown] = "unknown",
            };
            printf(
                "Adapter       = %s\n"
                "Driver        = %s\n"
                "Vendor        = %s\n"
                "Architecture  = %s\n"
                "Adapter Type  = %s\n",
                properties.name,
                properties.driverDescription,
                properties.vendorName,
                properties.architecture,
                adapter_types[properties.adapterType]);
        }
    }
    WGPUDevice device = NULL;
    {
        WGPUSupportedLimits supported = { 0 };
        wgpuAdapterGetLimits(adapter, &supported);
        // override "max" limits
        supported.limits.maxTextureDimension2D = kTextureWidth;
        supported.limits.maxBindGroups = 1;
        supported.limits.maxBindingsPerBindGroup = 3; // uniform buffer for vertex shader, and texture + sampler for fragment
        supported.limits.maxSampledTexturesPerShaderStage = 1;
        supported.limits.maxSamplersPerShaderStage = 1;
        supported.limits.maxUniformBuffersPerShaderStage = 1;
        supported.limits.maxUniformBufferBindingSize = 4 * 4 * sizeof(float); // 4x4 matrix
        supported.limits.maxVertexBuffers = 1;
        supported.limits.maxBufferSize = sizeof(kVertexData);
        supported.limits.maxVertexAttributes = 3; // pos, texcoord, color
        supported.limits.maxVertexBufferArrayStride = kVertexStride;
        supported.limits.maxColorAttachments = 1;
        WGPUDeviceDescriptor desc = {
            // extra features: https://dawn.googlesource.com/dawn/+/refs/heads/main/src/dawn/native/Features.cpp
            .requiredFeatureCount = 1,
            .requiredFeatures = (WGPUFeatureName[]){ WGPUFeatureName_TimestampQuery },
            .requiredLimits = &(WGPURequiredLimits){ .limits = supported.limits },
        };
        device = wgpuAdapterCreateDevice(adapter, &desc);
        OC_ASSERT(device && "Failed to create WebGPU device");
    }
    // notify on errors
    wgpuDeviceSetUncapturedErrorCallback(device, &OnDeviceError, NULL);
    // default device queue
    WGPUQueue queue = wgpuDeviceGetQueue(device);
    // ============================================================================================
    WGPUBuffer vbuffer;
    {
        WGPUBufferDescriptor desc = {
            .usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst,
            .size = sizeof(kVertexData),
        };
        vbuffer = wgpuDeviceCreateBuffer(device, &desc);
        wgpuQueueWriteBuffer(queue, vbuffer, 0, kVertexData, sizeof(kVertexData));
    }
    // uniform buffer for one 4x4 float matrix
    WGPUBuffer ubuffer;
    {
        WGPUBufferDescriptor desc = {
            .usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst,
            .size = 4 * 4 * sizeof(float), // 4x4 matrix
        };
        ubuffer = wgpuDeviceCreateBuffer(device, &desc);
    }
    WGPUTextureView texture_view;
    {
        // checkerboard texture, with 50% transparency on black colors
        unsigned int pixels[] = {
            0x80000000,
            0xffffffff,
            0xffffffff,
            0x80000000,
        };
        WGPUTextureDescriptor desc = {
            .usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst,
            .dimension = WGPUTextureDimension_2D,
            .size = { kTextureWidth, kTextureHeight, 1 },
            .format = WGPUTextureFormat_RGBA8Unorm,
            .mipLevelCount = 1,
            .sampleCount = 1,
        };
        WGPUTexture texture = wgpuDeviceCreateTexture(device, &desc);
        // upload pixels
        WGPUImageCopyTexture dst = {
            .texture = texture,
            .mipLevel = 0,
        };
        WGPUTextureDataLayout src = {
            .offset = 0,
            .bytesPerRow = kTextureWidth * 4, // 4 bytes per pixel
            .rowsPerImage = kTextureHeight,
        };
        wgpuQueueWriteTexture(queue, &dst, pixels, sizeof(pixels), &src, &desc.size);
        WGPUTextureViewDescriptor view_desc = {
            .format = desc.format,
            .dimension = WGPUTextureViewDimension_2D,
            .baseMipLevel = 0,
            .mipLevelCount = 1,
            .baseArrayLayer = 0,
            .arrayLayerCount = 1,
            .aspect = WGPUTextureAspect_All,
        };
        texture_view = wgpuTextureCreateView(texture, &view_desc);
        wgpuTextureRelease(texture);
    }
    // compile shader
    WGPUShaderModule shaders;
    {
        static const char wgsl[] =
            "struct VertexIn {                                                   \n"
            "    @location(0) aPos : vec2f,                                      \n"
            "    @location(1) aTex : vec2f,                                      \n"
            "    @location(2) aCol : vec3f                                       \n"
            "}                                                                   \n"
            "                                                                    \n"
            "struct VertexOut {                                                  \n"
            "    @builtin(position) vPos : vec4f,                                \n"
            "    @location(0)       vTex : vec2f,                                \n"
            "    @location(1)       vCol : vec3f                                 \n"
            "}                                                                   \n"
            "                                                                    \n"
            "@group(0) @binding(0) var<uniform> uTransform : mat4x4f;            \n"
            "@group(0) @binding(1) var myTexture : texture_2d<f32>;              \n"
            "@group(0) @binding(2) var mySampler : sampler;                      \n"
            "                                                                    \n"
            "@vertex                                                             \n"
            "fn vs(in : VertexIn) -> VertexOut {                                 \n"
            "    var out : VertexOut;                                            \n"
            "    out.vPos = uTransform * vec4f(in.aPos, 0.0, 1.0);               \n"
            "    out.vTex = in.aTex;                                             \n"
            "    out.vCol = in.aCol;                                             \n"
            "    return out;                                                     \n"
            "}                                                                   \n"
            "                                                                    \n"
            "@fragment                                                           \n"
            "fn fs(in : VertexOut) -> @location(0) vec4f {                       \n"
            "    var tex : vec4f = textureSample(myTexture, mySampler, in.vTex); \n"
            "    for(var i : i32 = 0; i<1000; i++)\n"
            "    {\n"
            "    }\n"
            "    return vec4f(in.vCol, 1.0) * tex;                               \n"
            "}                                                                   \n";
        WGPUShaderModuleDescriptor desc = {
            .nextInChain = &((WGPUShaderModuleWGSLDescriptor){
                                 .chain.sType = WGPUSType_ShaderModuleWGSLDescriptor,
                                 .code = wgsl,
                             })
                                .chain,
        };
        shaders = wgpuDeviceCreateShaderModule(device, &desc);
        // to get compiler error messages explicitly use wgpuShaderModuleGetCompilationInfo
        // otherwise they will be reported with device error callback
    }
    WGPUSampler sampler;
    {
        WGPUSamplerDescriptor desc = {
            .addressModeU = WGPUAddressMode_Repeat,
            .addressModeV = WGPUAddressMode_Repeat,
            .addressModeW = WGPUAddressMode_Repeat,
            .magFilter = WGPUFilterMode_Nearest,
            .minFilter = WGPUFilterMode_Nearest,
            .mipmapFilter = WGPUMipmapFilterMode_Nearest,
            .lodMinClamp = 0.f,
            .lodMaxClamp = 1.f,
            .compare = WGPUCompareFunction_Undefined,
            .maxAnisotropy = 1,
        };
        sampler = wgpuDeviceCreateSampler(device, &desc);
    }
    WGPUBindGroup bind_group;
    WGPURenderPipeline pipeline;
    {
        WGPUBindGroupLayoutDescriptor bind_group_layout_desc = {
            .entryCount = 3,
            .entries = (WGPUBindGroupLayoutEntry[]){
                // uniform buffer for vertex shader
                {
                    .binding = 0,
                    .visibility = WGPUShaderStage_Vertex,
                    .buffer.type = WGPUBufferBindingType_Uniform,
                },
                // texture for fragment shader
                {
                    .binding = 1,
                    .visibility = WGPUShaderStage_Fragment,
                    .texture.sampleType = WGPUTextureSampleType_Float,
                    .texture.viewDimension = WGPUTextureViewDimension_2D,
                    .texture.multisampled = 0,
                },
                // sampler for fragment shader
                {
                    .binding = 2,
                    .visibility = WGPUShaderStage_Fragment,
                    .sampler.type = WGPUSamplerBindingType_Filtering,
                },
            },
        };
        WGPUBindGroupLayout bind_group_layout = wgpuDeviceCreateBindGroupLayout(device, &bind_group_layout_desc);
        WGPUPipelineLayoutDescriptor pipeline_layout_desc = {
            .bindGroupLayoutCount = 1,
            .bindGroupLayouts = (WGPUBindGroupLayout[]){ bind_group_layout },
        };
        WGPUPipelineLayout pipeline_layout = wgpuDeviceCreatePipelineLayout(device, &pipeline_layout_desc);
        WGPURenderPipelineDescriptor pipeline_desc = {
            .layout = pipeline_layout,
            // draw triangle list, no index buffer, no culling
            .primitive = {
                .topology = WGPUPrimitiveTopology_TriangleList,
                // .stripIndexFormat = WGPUIndexFormat_Uint16,
                .frontFace = WGPUFrontFace_CCW,
                .cullMode = WGPUCullMode_None,
            },
            // vertex shader
            .vertex = {
                .module = shaders,
                .entryPoint = "vs",
                .bufferCount = 1,
                .buffers = (WGPUVertexBufferLayout[]){
                    // one vertex buffer as input
                    {
                        .arrayStride = kVertexStride,
                        .stepMode = WGPUVertexStepMode_Vertex,
                        .attributeCount = 3,
                        .attributes = (WGPUVertexAttribute[]){
                            { WGPUVertexFormat_Float32x2, offsetof(struct Vertex, position), 0 },
                            { WGPUVertexFormat_Float32x2, offsetof(struct Vertex, uv), 1 },
                            { WGPUVertexFormat_Float32x3, offsetof(struct Vertex, color), 2 },
                        },
                    },
                },
            },
            // fragment shader
            .fragment = &(WGPUFragmentState){
                .module = shaders,
                .entryPoint = "fs",
                .targetCount = 1,
                .targets = (WGPUColorTargetState[]){
                    // writing to one output, with alpha-blending enabled
                    {
                        .format = kSwapChainFormat,
                        .blend = &(WGPUBlendState){
                            .color.operation = WGPUBlendOperation_Add,
                            .color.srcFactor = WGPUBlendFactor_SrcAlpha,
                            .color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha,
                            .alpha.operation = WGPUBlendOperation_Add,
                            .alpha.srcFactor = WGPUBlendFactor_SrcAlpha,
                            .alpha.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha,
                        },
                        .writeMask = WGPUColorWriteMask_All,
                    },
                },
            },
            // if depth/stencil buffer usage/testing is needed
            //.depthStencil = &(WGPUDepthStencilState) { ... },
            // no multisampling
            .multisample = {
                .count = 1,
                .mask = 0xffffffff,
                .alphaToCoverageEnabled = 0,
            },
        };
        pipeline = wgpuDeviceCreateRenderPipeline(device, &pipeline_desc);
        wgpuPipelineLayoutRelease(pipeline_layout);
        WGPUBindGroupDescriptor bind_group_desc = {
            .layout = bind_group_layout,
            .entryCount = 3,
            .entries = (WGPUBindGroupEntry[]){
                // uniform buffer for vertex shader
                { .binding = 0, .buffer = ubuffer, .offset = 0, .size = 4 * 4 * sizeof(float) },
                // texure for fragment shader
                { .binding = 1, .textureView = texture_view },
                // sampler for fragment shader
                { .binding = 2, .sampler = sampler },
            },
        };
        bind_group = wgpuDeviceCreateBindGroup(device, &bind_group_desc);
        wgpuBindGroupLayoutRelease(bind_group_layout);
    }
    // release resources that are now owned by pipeline and will not be used in this code later
    wgpuSamplerRelease(sampler);
    wgpuTextureViewRelease(texture_view);

    //timestamp query
    WGPUQuerySet querySet;
    {
        WGPUQuerySetDescriptor desc = {
            .type = WGPUQueryType_Timestamp,
            .count = 2,
        };
        querySet = wgpuDeviceCreateQuerySet(device, &desc);
    }
    WGPURenderPassTimestampWrites timestampWrites = {
        .querySet = querySet,
        .beginningOfPassWriteIndex = 0,
        .endOfPassWriteIndex = 1,
    };

    WGPUBuffer timestampsResolveBuffer;
    {
        WGPUBufferDescriptor desc = {
            .usage = WGPUBufferUsage_QueryResolve | WGPUBufferUsage_CopySrc,
            .size = sizeof(u64) * 2,
        };
        timestampsResolveBuffer = wgpuDeviceCreateBuffer(device, &desc);
    }
    WGPUBuffer timestampsReadBuffer;
    {
        WGPUBufferDescriptor desc = {
            .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_MapRead,
            .size = sizeof(u64) * 2,
        };
        timestampsReadBuffer = wgpuDeviceCreateBuffer(device, &desc);
    }

    float angle = 0;
    int current_width = 0;
    int current_height = 0;

    oc_window_bring_to_front(window);

    while(!oc_should_quit())
    {
        bool readFrameStats = false;

        oc_arena_scope scratch = oc_scratch_begin();
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

                case OC_EVENT_KEYBOARD_KEY:
                {
                    if(event->key.action == OC_KEY_PRESS && event->key.keyCode == OC_KEY_SPACE)
                    {
                        readFrameStats = true;
                    }
                }

                default:
                    break;
            }
        }

        wgpuDeviceTick(device);

        oc_vec2 scale = oc_surface_contents_scaling(surface);
        oc_vec2 size = oc_surface_get_size(surface);
        float width = size.x * scale.x;
        float height = size.y * scale.y;

        WGPUSwapChain swapChain = oc_wgpu_surface_get_swapchain(surface, device);

        // render only if window size is non-zero, which means swap chain is created
        if(swapChain)
        {
            // update 4x4 rotation matrix in uniform
            {
                angle += 2.0f * (float)M_PI / 20.0f / 60;
                angle = fmodf(angle, 2.0f * (float)M_PI);
                float aspect = (float)height / width;
                float matrix[16] = {
                    cosf(angle) * aspect,
                    -sinf(angle),
                    0,
                    0,
                    sinf(angle) * aspect,
                    cosf(angle),
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    1,
                };
                wgpuQueueWriteBuffer(queue, ubuffer, 0, matrix, sizeof(matrix));
            }
            WGPUTextureView next = wgpuSwapChainGetCurrentTextureView(swapChain);
            if(!next)
            {
                printf("Cannot acquire next swap chain texture!");
                exit(-1);
            }
            WGPURenderPassDescriptor desc = {
                .colorAttachmentCount = 1,
                .colorAttachments = (WGPURenderPassColorAttachment[]){
                    // one output to write to, initially cleared with background color
                    {
                        .view = next,
                        .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
                        .loadOp = WGPULoadOp_Clear,
                        .storeOp = WGPUStoreOp_Store,
                        .clearValue = { 0.392, 0.584, 0.929, 1.0 }, // r,g,b,a
                    },
                },
                .timestampWrites = &timestampWrites,
            };
            WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, NULL);
            // encode render pass
            WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &desc);
            {
                wgpuRenderPassEncoderSetViewport(pass, 0.f, 0.f, (float)width, (float)height, 0.f, 1.f);
                // draw the triangle using 3 vertices in vertex buffer
                wgpuRenderPassEncoderSetPipeline(pass, pipeline);
                wgpuRenderPassEncoderSetBindGroup(pass, 0, bind_group, 0, NULL);
                wgpuRenderPassEncoderSetVertexBuffer(pass, 0, vbuffer, 0, WGPU_WHOLE_SIZE);
                wgpuRenderPassEncoderDraw(pass, 3, 1, 0, 0);
            }
            wgpuRenderPassEncoderEnd(pass);

            wgpuCommandEncoderResolveQuerySet(encoder, querySet, 0, 2, timestampsResolveBuffer, 0);

            wgpuCommandEncoderCopyBufferToBuffer(encoder, timestampsResolveBuffer, 0, timestampsReadBuffer, 0, 2 * sizeof(u64));

            // submit to queue
            WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, NULL);
            wgpuQueueSubmit(queue, 1, &command);
            wgpuCommandBufferRelease(command);
            wgpuCommandEncoderRelease(encoder);
            wgpuTextureViewRelease(next);

            // map query buffer
            if(readFrameStats)
            {
                wgpuBufferMapAsync(timestampsReadBuffer, WGPUMapMode_Read, 0, 2 * sizeof(u64), buffer_map_callback, timestampsReadBuffer);
                while(wgpuBufferGetMapState(timestampsReadBuffer) != WGPUBufferMapState_Unmapped)
                {
                    wgpuDeviceTick(device);
                }
            }

            // present to output
            wgpuSwapChainPresent(swapChain);
        }
        else
        {
            printf("couldn't get swapchain\n");
        }

        oc_scratch_end(scratch);
    }

    oc_surface_destroy(surface);
    oc_window_destroy(window);
    oc_terminate();

    return (0);
}
