
#include<metal_stdlib>
#include<simd/simd.h>

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
/*
kernel void BoundingBoxKernel(constant mg_vertex* vertexBuffer [[buffer(0)]],
		              constant uint* indexBuffer [[buffer(1)]],
		              constant mg_shape* shapeBuffer [[buffer(2)]],
		              device mg_triangle_data* triangleArray [[buffer(3)]],
		              device float4* boxArray [[buffer(4)]],
		              constant float* contentsScaling [[buffer(5)]],
		              uint gid [[thread_position_in_grid]])
{
	uint triangleIndex = gid;
	uint vertexIndex = triangleIndex*3;

	uint i0 = indexBuffer[vertexIndex];
	uint i1 = indexBuffer[vertexIndex+1];
	uint i2 = indexBuffer[vertexIndex+2];

	float2 p0 = vertexBuffer[i0].pos * contentsScaling[0];
	float2 p1 = vertexBuffer[i1].pos * contentsScaling[0];
	float2 p2 = vertexBuffer[i2].pos * contentsScaling[0];

	//NOTE(martin): compute triangle bounding box
	float2 boxMin = min(min(p0, p1), p2);
	float2 boxMax = max(max(p0, p1), p2);

	//NOTE(martin): clip bounding box against clip rect
	int shapeIndex = vertexBuffer[i0].shapeIndex;

	vector_float4 clip = contentsScaling[0]*shapeBuffer[shapeIndex].clip;

	//NOTE(martin): intersect with current clip
	boxMin = max(boxMin, clip.xy);
	boxMax = min(boxMax, clip.zw);

	//NOTE(martin): reorder triangle counter-clockwise and compute bias for each edge
	float cw = (p1 - p0).x*(p2 - p0).y - (p1 - p0).y*(p2 - p0).x;
	if(cw < 0)
	{
		uint tmpIndex = i1;
		i1 = i2;
		i2 = tmpIndex;

		float2 tmpPoint = p1;
		p1 = p2;
		p2 = tmpPoint;
	}
	int bias0 = is_top_left(p1, p2) ? 0 : -1;
	int bias1 = is_top_left(p2, p0) ? 0 : -1;
	int bias2 = is_top_left(p0, p1) ? 0 : -1;

	//NOTE(martin): fill triangle data
	boxArray[triangleIndex] = float4(boxMin.x, boxMin.y, boxMax.x, boxMax.y);

	triangleArray[triangleIndex].shapeIndex = shapeIndex;
	triangleArray[triangleIndex].i0 = i0;
	triangleArray[triangleIndex].i1 = i1;
	triangleArray[triangleIndex].i2 = i2;
	triangleArray[triangleIndex].p0 = p0;
	triangleArray[triangleIndex].p1 = p1;
	triangleArray[triangleIndex].p2 = p2;
	triangleArray[triangleIndex].bias0 = bias0;
	triangleArray[triangleIndex].bias1 = bias1;
	triangleArray[triangleIndex].bias2 = bias2;
}
*/

kernel void TileKernel(constant mg_vertex* vertexBuffer [[buffer(0)]],
		               constant uint* indexBuffer [[buffer(1)]],
		               constant mg_shape* shapeBuffer [[buffer(2)]],
		               device volatile atomic_uint* tileCounters [[buffer(3)]],
                       device uint* tileArrayBuffer [[buffer(4)]],
                       constant uint2* viewport [[buffer(5)]],
                       constant float* scaling [[buffer(6)]],
                       uint gid [[thread_position_in_grid]])
{
	uint2 tilesMatrixDim = (*viewport - 1) / RENDERER_TILE_SIZE + 1;
	int nTilesX = tilesMatrixDim.x;
	int nTilesY = tilesMatrixDim.y;

	uint triangleIndex = gid * 3;

	uint i0 = indexBuffer[triangleIndex];
	uint i1 = indexBuffer[triangleIndex+1u];
	uint i2 = indexBuffer[triangleIndex+2u];

	float2 p0 = vertexBuffer[i0].pos * scaling[0];
	float2 p1 = vertexBuffer[i1].pos * scaling[0];
	float2 p2 = vertexBuffer[i2].pos * scaling[0];

	int shapeIndex = vertexBuffer[i0].shapeIndex;
	float4 clip = shapeBuffer[shapeIndex].clip * scaling[0];

	float4 fbox = float4(max(min(min(p0.x, p1.x), p2.x), clip.x),
		             max(min(min(p0.y, p1.y), p2.y), clip.y),
		             min(max(max(p0.x, p1.x), p2.x), clip.z),
		             min(max(max(p0.y, p1.y), p2.y), clip.w));

	int4 box = int4(floor(fbox))/int(RENDERER_TILE_SIZE);

	//NOTE(martin): it's importat to do the computation with signed int, so that we can have negative xMax/yMax
	//              otherwise all triangles on the left or below the x/y axis are attributed to tiles on row/column 0.
	int xMin = max(0, box.x);
	int yMin = max(0, box.y);
	int xMax = min(box.z, nTilesX-1);
	int yMax = min(box.w, nTilesY-1);

	for(int y = yMin; y <= yMax; y++)
	{
		for(int x = xMin ; x <= xMax; x++)
		{
			int tileIndex = y*nTilesX + x;
			uint counter = atomic_fetch_add_explicit(&(tileCounters[tileIndex]), 1, memory_order_relaxed);
			if(counter < RENDERER_TILE_BUFFER_SIZE)
			{
				tileArrayBuffer[tileIndex*RENDERER_TILE_BUFFER_SIZE + counter] = triangleIndex;
			}
		}
	}
}

kernel void SortKernel(constant mg_vertex* vertexBuffer [[buffer(0)]],
                       constant uint* indexBuffer [[buffer(1)]],
                       constant mg_shape* shapeBuffer [[buffer(2)]],
                       const device uint* tileCounters [[buffer(3)]],
                       device uint* tileArrayBuffer [[buffer(4)]],
                       uint gid [[thread_position_in_grid]])
{
	uint tileIndex = gid;
	uint tileArrayOffset = tileIndex * RENDERER_TILE_BUFFER_SIZE;
	uint tileArrayCount = min(tileCounters[tileIndex], (uint)RENDERER_TILE_BUFFER_SIZE);

	for(uint tileArrayIndex=1; tileArrayIndex < tileArrayCount; tileArrayIndex++)
	{
		for(uint sortIndex = tileArrayIndex; sortIndex > 0u; sortIndex--)
		{
			uint triangleIndex = indexBuffer[tileArrayBuffer[tileArrayOffset + sortIndex]];
			uint prevTriangleIndex = indexBuffer[tileArrayBuffer[tileArrayOffset + sortIndex - 1]];

			int shapeIndex = vertexBuffer[triangleIndex].shapeIndex;
			int prevShapeIndex = vertexBuffer[prevTriangleIndex].shapeIndex;

			if(shapeIndex >= prevShapeIndex)
			{
				break;
			}
			uint tmp = tileArrayBuffer[tileArrayOffset + sortIndex];
			tileArrayBuffer[tileArrayOffset + sortIndex] = tileArrayBuffer[tileArrayOffset + sortIndex - 1];
			tileArrayBuffer[tileArrayOffset + sortIndex - 1] = tmp;
		}
	}
}


bool is_top_left(int2 a, int2 b)
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

//TODO: coalesce
int orient2d(int2 a, int2 b, int2 c)
{
	return((b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x));
}

int is_clockwise(int2 p0, int2 p1, int2 p2)
{
	return((p1 - p0).x*(p2 - p0).y - (p1 - p0).y*(p2 - p0).x);
}


kernel void RenderKernel(const device mg_vertex* vertexBuffer [[buffer(0)]],
                         const device uint* indexBuffer [[buffer(1)]],
                         const device mg_shape* shapeBuffer [[buffer(2)]],
                         const device uint* tileCounters [[buffer(3)]],
                         const device uint* tileArrayBuffer [[buffer(4)]],

                         constant float4* clearColor [[buffer(5)]],
                         constant int* useTexture [[buffer(6)]],
                         constant float* scaling [[buffer(7)]],

                         texture2d<float, access::write> outTexture [[texture(0)]],
                         texture2d<float> texAtlas [[texture(1)]],

                         uint2 gid [[thread_position_in_grid]],
                         uint2 tgid [[threadgroup_position_in_grid]],
                         uint2 threadsPerThreadgroup [[threads_per_threadgroup]],
                         uint2 gridSize [[threads_per_grid]])
{
	//TODO: guard against thread group size not equal to tile size?
	const int2 pixelCoord = int2(gid);
	const uint2 tileCoord = uint2(pixelCoord)/ RENDERER_TILE_SIZE;
	const uint2 tilesMatrixDim = (gridSize - 1) / RENDERER_TILE_SIZE + 1;
	const uint tileIndex = tileCoord.y * tilesMatrixDim.x + tileCoord.x;
	const uint tileCounter = min(tileCounters[tileIndex], (uint)RENDERER_TILE_BUFFER_SIZE);

#ifdef RENDERER_DEBUG_TILES
	//NOTE(martin): color code debug values and show the tile grid
	{
		float4 fragColor = float4(0);

		if( pixelCoord.x % 16 == 0
	  	  ||pixelCoord.y % 16 == 0)
		{
			fragColor = float4(0, 0, 0, 1);
		}
		else if(tileCounters[tileIndex] == 0xffffu)
		{
			fragColor = float4(1, 0, 1, 1);
		}
		else if(tileCounter != 0u)
		{
			fragColor = float4(0, 1, 0, 1);
		}
		else
		{
			fragColor = float4(1, 0, 0, 1);
		}
		outTexture.write(fragColor, gid);
		return;
	}
#endif

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

    for(uint tileArrayIndex=0; tileArrayIndex < tileCounter; tileArrayIndex++)
    {
    	int triangleIndex = tileArrayBuffer[RENDERER_TILE_BUFFER_SIZE * tileIndex + tileArrayIndex];

		uint i0 = indexBuffer[triangleIndex];
		uint i1 = indexBuffer[triangleIndex+1];
		uint i2 = indexBuffer[triangleIndex+2];

		int2 p0 = int2((vertexBuffer[i0].pos * scaling[0]) * subPixelFactor);
		int2 p1 = int2((vertexBuffer[i1].pos * scaling[0]) * subPixelFactor);
		int2 p2 = int2((vertexBuffer[i2].pos * scaling[0]) * subPixelFactor);

		int shapeIndex = vertexBuffer[i0].shapeIndex;
		float4 color = shapeBuffer[shapeIndex].color;
		color.rgb *= color.a;

		int4 clip = int4(round((shapeBuffer[shapeIndex].clip * scaling[0] + float4(0.5, 0.5, 0.5, 0.5)) * subPixelFactor));

		const device float* uvTransform2x3 = shapeBuffer[shapeIndex].uvTransform;
		matrix_float3x3 uvTransform = {{uvTransform2x3[0], uvTransform2x3[3], 0},
		                               {uvTransform2x3[1], uvTransform2x3[4], 0},
		                               {uvTransform2x3[2], uvTransform2x3[5], 1}};

		//NOTE(martin): reorder triangle counter-clockwise and compute bias for each edge
		int cw = is_clockwise(p0, p1, p2);
		if(cw < 0)
		{
			uint tmpIndex = i1;
			i1 = i2;
			i2 = tmpIndex;

			int2 tmpPoint = p1;
			p1 = p2;
			p2 = tmpPoint;
		}

		float4 cubic0 = vertexBuffer[i0].cubic;
		float4 cubic1 = vertexBuffer[i1].cubic;
		float4 cubic2 = vertexBuffer[i2].cubic;

		int bias0 = is_top_left(p1, p2) ? 0 : -1;
		int bias1 = is_top_left(p2, p0) ? 0 : -1;
		int bias2 = is_top_left(p0, p1) ? 0 : -1;

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

			int w0 = orient2d(p1, p2, samplePoint);
			int w1 = orient2d(p2, p0, samplePoint);
			int w2 = orient2d(p0, p1, samplePoint);

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
