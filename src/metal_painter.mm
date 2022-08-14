/************************************************************//**
*
*	@file: graphics.mm
*	@author: Martin Fouilleul
*	@date: 12/07/2020
*	@revision:
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

static const int MG_METAL_PAINTER_DEFAULT_BUFFER_LENGTH = 4<<20;

typedef struct mg_metal_painter
{
	mg_canvas_painter interface;

	mg_metal_surface* surface;

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

} mg_metal_painter;

void mg_metal_painter_draw_buffers(mg_canvas_painter* interface, u32 vertexCount, u32 indexCount, mg_color clearColor)
{
	mg_metal_painter* painter = (mg_metal_painter*)interface;
	mg_metal_surface* surface = painter->surface;

	@autoreleasepool
	{
		if(surface->commandBuffer == nil || surface->commandBuffer == nil)
		{
			mg_metal_surface_acquire_drawable_and_command_buffer(surface);
		}

		ASSERT(indexCount * sizeof(i32) < [painter->indexBuffer length]);

		f32 scale = surface->metalLayer.contentsScale;
		vector_uint2  viewportSize = {painter->viewPort.w * scale, painter->viewPort.h * scale};

		//-----------------------------------------------------------
		//NOTE(martin): encode the clear counter
		//-----------------------------------------------------------
		id<MTLBlitCommandEncoder> blitEncoder = [surface->commandBuffer blitCommandEncoder];
		[blitEncoder fillBuffer: painter->tileCounters range: NSMakeRange(0, RENDERER_MAX_TILES*sizeof(uint)) value: 0];
		[blitEncoder endEncoding];

		//-----------------------------------------------------------
		//NOTE(martin): encode the boxing pass
		//-----------------------------------------------------------
		id<MTLComputeCommandEncoder> boxEncoder = [surface->commandBuffer computeCommandEncoder];
		[boxEncoder setComputePipelineState: painter->boxingPipeline];
		[boxEncoder setBuffer: painter->vertexBuffer offset:0 atIndex: 0];
		[boxEncoder setBuffer: painter->indexBuffer offset:0 atIndex: 1];
		[boxEncoder setBuffer: painter->triangleArray offset:0 atIndex: 2];
		[boxEncoder setBuffer: painter->boxArray offset:0 atIndex: 3];
		[boxEncoder setBytes: &scale length: sizeof(float) atIndex: 4];

		MTLSize boxGroupSize = MTLSizeMake(painter->boxingPipeline.maxTotalThreadsPerThreadgroup, 1, 1);
		MTLSize boxGridSize = MTLSizeMake(indexCount/3, 1, 1);

		[boxEncoder dispatchThreads: boxGridSize threadsPerThreadgroup: boxGroupSize];
		[boxEncoder endEncoding];

		//-----------------------------------------------------------
		//NOTE(martin): encode the tiling pass
		//-----------------------------------------------------------

		id<MTLComputeCommandEncoder> tileEncoder = [surface->commandBuffer computeCommandEncoder];
		[tileEncoder setComputePipelineState: painter->tilingPipeline];
		[tileEncoder setBuffer: painter->boxArray offset:0 atIndex: 0];
		[tileEncoder setBuffer: painter->tileCounters offset:0 atIndex: 1];
		[tileEncoder setBuffer: painter->tilesArray offset:0 atIndex: 2];
		[tileEncoder setBytes: &viewportSize length: sizeof(vector_uint2) atIndex: 3];

		[tileEncoder dispatchThreads: boxGridSize threadsPerThreadgroup: boxGroupSize];
		[tileEncoder endEncoding];

		//-----------------------------------------------------------
		//NOTE(martin): encode the sorting pass
		//-----------------------------------------------------------

		id<MTLComputeCommandEncoder> sortEncoder = [surface->commandBuffer computeCommandEncoder];
		[sortEncoder setComputePipelineState: painter->sortingPipeline];
		[sortEncoder setBuffer: painter->tileCounters offset:0 atIndex: 0];
		[sortEncoder setBuffer: painter->triangleArray offset:0 atIndex: 1];
		[sortEncoder setBuffer: painter->tilesArray offset:0 atIndex: 2];
		[sortEncoder setBytes: &viewportSize length: sizeof(vector_uint2) atIndex: 3];

		u32     nTilesX = (viewportSize.x + RENDERER_TILE_SIZE - 1)/RENDERER_TILE_SIZE;
		u32     nTilesY = (viewportSize.y + RENDERER_TILE_SIZE - 1)/RENDERER_TILE_SIZE;

		MTLSize sortGroupSize = MTLSizeMake(painter->boxingPipeline.maxTotalThreadsPerThreadgroup, 1, 1);
		MTLSize sortGridSize = MTLSizeMake(nTilesX*nTilesY, 1, 1);

		[sortEncoder dispatchThreads: sortGridSize threadsPerThreadgroup: sortGroupSize];
		[sortEncoder endEncoding];

		//-----------------------------------------------------------
		//NOTE(martin): create compute encoder and encode commands
		//-----------------------------------------------------------
		vector_float4 clearColorVec4 = {clearColor.r, clearColor.g, clearColor.b, clearColor.a};

		id<MTLComputeCommandEncoder> encoder = [surface->commandBuffer computeCommandEncoder];
		[encoder setComputePipelineState:painter->computePipeline];
		[encoder setTexture: painter->outTexture atIndex: 0];
		[encoder setTexture: painter->atlasTexture atIndex: 1];
		[encoder setBuffer: painter->vertexBuffer offset:0 atIndex: 0];
		[encoder setBuffer: painter->tileCounters offset:0 atIndex: 1];
		[encoder setBuffer: painter->tilesArray offset:0 atIndex: 2];
		[encoder setBuffer: painter->triangleArray offset:0 atIndex: 3];
		[encoder setBuffer: painter->boxArray offset:0 atIndex: 4];
		[encoder setBytes: &clearColorVec4 length: sizeof(vector_float4) atIndex: 5];

		//TODO: check that we don't exceed maxTotalThreadsPerThreadgroup
		DEBUG_ASSERT(RENDERER_TILE_SIZE*RENDERER_TILE_SIZE <= painter->computePipeline.maxTotalThreadsPerThreadgroup);
		MTLSize threadGridSize = MTLSizeMake(viewportSize.x, viewportSize.y, 1);
		MTLSize threadGroupSize = MTLSizeMake(RENDERER_TILE_SIZE, RENDERER_TILE_SIZE, 1);

		[encoder dispatchThreads: threadGridSize threadsPerThreadgroup:threadGroupSize];
		[encoder endEncoding];

		//-----------------------------------------------------------
		//NOTE(martin): acquire drawable, create render encoder to blit texture to framebuffer
		//-----------------------------------------------------------

		MTLViewport viewport = {painter->viewPort.x * scale,
		                        painter->viewPort.y * scale,
		                        painter->viewPort.w * scale,
		                        painter->viewPort.h * scale,
		                        0,
		                        1};

		MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
		renderPassDescriptor.colorAttachments[0].texture = surface->drawable.texture;
		renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

		id<MTLRenderCommandEncoder> renderEncoder = [surface->commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
		[renderEncoder setViewport: viewport];
		[renderEncoder setRenderPipelineState: painter->renderPipeline];
		[renderEncoder setFragmentTexture: painter->outTexture atIndex: 0];
		[renderEncoder drawPrimitives: MTLPrimitiveTypeTriangle
			 vertexStart: 0
			 vertexCount: 3 ];
		[renderEncoder endEncoding];
	}
}

void mg_metal_painter_viewport(mg_canvas_painter* interface, mp_rect viewPort)
{
	mg_metal_painter* painter = (mg_metal_painter*)interface;
	mg_metal_surface* surface = painter->surface;

	painter->viewPort = viewPort;

	@autoreleasepool
	{
		f32 scale = surface->metalLayer.contentsScale;
		CGSize drawableSize = (CGSize){.width = viewPort.w * scale, .height = viewPort.h * scale};

		[painter->outTexture release];

		MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
		texDesc.textureType = MTLTextureType2D;
		texDesc.storageMode = MTLStorageModePrivate;
		texDesc.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
		texDesc.pixelFormat = MTLPixelFormatBGRA8Unorm;// MTLPixelFormatBGRA8Unorm_sRGB;
		texDesc.width = drawableSize.width;
		texDesc.height = drawableSize.height;

		painter->outTexture = [surface->device newTextureWithDescriptor:texDesc];
	}
}

void mg_metal_painter_update_vertex_layout(mg_metal_painter* painter)
{
	mg_metal_surface* surface = painter->surface;

	char* vertexBase = (char*)[painter->vertexBuffer contents];

	painter->interface.vertexLayout = (mgc_vertex_layout){
		    .maxVertexCount = MG_METAL_PAINTER_DEFAULT_BUFFER_LENGTH,
	        .maxIndexCount = MG_METAL_PAINTER_DEFAULT_BUFFER_LENGTH,
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
	        .clipsBuffer = vertexBase + offsetof(mg_vertex, clip),
	        .clipsStride = sizeof(mg_vertex),
	        .indexBuffer = [painter->indexBuffer contents],
	        .indexStride = sizeof(int)};
}


void mg_metal_painter_destroy(mg_canvas_painter* interface)
{
	mg_metal_painter* painter = (mg_metal_painter*)interface;

	@autoreleasepool
	{
		[painter->outTexture release];
		[painter->atlasTexture release];
		[painter->vertexBuffer release];
		[painter->indexBuffer release];
		[painter->tilesArray release];
		[painter->triangleArray release];
		[painter->boxArray release];
		[painter->computePipeline release];
	}
}

void mg_metal_painter_atlas_upload(mg_canvas_painter* interface, mp_rect rect, u8* bytes)
{@autoreleasepool{
	mg_metal_painter* painter = (mg_metal_painter*)interface;

	MTLRegion region = MTLRegionMake2D(rect.x, rect.y, rect.w, rect.h);
	[painter->atlasTexture replaceRegion:region
	                       mipmapLevel:0
	                       withBytes:(void*)bytes
	                       bytesPerRow: 4 * rect.w];
}}

extern "C" mg_canvas_painter* mg_metal_painter_create_for_surface(mg_metal_surface* surface, mp_rect viewPort)
{
	mg_metal_painter* painter = malloc_type(mg_metal_painter);
	painter->surface = surface;

	//NOTE(martin): setup interface functions
	painter->interface.drawBuffers = mg_metal_painter_draw_buffers;
	painter->interface.setViewPort = mg_metal_painter_viewport;
	painter->interface.destroy = mg_metal_painter_destroy;
	painter->interface.atlasUpload = mg_metal_painter_atlas_upload;

	painter->viewPort = viewPort;

	@autoreleasepool
	{
		f32 scale = surface->metalLayer.contentsScale;
		CGSize drawableSize = (CGSize){.width = viewPort.w * scale, .height = viewPort.h * scale};

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

		painter->outTexture = [surface->device newTextureWithDescriptor:texDesc];
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

		painter->atlasTexture = [surface->device newTextureWithDescriptor:texDesc];

		//-----------------------------------------------------------
		//NOTE(martin): create buffers for vertex and index
		//-----------------------------------------------------------

		MTLResourceOptions bufferOptions = MTLResourceCPUCacheModeWriteCombined
		                                 | MTLResourceStorageModeShared;

		painter->indexBuffer = [surface->device newBufferWithLength: MG_METAL_PAINTER_DEFAULT_BUFFER_LENGTH*sizeof(int)
		                                        options: bufferOptions];

		painter->vertexBuffer = [surface->device newBufferWithLength: MG_METAL_PAINTER_DEFAULT_BUFFER_LENGTH*sizeof(mg_vertex)
		                                        options: bufferOptions];

		painter->tilesArray = [surface->device newBufferWithLength: RENDERER_TILE_BUFFER_SIZE*sizeof(int)*RENDERER_MAX_TILES
							options: MTLResourceStorageModePrivate];

		painter->triangleArray = [surface->device newBufferWithLength: MG_METAL_PAINTER_DEFAULT_BUFFER_LENGTH*sizeof(mg_triangle_data)
							options: MTLResourceStorageModePrivate];

		painter->boxArray = [surface->device newBufferWithLength: MG_METAL_PAINTER_DEFAULT_BUFFER_LENGTH*sizeof(vector_float4)
							options: MTLResourceStorageModePrivate];

		//TODO(martin): retain ?
		//-----------------------------------------------------------
		//NOTE(martin): create and initialize tile counters
		//-----------------------------------------------------------
		painter->tileCounters = [surface->device newBufferWithLength: RENDERER_MAX_TILES*sizeof(uint)
		                                         options: MTLResourceStorageModePrivate];
		id<MTLCommandBuffer> commandBuffer = [surface->commandQueue commandBuffer];
		id<MTLBlitCommandEncoder> blitEncoder = [commandBuffer blitCommandEncoder];
		[blitEncoder fillBuffer: painter->tileCounters range: NSMakeRange(0, RENDERER_MAX_TILES*sizeof(uint)) value: 0];
		[blitEncoder endEncoding];
		[commandBuffer commit];

		//-----------------------------------------------------------
		//NOTE(martin): load the library
		//-----------------------------------------------------------

		//TODO(martin): filepath magic to find metallib path when not in the working directory
		char* shaderPath = 0;
		mp_app_get_resource_path("../resources/metal_shader.metallib", &shaderPath);
		NSString* metalFileName = [[NSString alloc] initWithCString: shaderPath encoding: NSUTF8StringEncoding];
		free(shaderPath);
		NSError* err = 0;
		id<MTLLibrary> library = [surface->device newLibraryWithFile: metalFileName error:&err];
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
		painter->computePipeline = [surface->device newComputePipelineStateWithFunction: computeFunction
		                                                                           error:&error];
		ASSERT(painter->computePipeline);

		MTLComputePipelineDescriptor* tilingPipelineDesc = [[MTLComputePipelineDescriptor alloc] init];
		tilingPipelineDesc.computeFunction = tilingFunction;
//		tilingPipelineDesc.threadGroupSizeIsMultipleOfThreadExecutionWidth = true;

		painter->tilingPipeline = [surface->device newComputePipelineStateWithDescriptor: tilingPipelineDesc
		                                           options: MTLPipelineOptionNone
		                                           reflection: nil
		                                           error: &error];

		MTLComputePipelineDescriptor* sortingPipelineDesc = [[MTLComputePipelineDescriptor alloc] init];
		sortingPipelineDesc.computeFunction = sortingFunction;
//		sortingPipelineDesc.threadGroupSizeIsMultipleOfThreadExecutionWidth = true;

		painter->sortingPipeline = [surface->device newComputePipelineStateWithDescriptor: sortingPipelineDesc
		                                           options: MTLPipelineOptionNone
		                                           reflection: nil
		                                           error: &error];

		MTLComputePipelineDescriptor* boxingPipelineDesc = [[MTLComputePipelineDescriptor alloc] init];
		boxingPipelineDesc.computeFunction = boxingFunction;
//		boxingPipelineDesc.threadGroupSizeIsMultipleOfThreadExecutionWidth = true;

		painter->boxingPipeline = [surface->device newComputePipelineStateWithDescriptor: boxingPipelineDesc
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
		pipelineStateDescriptor.colorAttachments[0].pixelFormat = surface->metalLayer.pixelFormat;

		// create render pipeline
		painter->renderPipeline = [surface->device newRenderPipelineStateWithDescriptor: pipelineStateDescriptor error:&err];
		if(err != nil)
		{
			const char* errStr = [[err localizedDescription] UTF8String];
			const char* descStr = [[err localizedFailureReason] UTF8String];
			const char* recovStr = [[err localizedRecoverySuggestion] UTF8String];
			LOG_ERROR("(%li) %s. %s. %s\n", [err code], errStr, descStr, recovStr);
			return(0);
		}
	}

	mg_metal_painter_update_vertex_layout(painter);

	return((mg_canvas_painter*)painter);
}


#undef LOG_SUBSYSTEM
