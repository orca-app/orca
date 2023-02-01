/************************************************************//**
*
*	@file: metal_canvas.m
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

#include"metal_shader.h"

#define LOG_SUBSYSTEM "Graphics"

static const int MG_METAL_CANVAS_DEFAULT_BUFFER_LENGTH = 4<<20;

typedef struct mg_metal_canvas_backend
{
	mg_canvas_backend interface;
	mg_surface surface;

	// permanent metal resources
	id<MTLComputePipelineState> tilingPipeline;
	id<MTLComputePipelineState> sortingPipeline;
	id<MTLComputePipelineState> boxingPipeline;
	id<MTLComputePipelineState> computePipeline;
	id<MTLRenderPipelineState> renderPipeline;

	mp_rect viewPort;

	// textures and buffers
	id<MTLTexture> outTexture;
	id<MTLTexture> atlasTexture;
	id<MTLBuffer> vertexBuffer;
	id<MTLBuffer> indexBuffer;
	id<MTLBuffer> tileCounters;
	id<MTLBuffer> tilesArray;
	id<MTLBuffer> triangleArray;
	id<MTLBuffer> boxArray;

} mg_metal_canvas_backend;

mg_metal_surface* mg_metal_canvas_get_surface(mg_metal_canvas_backend* canvas)
{
	mg_metal_surface* res = 0;
	mg_surface_data* data = mg_surface_data_from_handle(canvas->surface);
	if(data && data->backend == MG_BACKEND_METAL)
	{
		res = (mg_metal_surface*)data;
	}
	return(res);
}

void mg_metal_canvas_draw_buffers(mg_canvas_backend* interface, u32 vertexCount, u32 indexCount, mg_color clearColor)
{
	mg_metal_canvas_backend* backend = (mg_metal_canvas_backend*)interface;
	mg_metal_surface* surface = mg_metal_canvas_get_surface(backend);
	if(!surface)
	{
		return;
	}

	@autoreleasepool
	{
		if(surface->commandBuffer == nil || surface->commandBuffer == nil)
		{
			mg_metal_surface_acquire_drawable_and_command_buffer(surface);
		}

		ASSERT(indexCount * sizeof(i32) < [backend->indexBuffer length]);

		f32 scale = surface->metalLayer.contentsScale;
		vector_uint2  viewportSize = {backend->viewPort.w * scale, backend->viewPort.h * scale};

		//-----------------------------------------------------------
		//NOTE(martin): encode the clear counter
		//-----------------------------------------------------------
		id<MTLBlitCommandEncoder> blitEncoder = [surface->commandBuffer blitCommandEncoder];
		[blitEncoder fillBuffer: backend->tileCounters range: NSMakeRange(0, RENDERER_MAX_TILES*sizeof(uint)) value: 0];
		[blitEncoder endEncoding];

		//-----------------------------------------------------------
		//NOTE(martin): encode the boxing pass
		//-----------------------------------------------------------
		id<MTLComputeCommandEncoder> boxEncoder = [surface->commandBuffer computeCommandEncoder];
		[boxEncoder setComputePipelineState: backend->boxingPipeline];
		[boxEncoder setBuffer: backend->vertexBuffer offset:0 atIndex: 0];
		[boxEncoder setBuffer: backend->indexBuffer offset:0 atIndex: 1];
		[boxEncoder setBuffer: backend->triangleArray offset:0 atIndex: 2];
		[boxEncoder setBuffer: backend->boxArray offset:0 atIndex: 3];
		[boxEncoder setBytes: &scale length: sizeof(float) atIndex: 4];

		MTLSize boxGroupSize = MTLSizeMake(backend->boxingPipeline.maxTotalThreadsPerThreadgroup, 1, 1);
		MTLSize boxGridSize = MTLSizeMake(indexCount/3, 1, 1);

		[boxEncoder dispatchThreads: boxGridSize threadsPerThreadgroup: boxGroupSize];
		[boxEncoder endEncoding];

		//-----------------------------------------------------------
		//NOTE(martin): encode the tiling pass
		//-----------------------------------------------------------

		id<MTLComputeCommandEncoder> tileEncoder = [surface->commandBuffer computeCommandEncoder];
		[tileEncoder setComputePipelineState: backend->tilingPipeline];
		[tileEncoder setBuffer: backend->boxArray offset:0 atIndex: 0];
		[tileEncoder setBuffer: backend->tileCounters offset:0 atIndex: 1];
		[tileEncoder setBuffer: backend->tilesArray offset:0 atIndex: 2];
		[tileEncoder setBytes: &viewportSize length: sizeof(vector_uint2) atIndex: 3];

		[tileEncoder dispatchThreads: boxGridSize threadsPerThreadgroup: boxGroupSize];
		[tileEncoder endEncoding];

		//-----------------------------------------------------------
		//NOTE(martin): encode the sorting pass
		//-----------------------------------------------------------

		id<MTLComputeCommandEncoder> sortEncoder = [surface->commandBuffer computeCommandEncoder];
		[sortEncoder setComputePipelineState: backend->sortingPipeline];
		[sortEncoder setBuffer: backend->tileCounters offset:0 atIndex: 0];
		[sortEncoder setBuffer: backend->triangleArray offset:0 atIndex: 1];
		[sortEncoder setBuffer: backend->tilesArray offset:0 atIndex: 2];
		[sortEncoder setBytes: &viewportSize length: sizeof(vector_uint2) atIndex: 3];

		u32     nTilesX = (viewportSize.x + RENDERER_TILE_SIZE - 1)/RENDERER_TILE_SIZE;
		u32     nTilesY = (viewportSize.y + RENDERER_TILE_SIZE - 1)/RENDERER_TILE_SIZE;

		MTLSize sortGroupSize = MTLSizeMake(backend->boxingPipeline.maxTotalThreadsPerThreadgroup, 1, 1);
		MTLSize sortGridSize = MTLSizeMake(nTilesX*nTilesY, 1, 1);

		[sortEncoder dispatchThreads: sortGridSize threadsPerThreadgroup: sortGroupSize];
		[sortEncoder endEncoding];

		//-----------------------------------------------------------
		//NOTE(martin): create compute encoder and encode commands
		//-----------------------------------------------------------
		vector_float4 clearColorVec4 = {clearColor.r, clearColor.g, clearColor.b, clearColor.a};

		id<MTLComputeCommandEncoder> encoder = [surface->commandBuffer computeCommandEncoder];
		[encoder setComputePipelineState:backend->computePipeline];
		[encoder setTexture: backend->outTexture atIndex: 0];
		[encoder setTexture: backend->atlasTexture atIndex: 1];
		[encoder setBuffer: backend->vertexBuffer offset:0 atIndex: 0];
		[encoder setBuffer: backend->tileCounters offset:0 atIndex: 1];
		[encoder setBuffer: backend->tilesArray offset:0 atIndex: 2];
		[encoder setBuffer: backend->triangleArray offset:0 atIndex: 3];
		[encoder setBuffer: backend->boxArray offset:0 atIndex: 4];
		[encoder setBytes: &clearColorVec4 length: sizeof(vector_float4) atIndex: 5];

		//TODO: check that we don't exceed maxTotalThreadsPerThreadgroup
		DEBUG_ASSERT(RENDERER_TILE_SIZE*RENDERER_TILE_SIZE <= backend->computePipeline.maxTotalThreadsPerThreadgroup);
		MTLSize threadGridSize = MTLSizeMake(viewportSize.x, viewportSize.y, 1);
		MTLSize threadGroupSize = MTLSizeMake(RENDERER_TILE_SIZE, RENDERER_TILE_SIZE, 1);

		[encoder dispatchThreads: threadGridSize threadsPerThreadgroup:threadGroupSize];
		[encoder endEncoding];

		//-----------------------------------------------------------
		//NOTE(martin): acquire drawable, create render encoder to blit texture to framebuffer
		//-----------------------------------------------------------

		MTLViewport viewport = {backend->viewPort.x * scale,
		                        backend->viewPort.y * scale,
		                        backend->viewPort.w * scale,
		                        backend->viewPort.h * scale,
		                        0,
		                        1};

		MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
		renderPassDescriptor.colorAttachments[0].texture = surface->drawable.texture;
		renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

		id<MTLRenderCommandEncoder> renderEncoder = [surface->commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
		[renderEncoder setViewport: viewport];
		[renderEncoder setRenderPipelineState: backend->renderPipeline];
		[renderEncoder setFragmentTexture: backend->outTexture atIndex: 0];
		[renderEncoder drawPrimitives: MTLPrimitiveTypeTriangle
			 vertexStart: 0
			 vertexCount: 3 ];
		[renderEncoder endEncoding];
	}
}

/*
void mg_metal_canvas_viewport(mg_canvas_backend* interface, mp_rect viewPort)
{
	mg_metal_canvas_backend* backend = (mg_metal_canvas_backend*)interface;
	mg_metal_surface* surface = mg_metal_canvas_get_surface(backend);
	if(!surface)
	{
		return;
	}

	backend->viewPort = viewPort;

	@autoreleasepool
	{
		f32 scale = surface->metalLayer.contentsScale;
		CGSize drawableSize = (CGSize){.width = viewPort.w * scale, .height = viewPort.h * scale};

		[backend->outTexture release];

		MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
		texDesc.textureType = MTLTextureType2D;
		texDesc.storageMode = MTLStorageModePrivate;
		texDesc.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
		texDesc.pixelFormat = MTLPixelFormatBGRA8Unorm;// MTLPixelFormatBGRA8Unorm_sRGB;
		texDesc.width = drawableSize.width;
		texDesc.height = drawableSize.height;

		backend->outTexture = [surface->device newTextureWithDescriptor:texDesc];
	}
}
*/

void mg_metal_canvas_update_vertex_layout(mg_metal_canvas_backend* backend)
{
	char* vertexBase = (char*)[backend->vertexBuffer contents];

	backend->interface.vertexLayout = (mg_vertex_layout){
		    .maxVertexCount = MG_METAL_CANVAS_DEFAULT_BUFFER_LENGTH,
	        .maxIndexCount = MG_METAL_CANVAS_DEFAULT_BUFFER_LENGTH,
	        .posBuffer = vertexBase + offsetof(mg_vertex, pos),
	        .posStride = sizeof(mg_vertex),
	        .cubicBuffer = vertexBase + offsetof(mg_vertex, cubic),
	        .cubicStride = sizeof(mg_vertex),
	        .uvBuffer = vertexBase + offsetof(mg_vertex, uv),
	        .uvStride = sizeof(mg_vertex),
	        .colorBuffer = vertexBase + offsetof(mg_vertex, color),
	        .colorStride = sizeof(mg_vertex),
	        .zIndexBuffer = vertexBase + offsetof(mg_vertex, zIndex),
	        .zIndexStride = sizeof(mg_vertex),
	        .clipBuffer = vertexBase + offsetof(mg_vertex, clip),
	        .clipStride = sizeof(mg_vertex),
	        .indexBuffer = [backend->indexBuffer contents],
	        .indexStride = sizeof(int)};
}

void mg_metal_canvas_destroy(mg_canvas_backend* interface)
{
	mg_metal_canvas_backend* backend = (mg_metal_canvas_backend*)interface;

	@autoreleasepool
	{
		[backend->outTexture release];
		[backend->atlasTexture release];
		[backend->vertexBuffer release];
		[backend->indexBuffer release];
		[backend->tilesArray release];
		[backend->triangleArray release];
		[backend->boxArray release];
		[backend->computePipeline release];
	}
}

void mg_metal_canvas_atlas_upload(mg_canvas_backend* interface, mp_rect rect, u8* bytes)
{@autoreleasepool{
	mg_metal_canvas_backend* backend = (mg_metal_canvas_backend*)interface;

	MTLRegion region = MTLRegionMake2D(rect.x, rect.y, rect.w, rect.h);
	[backend->atlasTexture replaceRegion:region
	                       mipmapLevel:0
	                       withBytes:(void*)bytes
	                       bytesPerRow: 4 * rect.w];
}}

mg_canvas_backend* mg_metal_canvas_create(mg_surface surface)
{
	mg_metal_canvas_backend* backend = 0;

	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->backend == MG_BACKEND_METAL)
	{
		mg_metal_surface* metalSurface = (mg_metal_surface*)surfaceData;

		backend = malloc_type(mg_metal_canvas_backend);
		backend->surface = surface;

		//NOTE(martin): setup interface functions
		backend->interface.destroy = mg_metal_canvas_destroy;
		backend->interface.drawBuffers = mg_metal_canvas_draw_buffers;
		backend->interface.atlasUpload = mg_metal_canvas_atlas_upload;

		mp_rect frame = mg_surface_get_frame(surface);
		backend->viewPort = (mp_rect){0, 0, frame.w, frame.h};

		@autoreleasepool
		{
			f32 scale = metalSurface->metalLayer.contentsScale;
			CGSize drawableSize = (CGSize){.width = backend->viewPort.w * scale, .height = backend->viewPort.h * scale};

			//-----------------------------------------------------------
			//NOTE(martin): create our output texture
			//-----------------------------------------------------------
			MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
			texDesc.textureType = MTLTextureType2D;
			texDesc.storageMode = MTLStorageModePrivate;
			texDesc.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
			texDesc.pixelFormat = MTLPixelFormatBGRA8Unorm;// MTLPixelFormatBGRA8Unorm_sRGB;
			texDesc.width = drawableSize.width;
			texDesc.height = drawableSize.height;

			backend->outTexture = [metalSurface->device newTextureWithDescriptor:texDesc];
			//TODO(martin): retain ?

			//-----------------------------------------------------------
			//NOTE(martin): create our atlas texture
			//-----------------------------------------------------------
			texDesc.textureType = MTLTextureType2D;
			texDesc.storageMode = MTLStorageModeManaged;
			texDesc.usage = MTLTextureUsageShaderRead;
			texDesc.pixelFormat = MTLPixelFormatRGBA8Unorm; //MTLPixelFormatBGRA8Unorm;
			texDesc.width = MG_ATLAS_SIZE;
			texDesc.height = MG_ATLAS_SIZE;

			backend->atlasTexture = [metalSurface->device newTextureWithDescriptor:texDesc];

			//-----------------------------------------------------------
			//NOTE(martin): create buffers for vertex and index
			//-----------------------------------------------------------

			MTLResourceOptions bufferOptions = MTLResourceCPUCacheModeWriteCombined
		                                 	| MTLResourceStorageModeShared;

			backend->indexBuffer = [metalSurface->device newBufferWithLength: MG_METAL_CANVAS_DEFAULT_BUFFER_LENGTH*sizeof(int)
		                                        	options: bufferOptions];

			backend->vertexBuffer = [metalSurface->device newBufferWithLength: MG_METAL_CANVAS_DEFAULT_BUFFER_LENGTH*sizeof(mg_vertex)
		                                        	options: bufferOptions];

			backend->tilesArray = [metalSurface->device newBufferWithLength: RENDERER_TILE_BUFFER_SIZE*sizeof(int)*RENDERER_MAX_TILES
								options: MTLResourceStorageModePrivate];

			backend->triangleArray = [metalSurface->device newBufferWithLength: MG_METAL_CANVAS_DEFAULT_BUFFER_LENGTH*sizeof(mg_triangle_data)
								options: MTLResourceStorageModePrivate];

			backend->boxArray = [metalSurface->device newBufferWithLength: MG_METAL_CANVAS_DEFAULT_BUFFER_LENGTH*sizeof(vector_float4)
								options: MTLResourceStorageModePrivate];

			//TODO(martin): retain ?
			//-----------------------------------------------------------
			//NOTE(martin): create and initialize tile counters
			//-----------------------------------------------------------
			backend->tileCounters = [metalSurface->device newBufferWithLength: RENDERER_MAX_TILES*sizeof(uint)
		                                         	options: MTLResourceStorageModePrivate];
			id<MTLCommandBuffer> commandBuffer = [metalSurface->commandQueue commandBuffer];
			id<MTLBlitCommandEncoder> blitEncoder = [commandBuffer blitCommandEncoder];
			[blitEncoder fillBuffer: backend->tileCounters range: NSMakeRange(0, RENDERER_MAX_TILES*sizeof(uint)) value: 0];
			[blitEncoder endEncoding];
			[commandBuffer commit];

			//-----------------------------------------------------------
			//NOTE(martin): load the library
			//-----------------------------------------------------------

			//TODO(martin): filepath magic to find metallib path when not in the working directory
			str8 shaderPath = mp_app_get_resource_path(mem_scratch(), "../resources/metal_shader.metallib");
			NSString* metalFileName = [[NSString alloc] initWithBytes: shaderPath.ptr length:shaderPath.len encoding: NSUTF8StringEncoding];
			NSError* err = 0;
			id<MTLLibrary> library = [metalSurface->device newLibraryWithFile: metalFileName error:&err];
			if(err != nil)
			{
				const char* errStr = [[err localizedDescription] UTF8String];
				LOG_ERROR("error : %s\n", errStr);
				return(0);
			}
			id<MTLFunction> tilingFunction = [library newFunctionWithName:@"TileKernel"];
			id<MTLFunction> sortingFunction = [library newFunctionWithName:@"SortKernel"];
			id<MTLFunction> boxingFunction = [library newFunctionWithName:@"BoundingBoxKernel"];
			id<MTLFunction> computeFunction = [library newFunctionWithName:@"RenderKernel"];
			id<MTLFunction> vertexFunction = [library newFunctionWithName:@"VertexShader"];
			id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"FragmentShader"];

			//-----------------------------------------------------------
			//NOTE(martin): setup our data layout and pipeline state
			//-----------------------------------------------------------
			NSError* error = NULL;
			backend->computePipeline = [metalSurface->device newComputePipelineStateWithFunction: computeFunction
		                                                                           	error:&error];
			ASSERT(backend->computePipeline);

			MTLComputePipelineDescriptor* tilingPipelineDesc = [[MTLComputePipelineDescriptor alloc] init];
			tilingPipelineDesc.computeFunction = tilingFunction;
	//		tilingPipelineDesc.threadGroupSizeIsMultipleOfThreadExecutionWidth = true;

			backend->tilingPipeline = [metalSurface->device newComputePipelineStateWithDescriptor: tilingPipelineDesc
		                                           	options: MTLPipelineOptionNone
		                                           	reflection: nil
		                                           	error: &error];

			MTLComputePipelineDescriptor* sortingPipelineDesc = [[MTLComputePipelineDescriptor alloc] init];
			sortingPipelineDesc.computeFunction = sortingFunction;
	//		sortingPipelineDesc.threadGroupSizeIsMultipleOfThreadExecutionWidth = true;

			backend->sortingPipeline = [metalSurface->device newComputePipelineStateWithDescriptor: sortingPipelineDesc
		                                           	options: MTLPipelineOptionNone
		                                           	reflection: nil
		                                           	error: &error];

			MTLComputePipelineDescriptor* boxingPipelineDesc = [[MTLComputePipelineDescriptor alloc] init];
			boxingPipelineDesc.computeFunction = boxingFunction;
	//		boxingPipelineDesc.threadGroupSizeIsMultipleOfThreadExecutionWidth = true;

			backend->boxingPipeline = [metalSurface->device newComputePipelineStateWithDescriptor: boxingPipelineDesc
		                                           	options: MTLPipelineOptionNone
		                                           	reflection: nil
		                                           	error: &error];
			//-----------------------------------------------------------
			//NOTE(martin): setup our render pipeline state
			//-----------------------------------------------------------
			// create and initialize the pipeline state descriptor
			MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
			pipelineStateDescriptor.label = @"My simple pipeline";
			pipelineStateDescriptor.vertexFunction = vertexFunction;
			pipelineStateDescriptor.fragmentFunction = fragmentFunction;
			pipelineStateDescriptor.colorAttachments[0].pixelFormat = metalSurface->metalLayer.pixelFormat;

			// create render pipeline
			backend->renderPipeline = [metalSurface->device newRenderPipelineStateWithDescriptor: pipelineStateDescriptor error:&err];
			if(err != nil)
			{
				const char* errStr = [[err localizedDescription] UTF8String];
				const char* descStr = [[err localizedFailureReason] UTF8String];
				const char* recovStr = [[err localizedRecoverySuggestion] UTF8String];
				LOG_ERROR("(%li) %s. %s. %s\n", [err code], errStr, descStr, recovStr);
				return(0);
			}
		}

		mg_metal_canvas_update_vertex_layout(backend);
	}

	return((mg_canvas_backend*)backend);
}


#undef LOG_SUBSYSTEM
