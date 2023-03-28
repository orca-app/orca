/************************************************************//**
*
*	@file: mtl_canvas.m
*	@author: Martin Fouilleul
*	@date: 12/07/2020
*	@revision: 24/01/2023
*
*****************************************************************/
#import<Metal/Metal.h>
#import<QuartzCore/CAMetalLayer.h>
#include<simd/simd.h>

#include"graphics_internal.h"
#include"macro_helpers.h"
#include"osx_app.h"

#include"mtl_renderer.h"

#define LOG_SUBSYSTEM "Graphics"

typedef struct mg_mtl_canvas_backend
{
	mg_canvas_backend interface;
	mg_surface surface;

	id<MTLComputePipelineState> rasterPipeline;
	id<MTLRenderPipelineState> blitPipeline;

	id<MTLTexture> outTexture;

	id<MTLBuffer> pathBuffer;
	id<MTLBuffer> segmentBuffer;

} mg_mtl_canvas_backend;


static void mg_update_path_extents(vec4* extents, vec2 p)
{
	extents->x = minimum(extents->x, p.x);
	extents->y = minimum(extents->y, p.y);
	extents->z = maximum(extents->z, p.x);
	extents->w = maximum(extents->w, p.y);
}

void mg_mtl_canvas_render(mg_canvas_backend* interface,
                            u32 primitiveCount,
                            mg_primitive* primitives,
                            u32 eltCount,
                            mg_path_elt* pathElements)
{
	mg_mtl_canvas_backend* backend = (mg_mtl_canvas_backend*)interface;

	//TODO: update rolling buffers
	mg_mtl_segment* segmentBufferData = (mg_mtl_segment*)[backend->segmentBuffer contents];
	mg_mtl_path* pathBufferData = (mg_mtl_path*)[backend->pathBuffer contents];

	//NOTE: fill renderer input buffers
	int segCount = 0;
	int pathCount = 0;
	vec2 currentPos = {0};

	for(int primitiveIndex = 0; primitiveIndex < primitiveCount; primitiveIndex++)
	{
		mg_primitive* primitive = &primitives[primitiveIndex];
		if(primitive->cmd == MG_CMD_FILL && primitive->path.count)
		{
			vec4 pathExtents = (vec4){FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};

			for(int eltIndex = 0;
			    (eltIndex < primitive->path.count) && (primitive->path.startIndex + eltIndex < eltCount);
			    eltIndex++)
			{
				mg_path_elt* elt = &pathElements[primitive->path.startIndex + eltIndex];
				if(elt->type == MG_PATH_MOVE)
				{
					currentPos = elt->p[0];
				}
				else if(elt->type == MG_PATH_LINE)
				{
					//NOTE: transform and push path elt + update primitive bounding box
					vec2 p0 = mg_mat2x3_mul(primitive->attributes.transform, currentPos);
					vec2 p3 = mg_mat2x3_mul(primitive->attributes.transform, elt->p[0]);
					currentPos = elt->p[0];

					if(p0.y != p3.y)
					{
						mg_mtl_segment* seg = &segmentBufferData[segCount];
						segCount++;

						seg->pathIndex = primitiveIndex;
						seg->box = (vector_float4){minimum(p0.x, p3.x),
					                           	   minimum(p0.y, p3.y),
					                           	   maximum(p0.x, p3.x),
					                           	   maximum(p0.y, p3.y)};

						if( (p3.x > p0.x && p3.y < p0.y)
					  	||(p3.x <= p0.x && p3.y > p0.y))
						{
							seg->config = MG_MTL_TR;
						}
						else if( (p3.x > p0.x && p3.y > p0.y)
					       	||(p3.x <= p0.x && p3.y < p0.y))
						{
							seg->config = MG_MTL_BR;
						}

						seg->windingIncrement = (p3.y > p0.y)? 1 : -1;
					}
				}
			}

			//NOTE: push path
			mg_mtl_path* path = &pathBufferData[pathCount];
			pathCount++;

			path->cmd =	(mg_mtl_cmd)primitive->cmd;
			path->box = (vector_float4){maximum(primitive->attributes.clip.x, pathExtents.x),
		                                maximum(primitive->attributes.clip.y, pathExtents.y),
		                                minimum(primitive->attributes.clip.x + primitive->attributes.clip.w, pathExtents.z),
		                                minimum(primitive->attributes.clip.y + primitive->attributes.clip.h, pathExtents.w)};

			path->color = (vector_float4){primitive->attributes.color.r,
			                              primitive->attributes.color.g,
			                              primitive->attributes.color.b,
			                              primitive->attributes.color.a};

			//TODO: compute uv transform
		}
	}

	mg_mtl_surface* surface = (mg_mtl_surface*)mg_surface_data_from_handle(backend->surface);
	ASSERT(surface && surface->interface.backend == MG_BACKEND_METAL);

	mp_rect frame = mg_surface_get_frame(backend->surface);
	f32 scale = surface->mtlLayer.contentsScale;
	vec2 viewportSize = {frame.w * scale, frame.h * scale};

	//NOTE: encode GPU commands
	@autoreleasepool
	{
		mg_mtl_surface_acquire_command_buffer(surface);

		//NOTE: raster pass
		id<MTLComputeCommandEncoder> rasterEncoder = [surface->commandBuffer computeCommandEncoder];
		rasterEncoder.label = @"raster pass";
		[rasterEncoder setComputePipelineState: backend->rasterPipeline];

		[rasterEncoder setBytes:&pathCount length:sizeof(int) atIndex:0];
		[rasterEncoder setBuffer:backend->pathBuffer offset:0 atIndex:1];
		[rasterEncoder setBytes:&segCount length:sizeof(int) atIndex:2];
		[rasterEncoder setBuffer:backend->segmentBuffer offset:0 atIndex:3];

		[rasterEncoder setTexture:backend->outTexture atIndex:0];

		MTLSize rasterGridSize = MTLSizeMake(viewportSize.x, viewportSize.y, 1);
		MTLSize rasterGroupSize = MTLSizeMake(16, 16, 1);
		[rasterEncoder dispatchThreads: rasterGridSize threadsPerThreadgroup: rasterGroupSize];

		[rasterEncoder endEncoding];

		//NOTE: blit pass
		mg_mtl_surface_acquire_drawable(surface);
		if(surface->drawable != nil)
		{
			MTLViewport viewport = {0, 0, viewportSize.x, viewportSize.y, 0, 1};

			//TODO: clear here?
			MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
			renderPassDescriptor.colorAttachments[0].texture = surface->drawable.texture;
			renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
			renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

			id<MTLRenderCommandEncoder> renderEncoder = [surface->commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
			renderEncoder.label = @"blit pass";
			[renderEncoder setViewport: viewport];
			[renderEncoder setRenderPipelineState: backend->blitPipeline];
			[renderEncoder setFragmentTexture: backend->outTexture atIndex: 0];
			[renderEncoder drawPrimitives: MTLPrimitiveTypeTriangle
			 	vertexStart: 0
			 	vertexCount: 3 ];
			[renderEncoder endEncoding];
		}
	}
}

void mg_mtl_canvas_destroy(mg_canvas_backend* interface)
{
	mg_mtl_canvas_backend* backend = (mg_mtl_canvas_backend*)interface;

	@autoreleasepool
	{
		[backend->pathBuffer release];
		[backend->segmentBuffer release];
	}

	free(backend);
}

const u32 MG_MTL_PATH_BUFFER_SIZE    = (4<<20)*sizeof(mg_mtl_path),
          MG_MTL_SEGMENT_BUFFER_SIZE = (4<<20)*sizeof(mg_mtl_segment);

mg_canvas_backend* mg_mtl_canvas_create(mg_surface surface)
{
	mg_mtl_canvas_backend* backend = 0;

	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->backend == MG_BACKEND_METAL)
	{
		mg_mtl_surface* metalSurface = (mg_mtl_surface*)surfaceData;

		backend = malloc_type(mg_mtl_canvas_backend);
		memset(backend, 0, sizeof(mg_mtl_canvas_backend));

		backend->surface = surface;

		//NOTE(martin): setup interface functions
		backend->interface.destroy = mg_mtl_canvas_destroy;
		backend->interface.render = mg_mtl_canvas_render;

		@autoreleasepool{
			//NOTE: load metal library
			str8 shaderPath = mp_app_get_resource_path(mem_scratch(), "../resources/mtl_renderer.metallib");
			NSString* metalFileName = [[NSString alloc] initWithBytes: shaderPath.ptr length:shaderPath.len encoding: NSUTF8StringEncoding];
			NSError* err = 0;
			id<MTLLibrary> library = [metalSurface->device newLibraryWithFile: metalFileName error:&err];
			if(err != nil)
			{
				const char* errStr = [[err localizedDescription] UTF8String];
				LOG_ERROR("error : %s\n", errStr);
				return(0);
			}
			id<MTLFunction> rasterFunction = [library newFunctionWithName:@"mtl_raster"];
			id<MTLFunction> vertexFunction = [library newFunctionWithName:@"mtl_vertex_shader"];
			id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"mtl_fragment_shader"];

			//NOTE: create pipelines
			NSError* error = NULL;
			backend->rasterPipeline = [metalSurface->device newComputePipelineStateWithFunction: rasterFunction
		                                                                           	error:&error];

			MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
			pipelineStateDescriptor.label = @"blit pipeline";
			pipelineStateDescriptor.vertexFunction = vertexFunction;
			pipelineStateDescriptor.fragmentFunction = fragmentFunction;
			pipelineStateDescriptor.colorAttachments[0].pixelFormat = metalSurface->mtlLayer.pixelFormat;
			pipelineStateDescriptor.colorAttachments[0].blendingEnabled = YES;
			pipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
			pipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
			pipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
			pipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
			pipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
			pipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

			backend->blitPipeline = [metalSurface->device newRenderPipelineStateWithDescriptor: pipelineStateDescriptor error:&err];

			//NOTE: create textures
			mp_rect frame = mg_surface_get_frame(surface);
			f32 scale = metalSurface->mtlLayer.contentsScale;

			MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
			texDesc.textureType = MTLTextureType2D;
			texDesc.storageMode = MTLStorageModePrivate;
			texDesc.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
			texDesc.pixelFormat = MTLPixelFormatRGBA8Unorm;
			texDesc.width = frame.w * scale;
			texDesc.height = frame.h * scale;

			backend->outTexture = [metalSurface->device newTextureWithDescriptor:texDesc];

			//NOTE: create buffers
			MTLResourceOptions bufferOptions = MTLResourceCPUCacheModeWriteCombined
			                                 | MTLResourceStorageModeShared;

			backend->pathBuffer = [metalSurface->device newBufferWithLength: MG_MTL_PATH_BUFFER_SIZE
			                                                 options: bufferOptions];

			backend->segmentBuffer = [metalSurface->device newBufferWithLength: MG_MTL_SEGMENT_BUFFER_SIZE
			                                                   options: bufferOptions];
		}

	}
	return((mg_canvas_backend*)backend);
}

#undef LOG_SUBSYSTEM
