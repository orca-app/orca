/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "canvas_renderer.h"
#include "wgpu_surface.h"
#include "wgpu_renderer_shaders.h"
#include "wgpu_renderer_debug.h"

typedef struct oc_wgpu_path
{
    f32 uvTransform[12];
    oc_vec4 color;
    oc_vec4 box;
    oc_vec4 clip;
    i32 cmd;
    i32 textureID;

    char pad0[8];
    //...
} oc_wgpu_path;

typedef struct oc_wgpu_path_elt
{
    i32 pathIndex;
    i32 kind;
    oc_vec2 p[4];

} oc_wgpu_path_elt;

typedef struct oc_wgpu_segment
{
    int kind;
    int pathIndex;
    int windingIncrement;
    int config;
    oc_vec4 box;
    f32 implicitMatrix[12];
    oc_vec2 hullVertex;
    f32 sign;
    u32 debugID;
} oc_wgpu_segment;

typedef struct oc_wgpu_tile_op
{
    int kind;
    int next;
    int index;
    int windingOffsetOrCrossRight;
} oc_wgpu_tile_op;

typedef struct oc_wgpu_bin_queue
{
    int windingOffset;
    int first;
    int last;
} oc_wgpu_bin_queue;

typedef struct oc_wgpu_path_bin
{
    int area[4];
    int binQueues;
    char pad0[12];
} oc_wgpu_path_bin;

typedef struct oc_wgpu_tile_queue
{
    oc_vec2 tileCoord;
    int first;
    char pad0[4];
} oc_wgpu_tile_queue;

typedef struct oc_wgpu_chunk
{
    int first;
    int last;
} oc_wgpu_chunk;

typedef struct oc_wgpu_chunk_elt
{
    int next;
    int path;
} oc_wgpu_chunk_elt;

typedef struct oc_wgpu_debug_display_options
{
    u32 showTileBorders;
    u32 showPathArea;
    u32 showClip;
    u32 textureOff;
    u32 debugTileQueues;

} oc_wgpu_debug_display_options;

enum
{
    OC_WGPU_CANVAS_TIMESTAMP_INDEX_FRAME_BEGIN,
    OC_WGPU_CANVAS_TIMESTAMP_INDEX_FRAME_END,
    OC_WGPU_CANVAS_TIMESTAMPS_COUNT,
};

typedef struct oc_wgpu_canvas_frame_timestamps
{
    u64 frameBegin;
    u64 frameEnd;
} oc_wgpu_canvas_frame_timestamps;

enum
{
    OC_WGPU_CANVAS_MAX_SAMPLE_COUNT = 8,
    OC_WGPU_CANVAS_DEFAULT_SAMPLE_COUNT = 8,
    OC_WGPU_CANVAS_BUFFER_DEFAULT_LEN = 1024,
    OC_WGPU_CANVAS_MAX_IMAGE_BINDINGS = 8,
    OC_WGPU_CANVAS_CHUNK_SIZE = 256,
    OC_WGPU_CANVAS_ROLLING_BUFFER_COUNT = 3,
};

oc_vec2 OC_WGPU_CANVAS_OFFSETS[4][OC_WGPU_CANVAS_MAX_SAMPLE_COUNT] = {
    // msaaSampleCount = 1
    {
        { 0, 0 },
    },
    // msaaSampleCount = 2
    {
        { 0.25, 0.25 },
        { -0.25, -0.25 },
    },
    // msaaSampleCount = 4
    {
        { -2 / 16., 6 / 16. },
        { 6 / 16., 2 / 16. },
        { -6 / 16., -2 / 16. },
        { 2 / 16., -6 / 16. },
    },
    //msaaSampleCount = 8
    {
        { 1 / 16., 3 / 16. },
        { -1 / 16., -3 / 16. },
        { 5 / 16., -1 / 16. },
        { -3 / 16., 5 / 16. },
        { -5 / 16., -5 / 16. },
        { -7 / 16., 1 / 16. },
        { 3 / 16., -7 / 16. },
        { 7 / 16., 7 / 16. },
    },
};

//NOTE: get offsets table index from number of requested samples. Only sample counts
//      1, 2, 4, 8 are supported, other sample counts are lowered to the closest supported count.
u32 OC_WGPU_CANVAS_OFFSETS_LOOKUP[OC_WGPU_CANVAS_MAX_SAMPLE_COUNT] = {
    0, 1, 1, 2, 2, 2, 2, 3
};

typedef struct oc_wgpu_canvas_renderer oc_wgpu_canvas_renderer;

typedef struct oc_wgpu_canvas_timestamp_read_callback_data
{
    oc_wgpu_canvas_renderer* renderer;
    WGPUBuffer buffer;
    oc_wgpu_canvas_frame_counters* frameCounters;
    u64 mapSize;

} oc_wgpu_canvas_timestamp_read_callback_data;

typedef struct oc_wgpu_canvas_renderer
{
    oc_canvas_renderer_base base;

    i32 msaaSampleCount; //TODO don't cache and reupload each frame?

    bool hasTimestamps;
    WGPULimits limits;

    WGPUInstance instance;
    WGPUDevice device;
    WGPUQueue queue;

    WGPUBindGroup pathSetupBindGroup;
    WGPUBindGroup segmentSetupBindGroup;
    WGPUBindGroup backpropBindGroup;
    WGPUBindGroup chunkBindGroup;
    WGPUBindGroup mergeBindGroup;
    WGPUBindGroup balanceBindGroup;
    WGPUBindGroup rasterBindGroup;
    WGPUBindGroup srcTexturesBindGroup;
    WGPUBindGroup blitBindGroup;

    WGPUBindGroupLayout pathSetupBindGroupLayout;
    WGPUBindGroupLayout segmentSetupBindGroupLayout;
    WGPUBindGroupLayout backpropBindGroupLayout;
    WGPUBindGroupLayout chunkBindGroupLayout;
    WGPUBindGroupLayout mergeBindGroupLayout;
    WGPUBindGroupLayout balanceBindGroupLayout;
    WGPUBindGroupLayout rasterBindGroupLayout;
    WGPUBindGroupLayout srcTexturesBindGroupLayout;
    WGPUBindGroupLayout blitBindGroupLayout;

    WGPUComputePipeline pathSetupPipeline;
    WGPUComputePipeline segmentSetupPipeline;
    WGPUComputePipeline backpropPipeline;
    WGPUComputePipeline chunkPipeline;
    WGPUComputePipeline mergePipeline;
    WGPUComputePipeline balancePipeline;
    WGPUComputePipeline rasterPipeline;
    WGPURenderPipeline blitPipeline;

    WGPUBuffer pathBuffer;
    WGPUBuffer pathCountBuffer; // remove?

    WGPUBuffer elementBuffer;
    WGPUBuffer elementCountBuffer; // remove?

    WGPUBuffer segmentCountBuffer;
    WGPUBuffer segmentBuffer;
    WGPUBuffer pathBinBuffer;
    WGPUBuffer binQueueBuffer;
    WGPUBuffer binQueueCountBuffer;

    WGPUBuffer chunkBuffer;
    WGPUBuffer chunkEltBuffer;
    WGPUBuffer chunkEltCountBuffer;

    WGPUBuffer tileQueueBuffer;
    WGPUBuffer tileQueueCountBuffer;
    WGPUBuffer tileOpBuffer;
    WGPUBuffer tileOpCountBuffer;

    WGPUBuffer tileSizeBuffer;
    WGPUBuffer chunkSizeBuffer;

    WGPUBuffer msaaOffsetsBuffer;
    WGPUBuffer msaaSampleCountBuffer;

    WGPUBuffer maxWorkGroupsPerDimensionBuffer;

    WGPUTextureView outTextureView;
    oc_vec2 outTextureSize;

    WGPUTextureView dummyTextureView;

    //Debug
    WGPUQuerySet timestampsQuerySet;
    WGPUBuffer timestampsResolveBuffer;

    int rollingBufferIndex;
    oc_wgpu_canvas_timestamp_read_callback_data timestampsReadCallbackData[OC_WGPU_CANVAS_ROLLING_BUFFER_COUNT];
    WGPUBuffer timestampsReadBuffer[OC_WGPU_CANVAS_ROLLING_BUFFER_COUNT];

    u64 frameIndex;
    f64 lastFrameTimeStamp;
    oc_arena debugArena;

    oc_wgpu_canvas_record_options debugRecordOptions;
    u32 debugRecordsCount;
    oc_list debugRecords;
    oc_list frameCountersFreeList;
    oc_list batchCountersFreeList;

    oc_wgpu_canvas_stats_buffer gpuTime;
    oc_wgpu_canvas_stats_buffer cpuEncodeTime;
    oc_wgpu_canvas_stats_buffer cpuFrameTime;
    //TODO: presentation time

    oc_wgpu_canvas_debug_display_options debugDisplayOptions;
    WGPUBuffer debugDisplayOptionsBuffer;

} oc_wgpu_canvas_renderer;

typedef struct oc_wgpu_image
{
    oc_image_base base;
    WGPUTexture texture;
    WGPUTextureView textureView;

} oc_wgpu_image;

void oc_wgpu_canvas_submit(oc_canvas_renderer_base* rendererBase,
                           oc_surface surfaceHandle,
                           u32 sampleCount,
                           oc_color clearColor,
                           u32 primitiveCount,
                           oc_primitive* primitives,
                           u32 eltCount,
                           oc_path_elt* pathElements);

void oc_wgpu_canvas_present(oc_canvas_renderer_base* rendererBase, oc_surface surfaceHandle);

void oc_wgpu_canvas_destroy(oc_canvas_renderer_base* base);
oc_image_base* oc_wgpu_canvas_image_create(oc_canvas_renderer_base* base, oc_vec2 size);
void oc_wgpu_canvas_image_destroy(oc_canvas_renderer_base* rendererBase, oc_image_base* imageBase);
void oc_wgpu_canvas_image_upload_region(oc_canvas_renderer_base* rendererBase, oc_image_base* imageBase, oc_rect region, u8* pixels);

void oc_wgpu_canvas_stats_reset(oc_wgpu_canvas_stats_buffer* stats)
{
    stats->sampleCount = 0;
    stats->nextSample = 0;
}

void oc_wgpu_canvas_stats_add_sample(oc_wgpu_canvas_stats_buffer* buffer, f64 sample)
{
    buffer->samples[buffer->nextSample] = sample;
    buffer->nextSample = (buffer->nextSample + 1) % OC_WGPU_CANVAS_STATS_BUFFER_SIZE;
    buffer->sampleCount = oc_clamp_high(buffer->sampleCount + 1, OC_WGPU_CANVAS_STATS_BUFFER_SIZE);
}

void oc_wgpu_canvas_stats_remove_sample(oc_wgpu_canvas_stats_buffer* buffer)
{
    buffer->sampleCount = oc_clamp_low(buffer->sampleCount - 1, 0);
}

oc_wgpu_canvas_stats oc_wgpu_canvas_stats_resolve(oc_wgpu_canvas_stats_buffer* buffer, u32 desiredSampleCount)
{
    oc_wgpu_canvas_stats result = { 0 };

    if(buffer->sampleCount)
    {
        int sampleCount = oc_clamp(desiredSampleCount,
                                   OC_WGPU_CANVAS_STATS_MIN_SAMPLE_COUNT,
                                   buffer->sampleCount);

        int startIndex = (buffer->nextSample - sampleCount + OC_WGPU_CANVAS_STATS_BUFFER_SIZE)
                       % OC_WGPU_CANVAS_STATS_BUFFER_SIZE;

        f64 sum = 0;
        f64 sum2 = 0;
        f64 minSample = FLT_MAX;
        f64 maxSample = FLT_MIN;

        for(int i = 0; i < sampleCount; i++)
        {
            double sample = buffer->samples[(startIndex + i) % OC_WGPU_CANVAS_STATS_BUFFER_SIZE];
            minSample = oc_min(minSample, sample);
            maxSample = oc_max(maxSample, sample);
            sum += sample;
            sum2 += sample * sample;
        }
        result.sampleCount = sampleCount;
        result.minSample = minSample;
        result.maxSample = maxSample;
        result.avg = sum / sampleCount;
        result.std = sqrt(sum2 / sampleCount - result.avg * result.avg);
    }
    return (result);
}

int oc_wgpu_canvas_stats_sample_count_for_95_confidence(oc_wgpu_canvas_stats* stats, f32 marginRelativeToMean)
{
    // https://en.wikipedia.org/wiki/Sample_size_determination#Estimation_of_a_mean
    int count = OC_WGPU_CANVAS_STATS_MIN_SAMPLE_COUNT;
    if(stats->sampleCount > 1 && stats->avg > 0)
    {
        f64 Z = 1.96; // Z-score for 95% confidence interval
        f64 width = fabs(stats->avg) * marginRelativeToMean * 2.;
        f64 f = Z * stats->std / width;
        count = (int)ceilf(4 * f * f);
    }
    return (count);
}

oc_surface oc_wgpu_canvas_surface_create_for_window(oc_canvas_renderer_base* base, oc_window window)
{
    oc_wgpu_canvas_renderer* renderer = (oc_wgpu_canvas_renderer*)base;
    return (oc_wgpu_surface_create_for_window(renderer->instance, window));
}

static void oc_wgpu_canvas_on_adapter_request_ended(WGPURequestAdapterStatus status, WGPUAdapter adapter, const char* message, void* userdata)
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

static void oc_wgpu_canvas_on_device_error(WGPUErrorType type, const char* message, void* userdata)
{
    oc_log_error("%s\n", message);
    exit(-1);
}

static void oc_wgpu_canvas_on_shader_error(WGPUCompilationInfoRequestStatus status, struct WGPUCompilationInfo const* compilationInfo, void* userdata)
{
    for(int i = 0; i < compilationInfo->messageCount; i++)
    {

        const WGPUCompilationMessage* message = &compilationInfo->messages[i];
        oc_log_level level;
        switch(message->type)
        {
            case WGPUCompilationMessageType_Warning:
                level = OC_LOG_LEVEL_WARNING;
                break;

            case WGPUCompilationMessageType_Info:
                level = OC_LOG_LEVEL_INFO;
                break;

            default:
            case WGPUCompilationMessageType_Error:
                level = OC_LOG_LEVEL_ERROR;
                break;
        }
        oc_log_ext(level, "n/a", "n/a", message->lineNum, "shader compilation error: %s\n", message->message);
    }
}

void oc_wgpu_renderer_create_compute_pipeline(WGPUDevice device,
                                              const char* label,
                                              const char* src,
                                              const char* entryPoint,
                                              u32 bindGroupCount,
                                              WGPUBindGroupLayoutDescriptor* bindGroupLayoutDescs,
                                              WGPUBindGroupLayout* bindGroupLayouts,
                                              WGPUComputePipeline* pipeline)
{
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8_list list = { 0 };

    oc_str8_list_push(scratch.arena, &list, OC_STR8(src));
    oc_str8_list_push(scratch.arena, &list, OC_STR8(oc_wgsl_common));

    oc_str8 code = oc_str8_list_join(scratch.arena, list);

    WGPUShaderModuleDescriptor desc = {
        .nextInChain = &((WGPUShaderModuleWGSLDescriptor){
                             .chain.sType = WGPUSType_ShaderModuleWGSLDescriptor,
                             .code = code.ptr,
                         })
                            .chain,
    };

    WGPUShaderModule module = wgpuDeviceCreateShaderModule(device, &desc);
    wgpuShaderModuleGetCompilationInfo(module, oc_wgpu_canvas_on_shader_error, 0);

    for(int i = 0; i < bindGroupCount; i++)
    {
        bindGroupLayouts[i] = wgpuDeviceCreateBindGroupLayout(device, &bindGroupLayoutDescs[i]);
    }
    WGPUPipelineLayoutDescriptor pipelineLayoutDesc = {
        .bindGroupLayoutCount = bindGroupCount,
        .bindGroupLayouts = bindGroupLayouts,
    };

    WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(device, &pipelineLayoutDesc);

    WGPUComputePipelineDescriptor pipelineDesc = {
        .label = label,
        .layout = pipelineLayout,
        .compute = {
            .module = module,
            .entryPoint = entryPoint,
        },
    };
    *pipeline = wgpuDeviceCreateComputePipeline(device, &pipelineDesc);

    wgpuPipelineLayoutRelease(pipelineLayout);
    wgpuShaderModuleRelease(module);

    oc_scratch_end(scratch);
}

void oc_wgpu_renderer_create_render_pipeline(WGPUDevice device,
                                             const char* label,
                                             const char* src,
                                             const char* vertexEntryPoint,
                                             const char* fragmentEntryPoint,
                                             WGPUBindGroupLayoutDescriptor* bindGroupLayoutDesc,
                                             WGPUBindGroupLayout* bindGroupLayout,
                                             WGPURenderPipeline* pipeline)
{
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8_list list = { 0 };

    oc_str8_list_push(scratch.arena, &list, OC_STR8(src));
    oc_str8_list_push(scratch.arena, &list, OC_STR8(oc_wgsl_common));

    oc_str8 code = oc_str8_list_join(scratch.arena, list);

    WGPUShaderModuleDescriptor desc = {
        .nextInChain = &((WGPUShaderModuleWGSLDescriptor){
                             .chain.sType = WGPUSType_ShaderModuleWGSLDescriptor,
                             .code = code.ptr,
                         })
                            .chain,
    };

    WGPUShaderModule module = wgpuDeviceCreateShaderModule(device, &desc);
    wgpuShaderModuleGetCompilationInfo(module, oc_wgpu_canvas_on_shader_error, 0);

    *bindGroupLayout = wgpuDeviceCreateBindGroupLayout(device, bindGroupLayoutDesc);

    WGPUPipelineLayoutDescriptor pipelineLayoutDesc = {
        .bindGroupLayoutCount = 1,
        .bindGroupLayouts = (WGPUBindGroupLayout[]){
            *bindGroupLayout,
        },
    };
    WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(device, &pipelineLayoutDesc);

    WGPURenderPipelineDescriptor pipelineDesc = {
        .layout = pipelineLayout,
        // draw triangle list, no index buffer, no culling
        .primitive = {
            .topology = WGPUPrimitiveTopology_TriangleList,
            .frontFace = WGPUFrontFace_CCW,
            .cullMode = WGPUCullMode_None,
        },
        // vertex shader
        .vertex = {
            .module = module,
            .entryPoint = vertexEntryPoint,
            .bufferCount = 0,
        },
        // fragment shader
        .fragment = &(WGPUFragmentState){
            .module = module,
            .entryPoint = fragmentEntryPoint,
            .targetCount = 1,
            .targets = (WGPUColorTargetState[]){
                // writing to one output, with alpha-blending enabled
                {
                    .format = WGPUTextureFormat_BGRA8Unorm,
                    .blend = &(WGPUBlendState){
                        .color.operation = WGPUBlendOperation_Add,
                        .color.srcFactor = WGPUBlendFactor_One,
                        .color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha,
                        .alpha.operation = WGPUBlendOperation_Add,
                        .alpha.srcFactor = WGPUBlendFactor_One,
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

    *pipeline = wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);

    wgpuPipelineLayoutRelease(pipelineLayout);
    wgpuShaderModuleRelease(module);

    oc_scratch_end(scratch);
}

oc_canvas_renderer oc_canvas_renderer_create(void)
{
    oc_wgpu_canvas_renderer* renderer = oc_malloc_type(oc_wgpu_canvas_renderer);
    memset(renderer, 0, sizeof(oc_wgpu_canvas_renderer));

    //NOTE: setup base functions
    renderer->base.destroy = oc_wgpu_canvas_destroy;
    renderer->base.createSurfaceForWindow = oc_wgpu_canvas_surface_create_for_window;
    renderer->base.imageCreate = oc_wgpu_canvas_image_create;
    renderer->base.imageDestroy = oc_wgpu_canvas_image_destroy;
    renderer->base.imageUploadRegion = oc_wgpu_canvas_image_upload_region;
    renderer->base.submit = oc_wgpu_canvas_submit;
    renderer->base.present = oc_wgpu_canvas_present;

    {
        int enabledToggleCount = 1;
        const char* enabledToggles[] = { "allow_unsafe_apis" };

        int disabledToggleCount = 1;
        const char* disabledToggles[] = { "timestamp_quantization" };

        WGPUInstanceDescriptor desc = {
            .nextInChain = &((WGPUDawnTogglesDescriptor){
                                 .chain.sType = WGPUSType_DawnTogglesDescriptor,
                                 .enabledToggleCount = enabledToggleCount,
                                 .enabledToggles = enabledToggles,
                                 .disabledToggleCount = disabledToggleCount,
                                 .disabledToggles = disabledToggles,
                             })
                                .chain,
        };

        renderer->instance = wgpuCreateInstance(&desc);
    }
    //NOTE: create adapter
    WGPUAdapter adapter = 0;
    {
        WGPURequestAdapterOptions options = {
            .powerPreference = WGPUPowerPreference_HighPerformance,
        };
        wgpuInstanceRequestAdapter(renderer->instance, &options, &oc_wgpu_canvas_on_adapter_request_ended, &adapter);
        OC_ASSERT(adapter && "Failed to get WebGPU adapter");

        renderer->hasTimestamps = wgpuAdapterHasFeature(adapter, WGPUFeatureName_TimestampQuery);
    }

    //NOTE: create device
    {
        WGPUSupportedLimits supported = { 0 };
        wgpuAdapterGetLimits(adapter, &supported);

        renderer->limits = supported.limits;

        WGPUDeviceDescriptor desc = {
            .requiredLimits = &(WGPURequiredLimits){ .limits = supported.limits },
        };
        if(renderer->hasTimestamps)
        {
            desc.requiredFeatureCount = 1,
            desc.requiredFeatures = (WGPUFeatureName[]){ WGPUFeatureName_TimestampQuery };
        }
        renderer->device = wgpuAdapterCreateDevice(adapter, &desc);
        OC_ASSERT(renderer->device, "Failed to create WebGPU device");

        //NOTE: notify on errors
        wgpuDeviceSetUncapturedErrorCallback(renderer->device, &oc_wgpu_canvas_on_device_error, NULL);

        //NOTE: default device queue
        renderer->queue = wgpuDeviceGetQueue(renderer->device);
    }

    //NOTE:release adapter, we don't need it anymore
    wgpuAdapterRelease(adapter);

    //NOTE: create resources:
    //      variable size buffers are created/resized when input is uploaded to GPU

    //NOTE: create path count buffer
    {
        WGPUBufferDescriptor desc = {
            .label = "pathCount",
            .usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst,
            .size = sizeof(u32),
        };
        renderer->pathCountBuffer = wgpuDeviceCreateBuffer(renderer->device, &desc);
    }

    //NOTE: create element count buffer
    {
        WGPUBufferDescriptor desc = {
            .label = "elementCount",
            .usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst,
            .size = sizeof(u32),
        };
        renderer->elementCountBuffer = wgpuDeviceCreateBuffer(renderer->device, &desc);
    }

    //NOTE: create segment count buffer
    {
        WGPUBufferDescriptor desc = {
            .label = "segmentCount",
            .usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc,
            .size = sizeof(u32),
        };
        renderer->segmentCountBuffer = wgpuDeviceCreateBuffer(renderer->device, &desc);
    }

    //NOTE: create tile op count buffer
    {
        WGPUBufferDescriptor desc = {
            .label = "tileOpCount",
            .usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc,
            .size = sizeof(u32),
        };
        renderer->tileOpCountBuffer = wgpuDeviceCreateBuffer(renderer->device, &desc);
    }

    //NOTE: create bin queue count buffer
    {
        WGPUBufferDescriptor desc = {
            .label = "binQueueCount",
            .usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc,
            .size = sizeof(u32),
        };
        renderer->binQueueCountBuffer = wgpuDeviceCreateBuffer(renderer->device, &desc);
    }

    //NOTE: create tile queue count buffer
    {
        WGPUBufferDescriptor desc = {
            .label = "tileQueueCount",
            .usage = WGPUBufferUsage_Storage | WGPUBufferUsage_Indirect | WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc,
            .size = 3 * sizeof(u32),
        };
        renderer->tileQueueCountBuffer = wgpuDeviceCreateBuffer(renderer->device, &desc);
    }

    //NOTE: create chunk element count buffer
    {
        WGPUBufferDescriptor desc = {
            .label = "chunkEltCount",
            .usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst,
            .size = sizeof(u32),
        };
        renderer->chunkEltCountBuffer = wgpuDeviceCreateBuffer(renderer->device, &desc);
    }

    //NOTE: create tile size buffer
    {
        WGPUBufferDescriptor desc = {
            .label = "tileSize",
            .usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst,
            .size = sizeof(u32),
        };
        renderer->tileSizeBuffer = wgpuDeviceCreateBuffer(renderer->device, &desc);
    }

    //NOTE: create chunk size buffer
    {
        WGPUBufferDescriptor desc = {
            .label = "chunkSize",
            .usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst,
            .size = sizeof(u32),
        };
        renderer->chunkSizeBuffer = wgpuDeviceCreateBuffer(renderer->device, &desc);
    }

    //NOTE: create max workgroup per dimension buffer
    {
        WGPUBufferDescriptor desc = {
            .label = "maxWorkgroupsPerDimension",
            .usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst,
            .size = sizeof(u32),
        };
        renderer->maxWorkGroupsPerDimensionBuffer = wgpuDeviceCreateBuffer(renderer->device, &desc);
        wgpuQueueWriteBuffer(renderer->queue,
                             renderer->maxWorkGroupsPerDimensionBuffer,
                             0,
                             &renderer->limits.maxComputeWorkgroupsPerDimension,
                             sizeof(u32));
    }

    //NOTE: create msaa offsets buffer
    {
        WGPUBufferDescriptor desc = {
            .label = "msaa offsets",
            .usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst,
            .size = OC_WGPU_CANVAS_MAX_SAMPLE_COUNT * sizeof(oc_vec2),
        };
        renderer->msaaOffsetsBuffer = wgpuDeviceCreateBuffer(renderer->device, &desc);

        oc_vec2 msaaOffsets[OC_WGPU_CANVAS_MAX_SAMPLE_COUNT] = {
            { 1 / 16., 3 / 16. },
            { -1 / 16., -3 / 16. },
            { 5 / 16., -1 / 16. },
            { -3 / 16., 5 / 16. },
            { -5 / 16., -5 / 16. },
            { -7 / 16., 1 / 16. },
            { 3 / 16., -7 / 16. },
            { 7 / 16., 7 / 16. },
        };
        wgpuQueueWriteBuffer(renderer->queue, renderer->msaaOffsetsBuffer, 0, msaaOffsets, OC_WGPU_CANVAS_MAX_SAMPLE_COUNT * sizeof(oc_vec2));
    }

    //NOTE: create msaa sample count buffer
    {
        WGPUBufferDescriptor desc = {
            .label = "msaa sample count",
            .usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst,
            .size = sizeof(u32),
        };
        renderer->msaaSampleCountBuffer = wgpuDeviceCreateBuffer(renderer->device, &desc);

        u32 msaaSampleCount = 8;
        wgpuQueueWriteBuffer(renderer->queue, renderer->msaaSampleCountBuffer, 0, &msaaSampleCount, sizeof(u32));
    }

    //NOTE: create dummy texture
    {
        WGPUTextureDescriptor desc = {
            .usage = WGPUTextureUsage_TextureBinding,
            .dimension = WGPUTextureDimension_2D,
            .size = { 1, 1, 1 },
            .format = WGPUTextureFormat_RGBA8Unorm,
            .mipLevelCount = 1,
            .sampleCount = 1,
        };
        WGPUTexture texture = wgpuDeviceCreateTexture(renderer->device, &desc);

        WGPUTextureViewDescriptor viewDesc = {
            .format = desc.format,
            .dimension = WGPUTextureViewDimension_2D,
            .baseMipLevel = 0,
            .mipLevelCount = 1,
            .baseArrayLayer = 0,
            .arrayLayerCount = 1,
            .aspect = WGPUTextureAspect_All,
        };
        renderer->dummyTextureView = wgpuTextureCreateView(texture, &viewDesc);
        wgpuTextureRelease(texture);
    }

    //----------------------------------------------------------
    //NOTE: create pipelines
    //----------------------------------------------------------

    //NOTE: path setup pipeline
    {
        WGPUBindGroupLayoutDescriptor bindGroupLayoutDesc = {
            .entryCount = 6,
            .entries = (WGPUBindGroupLayoutEntry[]){
                {
                    .binding = 0,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_ReadOnlyStorage,
                },
                {
                    .binding = 1,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Uniform,
                },
                {
                    .binding = 2,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Storage,
                },
                {
                    .binding = 3,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Storage,
                },
                {
                    .binding = 4,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Storage,
                },
                {
                    .binding = 5,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Uniform,
                },
            },
        };

        oc_wgpu_renderer_create_compute_pipeline(renderer->device,
                                                 "path setup",
                                                 oc_wgsl_path_setup,
                                                 "path_setup",
                                                 1,
                                                 &bindGroupLayoutDesc,
                                                 &renderer->pathSetupBindGroupLayout,
                                                 &renderer->pathSetupPipeline);
    }

    //NOTE: segment setup pipeline
    {
        WGPUBindGroupLayoutDescriptor bindGroupLayoutDesc = {
            .entryCount = 10,
            .entries = (WGPUBindGroupLayoutEntry[]){
                {
                    .binding = 0,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_ReadOnlyStorage,
                },
                {
                    .binding = 1,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_ReadOnlyStorage,
                },
                {
                    .binding = 2,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Uniform,
                },
                {
                    .binding = 3,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Storage,
                },
                {
                    .binding = 4,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Storage,
                },
                {
                    .binding = 5,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Storage,
                },
                {
                    .binding = 6,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Storage,
                },
                {
                    .binding = 7,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Storage,
                },
                {
                    .binding = 8,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Storage,
                },
                {
                    .binding = 9,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Uniform,
                },
            },
        };

        oc_wgpu_renderer_create_compute_pipeline(renderer->device,
                                                 "segment setup",
                                                 oc_wgsl_segment_setup,
                                                 "segment_setup",
                                                 1,
                                                 &bindGroupLayoutDesc,
                                                 &renderer->segmentSetupBindGroupLayout,
                                                 &renderer->segmentSetupPipeline);
    }

    //NOTE: backprop pipeline
    {
        WGPUBindGroupLayoutDescriptor bindGroupLayoutDesc = {
            .entryCount = 3,
            .entries = (WGPUBindGroupLayoutEntry[]){
                {
                    .binding = 0,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_ReadOnlyStorage,
                },
                {
                    .binding = 1,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Uniform,
                },
                {
                    .binding = 2,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Storage,
                },
            },
        };

        oc_wgpu_renderer_create_compute_pipeline(renderer->device,
                                                 "backprop pass",
                                                 oc_wgsl_backprop,
                                                 "backprop",
                                                 1,
                                                 &bindGroupLayoutDesc,
                                                 &renderer->backpropBindGroupLayout,
                                                 &renderer->backpropPipeline);
    }

    //NOTE: chunk pipeline
    {
        WGPUBindGroupLayoutDescriptor bindGroupLayoutDesc = {
            .entryCount = 6,
            .entries = (WGPUBindGroupLayoutEntry[]){
                {
                    .binding = 0,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_ReadOnlyStorage,
                },
                {
                    .binding = 1,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Uniform,
                },
                {
                    .binding = 2,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Storage,
                },
                {
                    .binding = 3,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Storage,
                },
                {
                    .binding = 4,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Storage,
                },
                {
                    .binding = 5,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Uniform,
                },
            },
        };

        oc_wgpu_renderer_create_compute_pipeline(renderer->device,
                                                 "chunk pass",
                                                 oc_wgsl_chunk,
                                                 "chunk",
                                                 1,
                                                 &bindGroupLayoutDesc,
                                                 &renderer->chunkBindGroupLayout,
                                                 &renderer->chunkPipeline);
    }

    //NOTE: merge pipeline
    {
        WGPUBindGroupLayoutDescriptor bindGroupLayoutDesc = {
            .entryCount = 13,
            .entries = (WGPUBindGroupLayoutEntry[]){
                {
                    .binding = 0,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_ReadOnlyStorage,
                },
                {
                    .binding = 1,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Uniform,
                },
                {
                    .binding = 2,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_ReadOnlyStorage,
                },
                {
                    .binding = 3,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_ReadOnlyStorage,
                },
                {
                    .binding = 4,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_ReadOnlyStorage,
                },
                {
                    .binding = 5,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_ReadOnlyStorage,
                },
                {
                    .binding = 6,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_ReadOnlyStorage,
                },
                {
                    .binding = 7,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Storage,
                },
                {
                    .binding = 8,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Storage,
                },
                {
                    .binding = 9,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Storage,
                },
                {
                    .binding = 10,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Storage,
                },
                {
                    .binding = 11,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Uniform,
                },
                {
                    .binding = 12,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Uniform,
                },
            },
        };

        oc_wgpu_renderer_create_compute_pipeline(renderer->device,
                                                 "merge pass",
                                                 oc_wgsl_merge,
                                                 "merge",
                                                 1,
                                                 &bindGroupLayoutDesc,
                                                 &renderer->mergeBindGroupLayout,
                                                 &renderer->mergePipeline);
    }

    //NOTE: balance workgroup pipeline
    {
        WGPUBindGroupLayoutDescriptor bindGroupLayoutDesc = {
            .entryCount = 2,
            .entries = (WGPUBindGroupLayoutEntry[]){
                {
                    .binding = 0,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Storage,
                },
                {
                    .binding = 1,
                    .visibility = WGPUShaderStage_Compute,
                    .buffer.type = WGPUBufferBindingType_Uniform,
                },
            },
        };

        oc_wgpu_renderer_create_compute_pipeline(renderer->device,
                                                 "balance workgroups pass",
                                                 oc_wgsl_balance_workgroups,
                                                 "balance_workgroups",
                                                 1,
                                                 &bindGroupLayoutDesc,
                                                 &renderer->balanceBindGroupLayout,
                                                 &renderer->balancePipeline);

        //NOTE: Balance workgroups bindgroup doesn't change, so we build it here

        WGPUBindGroupDescriptor bindGroupDesc = {
            .layout = renderer->balanceBindGroupLayout,
            .entryCount = 2,
            .entries = (WGPUBindGroupEntry[]){
                {
                    .binding = 0,
                    .buffer = renderer->tileQueueCountBuffer,
                    .size = 3 * sizeof(u32),
                },
                {
                    .binding = 1,
                    .buffer = renderer->maxWorkGroupsPerDimensionBuffer,
                    .size = sizeof(u32),
                },
            }
        };

        renderer->balanceBindGroup = wgpuDeviceCreateBindGroup(renderer->device, &bindGroupDesc);
    }

    //NOTE: raster pipeline
    {
        WGPUBindGroupLayoutEntry sourceTextureEntries[OC_WGPU_CANVAS_MAX_IMAGE_BINDINGS];

        for(int i = 0; i < OC_WGPU_CANVAS_MAX_IMAGE_BINDINGS; i++)
        {
            sourceTextureEntries[i] = (WGPUBindGroupLayoutEntry){
                .binding = i,
                .visibility = WGPUShaderStage_Compute,
                .texture.sampleType = WGPUTextureSampleType_Float,
                .texture.viewDimension = WGPUTextureViewDimension_2D,
                .texture.multisampled = 0,
            };
        }

        WGPUBindGroupLayoutDescriptor bindGroupLayoutDescs[2] = {
            // raster bindgroup 0
            {
                .entryCount = 9,
                .entries = (WGPUBindGroupLayoutEntry[]){
                    {
                        .binding = 0,
                        .visibility = WGPUShaderStage_Compute,
                        .buffer.type = WGPUBufferBindingType_ReadOnlyStorage,
                    },
                    {
                        .binding = 1,
                        .visibility = WGPUShaderStage_Compute,
                        .buffer.type = WGPUBufferBindingType_ReadOnlyStorage,
                    },
                    {
                        .binding = 2,
                        .visibility = WGPUShaderStage_Compute,
                        .buffer.type = WGPUBufferBindingType_ReadOnlyStorage,
                    },
                    {
                        .binding = 3,
                        .visibility = WGPUShaderStage_Compute,
                        .buffer.type = WGPUBufferBindingType_ReadOnlyStorage,
                    },
                    {
                        .binding = 4,
                        .visibility = WGPUShaderStage_Compute,
                        .buffer.type = WGPUBufferBindingType_ReadOnlyStorage,
                    },
                    {
                        .binding = 5,
                        .visibility = WGPUShaderStage_Compute,
                        .buffer.type = WGPUBufferBindingType_Uniform,
                    },
                    {
                        .binding = 6,
                        .visibility = WGPUShaderStage_Compute,
                        .buffer.type = WGPUBufferBindingType_Uniform,
                    },
                    {
                        .binding = 7,
                        .visibility = WGPUShaderStage_Compute,
                        .storageTexture.access = WGPUStorageTextureAccess_WriteOnly,
                        .storageTexture.format = WGPUTextureFormat_RGBA8Unorm,
                        .storageTexture.viewDimension = WGPUTextureViewDimension_2D,
                    },
                    {
                        .binding = 8,
                        .visibility = WGPUShaderStage_Compute,
                        .buffer.type = WGPUBufferBindingType_Uniform,
                    },
                },
            },
            // bindgroup 1 (source textures)
            {
                .entryCount = OC_WGPU_CANVAS_MAX_IMAGE_BINDINGS,
                .entries = sourceTextureEntries,
            }
        };

        WGPUBindGroupLayout bindGroupLayouts[2] = { 0 };

        oc_wgpu_renderer_create_compute_pipeline(renderer->device,
                                                 "raster",
                                                 oc_wgsl_raster,
                                                 "raster",
                                                 2,
                                                 bindGroupLayoutDescs,
                                                 bindGroupLayouts,
                                                 &renderer->rasterPipeline);

        renderer->rasterBindGroupLayout = bindGroupLayouts[0];
        renderer->srcTexturesBindGroupLayout = bindGroupLayouts[1];
    }

    //NOTE: blit pipeline
    {
        WGPUBindGroupLayoutDescriptor bindGroupLayoutDesc = {
            .entryCount = 1,
            .entries = (WGPUBindGroupLayoutEntry[]){
                {
                    .binding = 0,
                    .visibility = WGPUShaderStage_Fragment,
                    .texture.sampleType = WGPUTextureSampleType_Float,
                    .texture.viewDimension = WGPUTextureViewDimension_2D,
                    .texture.multisampled = 0,
                },
            },
        };

        oc_wgpu_renderer_create_render_pipeline(renderer->device,
                                                "blit",
                                                oc_wgsl_blit,
                                                "vs",
                                                "fs",
                                                &bindGroupLayoutDesc,
                                                &renderer->blitBindGroupLayout,
                                                &renderer->blitPipeline);
    }

    //NOTE: create timestamps query set and buffers
    if(renderer->hasTimestamps)
    {
        {
            WGPUQuerySetDescriptor desc = {
                .type = WGPUQueryType_Timestamp,
                .count = OC_WGPU_CANVAS_TIMESTAMPS_COUNT,
            };
            renderer->timestampsQuerySet = wgpuDeviceCreateQuerySet(renderer->device, &desc);
        }

        {
            WGPUBufferDescriptor desc = {
                .label = "timestampsResolve",
                .usage = WGPUBufferUsage_QueryResolve | WGPUBufferUsage_CopySrc,
                .size = sizeof(oc_wgpu_canvas_frame_timestamps),
            };
            renderer->timestampsResolveBuffer = wgpuDeviceCreateBuffer(renderer->device, &desc);
        }

        {
            WGPUBufferDescriptor desc = {
                .label = "timestampsRead",
                .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_MapRead,
                .size = sizeof(oc_wgpu_canvas_frame_timestamps),
            };
            for(int i = 0; i < OC_WGPU_CANVAS_ROLLING_BUFFER_COUNT; i++)
            {
                renderer->timestampsReadBuffer[i] = wgpuDeviceCreateBuffer(renderer->device, &desc);
            }
        }
    }
    //NOTE: create debug display options buffer
    {
        WGPUBufferDescriptor desc = {
            .label = "debugDisplayOptions",
            .usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst,
            .size = sizeof(oc_wgpu_debug_display_options),
        };
        renderer->debugDisplayOptionsBuffer = wgpuDeviceCreateBuffer(renderer->device, &desc);
    }

    //NOTE: init debug stuff
    oc_arena_init(&renderer->debugArena);

    oc_wgpu_canvas_stats_reset(&renderer->gpuTime);
    oc_wgpu_canvas_stats_reset(&renderer->cpuEncodeTime);
    oc_wgpu_canvas_stats_reset(&renderer->cpuFrameTime);

    oc_canvas_renderer handle = oc_canvas_renderer_nil();
    if(renderer)
    {
        handle = oc_canvas_renderer_handle_alloc((oc_canvas_renderer_base*)renderer);
    }
    return (handle);
}

static void oc_update_box_extents(oc_vec4* extents, oc_vec2 p)
{
    extents->x = oc_min(extents->x, p.x);
    extents->y = oc_min(extents->y, p.y);
    extents->z = oc_max(extents->z, p.x);
    extents->w = oc_max(extents->w, p.y);
}

typedef struct oc_wgpu_canvas_encoding_context
{
    oc_wgpu_canvas_renderer* renderer;

    oc_arena* arena;

    u32 pathBatchStart;

    u32 inputPrimitiveCount;
    oc_primitive* inputPrimitives;

    u32 inputEltCount;
    oc_path_elt* inputElements;

    u32 pathCount;
    u32 pathCap;
    oc_wgpu_path* pathData;

    u32 eltCount;
    u32 eltCap;
    oc_wgpu_path_elt* elementData;

    u32 maxSegmentCount;
    u32 maxBinQueueCount;
    u32 maxTileOpCount;

    u32 chunkCount;
    u32 maxChunkEltCount;

    oc_primitive* primitive;

    oc_vec4 pathUserExtents;
    oc_vec4 pathScreenExtents;

    oc_vec2 screenSize;
    oc_vec2 scale;
    f32 tileSize;
    u32 screenTilesCount;

    i32 currentImageIndex;
    u32 imageCount;
    oc_image imageBindings[OC_WGPU_CANVAS_MAX_IMAGE_BINDINGS];

} oc_wgpu_canvas_encoding_context;

oc_wgpu_path_elt* oc_wgpu_canvas_push_element(oc_wgpu_canvas_encoding_context* context)
{
    if(context->eltCount >= context->eltCap)
    {
        int newCap = oc_max(OC_WGPU_CANVAS_BUFFER_DEFAULT_LEN, (int)(context->eltCap * 1.5));
        while(context->eltCount >= newCap)
        {
            newCap = (int)(newCap * 1.5);
        }
        oc_wgpu_path_elt* elements = oc_arena_push_array(context->arena, oc_wgpu_path_elt, newCap);
        memcpy(elements, context->elementData, context->eltCount * sizeof(oc_wgpu_path_elt));

        context->elementData = elements;
        context->eltCap = newCap;
    }

    oc_wgpu_path_elt* elt = &context->elementData[context->eltCount];
    context->eltCount++;
    return (elt);
}

oc_wgpu_path* oc_wgpu_canvas_push_path(oc_wgpu_canvas_encoding_context* context)
{
    if(context->pathCount >= context->pathCap)
    {
        int newCap = oc_max(OC_WGPU_CANVAS_BUFFER_DEFAULT_LEN, (int)(context->pathCap * 1.5));
        while(context->pathCount >= newCap)
        {
            newCap = (int)(newCap * 1.5);
        }
        oc_wgpu_path* paths = oc_arena_push_array(context->arena, oc_wgpu_path, newCap);
        memcpy(paths, context->pathData, context->pathCount * sizeof(oc_wgpu_path));

        context->pathData = paths;
        context->pathCap = newCap;
    }

    oc_wgpu_path* path = &context->pathData[context->pathCount];
    context->pathCount++;
    return (path);
}

static void oc_wgpu_canvas_encode_element(oc_wgpu_canvas_encoding_context* context, oc_path_elt_type kind, oc_vec2* p)
{
    oc_wgpu_path_elt* elt = oc_wgpu_canvas_push_element(context);
    if(elt)
    {
        elt->pathIndex = context->pathCount;
        int count = 0;
        elt->kind = kind;

        int maxSegmentCount = 0;
        switch(kind)
        {
            case OC_PATH_LINE:
                maxSegmentCount = 1;
                count = 2;
                break;

            case OC_PATH_QUADRATIC:
                maxSegmentCount = 3;
                count = 3;
                break;

            case OC_PATH_CUBIC:
                maxSegmentCount = 7;
                count = 4;
                break;

            default:
                break;
        }
        context->maxSegmentCount += maxSegmentCount;

        oc_vec4 segBox = { FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX };

        for(int i = 0; i < count; i++)
        {
            oc_update_box_extents(&context->pathUserExtents, p[i]);

            oc_vec2 screenP = oc_mat2x3_mul(context->primitive->attributes.transform, p[i]);

            elt->p[i] = (oc_vec2){ screenP.x * context->scale.x, screenP.y * context->scale.y };

            oc_update_box_extents(&context->pathScreenExtents, screenP);
            oc_update_box_extents(&segBox, screenP);
        }

        //NOTE: we make a conservative guess. An element can never use more tile ops than the number of tiles it covers
        // times the number of segments it can produce.

        int firstTileX = segBox.x * context->scale.x / context->tileSize;
        int firstTileY = segBox.y * context->scale.y / context->tileSize;
        int lastTileX = segBox.z * context->scale.x / context->tileSize;
        int lastTileY = segBox.w * context->scale.y / context->tileSize;

        int nTilesX = lastTileX - firstTileX + 1;
        int nTilesY = lastTileY - firstTileY + 1;

        context->maxTileOpCount += (nTilesX * nTilesY) * maxSegmentCount;
    }
}

void oc_wgpu_canvas_encode_path(oc_wgpu_canvas_encoding_context* context, oc_primitive* primitive)
{
    oc_wgpu_path* path = oc_wgpu_canvas_push_path(context);

    if(path)
    {
        path->cmd = primitive->cmd;

        path->box = (oc_vec4){
            context->pathScreenExtents.x * context->scale.x,
            context->pathScreenExtents.y * context->scale.y,
            context->pathScreenExtents.z * context->scale.x,
            context->pathScreenExtents.w * context->scale.y,
        };

        path->clip = (oc_vec4){
            primitive->attributes.clip.x * context->scale.x,
            primitive->attributes.clip.y * context->scale.y,
            (primitive->attributes.clip.x + primitive->attributes.clip.w) * context->scale.x,
            (primitive->attributes.clip.y + primitive->attributes.clip.h) * context->scale.y,
        };

        path->color = (oc_vec4){
            primitive->attributes.color.r,
            primitive->attributes.color.g,
            primitive->attributes.color.b,
            primitive->attributes.color.a
        };

        if(!oc_image_is_nil(primitive->attributes.image))
        {
            oc_rect srcRegion = primitive->attributes.srcRegion;

            oc_rect destRegion = {
                context->pathUserExtents.x,
                context->pathUserExtents.y,
                (context->pathUserExtents.z - context->pathUserExtents.x),
                (context->pathUserExtents.w - context->pathUserExtents.y),
            };

            oc_vec2 texSize = oc_image_size(primitive->attributes.image);

            oc_mat2x3 srcRegionToImage = {
                1 / texSize.x, 0, srcRegion.x / texSize.x,
                0, 1 / texSize.y, srcRegion.y / texSize.y
            };

            oc_mat2x3 destRegionToSrcRegion = {
                srcRegion.w / destRegion.w, 0, 0,
                0, srcRegion.h / destRegion.h, 0
            };

            oc_mat2x3 userToDestRegion = {
                1, 0, -destRegion.x,
                0, 1, -destRegion.y
            };

            oc_mat2x3 scaleM = {
                context->scale.x, 0, 0,
                0, context->scale.y, 0
            };
            oc_mat2x3 userToScreen = oc_mat2x3_mul_m(scaleM, primitive->attributes.transform);
            oc_mat2x3 screenToUser = oc_mat2x3_inv(userToScreen); //TODO should have scale here

            oc_mat2x3 uvTransform = srcRegionToImage;
            uvTransform = oc_mat2x3_mul_m(uvTransform, destRegionToSrcRegion);
            uvTransform = oc_mat2x3_mul_m(uvTransform, userToDestRegion);
            uvTransform = oc_mat2x3_mul_m(uvTransform, screenToUser);

            //NOTE: mat3 layout is an array of oc_vec3, which are padded to _oc_vec4_ alignment
            path->uvTransform[0] = uvTransform.m[0];
            path->uvTransform[1] = uvTransform.m[3];
            path->uvTransform[2] = 0;
            path->uvTransform[3] = 0;
            path->uvTransform[4] = uvTransform.m[1];
            path->uvTransform[5] = uvTransform.m[4];
            path->uvTransform[6] = 0;
            path->uvTransform[7] = 0;
            path->uvTransform[8] = uvTransform.m[2];
            path->uvTransform[9] = uvTransform.m[5];
            path->uvTransform[10] = 1;
            path->uvTransform[11] = 0;

            path->textureID = context->currentImageIndex;
        }
        else
        {
            path->textureID = -1;
        }

        int firstTileX = path->box.x / context->tileSize;
        int firstTileY = path->box.y / context->tileSize;
        int lastTileX = path->box.z / context->tileSize;
        int lastTileY = path->box.w / context->tileSize;

        int nTilesX = lastTileX - firstTileX + 1;
        int nTilesY = lastTileY - firstTileY + 1;

        context->maxBinQueueCount += (nTilesX * nTilesY);

        //NOTE: each tile covered by a path may have one start and end op, or one fill op
        context->maxTileOpCount += (nTilesX * nTilesY) * 2;
    }
}

static bool oc_intersect_hull_legs(oc_vec2 p0, oc_vec2 p1, oc_vec2 p2, oc_vec2 p3, oc_vec2* intersection)
{
    /*NOTE: check intersection of lines (p0-p1) and (p2-p3)

		P = p0 + u(p1-p0)
		P = p2 + w(p3-p2)
	*/
    bool found = false;

    f32 den = (p0.x - p1.x) * (p2.y - p3.y) - (p0.y - p1.y) * (p2.x - p3.x);
    if(fabs(den) > 0.0001)
    {
        f32 u = ((p0.x - p2.x) * (p2.y - p3.y) - (p0.y - p2.y) * (p2.x - p3.x)) / den;
        f32 w = ((p0.x - p2.x) * (p0.y - p1.y) - (p0.y - p2.y) * (p0.x - p1.x)) / den;

        intersection->x = p0.x + u * (p1.x - p0.x);
        intersection->y = p0.y + u * (p1.y - p0.y);
        found = true;
    }
    return (found);
}

static bool oc_offset_hull(int count, oc_vec2* p, oc_vec2* result, f32 offset)
{
    //NOTE: we should have no more than two coincident points here. This means the leg between
    //      those two points can't be offset, but we can set a double point at the start of first leg,
    //      end of first leg, or we can join the first and last leg to create a missing middle one

    oc_vec2 legs[3][2] = { 0 };
    bool valid[3] = { 0 };

    for(int i = 0; i < count - 1; i++)
    {
        oc_vec2 n = { p[i].y - p[i + 1].y,
                      p[i + 1].x - p[i].x };

        f32 norm = sqrt(n.x * n.x + n.y * n.y);
        if(norm >= 1e-6)
        {
            n = oc_vec2_mul(offset / norm, n);
            legs[i][0] = oc_vec2_add(p[i], n);
            legs[i][1] = oc_vec2_add(p[i + 1], n);
            valid[i] = true;
        }
    }

    //NOTE: now we find intersections

    // first point is either the start of the first or second leg
    if(valid[0])
    {
        result[0] = legs[0][0];
    }
    else
    {
        OC_ASSERT(valid[1]);
        result[0] = legs[1][0];
    }

    for(int i = 1; i < count - 1; i++)
    {
        //NOTE: we're computing the control point i, at the end of leg (i-1)

        if(!valid[i - 1])
        {
            OC_ASSERT(valid[i]);
            result[i] = legs[i][0];
        }
        else if(!valid[i])
        {
            OC_ASSERT(valid[i - 1]);
            result[i] = legs[i - 1][0];
        }
        else
        {
            if(!oc_intersect_hull_legs(legs[i - 1][0], legs[i - 1][1], legs[i][0], legs[i][1], &result[i]))
            {
                // legs don't intersect.
                return (false);
            }
        }
    }

    if(valid[count - 2])
    {
        result[count - 1] = legs[count - 2][1];
    }
    else
    {
        OC_ASSERT(valid[count - 3]);
        result[count - 1] = legs[count - 3][1];
    }

    return (true);
}

static oc_vec2 oc_quadratic_get_point(oc_vec2 p[3], f32 t)
{
    oc_vec2 r;

    f32 oneMt = 1 - t;
    f32 oneMt2 = oc_square(oneMt);
    f32 t2 = oc_square(t);

    r.x = oneMt2 * p[0].x + 2 * oneMt * t * p[1].x + t2 * p[2].x;
    r.y = oneMt2 * p[0].y + 2 * oneMt * t * p[1].y + t2 * p[2].y;

    return (r);
}

static void oc_quadratic_split(oc_vec2 p[3], f32 t, oc_vec2 outLeft[3], oc_vec2 outRight[3])
{
    //NOTE(martin): split bezier curve p at parameter t, using De Casteljau's algorithm
    //              the q_n are the points along the hull's segments at parameter t
    //              s is the split point.

    f32 oneMt = 1 - t;

    oc_vec2 q0 = { oneMt * p[0].x + t * p[1].x,
                   oneMt * p[0].y + t * p[1].y };

    oc_vec2 q1 = { oneMt * p[1].x + t * p[2].x,
                   oneMt * p[1].y + t * p[2].y };

    oc_vec2 s = { oneMt * q0.x + t * q1.x,
                  oneMt * q0.y + t * q1.y };

    outLeft[0] = p[0];
    outLeft[1] = q0;
    outLeft[2] = s;

    outRight[0] = s;
    outRight[1] = q1;
    outRight[2] = p[2];
}

static oc_vec2 oc_cubic_get_point(oc_vec2 p[4], f32 t)
{
    oc_vec2 r;

    f32 oneMt = 1 - t;
    f32 oneMt2 = oc_square(oneMt);
    f32 oneMt3 = oneMt2 * oneMt;
    f32 t2 = oc_square(t);
    f32 t3 = t2 * t;

    r.x = oneMt3 * p[0].x + 3 * oneMt2 * t * p[1].x + 3 * oneMt * t2 * p[2].x + t3 * p[3].x;
    r.y = oneMt3 * p[0].y + 3 * oneMt2 * t * p[1].y + 3 * oneMt * t2 * p[2].y + t3 * p[3].y;

    return (r);
}

static void oc_cubic_split(oc_vec2 p[4], f32 t, oc_vec2 outLeft[4], oc_vec2 outRight[4])
{
    //NOTE(martin): split bezier curve p at parameter t, using De Casteljau's algorithm
    //              the q_n are the points along the hull's segments at parameter t
    //              the r_n are the points along the (q_n, q_n+1) segments at parameter t
    //              s is the split point.

    oc_vec2 q0 = { (1 - t) * p[0].x + t * p[1].x,
                   (1 - t) * p[0].y + t * p[1].y };

    oc_vec2 q1 = { (1 - t) * p[1].x + t * p[2].x,
                   (1 - t) * p[1].y + t * p[2].y };

    oc_vec2 q2 = { (1 - t) * p[2].x + t * p[3].x,
                   (1 - t) * p[2].y + t * p[3].y };

    oc_vec2 r0 = { (1 - t) * q0.x + t * q1.x,
                   (1 - t) * q0.y + t * q1.y };

    oc_vec2 r1 = { (1 - t) * q1.x + t * q2.x,
                   (1 - t) * q1.y + t * q2.y };

    oc_vec2 s = { (1 - t) * r0.x + t * r1.x,
                  (1 - t) * r0.y + t * r1.y };
    ;

    outLeft[0] = p[0];
    outLeft[1] = q0;
    outLeft[2] = r0;
    outLeft[3] = s;

    outRight[0] = s;
    outRight[1] = r1;
    outRight[2] = q2;
    outRight[3] = p[3];
}

void oc_wgpu_encode_stroke_line(oc_wgpu_canvas_encoding_context* context, oc_vec2* p)
{
    if(p[0].x == p[1].x && p[0].y == p[1].y)
    {
        return;
    }

    f32 width = context->primitive->attributes.width;

    oc_vec2 v = { p[1].x - p[0].x, p[1].y - p[0].y };
    oc_vec2 n = { v.y, -v.x };
    f32 norm = sqrt(n.x * n.x + n.y * n.y);
    oc_vec2 offset = oc_vec2_mul(0.5 * width / norm, n);

    oc_vec2 left[2] = { oc_vec2_add(p[0], offset), oc_vec2_add(p[1], offset) };
    oc_vec2 right[2] = { oc_vec2_add(p[1], oc_vec2_mul(-1, offset)), oc_vec2_add(p[0], oc_vec2_mul(-1, offset)) };
    oc_vec2 joint0[2] = { oc_vec2_add(p[0], oc_vec2_mul(-1, offset)), oc_vec2_add(p[0], offset) };
    oc_vec2 joint1[2] = { oc_vec2_add(p[1], offset), oc_vec2_add(p[1], oc_vec2_mul(-1, offset)) };

    oc_wgpu_canvas_encode_element(context, OC_PATH_LINE, right);
    oc_wgpu_canvas_encode_element(context, OC_PATH_LINE, left);
    oc_wgpu_canvas_encode_element(context, OC_PATH_LINE, joint0);
    oc_wgpu_canvas_encode_element(context, OC_PATH_LINE, joint1);
}

enum
{
    OC_HULL_CHECK_SAMPLE_COUNT = 5
};

void oc_wgpu_encode_stroke_quadratic(oc_wgpu_canvas_encoding_context* context, oc_vec2* p)
{
    f32 width = context->primitive->attributes.width;
    f32 tolerance = oc_min(context->primitive->attributes.tolerance, 0.5 * width);

    //NOTE: check for degenerate line case
    const f32 equalEps = 1e-3;
    if(oc_vec2_close(p[0], p[1], equalEps))
    {
        oc_wgpu_encode_stroke_line(context, p + 1);
        return;
    }
    else if(oc_vec2_close(p[1], p[2], equalEps))
    {
        oc_wgpu_encode_stroke_line(context, p);
        return;
    }

    oc_vec2 leftHull[3];
    oc_vec2 rightHull[3];

    if(!oc_offset_hull(3, p, leftHull, width / 2)
       || !oc_offset_hull(3, p, rightHull, -width / 2))
    {
        //TODO split and recurse
        //NOTE: offsetting the hull failed, split the curve
        oc_vec2 splitLeft[3];
        oc_vec2 splitRight[3];
        oc_quadratic_split(p, 0.5, splitLeft, splitRight);
        oc_wgpu_encode_stroke_quadratic(context, splitLeft);
        oc_wgpu_encode_stroke_quadratic(context, splitRight);
    }
    else
    {
        f32 checkSamples[OC_HULL_CHECK_SAMPLE_COUNT] = { 1. / 6, 2. / 6, 3. / 6, 4. / 6, 5. / 6 };

        f32 d2LowBound = oc_square(0.5 * width - tolerance);
        f32 d2HighBound = oc_square(0.5 * width + tolerance);

        f32 maxOvershoot = 0;
        f32 maxOvershootParameter = 0;

        for(int i = 0; i < OC_HULL_CHECK_SAMPLE_COUNT; i++)
        {
            f32 t = checkSamples[i];

            oc_vec2 c = oc_quadratic_get_point(p, t);
            oc_vec2 cp = oc_quadratic_get_point(leftHull, t);
            oc_vec2 cn = oc_quadratic_get_point(rightHull, t);

            f32 positiveDistSquare = oc_square(c.x - cp.x) + oc_square(c.y - cp.y);
            f32 negativeDistSquare = oc_square(c.x - cn.x) + oc_square(c.y - cn.y);

            f32 positiveOvershoot = oc_max(positiveDistSquare - d2HighBound, d2LowBound - positiveDistSquare);
            f32 negativeOvershoot = oc_max(negativeDistSquare - d2HighBound, d2LowBound - negativeDistSquare);

            f32 overshoot = oc_max(positiveOvershoot, negativeOvershoot);

            if(overshoot > maxOvershoot)
            {
                maxOvershoot = overshoot;
                maxOvershootParameter = t;
            }
        }

        if(maxOvershoot > 0)
        {
            oc_vec2 splitLeft[3];
            oc_vec2 splitRight[3];
            oc_quadratic_split(p, maxOvershootParameter, splitLeft, splitRight);
            oc_wgpu_encode_stroke_quadratic(context, splitLeft);
            oc_wgpu_encode_stroke_quadratic(context, splitRight);
        }
        else
        {
            oc_vec2 tmp = leftHull[0];
            leftHull[0] = leftHull[2];
            leftHull[2] = tmp;

            oc_wgpu_canvas_encode_element(context, OC_PATH_QUADRATIC, rightHull);
            oc_wgpu_canvas_encode_element(context, OC_PATH_QUADRATIC, leftHull);

            oc_vec2 joint0[2] = { rightHull[2], leftHull[0] };
            oc_vec2 joint1[2] = { leftHull[2], rightHull[0] };
            oc_wgpu_canvas_encode_element(context, OC_PATH_LINE, joint0);
            oc_wgpu_canvas_encode_element(context, OC_PATH_LINE, joint1);
        }
    }
}

void oc_wgpu_encode_stroke_cubic(oc_wgpu_canvas_encoding_context* context, oc_vec2* p)
{
    f32 width = context->primitive->attributes.width;
    f32 tolerance = oc_min(context->primitive->attributes.tolerance, 0.5 * width);

    //NOTE: check degenerate line cases
    f32 equalEps = 1e-3;

    if((oc_vec2_close(p[0], p[1], equalEps) && oc_vec2_close(p[2], p[3], equalEps))
       || (oc_vec2_close(p[0], p[1], equalEps) && oc_vec2_close(p[1], p[2], equalEps))
       || (oc_vec2_close(p[1], p[2], equalEps) && oc_vec2_close(p[2], p[3], equalEps)))
    {
        oc_vec2 line[2] = { p[0], p[3] };
        oc_wgpu_encode_stroke_line(context, line);
        return;
    }
    else if(oc_vec2_close(p[0], p[1], equalEps) && oc_vec2_close(p[1], p[3], equalEps))
    {
        oc_vec2 line[2] = { p[0], oc_vec2_add(oc_vec2_mul(5. / 9, p[0]), oc_vec2_mul(4. / 9, p[2])) };
        oc_wgpu_encode_stroke_line(context, line);
        return;
    }
    else if(oc_vec2_close(p[0], p[2], equalEps) && oc_vec2_close(p[2], p[3], equalEps))
    {
        oc_vec2 line[2] = { p[0], oc_vec2_add(oc_vec2_mul(5. / 9, p[0]), oc_vec2_mul(4. / 9, p[1])) };
        oc_wgpu_encode_stroke_line(context, line);
        return;
    }

    oc_vec2 leftHull[4];
    oc_vec2 rightHull[4];

    if(!oc_offset_hull(4, p, leftHull, width / 2)
       || !oc_offset_hull(4, p, rightHull, -width / 2))
    {
        //TODO split and recurse
        //NOTE: offsetting the hull failed, split the curve
        oc_vec2 splitLeft[4];
        oc_vec2 splitRight[4];
        oc_cubic_split(p, 0.5, splitLeft, splitRight);
        oc_wgpu_encode_stroke_cubic(context, splitLeft);
        oc_wgpu_encode_stroke_cubic(context, splitRight);
    }
    else
    {
        f32 checkSamples[OC_HULL_CHECK_SAMPLE_COUNT] = { 1. / 6, 2. / 6, 3. / 6, 4. / 6, 5. / 6 };

        f32 d2LowBound = oc_square(0.5 * width - tolerance);
        f32 d2HighBound = oc_square(0.5 * width + tolerance);

        f32 maxOvershoot = 0;
        f32 maxOvershootParameter = 0;

        for(int i = 0; i < OC_HULL_CHECK_SAMPLE_COUNT; i++)
        {
            f32 t = checkSamples[i];

            oc_vec2 c = oc_cubic_get_point(p, t);
            oc_vec2 cp = oc_cubic_get_point(leftHull, t);
            oc_vec2 cn = oc_cubic_get_point(rightHull, t);

            f32 positiveDistSquare = oc_square(c.x - cp.x) + oc_square(c.y - cp.y);
            f32 negativeDistSquare = oc_square(c.x - cn.x) + oc_square(c.y - cn.y);

            f32 positiveOvershoot = oc_max(positiveDistSquare - d2HighBound, d2LowBound - positiveDistSquare);
            f32 negativeOvershoot = oc_max(negativeDistSquare - d2HighBound, d2LowBound - negativeDistSquare);

            f32 overshoot = oc_max(positiveOvershoot, negativeOvershoot);

            if(overshoot > maxOvershoot)
            {
                maxOvershoot = overshoot;
                maxOvershootParameter = t;
            }
        }

        if(maxOvershoot > 0)
        {
            oc_vec2 splitLeft[4];
            oc_vec2 splitRight[4];
            oc_cubic_split(p, maxOvershootParameter, splitLeft, splitRight);
            oc_wgpu_encode_stroke_cubic(context, splitLeft);
            oc_wgpu_encode_stroke_cubic(context, splitRight);
        }
        else
        {
            oc_vec2 tmp = leftHull[0];
            leftHull[0] = leftHull[3];
            leftHull[3] = tmp;
            tmp = leftHull[1];
            leftHull[1] = leftHull[2];
            leftHull[2] = tmp;

            oc_wgpu_canvas_encode_element(context, OC_PATH_CUBIC, rightHull);
            oc_wgpu_canvas_encode_element(context, OC_PATH_CUBIC, leftHull);

            oc_vec2 joint0[2] = { rightHull[3], leftHull[0] };
            oc_vec2 joint1[2] = { leftHull[3], rightHull[0] };
            oc_wgpu_canvas_encode_element(context, OC_PATH_LINE, joint0);
            oc_wgpu_canvas_encode_element(context, OC_PATH_LINE, joint1);
        }
    }
}

void oc_wgpu_encode_stroke_element(oc_wgpu_canvas_encoding_context* context,
                                   oc_path_elt* element,
                                   oc_vec2 currentPoint,
                                   oc_vec2* startTangent,
                                   oc_vec2* endTangent,
                                   oc_vec2* endPoint)
{
    oc_vec2 controlPoints[4] = { currentPoint, element->p[0], element->p[1], element->p[2] };
    int endPointIndex = 0;

    switch(element->type)
    {
        case OC_PATH_LINE:
            oc_wgpu_encode_stroke_line(context, controlPoints);
            endPointIndex = 1;
            break;

        case OC_PATH_QUADRATIC:
            oc_wgpu_encode_stroke_quadratic(context, controlPoints);
            endPointIndex = 2;
            break;

        case OC_PATH_CUBIC:
            oc_wgpu_encode_stroke_cubic(context, controlPoints);
            endPointIndex = 3;
            break;

        case OC_PATH_MOVE:
            OC_ASSERT(0, "should be unreachable");
            break;
    }

    //NOTE: ensure tangents are properly computed even in presence of coincident points
    //TODO: see if we can do this in a less hacky way

    for(int i = 1; i < 4; i++)
    {
        if(controlPoints[i].x != controlPoints[0].x
           || controlPoints[i].y != controlPoints[0].y)
        {
            *startTangent = (oc_vec2){ .x = controlPoints[i].x - controlPoints[0].x,
                                       .y = controlPoints[i].y - controlPoints[0].y };
            break;
        }
    }
    *endPoint = controlPoints[endPointIndex];

    for(int i = endPointIndex - 1; i >= 0; i++)
    {
        if(controlPoints[i].x != endPoint->x
           || controlPoints[i].y != endPoint->y)
        {
            *endTangent = (oc_vec2){ .x = endPoint->x - controlPoints[i].x,
                                     .y = endPoint->y - controlPoints[i].y };
            break;
        }
    }
    OC_DEBUG_ASSERT(startTangent->x != 0 || startTangent->y != 0);
}

void oc_wgpu_stroke_cap(oc_wgpu_canvas_encoding_context* context,
                        oc_vec2 p0,
                        oc_vec2 direction)
{
    oc_attributes* attributes = &context->primitive->attributes;

    //NOTE(martin): compute the tangent and normal vectors (multiplied by half width) at the cap point
    f32 dn = sqrt(oc_square(direction.x) + oc_square(direction.y));
    f32 alpha = 0.5 * attributes->width / dn;

    oc_vec2 n0 = { -alpha * direction.y,
                   alpha * direction.x };

    oc_vec2 m0 = { alpha * direction.x,
                   alpha * direction.y };

    oc_vec2 points[] = { { p0.x + n0.x, p0.y + n0.y },
                         { p0.x + n0.x + m0.x, p0.y + n0.y + m0.y },
                         { p0.x - n0.x + m0.x, p0.y - n0.y + m0.y },
                         { p0.x - n0.x, p0.y - n0.y },
                         { p0.x + n0.x, p0.y + n0.y } };

    oc_wgpu_canvas_encode_element(context, OC_PATH_LINE, points);
    oc_wgpu_canvas_encode_element(context, OC_PATH_LINE, points + 1);
    oc_wgpu_canvas_encode_element(context, OC_PATH_LINE, points + 2);
    oc_wgpu_canvas_encode_element(context, OC_PATH_LINE, points + 3);
}

void oc_wgpu_stroke_joint(oc_wgpu_canvas_encoding_context* context,
                          oc_vec2 p0,
                          oc_vec2 t0,
                          oc_vec2 t1)
{
    oc_attributes* attributes = &context->primitive->attributes;

    //NOTE(martin): compute the normals at the joint point
    f32 norm_t0 = sqrt(oc_square(t0.x) + oc_square(t0.y));
    f32 norm_t1 = sqrt(oc_square(t1.x) + oc_square(t1.y));

    oc_vec2 n0 = { -t0.y, t0.x };
    n0.x /= norm_t0;
    n0.y /= norm_t0;

    oc_vec2 n1 = { -t1.y, t1.x };
    n1.x /= norm_t1;
    n1.y /= norm_t1;

    //NOTE(martin): the sign of the cross product determines if the normals are facing outwards or inwards the angle.
    //              we flip them to face outwards if needed
    f32 crossZ = n0.x * n1.y - n0.y * n1.x;
    if(crossZ > 0)
    {
        n0.x *= -1;
        n0.y *= -1;
        n1.x *= -1;
        n1.y *= -1;
    }

    //NOTE(martin): use the same code as hull offset to find mitter point...
    /*NOTE(martin): let vector u = (n0+n1) and vector v = pIntersect - p1
		then v = u * (2*offset / norm(u)^2)
		(this can be derived from writing the pythagoras theorems in the triangles of the joint)
	*/
    f32 halfW = 0.5 * attributes->width;
    oc_vec2 u = { n0.x + n1.x, n0.y + n1.y };
    f32 uNormSquare = u.x * u.x + u.y * u.y;
    f32 alpha = attributes->width / uNormSquare;
    oc_vec2 v = { u.x * alpha, u.y * alpha };

    f32 excursionSquare = uNormSquare * oc_square(alpha - attributes->width / 4);

    if(attributes->joint == OC_JOINT_MITER
       && excursionSquare <= oc_square(attributes->maxJointExcursion))
    {
        //NOTE(martin): add a mitter joint
        oc_vec2 points[] = { p0,
                             { p0.x + n0.x * halfW, p0.y + n0.y * halfW },
                             { p0.x + v.x, p0.y + v.y },
                             { p0.x + n1.x * halfW, p0.y + n1.y * halfW },
                             p0 };

        oc_wgpu_canvas_encode_element(context, OC_PATH_LINE, points);
        oc_wgpu_canvas_encode_element(context, OC_PATH_LINE, points + 1);
        oc_wgpu_canvas_encode_element(context, OC_PATH_LINE, points + 2);
        oc_wgpu_canvas_encode_element(context, OC_PATH_LINE, points + 3);
    }
    else
    {
        //NOTE(martin): add a bevel joint
        oc_vec2 points[] = { p0,
                             { p0.x + n0.x * halfW, p0.y + n0.y * halfW },
                             { p0.x + n1.x * halfW, p0.y + n1.y * halfW },
                             p0 };

        oc_wgpu_canvas_encode_element(context, OC_PATH_LINE, points);
        oc_wgpu_canvas_encode_element(context, OC_PATH_LINE, points + 1);
        oc_wgpu_canvas_encode_element(context, OC_PATH_LINE, points + 2);
    }
}

u32 oc_wgpu_encode_stroke_subpath(oc_wgpu_canvas_encoding_context* context,
                                  oc_path_elt* elements,
                                  oc_path_descriptor* path,
                                  u32 startIndex,
                                  oc_vec2 startPoint)
{
    u32 eltCount = path->count;
    OC_DEBUG_ASSERT(startIndex < eltCount);

    oc_vec2 currentPoint = startPoint;
    oc_vec2 endPoint = { 0, 0 };
    oc_vec2 previousEndTangent = { 0, 0 };
    oc_vec2 firstTangent = { 0, 0 };
    oc_vec2 startTangent = { 0, 0 };
    oc_vec2 endTangent = { 0, 0 };

    //NOTE(martin): encode first element and compute first tangent
    oc_wgpu_encode_stroke_element(context, elements + startIndex, currentPoint, &startTangent, &endTangent, &endPoint);

    firstTangent = startTangent;
    previousEndTangent = endTangent;
    currentPoint = endPoint;

    //NOTE(martin): encode subsequent elements along with their joints

    oc_attributes* attributes = &context->primitive->attributes;

    u32 eltIndex = startIndex + 1;
    for(;
        eltIndex < eltCount && elements[eltIndex].type != OC_PATH_MOVE;
        eltIndex++)
    {
        oc_wgpu_encode_stroke_element(context, elements + eltIndex, currentPoint, &startTangent, &endTangent, &endPoint);

        if(attributes->joint != OC_JOINT_NONE)
        {
            oc_wgpu_stroke_joint(context, currentPoint, previousEndTangent, startTangent);
        }
        previousEndTangent = endTangent;
        currentPoint = endPoint;
    }
    u32 subPathEltCount = eltIndex - startIndex;

    //NOTE(martin): draw end cap / joint. We ensure there's at least two segments to draw a closing joint
    if(subPathEltCount > 1
       && startPoint.x == endPoint.x
       && startPoint.y == endPoint.y)
    {
        if(attributes->joint != OC_JOINT_NONE)
        {
            //NOTE(martin): add a closing joint if the path is closed
            oc_wgpu_stroke_joint(context, endPoint, endTangent, firstTangent);
        }
    }
    else if(attributes->cap == OC_CAP_SQUARE)
    {
        //NOTE(martin): add start and end cap
        oc_wgpu_stroke_cap(context, startPoint, (oc_vec2){ -startTangent.x, -startTangent.y });
        oc_wgpu_stroke_cap(context, endPoint, endTangent);
    }
    return (eltIndex);
}

void oc_wgpu_encode_stroke(oc_wgpu_canvas_encoding_context* context,
                           oc_path_elt* elements,
                           oc_path_descriptor* path)
{
    u32 eltCount = path->count;
    OC_DEBUG_ASSERT(eltCount);

    oc_vec2 startPoint = path->startPoint;
    u32 startIndex = 0;

    while(startIndex < eltCount)
    {
        //NOTE(martin): eliminate leading moves
        while(startIndex < eltCount && elements[startIndex].type == OC_PATH_MOVE)
        {
            startPoint = elements[startIndex].p[0];
            startIndex++;
        }
        if(startIndex < eltCount)
        {
            startIndex = oc_wgpu_encode_stroke_subpath(context, elements, path, startIndex, startPoint);
        }
    }
}

bool oc_wgpu_grow_buffer_if_needed(oc_wgpu_canvas_renderer* renderer,
                                   WGPUBuffer* buffer,
                                   u32 minCap,
                                   u32 eltSize,
                                   const char* label,
                                   WGPUBufferUsageFlags usage)
{
    bool updateBuffer = false;

    if((*buffer == 0)
       || (wgpuBufferGetSize(*buffer) < minCap * eltSize))
    {
        if(*buffer)
        {
            wgpuBufferRelease(*buffer);
        }

        u32 newCap = oc_max(minCap * 1.5, OC_WGPU_CANVAS_BUFFER_DEFAULT_LEN);
        u32 bufferLimit = oc_min(renderer->limits.maxBufferSize, renderer->limits.maxStorageBufferBindingSize);
        u32 newSize = oc_clamp_high(newCap * eltSize, bufferLimit);

        WGPUBufferDescriptor desc = {
            .label = label,
            .usage = usage,
            .size = newSize,
        };
        *buffer = wgpuDeviceCreateBuffer(renderer->device, &desc);

        updateBuffer = true;

        //oc_log_info("grow %s, elt = %i, size = %i\n", label, newCap, desc.size);
    }

    return (updateBuffer);
}

void oc_wgpu_canvas_update_resources_if_needed(oc_wgpu_canvas_encoding_context* context)
{
    oc_wgpu_canvas_renderer* renderer = context->renderer;

    bool updatePathBuffer = oc_wgpu_grow_buffer_if_needed(renderer,
                                                          &renderer->pathBuffer,
                                                          context->pathCount,
                                                          sizeof(oc_wgpu_path),
                                                          "path buffer",
                                                          WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst);

    bool updateElementBuffer = oc_wgpu_grow_buffer_if_needed(renderer,
                                                             &renderer->elementBuffer,
                                                             context->eltCount,
                                                             sizeof(oc_wgpu_path_elt),
                                                             "element buffer",
                                                             WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst);

    bool updateSegmentBuffer = oc_wgpu_grow_buffer_if_needed(renderer,
                                                             &renderer->segmentBuffer,
                                                             context->maxSegmentCount,
                                                             sizeof(oc_wgpu_segment),
                                                             "segment buffer",
                                                             WGPUBufferUsage_Storage);

    bool updatePathBinBuffer = oc_wgpu_grow_buffer_if_needed(renderer,
                                                             &renderer->pathBinBuffer,
                                                             context->pathCount,
                                                             sizeof(oc_wgpu_path_bin),
                                                             "path bins buffer",
                                                             WGPUBufferUsage_Storage);

    bool updateBinQueueBuffer = oc_wgpu_grow_buffer_if_needed(renderer,
                                                              &renderer->binQueueBuffer,
                                                              context->maxBinQueueCount,
                                                              sizeof(oc_wgpu_bin_queue),
                                                              "bin queues buffer",
                                                              WGPUBufferUsage_Storage);

    u32 maxTileQueues = oc_min(context->maxBinQueueCount, context->screenTilesCount);
    bool updateTileQueueBuffer = oc_wgpu_grow_buffer_if_needed(renderer,
                                                               &renderer->tileQueueBuffer,
                                                               maxTileQueues,
                                                               sizeof(oc_wgpu_tile_queue),
                                                               "tile queues buffer",
                                                               WGPUBufferUsage_Storage);

    bool updateTileOpBuffer = oc_wgpu_grow_buffer_if_needed(renderer,
                                                            &renderer->tileOpBuffer,
                                                            context->maxTileOpCount,
                                                            sizeof(oc_wgpu_tile_op),
                                                            "tile ops buffer",
                                                            WGPUBufferUsage_Storage);

    bool updateChunkBuffer = oc_wgpu_grow_buffer_if_needed(renderer,
                                                           &renderer->chunkBuffer,
                                                           context->chunkCount,
                                                           sizeof(oc_wgpu_chunk),
                                                           "chunks buffer",
                                                           WGPUBufferUsage_Storage);

    bool updateChunkEltBuffer = oc_wgpu_grow_buffer_if_needed(renderer,
                                                              &renderer->chunkEltBuffer,
                                                              context->maxChunkEltCount,
                                                              sizeof(oc_wgpu_chunk_elt),
                                                              "chunk elements buffer",
                                                              WGPUBufferUsage_Storage);

    //NOTE: resize outTexture if needed
    bool updateTexture = (renderer->outTextureView == 0)
                      || (renderer->outTextureSize.x != context->screenSize.x)
                      || (renderer->outTextureSize.y != context->screenSize.y);

    if(updateTexture)
    {
        if(renderer->outTextureView)
        {
            wgpuTextureViewRelease(renderer->outTextureView);
        }

        WGPUTextureDescriptor desc = {
            .usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_StorageBinding | WGPUTextureUsage_RenderAttachment,
            .dimension = WGPUTextureDimension_2D,
            .size = { context->screenSize.x, context->screenSize.y, 1 },
            .format = WGPUTextureFormat_RGBA8Unorm,
            .mipLevelCount = 1,
            .sampleCount = 1,
        };
        WGPUTexture texture = wgpuDeviceCreateTexture(renderer->device, &desc);

        WGPUTextureViewDescriptor viewDesc = {
            .format = desc.format,
            .dimension = WGPUTextureViewDimension_2D,
            .baseMipLevel = 0,
            .mipLevelCount = 1,
            .baseArrayLayer = 0,
            .arrayLayerCount = 1,
            .aspect = WGPUTextureAspect_All,
        };
        renderer->outTextureView = wgpuTextureCreateView(texture, &viewDesc);
        wgpuTextureRelease(texture);

        renderer->outTextureSize = context->screenSize;
    }

    //----------------------------------------------------------------------------------------
    //NOTE: path setup bindgroup

    if((renderer->pathSetupBindGroup == 0)
       || updatePathBuffer
       || updatePathBinBuffer
       || updateBinQueueBuffer)
    {
        if(renderer->pathSetupBindGroup)
        {
            wgpuBindGroupRelease(renderer->pathSetupBindGroup);
        }

        WGPUBindGroupDescriptor bindGroupDesc = {
            .layout = renderer->pathSetupBindGroupLayout,
            .entryCount = 6,
            .entries = (WGPUBindGroupEntry[]){
                {
                    .binding = 0,
                    .buffer = renderer->pathBuffer,
                    .size = wgpuBufferGetSize(renderer->pathBuffer),
                },
                {
                    .binding = 1,
                    .buffer = renderer->pathCountBuffer,
                    .size = sizeof(u32),
                },
                {
                    .binding = 2,
                    .buffer = renderer->pathBinBuffer,
                    .size = wgpuBufferGetSize(renderer->pathBinBuffer),
                },
                {
                    .binding = 3,
                    .buffer = renderer->binQueueBuffer,
                    .size = wgpuBufferGetSize(renderer->binQueueBuffer),
                },
                {
                    .binding = 4,
                    .buffer = renderer->binQueueCountBuffer,
                    .size = sizeof(u32),
                },
                {
                    .binding = 5,
                    .buffer = renderer->tileSizeBuffer,
                    .size = sizeof(u32),
                },
            }
        };
        renderer->pathSetupBindGroup = wgpuDeviceCreateBindGroup(renderer->device, &bindGroupDesc);
    }

    //----------------------------------------------------------------------------------------
    //NOTE: segment setup pass bindgroup

    if((renderer->segmentSetupBindGroup == 0)
       || updatePathBuffer
       || updateElementBuffer
       || updateSegmentBuffer
       || updatePathBinBuffer
       || updateBinQueueBuffer
       || updateTileOpBuffer)
    {
        if(renderer->segmentSetupBindGroup)
        {
            wgpuBindGroupRelease(renderer->segmentSetupBindGroup);
        }

        WGPUBindGroupDescriptor bindGroupDesc = {
            .layout = renderer->segmentSetupBindGroupLayout,
            .entryCount = 10,
            .entries = (WGPUBindGroupEntry[]){
                {
                    .binding = 0,
                    .buffer = renderer->pathBuffer,
                    .size = wgpuBufferGetSize(renderer->pathBuffer),
                },
                {
                    .binding = 1,
                    .buffer = renderer->elementBuffer,
                    .size = wgpuBufferGetSize(renderer->elementBuffer),
                },
                {
                    .binding = 2,
                    .buffer = renderer->elementCountBuffer,
                    .size = sizeof(u32),
                },
                {
                    .binding = 3,
                    .buffer = renderer->segmentBuffer,
                    .size = wgpuBufferGetSize(renderer->segmentBuffer),
                },
                {
                    .binding = 4,
                    .buffer = renderer->segmentCountBuffer,
                    .size = sizeof(u32),
                },
                {
                    .binding = 5,
                    .buffer = renderer->pathBinBuffer,
                    .size = wgpuBufferGetSize(renderer->pathBinBuffer),
                },
                {
                    .binding = 6,
                    .buffer = renderer->binQueueBuffer,
                    .size = wgpuBufferGetSize(renderer->binQueueBuffer),
                },
                {
                    .binding = 7,
                    .buffer = renderer->tileOpBuffer,
                    .size = wgpuBufferGetSize(renderer->tileOpBuffer),
                },
                {
                    .binding = 8,
                    .buffer = renderer->tileOpCountBuffer,
                    .size = sizeof(u32),
                },
                {
                    .binding = 9,
                    .buffer = renderer->tileSizeBuffer,
                    .size = sizeof(u32),
                },
            }
        };

        renderer->segmentSetupBindGroup = wgpuDeviceCreateBindGroup(renderer->device, &bindGroupDesc);
    }

    //----------------------------------------------------------------------------------------
    //NOTE: backprop pass bindgroup

    if(renderer->backpropBindGroup == 0
       || updatePathBinBuffer
       || updateBinQueueBuffer)
    {
        if(renderer->backpropBindGroup)
        {
            wgpuBindGroupRelease(renderer->backpropBindGroup);
        }

        WGPUBindGroupDescriptor bindGroupDesc = {
            .layout = renderer->backpropBindGroupLayout,
            .entryCount = 3,
            .entries = (WGPUBindGroupEntry[]){
                {
                    .binding = 0,
                    .buffer = renderer->pathBinBuffer,
                    .size = wgpuBufferGetSize(renderer->pathBinBuffer),
                },
                {
                    .binding = 1,
                    .buffer = renderer->pathCountBuffer,
                    .size = sizeof(u32),
                },
                {
                    .binding = 2,
                    .buffer = renderer->binQueueBuffer,
                    .size = wgpuBufferGetSize(renderer->binQueueBuffer),
                },
            }
        };

        renderer->backpropBindGroup = wgpuDeviceCreateBindGroup(renderer->device, &bindGroupDesc);
    }

    //----------------------------------------------------------------------------------------
    //NOTE: chunk pass bindgroup

    if(renderer->chunkBindGroup == 0
       || updatePathBuffer
       || updateChunkBuffer
       || updateChunkEltBuffer)
    {
        if(renderer->chunkBindGroup)
        {
            wgpuBindGroupRelease(renderer->chunkBindGroup);
        }

        WGPUBindGroupDescriptor bindGroupDesc = {
            .layout = renderer->chunkBindGroupLayout,
            .entryCount = 6,
            .entries = (WGPUBindGroupEntry[]){
                {
                    .binding = 0,
                    .buffer = renderer->pathBuffer,
                    .size = wgpuBufferGetSize(renderer->pathBuffer),
                },
                {
                    .binding = 1,
                    .buffer = renderer->pathCountBuffer,
                    .size = sizeof(u32),
                },
                {
                    .binding = 2,
                    .buffer = renderer->chunkBuffer,
                    .size = wgpuBufferGetSize(renderer->chunkBuffer),
                },
                {
                    .binding = 3,
                    .buffer = renderer->chunkEltBuffer,
                    .size = wgpuBufferGetSize(renderer->chunkEltBuffer),
                },
                {
                    .binding = 4,
                    .buffer = renderer->chunkEltCountBuffer,
                    .size = sizeof(u32),
                },
                {
                    .binding = 5,
                    .buffer = renderer->chunkSizeBuffer,
                    .size = sizeof(u32),
                },
            },
        };

        renderer->chunkBindGroup = wgpuDeviceCreateBindGroup(renderer->device, &bindGroupDesc);
    }

    //----------------------------------------------------------------------------------------
    //NOTE: merge pass bindgroup

    if((renderer->mergeBindGroup == 0)
       || updatePathBuffer
       || updatePathBinBuffer
       || updateBinQueueBuffer
       || updateTileOpBuffer
       || updateTileQueueBuffer
       || updateChunkBuffer
       || updateChunkEltBuffer)
    {
        if(renderer->mergeBindGroup)
        {
            wgpuBindGroupRelease(renderer->mergeBindGroup);
        }

        WGPUBindGroupDescriptor bindGroupDesc = {
            .layout = renderer->mergeBindGroupLayout,
            .entryCount = 13,
            .entries = (WGPUBindGroupEntry[]){
                {
                    .binding = 0,
                    .buffer = renderer->pathBuffer,
                    .size = wgpuBufferGetSize(renderer->pathBuffer),
                },
                {
                    .binding = 1,
                    .buffer = renderer->pathCountBuffer,
                    .size = sizeof(u32),
                },
                {
                    .binding = 2,
                    .buffer = renderer->chunkBuffer,
                    .size = wgpuBufferGetSize(renderer->chunkBuffer),
                },
                {
                    .binding = 3,
                    .buffer = renderer->chunkEltBuffer,
                    .size = wgpuBufferGetSize(renderer->chunkEltBuffer),
                },
                {
                    .binding = 4,
                    .buffer = renderer->pathBinBuffer,
                    .size = wgpuBufferGetSize(renderer->pathBinBuffer),
                },
                {
                    .binding = 5,
                    .buffer = renderer->binQueueBuffer,
                    .size = wgpuBufferGetSize(renderer->binQueueBuffer),
                },
                {
                    .binding = 6,
                    .buffer = renderer->binQueueCountBuffer,
                    .size = sizeof(u32),
                },
                {
                    .binding = 7,
                    .buffer = renderer->tileOpBuffer,
                    .size = wgpuBufferGetSize(renderer->tileOpBuffer),
                },
                {
                    .binding = 8,
                    .buffer = renderer->tileOpCountBuffer,
                    .size = sizeof(u32),
                },
                {
                    .binding = 9,
                    .buffer = renderer->tileQueueBuffer,
                    .size = wgpuBufferGetSize(renderer->tileQueueBuffer),
                },
                {
                    .binding = 10,
                    .buffer = renderer->tileQueueCountBuffer,
                    .size = sizeof(u32),
                },
                {
                    .binding = 11,
                    .buffer = renderer->tileSizeBuffer,
                    .size = sizeof(u32),
                },
                {
                    .binding = 12,
                    .buffer = renderer->chunkSizeBuffer,
                    .size = sizeof(u32),
                },
            }
        };

        renderer->mergeBindGroup = wgpuDeviceCreateBindGroup(renderer->device, &bindGroupDesc);
    }

    //----------------------------------------------------------------------------------------
    //NOTE: raster pass bindgroup

    if((renderer->rasterBindGroup == 0)
       || updatePathBuffer
       || updateSegmentBuffer
       || updateTileQueueBuffer
       || updateTileOpBuffer
       || updateTexture)
    {
        if(renderer->rasterBindGroup)
        {
            wgpuBindGroupRelease(renderer->rasterBindGroup);
        }

        WGPUBindGroupDescriptor bindGroupDesc = {
            .layout = renderer->rasterBindGroupLayout,
            .entryCount = 9,
            .entries = (WGPUBindGroupEntry[]){
                {
                    .binding = 0,
                    .buffer = renderer->pathBuffer,
                    .size = wgpuBufferGetSize(renderer->pathBuffer),
                },
                {
                    .binding = 1,
                    .buffer = renderer->segmentBuffer,
                    .size = wgpuBufferGetSize(renderer->segmentBuffer),
                },
                {
                    .binding = 2,
                    .buffer = renderer->tileQueueBuffer,
                    .size = wgpuBufferGetSize(renderer->tileQueueBuffer),
                },
                {
                    .binding = 3,
                    .buffer = renderer->tileOpBuffer,
                    .size = wgpuBufferGetSize(renderer->tileOpBuffer),
                },
                {
                    .binding = 4,
                    .buffer = renderer->msaaOffsetsBuffer,
                    .size = OC_WGPU_CANVAS_MAX_SAMPLE_COUNT * sizeof(oc_vec2),
                },
                {
                    .binding = 5,
                    .buffer = renderer->msaaSampleCountBuffer,
                    .size = sizeof(u32),
                },
                {
                    .binding = 6,
                    .buffer = renderer->tileSizeBuffer,
                    .size = sizeof(u32),
                },
                {
                    .binding = 7,
                    .textureView = renderer->outTextureView,
                },
                {
                    .binding = 8,
                    .buffer = renderer->debugDisplayOptionsBuffer,
                    .size = sizeof(oc_wgpu_debug_display_options),
                },
            }
        };

        renderer->rasterBindGroup = wgpuDeviceCreateBindGroup(renderer->device, &bindGroupDesc);
    }

    //----------------------------------------------------------------------------------------
    //NOTE: source textures bindgroup. For now, always recreate

    {
        if(renderer->srcTexturesBindGroup)
        {
            wgpuBindGroupRelease(renderer->srcTexturesBindGroup);
        }

        WGPUBindGroupEntry entries[OC_WGPU_CANVAS_MAX_IMAGE_BINDINGS];
        for(int i = 0; i < OC_WGPU_CANVAS_MAX_IMAGE_BINDINGS; i++)
        {
            oc_wgpu_image* image = (oc_wgpu_image*)oc_image_from_handle(context->imageBindings[i]);
            if(image)
            {
                entries[i] = (WGPUBindGroupEntry){
                    .binding = i,
                    .textureView = image->textureView,
                };
            }
            else
            {
                entries[i] = (WGPUBindGroupEntry){
                    .binding = i,
                    .textureView = renderer->dummyTextureView,
                };
            }
        }

        WGPUBindGroupDescriptor bindGroupDesc = {
            .layout = renderer->srcTexturesBindGroupLayout,
            .entryCount = OC_WGPU_CANVAS_MAX_IMAGE_BINDINGS,
            .entries = entries,
        };
        renderer->srcTexturesBindGroup = wgpuDeviceCreateBindGroup(renderer->device, &bindGroupDesc);
    }

    //----------------------------------------------------------------------------------------
    // Blit bindgroup

    if((renderer->blitBindGroup == 0)
       || updateTexture)
    {
        if(renderer->blitBindGroup)
        {
            wgpuBindGroupRelease(renderer->blitBindGroup);
        }

        WGPUBindGroupDescriptor bindGroupDesc = {
            .layout = renderer->blitBindGroupLayout,
            .entryCount = 1,
            .entries = (WGPUBindGroupEntry[]){
                {
                    .binding = 0,
                    .textureView = renderer->outTextureView,
                },
            }
        };
        renderer->blitBindGroup = wgpuDeviceCreateBindGroup(renderer->device, &bindGroupDesc);
    }
}

bool oc_wgpu_canvas_encode_batch(oc_wgpu_canvas_encoding_context* context)
{
    if(context->pathBatchStart >= context->inputPrimitiveCount)
    {
        return (false);
    }
    else
    {
        oc_wgpu_canvas_renderer* renderer = context->renderer;

        //convert primitives to wgpu_paths
        oc_arena_scope scratch = oc_scratch_begin();
        context->arena = scratch.arena;

        context->pathData = 0;
        context->pathCap = 0;
        context->pathCount = 0;

        context->elementData = 0;
        context->eltCap = 0;
        context->eltCount = 0;

        context->maxSegmentCount = 0;
        context->maxBinQueueCount = 0;
        context->maxTileOpCount = 0;

        context->imageCount = 0;
        context->currentImageIndex = -1;

        // should be conveyed from one batch to the other
        oc_vec2 currentPos = (oc_vec2){ 0, 0 };

        int primitiveIndex = 0;
        for(; primitiveIndex + context->pathBatchStart < context->inputPrimitiveCount; primitiveIndex++)
        {
            oc_primitive* primitive = &context->inputPrimitives[context->pathBatchStart + primitiveIndex];

            if(primitive->attributes.image.h != 0)
            {
                context->currentImageIndex = -1;
                for(int i = 0; i < context->imageCount; i++)
                {
                    if(context->imageBindings[i].h == primitive->attributes.image.h)
                    {
                        context->currentImageIndex = i;
                    }
                }
                if(context->currentImageIndex <= 0)
                {
                    if(context->imageCount < OC_WGPU_CANVAS_MAX_IMAGE_BINDINGS)
                    {
                        context->imageBindings[context->imageCount] = primitive->attributes.image;
                        context->currentImageIndex = context->imageCount;
                        context->imageCount++;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else
            {
                context->currentImageIndex = -1;
            }

            u32 lastPathCount = context->pathCount;
            u32 lastEltCount = context->eltCount;
            u32 lastMaxSegmentCount = context->maxSegmentCount;
            u32 lastMaxBinQueueCount = context->maxBinQueueCount;
            u32 lastMaxTileOpCount = context->maxTileOpCount;

            if(primitive->path.count)
            {
                context->primitive = primitive;
                context->pathScreenExtents = (oc_vec4){ FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX };
                context->pathUserExtents = (oc_vec4){ FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX };

                if(primitive->cmd == OC_CMD_STROKE)
                {
                    oc_wgpu_encode_stroke(context, context->inputElements + primitive->path.startIndex, &primitive->path);
                }
                else
                {
                    for(int eltIndex = 0;
                        (eltIndex < primitive->path.count) && (primitive->path.startIndex + eltIndex < context->inputEltCount);
                        eltIndex++)
                    {
                        oc_path_elt* elt = &context->inputElements[primitive->path.startIndex + eltIndex];

                        if(elt->type != OC_PATH_MOVE)
                        {
                            oc_vec2 p[4] = { currentPos, elt->p[0], elt->p[1], elt->p[2] };
                            oc_wgpu_canvas_encode_element(context, elt->type, p);
                        }
                        switch(elt->type)
                        {
                            case OC_PATH_MOVE:
                                currentPos = elt->p[0];
                                break;

                            case OC_PATH_LINE:
                                currentPos = elt->p[0];
                                break;

                            case OC_PATH_QUADRATIC:
                                currentPos = elt->p[1];
                                break;

                            case OC_PATH_CUBIC:
                                currentPos = elt->p[2];
                                break;
                        }
                    }
                }
                oc_wgpu_canvas_encode_path(context, primitive);

                //NOTE: check encoding limits
                {
                    u32 bufferLimit = oc_min(renderer->limits.maxBufferSize, renderer->limits.maxStorageBufferBindingSize);
                    u32 maxTileQueues = oc_min(context->maxBinQueueCount, context->screenTilesCount);

                    if(context->pathCount * sizeof(oc_wgpu_path) >= bufferLimit
                       || context->eltCount * sizeof(oc_wgpu_path_elt) >= bufferLimit
                       || context->maxSegmentCount * sizeof(oc_wgpu_segment) >= bufferLimit
                       || context->pathCount * sizeof(oc_wgpu_path_bin) >= bufferLimit
                       || context->maxBinQueueCount * sizeof(oc_wgpu_bin_queue) >= bufferLimit
                       || maxTileQueues * sizeof(oc_wgpu_tile_queue) >= bufferLimit
                       || context->maxTileOpCount * sizeof(oc_wgpu_tile_op) >= bufferLimit)
                    {
                        //NOTE: if batch overflows max GPU buffer size, rewind to last path and break the batch here.
                        context->pathCount = lastPathCount;
                        context->eltCount = lastEltCount;
                        context->maxSegmentCount = lastMaxSegmentCount;
                        context->maxBinQueueCount = lastMaxBinQueueCount;
                        context->maxTileOpCount = lastMaxTileOpCount;
                        //TODO flush counter?
                        break;
                    }
                }
            }
        }

        int nChunkX = ((int)context->screenSize.x + OC_WGPU_CANVAS_CHUNK_SIZE - 1) / OC_WGPU_CANVAS_CHUNK_SIZE;
        int nChunkY = ((int)context->screenSize.y + OC_WGPU_CANVAS_CHUNK_SIZE - 1) / OC_WGPU_CANVAS_CHUNK_SIZE;
        context->chunkCount = nChunkX * nChunkY;
        context->maxChunkEltCount = context->chunkCount * context->pathCount;

        oc_wgpu_canvas_update_resources_if_needed(context);

        wgpuQueueWriteBuffer(renderer->queue, renderer->pathBuffer, 0, context->pathData, sizeof(oc_wgpu_path) * context->pathCount);
        wgpuQueueWriteBuffer(renderer->queue, renderer->elementBuffer, 0, context->elementData, sizeof(oc_wgpu_path_elt) * context->eltCount);

        context->pathBatchStart += primitiveIndex;

        oc_scratch_end(scratch);

        return (true);
    }
}

void oc_wgpu_canvas_timestamp_read_callback(WGPUBufferMapAsyncStatus status, void* user)
{
    oc_wgpu_canvas_timestamp_read_callback_data* data = (oc_wgpu_canvas_timestamp_read_callback_data*)user;
    oc_wgpu_canvas_renderer* renderer = data->renderer;
    oc_wgpu_canvas_frame_counters* frameCounters = data->frameCounters;

    u64 mapSize = data->mapSize;
    const char* mappedBuffer = (const char*)wgpuBufferGetConstMappedRange(data->buffer,
                                                                          0,
                                                                          mapSize);

    const oc_wgpu_canvas_frame_timestamps* frameTimestamps = (const oc_wgpu_canvas_frame_timestamps*)mappedBuffer;

    if(frameTimestamps->frameEnd > frameTimestamps->frameBegin)
    {
        frameCounters->gpuTime = (frameTimestamps->frameEnd - frameTimestamps->frameBegin) / 1000000.;

        if(frameCounters->gpuTime)
        {
            oc_wgpu_canvas_stats_add_sample(&renderer->gpuTime, frameCounters->gpuTime);
        }
    }

    wgpuBufferUnmap(data->buffer);
}

void oc_wgpu_canvas_submit(oc_canvas_renderer_base* rendererBase,
                           oc_surface surfaceHandle,
                           u32 msaaSampleCount,
                           oc_color clearColor,
                           u32 primitiveCount,
                           oc_primitive* primitives,
                           u32 eltCount,
                           oc_path_elt* elements)
{
    oc_wgpu_canvas_renderer* renderer = (oc_wgpu_canvas_renderer*)rendererBase;

    wgpuDeviceTick(renderer->device);

    WGPUSwapChain swapChain = oc_wgpu_surface_get_swapchain(surfaceHandle, renderer->device);

    if(swapChain && primitiveCount && eltCount)
    {
        WGPUTextureView frameBuffer = wgpuSwapChainGetCurrentTextureView(swapChain);
        OC_ASSERT(frameBuffer);

        renderer->rollingBufferIndex = (renderer->rollingBufferIndex + 1) % OC_WGPU_CANVAS_ROLLING_BUFFER_COUNT;

        if(renderer->hasTimestamps)
        {
            while(wgpuBufferGetMapState(renderer->timestampsReadBuffer[renderer->rollingBufferIndex]) != WGPUBufferMapState_Unmapped)
            {
                wgpuDeviceTick(renderer->device);
            }
        }

        f64 submitStart = oc_clock_time(OC_CLOCK_MONOTONIC);

        oc_vec2 screenSize = oc_surface_get_size(surfaceHandle);
        oc_vec2 scale = oc_surface_contents_scaling(surfaceHandle);
        screenSize.x *= scale.x;
        screenSize.y *= scale.y;

        //TODO: move that to enum
        i32 tileSize = 16;
        i32 chunkSize = 256;
        i32 nTilesX = (i32)(screenSize.x + tileSize - 1) / tileSize;
        i32 nTilesY = (i32)(screenSize.y + tileSize - 1) / tileSize;

        oc_wgpu_canvas_encoding_context encodingContext = {
            .renderer = renderer,
            .inputPrimitiveCount = primitiveCount,
            .inputPrimitives = primitives,
            .inputEltCount = eltCount,
            .inputElements = elements,
            .screenSize = screenSize,
            .scale = scale,
            .tileSize = tileSize,
            .screenTilesCount = nTilesX * nTilesY,
        };

        if(renderer->debugDisplayOptions.pathCount)
        {
            i32 pathStart = oc_clamp(renderer->debugDisplayOptions.pathStart, 0, primitiveCount);
            i32 pathCount = oc_clamp(renderer->debugDisplayOptions.pathCount,
                                     1,
                                     primitiveCount - pathStart);

            encodingContext.inputPrimitiveCount = pathCount;
            encodingContext.inputPrimitives = primitives + pathStart;
        }

        //NOTE: allocate a new frame counters record
        oc_wgpu_canvas_frame_counters* frameCounters = 0;
        if(renderer->debugRecordOptions.maxRecordCount)
        {
            bool removeOld = false;
            oc_wgpu_canvas_frame_counters* oldFrameCounters = oc_list_first_entry(renderer->debugRecords,
                                                                                  oc_wgpu_canvas_frame_counters,
                                                                                  listElt);
            if(oldFrameCounters && (renderer->frameIndex - oldFrameCounters->frameIndex >= renderer->debugRecordOptions.maxRecordCount))
            {
                //NOTE: recycle first (oldest) record
                oc_list_pop(&renderer->debugRecords);

                for(oc_wgpu_canvas_batch_counters* batchCounters = oc_list_pop_entry(&oldFrameCounters->batches, oc_wgpu_canvas_batch_counters, listElt);
                    batchCounters != 0;
                    batchCounters = oc_list_pop_entry(&oldFrameCounters->batches, oc_wgpu_canvas_batch_counters, listElt))
                {
                    oc_list_push(&renderer->batchCountersFreeList, &batchCounters->listElt);
                }

                if(oldFrameCounters->gpuTime)
                {
                    oc_wgpu_canvas_stats_remove_sample(&renderer->gpuTime);
                }
                oc_wgpu_canvas_stats_remove_sample(&renderer->cpuEncodeTime);
                oc_wgpu_canvas_stats_remove_sample(&renderer->cpuFrameTime);

                oc_list_push(&renderer->frameCountersFreeList, &oldFrameCounters->listElt);
            }

            //NOTE: get new record from free list or allocate it fresh from debug arena
            frameCounters = oc_list_pop_entry(&renderer->frameCountersFreeList,
                                              oc_wgpu_canvas_frame_counters,
                                              listElt);
            if(!frameCounters)
            {
                frameCounters = oc_arena_push_type(&renderer->debugArena, oc_wgpu_canvas_frame_counters);
            }

            if(frameCounters)
            {
                memset(frameCounters, 0, sizeof(oc_wgpu_canvas_frame_counters));

                frameCounters->frameIndex = renderer->frameIndex;
                frameCounters->inputPathCount = encodingContext.inputPrimitiveCount;
                frameCounters->inputElementCount = encodingContext.inputEltCount;

                oc_list_push_back(&renderer->debugRecords, &frameCounters->listElt);
                renderer->debugRecordsCount++;
            }
            else
            {
                oc_log_error("Could not allocate frame counters record\n");
            }
        }

        //TODO: move that elsewhere
        wgpuQueueWriteBuffer(renderer->queue, renderer->tileSizeBuffer, 0, &tileSize, sizeof(i32));
        wgpuQueueWriteBuffer(renderer->queue, renderer->chunkSizeBuffer, 0, &chunkSize, sizeof(i32));

        {
            oc_wgpu_debug_display_options options = {
                .showTileBorders = renderer->debugDisplayOptions.showTileBorders ? 1 : 0,
                .showPathArea = renderer->debugDisplayOptions.showPathArea ? 1 : 0,
                .showClip = renderer->debugDisplayOptions.showClip ? 1 : 0,
                .textureOff = renderer->debugDisplayOptions.textureOff ? 1 : 0,
                .debugTileQueues = renderer->debugDisplayOptions.debugTileQueues ? 1 : 0,
            };
            wgpuQueueWriteBuffer(renderer->queue,
                                 renderer->debugDisplayOptionsBuffer,
                                 0,
                                 &options,
                                 sizeof(oc_wgpu_debug_display_options));
        }

        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(renderer->device, NULL);

        if(renderer->hasTimestamps && (renderer->debugRecordOptions.timingFlags & OC_WGPU_CANVAS_TIMING_FRAME))
        {
            wgpuCommandEncoderWriteTimestamp(encoder, renderer->timestampsQuerySet, OC_WGPU_CANVAS_TIMESTAMP_INDEX_FRAME_BEGIN);
        }

        //----------------------------------------------------------------------------------------
        //NOTE: clear framebuffer
        {
            WGPURenderPassDescriptor desc = {
                .colorAttachmentCount = 1,
                .colorAttachments = (WGPURenderPassColorAttachment[]){
                    {
                        .view = frameBuffer,
                        .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
                        .loadOp = WGPULoadOp_Clear,
                        .storeOp = WGPUStoreOp_Store,
                        .clearValue = { clearColor.r, clearColor.g, clearColor.b, clearColor.a },
                    },
                },
            };

            WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &desc);
            {
                wgpuRenderPassEncoderSetViewport(pass, 0.f, 0.f, screenSize.x, screenSize.y, 0.f, 1.f);
            }
            wgpuRenderPassEncoderEnd(pass);
        }

        WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, NULL);
        wgpuQueueSubmit(renderer->queue, 1, &command);
        wgpuCommandBufferRelease(command);
        wgpuCommandEncoderRelease(encoder);

        u32 batchCount = 0;

        while(oc_wgpu_canvas_encode_batch(&encodingContext))
        {
            //NOTE: allocate new batch counters
            oc_wgpu_canvas_batch_counters* batchCounters = 0;
            if(frameCounters)
            {
                batchCounters = oc_list_pop_entry(&renderer->batchCountersFreeList,
                                                  oc_wgpu_canvas_batch_counters,
                                                  listElt);
                if(!batchCounters)
                {
                    batchCounters = oc_arena_push_type(&renderer->debugArena, oc_wgpu_canvas_batch_counters);
                }
                if(!batchCounters)
                {
                    oc_log_error("Could not allocate batch counters record\n");
                }

                if(batchCounters)
                {
                    batchCounters->encodedPathCount = encodingContext.pathCount;
                    batchCounters->encodedElementCount = encodingContext.eltCount;
                    oc_list_push(&frameCounters->batches, &batchCounters->listElt);
                }
                else
                {
                    oc_log_error("Could not allocate batch counters record\n");
                }
            }

            encoder = wgpuDeviceCreateCommandEncoder(renderer->device, NULL);

            //----------------------------------------------------------------------------------------
            //NOTE: reset counters
            {
                u32 zero = 0;
                wgpuQueueWriteBuffer(renderer->queue, renderer->segmentCountBuffer, 0, &zero, sizeof(u32));
                wgpuQueueWriteBuffer(renderer->queue, renderer->binQueueCountBuffer, 0, &zero, sizeof(u32));
                wgpuQueueWriteBuffer(renderer->queue, renderer->tileOpCountBuffer, 0, &zero, sizeof(u32));
                wgpuQueueWriteBuffer(renderer->queue, renderer->chunkEltCountBuffer, 0, &zero, sizeof(u32));

                u32 dispatchInit[3] = { 0, 1, 1 };
                wgpuQueueWriteBuffer(renderer->queue, renderer->tileQueueCountBuffer, 0, &dispatchInit, 3 * sizeof(u32));
            }

            //----------------------------------------------------------------------------------------
            //NOTE: clear texture
            {
                WGPURenderPassDescriptor desc = {
                    .colorAttachmentCount = 1,
                    .colorAttachments = (WGPURenderPassColorAttachment[]){
                        {
                            .view = renderer->outTextureView,
                            .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
                            .loadOp = WGPULoadOp_Clear,
                            .storeOp = WGPUStoreOp_Store,
                            .clearValue = { 0, 0, 0, 0 },
                        },
                    },
                };

                WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &desc);
                {
                    wgpuRenderPassEncoderSetViewport(pass, 0.f, 0.f, screenSize.x, screenSize.y, 0.f, 1.f);
                }
                wgpuRenderPassEncoderEnd(pass);
            }

            /*NOTE(martin): On dispatch limits.

                There are several limits to how many invocations and workgroups can be dispatched in one call.
                (see https://www.w3.org/TR/webgpu/#limits)

                - The requires minimum for maxComputeWorkgroupSizeX and maxComputeWorkgroupSizeY is 256.
                - The required minimum for maxComputeInvocationsPerWorkgroup is 256
                - The required minimum for maxComputeWorkgroupsPerDimension is 65535 = 2^16

                So, we use 16*16 workgroup sizes. Limits impact each pass as follows:

                * Path setup: we use one invocation per path, in 16*16 workgroup sizes, and distribute
                  workgroups on both axes. This gives a minimum limit of 2^40 paths per dispatch.

                * Segment setup: we use one invocation per element, in 16*16 workgroup sizes, and distribute
                  workgroups on both axes. This gives a minimum limit of 2^40 elements per dispatch.

                * Backprop pass: we use one workgroup of 16*1 invocations per path, and distribute workgroups on both axes.
                  This gives a minimum limit of 2^32 paths per dispatch.

                * Merge pass: we use one workgroup of 1*1 invocations per screen tile, which gives a minimum limit of 2^16
                  tiles (= 2^20 pixels) on each axis.

                * Raster pass: we use one invocation per pixel, in 16*16 workgroups (ie, one workgroup per tile). The total
                  number of workgroups is distributed on each axis in the balance_workgroups pass, which sets the indirect
                  dispatch buffer for the raster pass. This gives, a max of 2^32 tiles total.

                  The first three limits are higher than the number of paths/elements we can index with an u32, while the
                  last two are sufficient to completely cover a 10k screen, so we don't bother doing multiple dispatch passes.
            */
            //----------------------------------------------------------------------------------------
            //NOTE: path setup pass
            {
                u32 invocationsPerWorkGroup = 16 * 16;
                wgpuQueueWriteBuffer(renderer->queue, renderer->pathCountBuffer, 0, &encodingContext.pathCount, sizeof(u32));

                WGPUComputePassDescriptor desc = {
                    .label = "path setup",
                };

                WGPUComputePassEncoder pass = wgpuCommandEncoderBeginComputePass(encoder, &desc);
                {
                    wgpuComputePassEncoderSetPipeline(pass, renderer->pathSetupPipeline);
                    wgpuComputePassEncoderSetBindGroup(pass, 0, renderer->pathSetupBindGroup, 0, NULL);

                    u32 totalWorkGroupCount = (encodingContext.pathCount + invocationsPerWorkGroup - 1) / invocationsPerWorkGroup;
                    u32 workGroupCountX = oc_min(totalWorkGroupCount, renderer->limits.maxComputeWorkgroupsPerDimension);
                    u32 workGroupCountY = (totalWorkGroupCount + renderer->limits.maxComputeWorkgroupsPerDimension - 1)
                                        / renderer->limits.maxComputeWorkgroupsPerDimension;

                    wgpuComputePassEncoderDispatchWorkgroups(pass, workGroupCountX, workGroupCountY, 1);
                }
                wgpuComputePassEncoderEnd(pass);
            }

            //----------------------------------------------------------------------------------------
            //NOTE: segment setup pass
            {
                u32 invocationsPerWorkGroup = 16 * 16;

                wgpuQueueWriteBuffer(renderer->queue, renderer->elementCountBuffer, 0, &encodingContext.eltCount, sizeof(u32));

                WGPUComputePassDescriptor desc = {
                    .label = "segment setup",
                };

                WGPUComputePassEncoder pass = wgpuCommandEncoderBeginComputePass(encoder, &desc);
                {
                    wgpuComputePassEncoderSetPipeline(pass, renderer->segmentSetupPipeline);
                    wgpuComputePassEncoderSetBindGroup(pass, 0, renderer->segmentSetupBindGroup, 0, NULL);

                    u32 workGroupCountX = (encodingContext.eltCount + invocationsPerWorkGroup - 1)
                                        / invocationsPerWorkGroup;

                    wgpuComputePassEncoderDispatchWorkgroups(pass, workGroupCountX, 1, 1);
                }
                wgpuComputePassEncoderEnd(pass);
            }

            //----------------------------------------------------------------------------------------
            //NOTE: backprop pass
            {
                WGPUComputePassDescriptor desc = {
                    .label = "backprop",
                };

                WGPUComputePassEncoder pass = wgpuCommandEncoderBeginComputePass(encoder, &desc);
                {
                    wgpuComputePassEncoderSetPipeline(pass, renderer->backpropPipeline);
                    wgpuComputePassEncoderSetBindGroup(pass, 0, renderer->backpropBindGroup, 0, NULL);

                    u32 workGroupCountX = oc_min(encodingContext.pathCount, renderer->limits.maxComputeWorkgroupsPerDimension);
                    u32 workGroupCountY = (encodingContext.pathCount + renderer->limits.maxComputeWorkgroupsPerDimension - 1)
                                        / renderer->limits.maxComputeWorkgroupsPerDimension;

                    wgpuComputePassEncoderDispatchWorkgroups(pass, workGroupCountX, workGroupCountY, 1);
                }
                wgpuComputePassEncoderEnd(pass);
            }

            //----------------------------------------------------------------------------------------
            //NOTE: chunk pass
            {
                WGPUComputePassDescriptor desc = {
                    .label = "chunk",
                };

                WGPUComputePassEncoder pass = wgpuCommandEncoderBeginComputePass(encoder, &desc);
                {
                    wgpuComputePassEncoderSetPipeline(pass, renderer->chunkPipeline);
                    wgpuComputePassEncoderSetBindGroup(pass, 0, renderer->chunkBindGroup, 0, NULL);

                    u32 workGroupCountX = (screenSize.x + OC_WGPU_CANVAS_CHUNK_SIZE - 1) / OC_WGPU_CANVAS_CHUNK_SIZE;
                    u32 workGroupCountY = (screenSize.y + OC_WGPU_CANVAS_CHUNK_SIZE - 1) / OC_WGPU_CANVAS_CHUNK_SIZE;

                    wgpuComputePassEncoderDispatchWorkgroups(pass, workGroupCountX, workGroupCountY, 1);
                }
                wgpuComputePassEncoderEnd(pass);
            }

            //----------------------------------------------------------------------------------------
            //NOTE: tile merge pass
            {
                WGPUComputePassDescriptor desc = {
                    .label = "tile merge",
                };

                WGPUComputePassEncoder pass = wgpuCommandEncoderBeginComputePass(encoder, &desc);
                {
                    wgpuComputePassEncoderSetPipeline(pass, renderer->mergePipeline);
                    wgpuComputePassEncoderSetBindGroup(pass, 0, renderer->mergeBindGroup, 0, NULL);

                    u32 workGroupCountX = nTilesX;
                    u32 workGroupCountY = nTilesY;
                    wgpuComputePassEncoderDispatchWorkgroups(pass, workGroupCountX, workGroupCountY, 1);
                }
                wgpuComputePassEncoderEnd(pass);
            }

            //----------------------------------------------------------------------------------------
            //NOTE: balance workgroups for raster dispatch indirect command
            {
                WGPUComputePassDescriptor desc = {
                    .label = "balance workgroups",
                };

                WGPUComputePassEncoder pass = wgpuCommandEncoderBeginComputePass(encoder, &desc);
                {
                    wgpuComputePassEncoderSetPipeline(pass, renderer->balancePipeline);
                    wgpuComputePassEncoderSetBindGroup(pass, 0, renderer->balanceBindGroup, 0, NULL);
                    wgpuComputePassEncoderDispatchWorkgroups(pass, 1, 1, 1);
                }
                wgpuComputePassEncoderEnd(pass);
            }

            //----------------------------------------------------------------------------------------
            //NOTE: raster pass
            {
                msaaSampleCount = oc_clamp(msaaSampleCount, 0, OC_WGPU_CANVAS_MAX_SAMPLE_COUNT);
                if(msaaSampleCount == 0)
                {
                    msaaSampleCount = OC_WGPU_CANVAS_DEFAULT_SAMPLE_COUNT;
                }

                if(renderer->msaaSampleCount != msaaSampleCount)
                {
                    renderer->msaaSampleCount = msaaSampleCount;

                    u32 sampleOffsetsIndex = OC_WGPU_CANVAS_OFFSETS_LOOKUP[msaaSampleCount - 1];
                    oc_vec2* offsets = OC_WGPU_CANVAS_OFFSETS[sampleOffsetsIndex];

                    wgpuQueueWriteBuffer(renderer->queue, renderer->msaaOffsetsBuffer, 0, offsets, OC_WGPU_CANVAS_MAX_SAMPLE_COUNT * sizeof(oc_vec2));
                    wgpuQueueWriteBuffer(renderer->queue, renderer->msaaSampleCountBuffer, 0, &renderer->msaaSampleCount, sizeof(u32));
                }

                //TODO: remove?
                wgpuQueueWriteBuffer(renderer->queue, renderer->pathCountBuffer, 0, &encodingContext.pathCount, sizeof(u32));

                WGPUComputePassDescriptor desc = {
                    .label = "raster",
                };

                WGPUComputePassEncoder pass = wgpuCommandEncoderBeginComputePass(encoder, &desc);
                {
                    wgpuComputePassEncoderSetPipeline(pass, renderer->rasterPipeline);
                    wgpuComputePassEncoderSetBindGroup(pass, 0, renderer->rasterBindGroup, 0, NULL);
                    wgpuComputePassEncoderSetBindGroup(pass, 1, renderer->srcTexturesBindGroup, 0, NULL);

                    wgpuComputePassEncoderDispatchWorkgroupsIndirect(pass, renderer->tileQueueCountBuffer, 0);
                }
                wgpuComputePassEncoderEnd(pass);
            }

            //----------------------------------------------------------------------------------------
            //NOTE: blit pass
            {
                WGPURenderPassDescriptor desc = {
                    .label = "blit",
                    .colorAttachmentCount = 1,
                    .colorAttachments = (WGPURenderPassColorAttachment[]){
                        {
                            .view = frameBuffer,
                            .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
                            .loadOp = WGPULoadOp_Load,
                            .storeOp = WGPUStoreOp_Store,
                        },
                    },
                };

                WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &desc);
                {
                    wgpuRenderPassEncoderSetViewport(pass, 0.f, 0.f, screenSize.x, screenSize.y, 0.f, 1.f);
                    wgpuRenderPassEncoderSetPipeline(pass, renderer->blitPipeline);
                    wgpuRenderPassEncoderSetBindGroup(pass, 0, renderer->blitBindGroup, 0, NULL);

                    //    wgpuRenderPassEncoderSetVertexBuffer(pass, 0, renderer->vBuffer, 0, WGPU_WHOLE_SIZE);
                    wgpuRenderPassEncoderDraw(pass, 4, 1, 0, 0);
                }
                wgpuRenderPassEncoderEnd(pass);
            }

            // submit to queue
            WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, NULL);
            wgpuQueueSubmit(renderer->queue, 1, &command);
            wgpuCommandBufferRelease(command);
            wgpuCommandEncoderRelease(encoder);

            batchCount++;
        }

        //NOTE: resolve frame timestamps
        if(renderer->hasTimestamps && (renderer->debugRecordOptions.timingFlags & OC_WGPU_CANVAS_TIMING_FRAME))
        {
            WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(renderer->device, NULL);

            wgpuCommandEncoderWriteTimestamp(encoder, renderer->timestampsQuerySet, OC_WGPU_CANVAS_TIMESTAMP_INDEX_FRAME_END);

            wgpuCommandEncoderResolveQuerySet(encoder,
                                              renderer->timestampsQuerySet,
                                              OC_WGPU_CANVAS_TIMESTAMP_INDEX_FRAME_BEGIN,
                                              2,
                                              renderer->timestampsResolveBuffer,
                                              0);

            wgpuCommandEncoderCopyBufferToBuffer(encoder,
                                                 renderer->timestampsResolveBuffer,
                                                 0,
                                                 renderer->timestampsReadBuffer[renderer->rollingBufferIndex],
                                                 0,
                                                 2 * sizeof(u64));

            WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, NULL);
            wgpuQueueSubmit(renderer->queue, 1, &command);
            wgpuCommandBufferRelease(command);
            wgpuCommandEncoderRelease(encoder);
        }

        //NOTE: release transient stuff
        wgpuTextureViewRelease(frameBuffer);

        f64 submitEnd = oc_clock_time(OC_CLOCK_MONOTONIC);

        //NOTE: read back frame timestamps
        if(frameCounters)
        {
            frameCounters->batchCount = batchCount;

            if(renderer->hasTimestamps && (renderer->debugRecordOptions.timingFlags & OC_WGPU_CANVAS_TIMING_ALL))
            {
                oc_wgpu_canvas_timestamp_read_callback_data* data = &renderer->timestampsReadCallbackData[renderer->rollingBufferIndex];
                *data = (oc_wgpu_canvas_timestamp_read_callback_data){
                    .renderer = renderer,
                    .frameCounters = frameCounters,
                    .mapSize = sizeof(u64) * OC_WGPU_CANVAS_TIMESTAMPS_COUNT,
                    .buffer = renderer->timestampsReadBuffer[renderer->rollingBufferIndex],
                };
                wgpuBufferMapAsync(renderer->timestampsReadBuffer[renderer->rollingBufferIndex],
                                   WGPUMapMode_Read,
                                   0,
                                   data->mapSize,
                                   oc_wgpu_canvas_timestamp_read_callback,
                                   data);
            }

            frameCounters->cpuEncodeTime = (submitEnd - submitStart) * 1000.;
            frameCounters->cpuFrameTime = (submitStart - renderer->lastFrameTimeStamp) * 1000.;

            if(renderer->frameIndex)
            {
                oc_wgpu_canvas_stats_add_sample(&renderer->cpuFrameTime, frameCounters->cpuFrameTime);
            }
            oc_wgpu_canvas_stats_add_sample(&renderer->cpuEncodeTime, frameCounters->cpuEncodeTime);
        }
        renderer->lastFrameTimeStamp = submitStart;
    }
    renderer->frameIndex++;
}

void oc_wgpu_canvas_present(oc_canvas_renderer_base* rendererBase, oc_surface surfaceHandle)
{
    oc_wgpu_canvas_renderer* renderer = (oc_wgpu_canvas_renderer*)rendererBase;
    WGPUSwapChain swapChain = oc_wgpu_surface_get_swapchain(surfaceHandle, renderer->device);

    if(swapChain)
    {
        wgpuSwapChainPresent(swapChain);
    }
}

void oc_wgpu_canvas_destroy(oc_canvas_renderer_base* base)
{
    oc_wgpu_canvas_renderer* renderer = (oc_wgpu_canvas_renderer*)base;

// release bind groups
#define release_bindgroup_if_needed(x) \
    if(x)                              \
    {                                  \
        wgpuBindGroupRelease(x);       \
    }

    release_bindgroup_if_needed(renderer->pathSetupBindGroup);
    release_bindgroup_if_needed(renderer->segmentSetupBindGroup);
    release_bindgroup_if_needed(renderer->backpropBindGroup);
    release_bindgroup_if_needed(renderer->chunkBindGroup);
    release_bindgroup_if_needed(renderer->mergeBindGroup);
    release_bindgroup_if_needed(renderer->balanceBindGroup);
    release_bindgroup_if_needed(renderer->rasterBindGroup);
    release_bindgroup_if_needed(renderer->srcTexturesBindGroup);
    release_bindgroup_if_needed(renderer->blitBindGroup);

#undef release_bindgroup_if_needed

    // release bind group layouts
    wgpuBindGroupLayoutRelease(renderer->pathSetupBindGroupLayout);
    wgpuBindGroupLayoutRelease(renderer->segmentSetupBindGroupLayout);
    wgpuBindGroupLayoutRelease(renderer->backpropBindGroupLayout);
    wgpuBindGroupLayoutRelease(renderer->chunkBindGroupLayout);
    wgpuBindGroupLayoutRelease(renderer->mergeBindGroupLayout);
    wgpuBindGroupLayoutRelease(renderer->balanceBindGroupLayout);
    wgpuBindGroupLayoutRelease(renderer->rasterBindGroupLayout);
    wgpuBindGroupLayoutRelease(renderer->srcTexturesBindGroupLayout);
    wgpuBindGroupLayoutRelease(renderer->blitBindGroupLayout);

    // release pipelines
    wgpuComputePipelineRelease(renderer->pathSetupPipeline);
    wgpuComputePipelineRelease(renderer->segmentSetupPipeline);
    wgpuComputePipelineRelease(renderer->backpropPipeline);
    wgpuComputePipelineRelease(renderer->chunkPipeline);
    wgpuComputePipelineRelease(renderer->mergePipeline);
    wgpuComputePipelineRelease(renderer->balancePipeline);
    wgpuComputePipelineRelease(renderer->rasterPipeline);
    wgpuRenderPipelineRelease(renderer->blitPipeline);

#define release_buffer_if_needed(x) \
    if(x)                           \
    {                               \
        wgpuBufferRelease(x);       \
    }

    // release buffers
    release_buffer_if_needed(renderer->pathBuffer);
    release_buffer_if_needed(renderer->pathCountBuffer);

    release_buffer_if_needed(renderer->elementBuffer);
    release_buffer_if_needed(renderer->elementCountBuffer);

    release_buffer_if_needed(renderer->segmentCountBuffer);
    release_buffer_if_needed(renderer->segmentBuffer);
    release_buffer_if_needed(renderer->pathBinBuffer);
    release_buffer_if_needed(renderer->binQueueBuffer);
    release_buffer_if_needed(renderer->binQueueCountBuffer);

    release_buffer_if_needed(renderer->chunkBuffer);
    release_buffer_if_needed(renderer->chunkEltBuffer);
    release_buffer_if_needed(renderer->chunkEltCountBuffer);

    release_buffer_if_needed(renderer->tileQueueBuffer);
    release_buffer_if_needed(renderer->tileQueueCountBuffer);
    release_buffer_if_needed(renderer->tileOpBuffer);
    release_buffer_if_needed(renderer->tileOpCountBuffer);

    release_buffer_if_needed(renderer->tileSizeBuffer);
    release_buffer_if_needed(renderer->chunkSizeBuffer);

    release_buffer_if_needed(renderer->msaaOffsetsBuffer);
    release_buffer_if_needed(renderer->msaaSampleCountBuffer);

    release_buffer_if_needed(renderer->maxWorkGroupsPerDimensionBuffer);

    release_buffer_if_needed(renderer->timestampsResolveBuffer);
    for(int i = 0; i < OC_WGPU_CANVAS_ROLLING_BUFFER_COUNT; i++)
    {
        release_buffer_if_needed(renderer->timestampsReadBuffer[0]);
    }

    release_buffer_if_needed(renderer->debugDisplayOptionsBuffer);

#undef release_buffer_if_needed

    // release query set
    wgpuQuerySetRelease(renderer->timestampsQuerySet);

    // release texture views and sample
    if(renderer->outTextureView)
    {
        wgpuTextureViewRelease(renderer->outTextureView);
    }
    if(renderer->dummyTextureView)
    {
        wgpuTextureViewRelease(renderer->dummyTextureView);
    }

    // release queue/device/instance
    wgpuQueueRelease(renderer->queue);
    wgpuDeviceRelease(renderer->device);
    wgpuInstanceRelease(renderer->instance);

    free(renderer);
}

oc_image_base* oc_wgpu_canvas_image_create(oc_canvas_renderer_base* base, oc_vec2 size)
{
    oc_wgpu_canvas_renderer* renderer = (oc_wgpu_canvas_renderer*)base;

    oc_wgpu_image* image = oc_malloc_type(oc_wgpu_image);
    if(image)
    {
        WGPUTextureDescriptor desc = {
            .usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst,
            .dimension = WGPUTextureDimension_2D,
            .size = { size.x, size.y, 1 },
            .format = WGPUTextureFormat_RGBA8Unorm,
            .mipLevelCount = 1,
            .sampleCount = 1,
        };
        image->texture = wgpuDeviceCreateTexture(renderer->device, &desc);

        WGPUTextureViewDescriptor viewDesc = {
            .format = desc.format,
            .dimension = WGPUTextureViewDimension_2D,
            .baseMipLevel = 0,
            .mipLevelCount = 1,
            .baseArrayLayer = 0,
            .arrayLayerCount = 1,
            .aspect = WGPUTextureAspect_All,
        };
        image->textureView = wgpuTextureCreateView(image->texture, &viewDesc);

        //TODO: check fail

        image->base.size = size;
    }
    return ((oc_image_base*)image);
}

void oc_wgpu_canvas_image_destroy(oc_canvas_renderer_base* rendererBase, oc_image_base* imageBase)
{
    oc_wgpu_canvas_renderer* renderer = (oc_wgpu_canvas_renderer*)rendererBase;
    oc_wgpu_image* image = (oc_wgpu_image*)imageBase;

    //TODO: check image was created with the same renderer

    wgpuTextureViewRelease(image->textureView);
    wgpuTextureRelease(image->texture);
    free(image);
}

void oc_wgpu_canvas_image_upload_region(oc_canvas_renderer_base* rendererBase, oc_image_base* imageBase, oc_rect region, u8* pixels)
{
    oc_wgpu_canvas_renderer* renderer = (oc_wgpu_canvas_renderer*)rendererBase;
    oc_wgpu_image* image = (oc_wgpu_image*)imageBase;

    //TODO: check image was created with the same renderer

    WGPUImageCopyTexture dst = {
        .texture = image->texture,
        .mipLevel = 0,
        .origin = { (u32)region.x, (u32)region.y },
    };
    WGPUTextureDataLayout src = {
        .offset = 0,
        .bytesPerRow = region.w * 4, // 4 bytes per pixel
        .rowsPerImage = region.h,
    };
    u32 pixelsSize = region.w * region.h * 4;
    wgpuQueueWriteTexture(renderer->queue, &dst, pixels, pixelsSize, &src, &(WGPUExtent3D){ region.w, region.h, 1 });
}

void oc_wgpu_canvas_debug_set_record_options(oc_canvas_renderer handle, oc_wgpu_canvas_record_options* options)
{
    oc_canvas_renderer_base* base = oc_canvas_renderer_from_handle(handle);
    //TODO: should check that the impl is webgpu
    if(base)
    {
        oc_wgpu_canvas_renderer* renderer = (oc_wgpu_canvas_renderer*)base;
        renderer->debugRecordOptions = *options;

        if(options->maxRecordCount && !renderer->debugArena.base)
        {
            //NOTE: init the debug arena the first time we set record max count > 0
            oc_arena_init(&renderer->debugArena);
        }
        if(!renderer->hasTimestamps && (options->timingFlags & OC_WGPU_CANVAS_TIMING_FRAME))
        {
            oc_log_error("WebGPU adapter does not support timestamp queries, GPU time won't be recorded\n");
        }
    }
}

oc_wgpu_canvas_record_options oc_wgpu_canvas_debug_get_record_options(oc_canvas_renderer handle)
{
    oc_wgpu_canvas_record_options options = { 0 };
    oc_canvas_renderer_base* base = oc_canvas_renderer_from_handle(handle);
    //TODO: should check that the impl is webgpu
    if(base)
    {
        oc_wgpu_canvas_renderer* renderer = (oc_wgpu_canvas_renderer*)base;
        options = renderer->debugRecordOptions;
    }
    return (options);
}

void oc_wgpu_canvas_debug_clear_records(oc_canvas_renderer handle)
{
    oc_canvas_renderer_base* base = oc_canvas_renderer_from_handle(handle);
    //TODO: should check that the impl is webgpu
    if(base)
    {
        oc_wgpu_canvas_renderer* renderer = (oc_wgpu_canvas_renderer*)base;
        oc_arena_clear(&renderer->debugArena);
        oc_list_init(&renderer->debugRecords);
        oc_list_init(&renderer->frameCountersFreeList);
        oc_list_init(&renderer->batchCountersFreeList);
        renderer->debugRecordsCount = 0;
    }
}

oc_list oc_wgpu_canvas_debug_get_records(oc_canvas_renderer handle)
{
    oc_list result = { 0 };
    oc_canvas_renderer_base* base = oc_canvas_renderer_from_handle(handle);
    //TODO: should check that the impl is webgpu
    if(base)
    {
        oc_wgpu_canvas_renderer* renderer = (oc_wgpu_canvas_renderer*)base;
        result = renderer->debugRecords;
    }
    return (result);
}

void oc_wgpu_canvas_debug_log_records(oc_list debugRecords)
{
    oc_arena_scope scratch = oc_scratch_begin();

    oc_list_for(debugRecords, record, oc_wgpu_canvas_frame_counters, listElt)
    {
        oc_str8_list debugStringList = { 0 };

        oc_str8_list_pushf(scratch.arena,
                           &debugStringList,
                           "Record:\n"
                           "  pathCount        = %u\n"
                           "  elementCount     = %u\n"
                           "  frame            = %.2f\n",
                           record->inputPathCount,
                           record->inputElementCount,
                           record->gpuTime);

        u32 batchIndex = 0;
        oc_list_for(record->batches, batch, oc_wgpu_canvas_batch_counters, listElt)
        {
            oc_str8_list_pushf(scratch.arena,
                               &debugStringList,
                               "  Batch %i:\n"
                               "    encodedPaths    = %u\n"
                               "    encodedElements = %u\n",
                               batchIndex,
                               batch->encodedPathCount,
                               batch->encodedElementCount);
            batchIndex++;
        }
        oc_str8_list_pushf(scratch.arena, &debugStringList, "\n");
        oc_str8 debugString = oc_str8_list_join(scratch.arena, debugStringList);
        oc_log_info(debugString.ptr);
    }

    oc_scratch_end(scratch);
}

oc_wgpu_canvas_frame_stats oc_wgpu_canvas_get_frame_stats(oc_canvas_renderer handle, int desiredSampleCount)
{
    oc_wgpu_canvas_frame_stats stats = { 0 };
    oc_canvas_renderer_base* base = oc_canvas_renderer_from_handle(handle);
    //TODO: should check that the impl is webgpu
    if(base)
    {
        oc_wgpu_canvas_renderer* renderer = (oc_wgpu_canvas_renderer*)base;
        stats = (oc_wgpu_canvas_frame_stats){
            .gpuTime = oc_wgpu_canvas_stats_resolve(&renderer->gpuTime, desiredSampleCount),
            .cpuEncodeTime = oc_wgpu_canvas_stats_resolve(&renderer->cpuEncodeTime, desiredSampleCount),
            .cpuFrameTime = oc_wgpu_canvas_stats_resolve(&renderer->cpuFrameTime, desiredSampleCount),
        };
    }
    return (stats);
}

void oc_wgpu_canvas_debug_set_display_options(oc_canvas_renderer handle, oc_wgpu_canvas_debug_display_options* options)
{
    oc_canvas_renderer_base* base = oc_canvas_renderer_from_handle(handle);
    //TODO: should check that the impl is webgpu
    if(base)
    {
        oc_wgpu_canvas_renderer* renderer = (oc_wgpu_canvas_renderer*)base;
        renderer->debugDisplayOptions = *options;
        renderer->debugDisplayOptions.pathStart = oc_clamp_low(renderer->debugDisplayOptions.pathStart, 0);
        renderer->debugDisplayOptions.pathCount = oc_clamp_low(renderer->debugDisplayOptions.pathCount, 0);

        oc_log_info("options.pathStart = %i, options.pathCount = %i\n",
                    renderer->debugDisplayOptions.pathStart,
                    renderer->debugDisplayOptions.pathCount);
    }
}

oc_wgpu_canvas_debug_display_options oc_wgpu_canvas_debug_get_display_options(oc_canvas_renderer handle)
{
    oc_wgpu_canvas_debug_display_options options = { 0 };
    oc_canvas_renderer_base* base = oc_canvas_renderer_from_handle(handle);
    //TODO: should check that the impl is webgpu
    if(base)
    {
        oc_wgpu_canvas_renderer* renderer = (oc_wgpu_canvas_renderer*)base;
        options = renderer->debugDisplayOptions;
    }
    return (options);
}
