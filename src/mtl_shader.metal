
#include<metal_stdlib>
#include<simd/simd.h>
#include<metal_simdgroup>

#include"mtl_shader.h"

using namespace metal;

struct vs_out
{
    float4 pos [[position]];
    float2 uv;
};

vertex vs_out VertexShader(ushort vid [[vertex_id]])
{
	vs_out out;
	out.uv = float2((vid << 1) & 2, vid & 2);
	out.pos = float4(out.uv * float2(2, -2) + float2(-1, 1), 0, 1);
	return(out);
}

fragment float4 FragmentShader(vs_out i [[stage_in]], texture2d<float> tex [[texture(0)]])
{
	constexpr sampler smp(mip_filter::nearest, mag_filter::linear, min_filter::linear);
	return(tex.sample(smp, i.uv));
}

bool is_top_left(float2 a, float2 b)
{
	return( (a.y == b.y && b.x < a.x)
	      ||(b.y < a.y));
}

//////////////////////////////////////////////////////////////////////////////
//TODO: we should do these computations on 64bits, because otherwise
//      we might overflow for values > 2048.
//		Unfortunately this is costly.
//	    Another way is to precompute triangle edges (b - a) in full precision
//      once to avoid doing it all the time...
//////////////////////////////////////////////////////////////////////////////

int orient2d(int2 a, int2 b, int2 c)
{
	return((b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x));
}

device uchar* arena_allocate(device uchar* arenaBuffer,
                             device volatile atomic_uint* arenaOffset,
                             uint size)
{
	uint index = atomic_fetch_add_explicit(arenaOffset, size, memory_order_relaxed);
	return(&arenaBuffer[index]);
}

//NOTE: shape setup allocates tile queues for each shape

kernel void ShapeSetup(constant mg_shape* shapeBuffer [[buffer(0)]],
                       device mg_shape_queue* shapeQueueBuffer [[buffer(1)]],
                       device uchar* arenaBuffer [[buffer(2)]],
                       device volatile atomic_uint* arenaOffset [[buffer(3)]],
                       constant float* scaling [[buffer(4)]],
                       uint gid [[thread_position_in_grid]])
{

	float4 box = shapeBuffer[gid].clip * scaling[0];

	int2 firstTile = int2(box.xy)/RENDERER_TILE_SIZE;

	//WARN: the following can result in a 1x1 tile allocated even for empty boxes. But if we didn't allocate
	//      any tile queue, the tileQueues pointer for that shape would alias the tileQueues pointer of another
	//      shape, and we would have to detect that in the tiling and drawing kernels. Instead, just accept some
	//      waste and keep the other kernels more uniforms for now...
	int nTilesX = int(box.z)/RENDERER_TILE_SIZE - firstTile.x + 1;
	int nTilesY = int(box.w)/RENDERER_TILE_SIZE - firstTile.y + 1;

	int tileCount = nTilesX * nTilesY;
	int tileArraySize =  tileCount * sizeof(mg_tile_queue);

	shapeQueueBuffer[gid].area = int4(firstTile.x, firstTile.y, nTilesX, nTilesY);
	shapeQueueBuffer[gid].tileQueues = (device mg_tile_queue*)arena_allocate(arenaBuffer, arenaOffset, tileArraySize);

	for(int i=0; i<tileCount; i++)
	{
		atomic_store_explicit(&shapeQueueBuffer[gid].tileQueues[i].first, -1, memory_order_relaxed);
	}
}

//NOTE: setup triangle data and bucket triangle into tile queues

kernel void TriangleKernel(constant mg_vertex* vertexBuffer [[buffer(0)]],
		                   constant uint* indexBuffer [[buffer(1)]],
		                   constant mg_shape* shapeBuffer [[buffer(2)]],
                           device mg_triangle_data* triangleArray [[buffer(3)]],
                           device mg_shape_queue* shapeQueueBuffer [[buffer(4)]],
                           device uchar* arenaBuffer [[buffer(5)]],
                           device volatile atomic_uint* arenaOffset [[buffer(6)]],
                           constant float* scaling [[buffer(7)]],
                           uint gid [[thread_position_in_grid]])
{
	//NOTE: triangle setup
	uint triangleIndex = gid * 3;

	uint i0 = indexBuffer[triangleIndex];
	uint i1 = indexBuffer[triangleIndex+1];
	uint i2 = indexBuffer[triangleIndex+2];

	float2 p0 = vertexBuffer[i0].pos * scaling[0];
	float2 p1 = vertexBuffer[i1].pos * scaling[0];
	float2 p2 = vertexBuffer[i2].pos * scaling[0];

	int shapeIndex = vertexBuffer[i0].shapeIndex;

	//NOTE(martin): compute triangle bounding box and clip it
	float4 clip = shapeBuffer[shapeIndex].clip * scaling[0];
	float4 fbox = float4(min(min(p0, p1), p2), max(max(p0, p1), p2));
	fbox = float4(max(fbox.xy, clip.xy), min(fbox.zw, clip.zw));

	//NOTE(martin): fill triangle data
	const float subPixelFactor = 16;

	triangleArray[gid].box = int4(fbox * subPixelFactor);
	triangleArray[gid].shapeIndex = shapeIndex;

	triangleArray[gid].color = shapeBuffer[shapeIndex].color;

	constant float* uvTransform2x3 = shapeBuffer[shapeIndex].uvTransform;
	triangleArray[gid].uvTransform = (matrix_float3x3){{uvTransform2x3[0], uvTransform2x3[3], 0},
		                                               {uvTransform2x3[1], uvTransform2x3[4], 0},
		                                               {uvTransform2x3[2], uvTransform2x3[5], 1}};

	triangleArray[gid].cubic0 = vertexBuffer[i0].cubic;
	triangleArray[gid].cubic1 = vertexBuffer[i1].cubic;
	triangleArray[gid].cubic2 = vertexBuffer[i2].cubic;

	int2 ip0 = int2(p0 * subPixelFactor);
	int2 ip1 = int2(p1 * subPixelFactor);
	int2 ip2 = int2(p2 * subPixelFactor);

	triangleArray[gid].p0 = ip0;
	triangleArray[gid].p1 = ip1;
	triangleArray[gid].p2 = ip2;

	int cw = orient2d(ip0, ip1, ip2) > 0 ? 1 : -1;

	triangleArray[gid].cw = cw;
	triangleArray[gid].bias0 = is_top_left(p1, p2) ? -(1-cw)/2 : -(1+cw)/2;
	triangleArray[gid].bias1 = is_top_left(p2, p0) ? -(1-cw)/2 : -(1+cw)/2;
	triangleArray[gid].bias2 = is_top_left(p0, p1) ? -(1-cw)/2 : -(1+cw)/2;

	int4 tileBox = int4(fbox)/RENDERER_TILE_SIZE;
	triangleArray[gid].tileBox = tileBox;


	//NOTE: bucket triangle into tiles
	device mg_shape_queue* shapeQueue = &shapeQueueBuffer[shapeIndex];

	int xMin = max(0, tileBox.x - shapeQueue->area.x);
	int yMin = max(0, tileBox.y - shapeQueue->area.y);
	int xMax = min(tileBox.z - shapeQueue->area.x, shapeQueue->area.z-1);
	int yMax = min(tileBox.w - shapeQueue->area.y, shapeQueue->area.w-1);

	//NOTE(martin): it's important to do the computation with signed int, so that we can have negative xMax/yMax
	//              otherwise all triangles on the left or below the x/y axis are attributed to tiles on row/column 0.

	for(int y = yMin; y <= yMax; y++)
	{
		for(int x = xMin ; x <= xMax; x++)
		{
			int tileIndex = y*shapeQueue->area.z + x;

			device mg_tile_queue* tileQueue = &shapeQueue->tileQueues[tileIndex];
			device mg_queue_elt* elt = (device mg_queue_elt*)arena_allocate(arenaBuffer, arenaOffset, sizeof(mg_queue_elt));
			int eltIndex = (device uchar*)elt - arenaBuffer;

			elt->next = atomic_exchange_explicit(&tileQueue->first, eltIndex, memory_order_relaxed);

			elt->triangleIndex = gid;
		}
	}
}

kernel void RenderKernel(const device mg_shape_queue* shapeQueueBuffer [[buffer(0)]],
                         const device mg_triangle_data* triangleArray [[buffer(1)]],
                         const device uchar* arenaBuffer [[buffer(2)]],

                         constant int* shapeCount [[buffer(3)]],
                         constant int* useTexture [[buffer(4)]],
                         constant float* scaling [[buffer(5)]],

                         texture2d<float, access::write> outTexture [[texture(0)]],
                         texture2d<float> texAtlas [[texture(1)]],

                         uint2 gid [[thread_position_in_grid]],
                         uint2 tgid [[threadgroup_position_in_grid]],
                         uint2 threadsPerThreadgroup [[threads_per_threadgroup]],
                         uint2 gridSize [[threads_per_grid]])
{
	//TODO: guard against thread group size not equal to tile size?
	const int2 pixelCoord = int2(gid);
	const int2 tileCoord = pixelCoord/ RENDERER_TILE_SIZE;

	const int subPixelFactor = 16;
	const int2 centerPoint = int2((float2(pixelCoord) + float2(0.5, 0.5)) * subPixelFactor);

	const int sampleCount = 8;
	int2 samplePoints[sampleCount] = {centerPoint + int2(1, 3),
	                                  centerPoint + int2(-1, -3),
	                                  centerPoint + int2(5, -1),
	                                  centerPoint + int2(-3, 5),
	                                  centerPoint + int2(-5, -5),
	                                  centerPoint + int2(-7, 1),
	                                  centerPoint + int2(3, -7),
	                                  centerPoint + int2(7, 7)};

	float4 sampleColor[sampleCount];
	float4 currentColor[sampleCount];
    int currentShapeIndex[sampleCount];
    int flipCount[sampleCount];

    for(int i=0; i<sampleCount; i++)
    {
		currentShapeIndex[i] = -1;
		flipCount[i] = 0;
		sampleColor[i] = float4(0, 0, 0, 0);
		currentColor[i] = float4(0, 0, 0, 0);
    }

    for(int shapeIndex = 0; shapeIndex < shapeCount[0]; shapeIndex++)
    {
		const device mg_shape_queue* shapeQueue = &shapeQueueBuffer[shapeIndex];

		// get the tile queue that corresponds to our tile in the shape area
		int2 tileQueueCoord = tileCoord - shapeQueue->area.xy;
		if(  tileQueueCoord.x >= 0
		  && tileQueueCoord.y >= 0
		  && tileQueueCoord.x < shapeQueue->area.z
		  && tileQueueCoord.y < shapeQueue->area.w)
		{
			int tileQueueIndex = tileQueueCoord.y * shapeQueue->area.z + tileQueueCoord.x;
			device mg_tile_queue* tileQueue = &shapeQueue->tileQueues[tileQueueIndex];

			int firstEltIndex = atomic_load_explicit(&tileQueue->first, memory_order_relaxed);
			device mg_queue_elt* elt = 0;

			for(int eltIndex = firstEltIndex; eltIndex >= 0; eltIndex = elt->next)
			{
				elt = (device mg_queue_elt*)(arenaBuffer + eltIndex);
				const device mg_triangle_data* triangle = &triangleArray[elt->triangleIndex];

				int2 p0 = triangle->p0;
				int2 p1 = triangle->p1;
				int2 p2 = triangle->p2;

				int cw = triangle->cw;

				int bias0 = triangle->bias0;
				int bias1 = triangle->bias1;
				int bias2 = triangle->bias2;

				float4 cubic0 = triangle->cubic0;
				float4 cubic1 = triangle->cubic1;
				float4 cubic2 = triangle->cubic2;

				int shapeIndex = triangle->shapeIndex;
				float4 color = triangle->color;
				color.rgb *= color.a;

				int4 clip = triangle->box;

				matrix_float3x3 uvTransform = triangle->uvTransform;

				for(int sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
				{
					int2 samplePoint = samplePoints[sampleIndex];

					if(  samplePoint.x < clip.x
			  		|| samplePoint.x > clip.z
			  		|| samplePoint.y < clip.y
			  		|| samplePoint.y > clip.w)
					{
						continue;
					}

					int w0 = cw*orient2d(p1, p2, samplePoint);
					int w1 = cw*orient2d(p2, p0, samplePoint);
					int w2 = cw*orient2d(p0, p1, samplePoint);

					if((w0+bias0) >= 0 && (w1+bias1) >= 0 && (w2+bias2) >= 0)
					{
						float4 cubic = (cubic0*w0 + cubic1*w1 + cubic2*w2)/(w0+w1+w2);

						float eps = 0.0001;
						if(cubic.w*(cubic.x*cubic.x*cubic.x - cubic.y*cubic.z) <= eps)
						{
							if(shapeIndex == currentShapeIndex[sampleIndex])
							{
								flipCount[sampleIndex]++;
							}
							else
							{
								if(flipCount[sampleIndex] & 0x01)
								{
									sampleColor[sampleIndex] = currentColor[sampleIndex];
								}

								float4 nextColor = color;

								if(useTexture[0])
								{
									float3 sampleFP = float3(float2(samplePoint).xy/(subPixelFactor*2.), 1);
									float2 uv = (uvTransform * sampleFP).xy;

									constexpr sampler smp(mip_filter::nearest, mag_filter::linear, min_filter::linear);
									float4 texColor = texAtlas.sample(smp, uv);

									texColor.rgb *= texColor.a;
									nextColor *= texColor;
								}

								currentColor[sampleIndex] = sampleColor[sampleIndex]*(1.-nextColor.a) + nextColor;
								currentShapeIndex[sampleIndex] = shapeIndex;
								flipCount[sampleIndex] = 1;
							}
						}
					}
				}
			}
		}
	}

    float4 pixelColor = float4(0);
    for(int sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
    {
    	if(flipCount[sampleIndex] & 0x01)
    	{
			sampleColor[sampleIndex] = currentColor[sampleIndex];
    	}
    	pixelColor += sampleColor[sampleIndex];
	}

	outTexture.write(pixelColor/float(sampleCount), gid);

}
