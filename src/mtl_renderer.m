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

const int MG_MTL_INPUT_BUFFERS_COUNT = 3,
          MG_MTL_TILE_SIZE = 16;

typedef struct mg_mtl_canvas_backend
{
	mg_canvas_backend interface;
	mg_surface surface;

	id<MTLComputePipelineState> pathPipeline;
	id<MTLComputePipelineState> segmentPipeline;
	id<MTLComputePipelineState> backpropPipeline;
	id<MTLComputePipelineState> mergePipeline;
	id<MTLComputePipelineState> rasterPipeline;
	id<MTLRenderPipelineState> blitPipeline;

	id<MTLTexture> outTexture;

	int bufferIndex;
	dispatch_semaphore_t bufferSemaphore;

	id<MTLBuffer> pathBuffer[MG_MTL_INPUT_BUFFERS_COUNT];
	id<MTLBuffer> elementBuffer[MG_MTL_INPUT_BUFFERS_COUNT];

	id<MTLBuffer> segmentCountBuffer;
	id<MTLBuffer> segmentBuffer;
	id<MTLBuffer> pathQueueBuffer;
	id<MTLBuffer> tileQueueBuffer;
	id<MTLBuffer> tileQueueCountBuffer;
	id<MTLBuffer> tileOpBuffer;
	id<MTLBuffer> tileOpCountBuffer;
	id<MTLBuffer> screenTilesBuffer;

} mg_mtl_canvas_backend;


static void mg_update_path_extents(vec4* extents, vec2 p)
{
	extents->x = minimum(extents->x, p.x);
	extents->y = minimum(extents->y, p.y);
	extents->z = maximum(extents->z, p.x);
	extents->w = maximum(extents->w, p.y);
}

void mg_mtl_canvas_render(mg_canvas_backend* interface,
                          mg_color clearColor,
                          u32 primitiveCount,
                          mg_primitive* primitives,
                          u32 eltCount,
                          mg_path_elt* pathElements)
{
	mg_mtl_canvas_backend* backend = (mg_mtl_canvas_backend*)interface;

	//NOTE: update rolling buffers
	dispatch_semaphore_wait(backend->bufferSemaphore, DISPATCH_TIME_FOREVER);
	backend->bufferIndex = (backend->bufferIndex + 1) % MG_MTL_INPUT_BUFFERS_COUNT;

	mg_mtl_path_elt* elementBufferData = (mg_mtl_path_elt*)[backend->elementBuffer[backend->bufferIndex] contents];
	mg_mtl_path* pathBufferData = (mg_mtl_path*)[backend->pathBuffer[backend->bufferIndex] contents];

	//NOTE: fill renderer input buffers
	int pathCount = 0;
	int mtlEltCount = 0;
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
					/////////////////////////////////////////////////////////////////////////////////////
					//TODO: order control points so that we can collapse all elements into same codepath
					/////////////////////////////////////////////////////////////////////////////////////

					//NOTE: transform and push path elt + update primitive bounding box
					vec2 p0 = mg_mat2x3_mul(primitive->attributes.transform, currentPos);
					vec2 p3 = mg_mat2x3_mul(primitive->attributes.transform, elt->p[0]);
					currentPos = elt->p[0];

					mg_update_path_extents(&pathExtents, p0);
					mg_update_path_extents(&pathExtents, p3);

					mg_mtl_path_elt* mtlElt = &elementBufferData[mtlEltCount];
					mtlEltCount++;

					mtlElt->pathIndex = primitiveIndex;
					mtlElt->kind = (mg_mtl_seg_kind)elt->type;
					mtlElt->p[0] = (vector_float2){p0.x, p0.y};
					mtlElt->p[3] = (vector_float2){p3.x, p3.y};
				}
				else if(elt->type == MG_PATH_QUADRATIC)
				{
					vec2 p0 = mg_mat2x3_mul(primitive->attributes.transform, currentPos);
					vec2 p1 = mg_mat2x3_mul(primitive->attributes.transform, elt->p[0]);
					vec2 p3 = mg_mat2x3_mul(primitive->attributes.transform, elt->p[1]);
					currentPos = elt->p[1];

					mg_update_path_extents(&pathExtents, p0);
					mg_update_path_extents(&pathExtents, p1);
					mg_update_path_extents(&pathExtents, p3);

					mg_mtl_path_elt* mtlElt = &elementBufferData[mtlEltCount];
					mtlEltCount++;

					mtlElt->pathIndex = primitiveIndex;
					mtlElt->kind = (mg_mtl_seg_kind)elt->type;
					mtlElt->p[0] = (vector_float2){p0.x, p0.y};
					mtlElt->p[1] = (vector_float2){p1.x, p1.y};
					mtlElt->p[3] = (vector_float2){p3.x, p3.y};
				}
				else if(elt->type == MG_PATH_CUBIC)
				{
					vec2 p0 = mg_mat2x3_mul(primitive->attributes.transform, currentPos);
					vec2 p1 = mg_mat2x3_mul(primitive->attributes.transform, elt->p[0]);
					vec2 p2 = mg_mat2x3_mul(primitive->attributes.transform, elt->p[1]);
					vec2 p3 = mg_mat2x3_mul(primitive->attributes.transform, elt->p[2]);
					currentPos = elt->p[2];

					mg_update_path_extents(&pathExtents, p0);
					mg_update_path_extents(&pathExtents, p1);
					mg_update_path_extents(&pathExtents, p2);
					mg_update_path_extents(&pathExtents, p3);

					mg_mtl_path_elt* mtlElt = &elementBufferData[mtlEltCount];
					mtlEltCount++;

					mtlElt->pathIndex = primitiveIndex;
					mtlElt->kind = (mg_mtl_seg_kind)elt->type;
					mtlElt->p[0] = (vector_float2){p0.x, p0.y};
					mtlElt->p[1] = (vector_float2){p1.x, p1.y};
					mtlElt->p[1] = (vector_float2){p2.x, p2.y};
					mtlElt->p[3] = (vector_float2){p3.x, p3.y};
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
	int tileSize = MG_MTL_TILE_SIZE;
	int nTilesX = (int)(frame.w * scale + tileSize - 1)/tileSize;
	int nTilesY = (int)(frame.h * scale + tileSize - 1)/tileSize;

	/////////////////////////////////////////////////////////////////////////////////////
	//TODO: ensure screen tiles buffer is correct size
	/////////////////////////////////////////////////////////////////////////////////////

	//NOTE: encode GPU commands
	@autoreleasepool
	{
		mg_mtl_surface_acquire_command_buffer(surface);

		//NOTE: clear counters
		id<MTLBlitCommandEncoder> blitEncoder = [surface->commandBuffer blitCommandEncoder];
		blitEncoder.label = @"clear counters";
		[blitEncoder fillBuffer: backend->segmentCountBuffer range: NSMakeRange(0, sizeof(int)) value: 0];
		[blitEncoder fillBuffer: backend->tileQueueCountBuffer range: NSMakeRange(0, sizeof(int)) value: 0];
		[blitEncoder fillBuffer: backend->tileOpCountBuffer range: NSMakeRange(0, sizeof(int)) value: 0];
		[blitEncoder endEncoding];

		//NOTE: path setup pass
		id<MTLComputeCommandEncoder> pathEncoder = [surface->commandBuffer computeCommandEncoder];
		pathEncoder.label = @"path pass";
		[pathEncoder setComputePipelineState: backend->pathPipeline];

		[pathEncoder setBytes:&pathCount length:sizeof(int) atIndex:0];
		[pathEncoder setBuffer:backend->pathBuffer[backend->bufferIndex] offset:0 atIndex:1];
		[pathEncoder setBuffer:backend->pathQueueBuffer offset:0 atIndex:2];
		[pathEncoder setBuffer:backend->tileQueueBuffer offset:0 atIndex:3];
		[pathEncoder setBuffer:backend->tileQueueCountBuffer offset:0 atIndex:4];
		[pathEncoder setBytes:&tileSize length:sizeof(int) atIndex:5];

		MTLSize pathGridSize = MTLSizeMake(pathCount, 1, 1);
		MTLSize pathGroupSize = MTLSizeMake([backend->pathPipeline maxTotalThreadsPerThreadgroup], 1, 1);

		[pathEncoder dispatchThreads: pathGridSize threadsPerThreadgroup: pathGroupSize];
		[pathEncoder endEncoding];

		//NOTE: segment setup pass
		id<MTLComputeCommandEncoder> segmentEncoder = [surface->commandBuffer computeCommandEncoder];
		segmentEncoder.label = @"segment pass";
		[segmentEncoder setComputePipelineState: backend->segmentPipeline];

		[segmentEncoder setBytes:&eltCount length:sizeof(int) atIndex:0];
		[segmentEncoder setBuffer:backend->elementBuffer[backend->bufferIndex] offset:0 atIndex:1];
		[segmentEncoder setBuffer:backend->segmentCountBuffer offset:0 atIndex:2];
		[segmentEncoder setBuffer:backend->segmentBuffer offset:0 atIndex:3];
		[segmentEncoder setBuffer:backend->pathQueueBuffer offset:0 atIndex:4];
		[segmentEncoder setBuffer:backend->tileQueueBuffer offset:0 atIndex:5];
		[segmentEncoder setBuffer:backend->tileOpBuffer offset:0 atIndex:6];
		[segmentEncoder setBuffer:backend->tileOpCountBuffer offset:0 atIndex:7];
		[segmentEncoder setBytes:&tileSize length:sizeof(int) atIndex:8];

		MTLSize segmentGridSize = MTLSizeMake(mtlEltCount, 1, 1);
		MTLSize segmentGroupSize = MTLSizeMake([backend->segmentPipeline maxTotalThreadsPerThreadgroup], 1, 1);

		[segmentEncoder dispatchThreads: segmentGridSize threadsPerThreadgroup: segmentGroupSize];
		[segmentEncoder endEncoding];

		//NOTE: backprop pass
		id<MTLComputeCommandEncoder> backpropEncoder = [surface->commandBuffer computeCommandEncoder];
		backpropEncoder.label = @"backprop pass";
		[backpropEncoder setComputePipelineState: backend->backpropPipeline];

		[backpropEncoder setBuffer:backend->pathQueueBuffer offset:0 atIndex:0];
		[backpropEncoder setBuffer:backend->tileQueueBuffer offset:0 atIndex:1];

		MTLSize backpropGroupSize = MTLSizeMake([backend->backpropPipeline maxTotalThreadsPerThreadgroup], 1, 1);
		MTLSize backpropGridSize = MTLSizeMake(pathCount*backpropGroupSize.width, 1, 1);

		[backpropEncoder dispatchThreads: backpropGridSize threadsPerThreadgroup: backpropGroupSize];
		[backpropEncoder endEncoding];

		//NOTE: merge pass
		id<MTLComputeCommandEncoder> mergeEncoder = [surface->commandBuffer computeCommandEncoder];
		mergeEncoder.label = @"merge pass";
		[mergeEncoder setComputePipelineState: backend->mergePipeline];

		[mergeEncoder setBytes:&pathCount length:sizeof(int) atIndex:0];
		[mergeEncoder setBuffer:backend->pathBuffer[backend->bufferIndex] offset:0 atIndex:1];
		[mergeEncoder setBuffer:backend->pathQueueBuffer offset:0 atIndex:2];
		[mergeEncoder setBuffer:backend->tileQueueBuffer offset:0 atIndex:3];
		[mergeEncoder setBuffer:backend->tileOpBuffer offset:0 atIndex:4];
		[mergeEncoder setBuffer:backend->tileOpCountBuffer offset:0 atIndex:5];
		[mergeEncoder setBuffer:backend->screenTilesBuffer offset:0 atIndex:6];

		MTLSize mergeGridSize = MTLSizeMake(nTilesX, nTilesY, 1);
		MTLSize mergeGroupSize = MTLSizeMake(16, 16, 1);

		[mergeEncoder dispatchThreads: mergeGridSize threadsPerThreadgroup: mergeGroupSize];
		[mergeEncoder endEncoding];

		//NOTE: raster pass
		id<MTLComputeCommandEncoder> rasterEncoder = [surface->commandBuffer computeCommandEncoder];
		rasterEncoder.label = @"raster pass";
		[rasterEncoder setComputePipelineState: backend->rasterPipeline];

		[rasterEncoder setBuffer:backend->screenTilesBuffer offset:0 atIndex:0];
		[rasterEncoder setBuffer:backend->tileOpBuffer offset:0 atIndex:1];
		[rasterEncoder setBuffer:backend->pathBuffer[backend->bufferIndex] offset:0 atIndex:2];
		[rasterEncoder setBuffer:backend->segmentBuffer offset:0 atIndex:3];
		[rasterEncoder setBytes:&tileSize length:sizeof(int) atIndex:4];

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
			renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
			renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
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

		//NOTE: finalize
		[surface->commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> commandBuffer)
			{
				dispatch_semaphore_signal(backend->bufferSemaphore);
			}];
	}
}

void mg_mtl_canvas_destroy(mg_canvas_backend* interface)
{
	mg_mtl_canvas_backend* backend = (mg_mtl_canvas_backend*)interface;

	@autoreleasepool
	{
		[backend->pathPipeline release];
		[backend->segmentPipeline release];
		[backend->backpropPipeline release];
		[backend->mergePipeline release];
		[backend->rasterPipeline release];
		[backend->blitPipeline release];

		for(int i=0; i<MG_MTL_INPUT_BUFFERS_COUNT; i++)
		{
			[backend->pathBuffer[i] release];
			[backend->elementBuffer[i] release];
		}
		[backend->segmentCountBuffer release];
		[backend->segmentBuffer release];
		[backend->tileQueueBuffer release];
		[backend->tileQueueCountBuffer release];
		[backend->tileOpBuffer release];
		[backend->tileOpCountBuffer release];
		[backend->screenTilesBuffer release];
	}

	free(backend);
}

const u32 MG_MTL_PATH_BUFFER_SIZE       = (4<<20)*sizeof(mg_mtl_path),
          MG_MTL_ELEMENT_BUFFER_SIZE    = (4<<20)*sizeof(mg_mtl_path_elt),
          MG_MTL_SEGMENT_BUFFER_SIZE    = (4<<20)*sizeof(mg_mtl_segment),
          MG_MTL_PATH_QUEUE_BUFFER_SIZE = (4<<20)*sizeof(mg_mtl_path_queue),
          MG_MTL_TILE_QUEUE_BUFFER_SIZE = (4<<20)*sizeof(mg_mtl_tile_queue),
          MG_MTL_TILE_OP_BUFFER_SIZE    = (4<<20)*sizeof(mg_mtl_tile_op);

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
			id<MTLFunction> pathFunction = [library newFunctionWithName:@"mtl_path_setup"];
			id<MTLFunction> segmentFunction = [library newFunctionWithName:@"mtl_segment_setup"];
			id<MTLFunction> backpropFunction = [library newFunctionWithName:@"mtl_backprop"];
			id<MTLFunction> mergeFunction = [library newFunctionWithName:@"mtl_merge"];
			id<MTLFunction> rasterFunction = [library newFunctionWithName:@"mtl_raster"];
			id<MTLFunction> vertexFunction = [library newFunctionWithName:@"mtl_vertex_shader"];
			id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"mtl_fragment_shader"];

			//NOTE: create pipelines
			NSError* error = NULL;

			backend->pathPipeline = [metalSurface->device newComputePipelineStateWithFunction: pathFunction
		                                                                           	error:&error];

			backend->segmentPipeline = [metalSurface->device newComputePipelineStateWithFunction: segmentFunction
		                                                                           	error:&error];

			backend->backpropPipeline = [metalSurface->device newComputePipelineStateWithFunction: backpropFunction
		                                                                           	error:&error];

			backend->mergePipeline = [metalSurface->device newComputePipelineStateWithFunction: mergeFunction
		                                                                           	error:&error];

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

			backend->bufferSemaphore = dispatch_semaphore_create(MG_MTL_INPUT_BUFFERS_COUNT);
			backend->bufferIndex = 0;

			MTLResourceOptions bufferOptions = MTLResourceCPUCacheModeWriteCombined
			                                 | MTLResourceStorageModeShared;

			for(int i=0; i<MG_MTL_INPUT_BUFFERS_COUNT; i++)
			{
				backend->pathBuffer[i] = [metalSurface->device newBufferWithLength: MG_MTL_PATH_BUFFER_SIZE
			                                                 	options: bufferOptions];

				backend->elementBuffer[i] = [metalSurface->device newBufferWithLength: MG_MTL_ELEMENT_BUFFER_SIZE
			                                                   	options: bufferOptions];
			}

			bufferOptions = MTLResourceStorageModePrivate;
			backend->segmentBuffer = [metalSurface->device newBufferWithLength: MG_MTL_SEGMENT_BUFFER_SIZE
			                                                   options: bufferOptions];

			backend->segmentCountBuffer = [metalSurface->device newBufferWithLength: sizeof(int)
			                                                   options: bufferOptions];


			backend->pathQueueBuffer = [metalSurface->device newBufferWithLength: MG_MTL_PATH_QUEUE_BUFFER_SIZE
			                                                   options: bufferOptions];

			backend->tileQueueBuffer = [metalSurface->device newBufferWithLength: MG_MTL_TILE_QUEUE_BUFFER_SIZE
			                                                   options: bufferOptions];

			backend->tileQueueCountBuffer = [metalSurface->device newBufferWithLength: sizeof(int)
			                                                   options: bufferOptions];

			backend->tileOpBuffer = [metalSurface->device newBufferWithLength: MG_MTL_TILE_OP_BUFFER_SIZE
			                                                   options: bufferOptions];

			backend->tileOpCountBuffer = [metalSurface->device newBufferWithLength: sizeof(int)
			                                                   options: bufferOptions];

			int tileSize = MG_MTL_TILE_SIZE;
			int nTilesX = (int)(frame.w * scale + tileSize - 1)/tileSize;
			int nTilesY = (int)(frame.h * scale + tileSize - 1)/tileSize;
			backend->screenTilesBuffer = [metalSurface->device newBufferWithLength: nTilesX*nTilesY*sizeof(int)
			                                                   options: bufferOptions];
		}
	}
	return((mg_canvas_backend*)backend);
}

#undef LOG_SUBSYSTEM
