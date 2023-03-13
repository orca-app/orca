
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

kernel void TileKernel(const device float4* boxArray [[buffer(0)]],
                       device volatile atomic_uint* tileCounters [[buffer(1)]],
                       device uint* tilesArray [[buffer(2)]],
                       constant vector_uint2* viewport [[buffer(3)]],
                       uint gid [[thread_position_in_grid]])
{
	uint2 tilesMatrixDim = (*viewport - 1) / RENDERER_TILE_SIZE + 1;
	uint nTilesX = tilesMatrixDim.x;
	uint nTilesY = tilesMatrixDim.y;

	uint triangleIndex = gid;
	uint4 box = uint4(floor(boxArray[triangleIndex]))/RENDERER_TILE_SIZE;
	uint xMin = max((uint)0, box.x);
	uint yMin = max((uint)0, box.y);
	uint xMax = min(box.z, nTilesX-1);
	uint yMax = min(box.w, nTilesY-1);

	for(uint y = yMin; y <= yMax; y++)
	{
		for(uint x = xMin ; x <= xMax; x++)
		{
			uint tileIndex = y*nTilesX + x;
			device uint* tileBuffer = tilesArray + tileIndex*RENDERER_TILE_BUFFER_SIZE;
			uint counter = atomic_fetch_add_explicit(&(tileCounters[tileIndex]), 1, memory_order_relaxed);
			tileBuffer[counter] = triangleIndex;
		}
	}
}

kernel void SortKernel(const device uint* tileCounters [[buffer(0)]],
                       const device mg_triangle_data* triangleArray [[buffer(1)]],
                       device uint* tilesArray [[buffer(2)]],
                       constant vector_uint2* viewport [[buffer(3)]],
                       uint gid [[thread_position_in_grid]])
{
	uint tileIndex = gid;
	device uint* tileBuffer = tilesArray + tileIndex*RENDERER_TILE_BUFFER_SIZE;
	uint tileBufferSize = tileCounters[tileIndex];

	for(int eltIndex=0; eltIndex < (int)tileBufferSize; eltIndex++)
	{
		uint elt = tileBuffer[eltIndex];
		uint eltZIndex = triangleArray[elt].shapeIndex;

		int backIndex = eltIndex-1;
		for(; backIndex >= 0; backIndex--)
		{
			uint backElt = tileBuffer[backIndex];
			uint backEltZIndex = triangleArray[backElt].shapeIndex;
			if(eltZIndex >= backEltZIndex)
			{
				break;
			}
			else
			{
				tileBuffer[backIndex+1] = backElt;
			}
		}
		tileBuffer[backIndex+1] = elt;
	}
}

int orient2d(int2 a, int2 b, int2 c)
{
	return((b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x));
}

kernel void RenderKernel(texture2d<float, access::write> outTexture [[texture(0)]],
                         texture2d<float> texAtlas [[texture(1)]],
                         const device mg_vertex* vertexBuffer [[buffer(0)]],
                         const device mg_shape* shapeBuffer [[buffer(1)]],
                         device uint* tileCounters [[buffer(2)]],
                         const device uint* tilesArray [[buffer(3)]],
                         const device mg_triangle_data* triangleArray [[buffer(4)]],
                         const device float4* boxArray [[buffer(5)]],
                         constant vector_float4* clearColor [[buffer(6)]],
                         constant int* useTexture [[buffer(7)]],
                         constant float* contentsScaling [[buffer(8)]],
                         uint2 gid [[thread_position_in_grid]],
                         uint2 tgid [[threadgroup_position_in_grid]],
                         uint2 threadsPerThreadgroup [[threads_per_threadgroup]],
                         uint2 gridSize [[threads_per_grid]])
{
	//TODO: guard against thread group size not equal to tile size?
	const uint2 tilesMatrixDim = (gridSize - 1) / RENDERER_TILE_SIZE + 1;
	const uint2 tilePos = gid/RENDERER_TILE_SIZE;
	const uint tileIndex = tilePos.y * tilesMatrixDim.x + tilePos.x;
	const device uint* tileBuffer = tilesArray + tileIndex * RENDERER_TILE_BUFFER_SIZE;

	const uint tileBufferSize = tileCounters[tileIndex];

#ifdef RENDERER_DEBUG_TILES
	//NOTE(martin): color code debug values and show the tile grid
	uint nTileX = tilesMatrixDim.x;
	uint nTileY = tilesMatrixDim.y;

	if(tilePos.x > nTileX || tilePos.y > nTileY)
	{
		outTexture.write(float4(0, 1, 1, 1), gid);
		return;
	}

	if((gid.x % RENDERER_TILE_SIZE == 0) || (gid.y % RENDERER_TILE_SIZE == 0))
	{
		outTexture.write(float4(0, 0, 0, 1), gid);
		return;
	}
	if(tileBufferSize <= 0)
	{
		outTexture.write(float4(0, 1, 0, 1), gid);
		return;
	}
	else
	{
		outTexture.write(float4(1, 0, 0, 1), gid);
		return;
	}
#endif

	int subPixelFactor = 16;
	int2 pixelCoord = int2(gid);
	int2 centerPoint = int2((float2(pixelCoord) + float2(0.5, 0.5)) * subPixelFactor);

	const int sampleCount = 8;
	int2 samplePoints[sampleCount] = {centerPoint + int2(1, 3),
	                                  centerPoint + int2(-1, -3),
	                                  centerPoint + int2(5, -1),
	                                  centerPoint + int2(-3, 5),
	                                  centerPoint + int2(-5, -5),
	                                  centerPoint + int2(-7, 1),
	                                  centerPoint + int2(3, -7),
	                                  centerPoint + int2(7, 7)};
	int zIndices[sampleCount];
	uint flipCounts[sampleCount];
	float4 pixelColors[sampleCount];
	float4 nextColors[sampleCount];
	for(int i=0; i<sampleCount; i++)
	{
		zIndices[i] = -1;
		flipCounts[i] = 0;
		pixelColors[i] = float4(0, 0, 0, 0);
		nextColors[i] = float4(0, 0, 0, 0);
	}

	for(uint tileBufferIndex=0; tileBufferIndex < tileBufferSize; tileBufferIndex++)
	{
//		float4 box = boxArray[tileBuffer[tileBufferIndex]];
		const device mg_triangle_data* triangle = &triangleArray[tileBuffer[tileBufferIndex]];

		int2 p0 = int2(triangle->p0 * subPixelFactor);
		int2 p1 = int2(triangle->p1 * subPixelFactor);
		int2 p2 = int2(triangle->p2 * subPixelFactor);

		int bias0 = triangle->bias0;
		int bias1 = triangle->bias1;
		int bias2 = triangle->bias2;

		const device mg_vertex* v0 = &(vertexBuffer[triangle->i0]);
		const device mg_vertex* v1 = &(vertexBuffer[triangle->i1]);
		const device mg_vertex* v2 = &(vertexBuffer[triangle->i2]);

		float4 cubic0 = v0->cubic;
		float4 cubic1 = v1->cubic;
		float4 cubic2 = v2->cubic;

		int shapeIndex = v0->shapeIndex;
		float4 color = shapeBuffer[shapeIndex].color;
		color.rgb *= color.a;

		const device float* uvTransform2x3 = shapeBuffer[shapeIndex].uvTransform;
		matrix_float3x3 uvTransform = {{uvTransform2x3[0], uvTransform2x3[3], 0},
		                               {uvTransform2x3[1], uvTransform2x3[4], 0},
		                               {uvTransform2x3[2], uvTransform2x3[5], 1}};

		for(int i=0; i<sampleCount; i++)
		{
			int2 samplePoint = samplePoints[i];

			//NOTE(martin): cull if pixel is outside box
			/*
			// if we use this, make sure box is in fixed points coords
			if(samplePoint.x < box.x || samplePoint.x > box.z || samplePoint.y < box.y || samplePoint.y > box.w)
			{
				continue;
			}
			*/

			int w0 = orient2d(p1, p2, samplePoint);
			int w1 = orient2d(p2, p0, samplePoint);
			int w2 = orient2d(p0, p1, samplePoint);

			if((w0+bias0) >= 0 && (w1+bias1) >= 0 && (w2+bias2) >= 0)
			{
				float4 cubic = (cubic0*w0 + cubic1*w1 + cubic2*w2)/(w0+w1+w2);

				//float2 uv = (uv0*w0 + uv1*w1 + uv2*w2)/(w0+w1+w2);
				float2 sampleFP = float2(samplePoint)/subPixelFactor;
				float2 uv = (uvTransform*(float3(sampleFP/contentsScaling[0], 1))).xy;

				float4 texColor = float4(1, 1, 1, 1);
				if(*useTexture)
				{
					constexpr sampler smp(mip_filter::nearest, mag_filter::linear, min_filter::linear);
					texColor = texAtlas.sample(smp, uv);
					texColor.rgb *= texColor.a;
				}
				//TODO(martin): this is a quick and dirty fix for solid polygons where we use
				//              cubic = (1, 1, 1, 1) on all vertices, which can cause small errors to
				//              flip the sign.
				//              We should really use another value that always lead to <= 0, but we must
				//              make sure we never share these vertices with bezier shapes.
				//              Alternatively, an ugly (but maybe less than this one) solution would be
				//              to check if uvs are equal on all vertices of the triangle and always render
				//              those triangles.
				float eps = 0.0001;
				if(cubic.w*(cubic.x*cubic.x*cubic.x - cubic.y*cubic.z) <= eps)
				{
					if(shapeIndex == zIndices[i])
					{
						flipCounts[i]++;
					}
					else
					{
						if(flipCounts[i] & 0x01)
						{
							pixelColors[i] = nextColors[i];
						}

						float4 nextCol = color*texColor;
						nextColors[i] = pixelColors[i]*(1-nextCol.a) +nextCol.a*nextCol;

						zIndices[i] = shapeIndex;
						flipCounts[i] = 1;
					}
				}
			}
		}
	}
	float4 out = float4(0, 0, 0, 0);
	for(int i=0; i<sampleCount; i++)
	{
		if(flipCounts[i] & 0x01)
		{
			pixelColors[i] = nextColors[i];
		}
		out += pixelColors[i];
	}
	out = out/sampleCount;
	outTexture.write(out, gid);
}
