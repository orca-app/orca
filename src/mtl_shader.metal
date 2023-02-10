
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
	out.pos = float4(out.uv * float2(2, 2) + float2(-1, -1), 0, 1);
	return(out);
}

fragment float4 FragmentShader(vs_out i [[stage_in]], texture2d<float> tex [[texture(0)]])
{
	constexpr sampler smp(mip_filter::nearest, mag_filter::linear, min_filter::linear);
	return(float4(tex.sample(smp, i.uv).rgb, 1));
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
	int shapeIndex = vertexBuffer[i0].zIndex;

	vector_float4 clip = contentsScaling[0]*shapeBuffer[shapeIndex].clip;
	float2 clipMin(clip.x, clip.y);
	float2 clipMax(clip.x + clip.z-1, clip.y + clip.w-1);

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

	triangleArray[triangleIndex].zIndex = shapeIndex;
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
		uint eltZIndex = triangleArray[elt].zIndex;

		int backIndex = eltIndex-1;
		for(; backIndex >= 0; backIndex--)
		{
			uint backElt = tileBuffer[backIndex];
			uint backEltZIndex = triangleArray[backElt].zIndex;
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

float orient2d(float2 a, float2 b, float2 c)
{
	//////////////////////////////////////////////////////////////////////////////////////////
	//TODO(martin): FIX this. This is a **horrible** quick hack to fix the precision issues
	//              arising when a, b, and c are close. But it degrades when a, c, and c
	//              are big. The proper solution is to change the expression to avoid
	//              precision loss but I'm too busy/lazy to do it now.
	//////////////////////////////////////////////////////////////////////////////////////////
	a *= 10;
	b *= 10;
	c *= 10;
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
                         uint2 gid [[thread_position_in_grid]],
                         uint2 tgid [[threadgroup_position_in_grid]],
                         uint2 threadsPerThreadgroup [[threads_per_threadgroup]],
                         uint2 gridSize [[threads_per_grid]])
{
	//TODO: guard against thread group size not equal to tile size?
	const uint2 tilesMatrixDim = (gridSize - 1) / RENDERER_TILE_SIZE + 1;
//	const uint2 tilePos = tgid * threadsPerThreadgroup / RENDERER_TILE_SIZE;
	const uint2 tilePos = gid/RENDERER_TILE_SIZE;
	const uint tileIndex = tilePos.y * tilesMatrixDim.x + tilePos.x;
	const device uint* tileBuffer = tilesArray + tileIndex * RENDERER_TILE_BUFFER_SIZE;

	const uint tileBufferSize = tileCounters[tileIndex];


//#define RENDERER_DEBUG_TILES
#ifdef RENDERER_DEBUG_TILES
	//NOTE(martin): color code debug values and show the tile grid
	uint nTileX = tilesMatrixDim.x;
	uint nTileY = tilesMatrixDim.y;

	if(tilePos.x == 2 && tilePos.y == 12)
	{
		outTexture.write(float4(1, 0.5, 1, 1), gid);
		return;
	}

	if(nTileY != 13 || nTileX != 13)
	{
		outTexture.write(float4(1, 1, 0, 1), gid);
		return;
	}

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
	const float2 sampleOffsets[6] = { float2(5./12, 5./12),
	                                  float2(-3./12, 3./12),
	                                  float2(1./12, 1./12),
	                                  float2(3./12, -1./12),
	                                  float2(-5./12, -3./12),
	                                  float2(-1./12, -5./12)};

	int zIndices[6];
	uint flipCounts[6];
	float4 pixelColors[6];
	float4 nextColors[6];
	for(int i=0; i<6; i++)
	{
		zIndices[i] = -1;
		flipCounts[i] = 0;
		pixelColors[i] = *clearColor;
		nextColors[i] = *clearColor;
	}

	for(uint tileBufferIndex=0; tileBufferIndex < tileBufferSize; tileBufferIndex++)
	{
		float4 box = boxArray[tileBuffer[tileBufferIndex]];
		const device mg_triangle_data* triangle = &triangleArray[tileBuffer[tileBufferIndex]];

		float2 p0 = triangle->p0;
		float2 p1 = triangle->p1;
		float2 p2 = triangle->p2;

		int bias0 = triangle->bias0;
		int bias1 = triangle->bias1;
		int bias2 = triangle->bias2;

		const device mg_vertex* v0 = &(vertexBuffer[triangle->i0]);
		const device mg_vertex* v1 = &(vertexBuffer[triangle->i1]);
		const device mg_vertex* v2 = &(vertexBuffer[triangle->i2]);

		float4 cubic0 = v0->cubic;
		float4 cubic1 = v1->cubic;
		float4 cubic2 = v2->cubic;

		int zIndex = v0->zIndex;
		float4 color = shapeBuffer[zIndex].color;

		/////////////////////////////////////////////////////////////////////////
		//TODO: dummy uv while we figure out image handling.
		float2 uv0 = shapeBuffer[zIndex].uv;
		float2 uv1 = uv0;
		float2 uv2 = uv0;
		/////////////////////////////////////////////////////////////////////////


		for(int i=0; i<6; i++)
		{
			float2 samplePoint = (float2)gid + sampleOffsets[i];

			//NOTE(martin): cull if pixel is outside box
			if(samplePoint.x < box.x || samplePoint.x > box.z || samplePoint.y < box.y || samplePoint.y > box.w)
			{
				continue;
			}

			float w0 = orient2d(p1, p2, samplePoint);
			float w1 = orient2d(p2, p0, samplePoint);
			float w2 = orient2d(p0, p1, samplePoint);

			if(((int)w0+bias0) >= 0 && ((int)w1+bias1) >= 0 && ((int)w2+bias2) >= 0)
			{
				float4 cubic = (cubic0*w0 + cubic1*w1 + cubic2*w2)/(w0+w1+w2);
				float2 uv = (uv0*w0 + uv1*w1 + uv2*w2)/(w0+w1+w2);

				constexpr sampler smp(mip_filter::nearest, mag_filter::linear, min_filter::linear);
				float4 texColor = texAtlas.sample(smp, uv);

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
					if(zIndex == zIndices[i])
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

						zIndices[i] = zIndex;
						flipCounts[i] = 1;
					}
				}
			}
		}
	}
	float4 out = float4(0, 0, 0, 0);
	for(int i=0; i<6; i++)
	{
		if(flipCounts[i] & 0x01)
		{
			pixelColors[i] = nextColors[i];
		}
		out += pixelColors[i];
	}
	out = float4(out.xyz/6, 1);
	outTexture.write(out, gid);
}
