
#include<metal_stdlib>
#include<simd/simd.h>
#include<metal_simdgroup>

#include"mtl_renderer.h"

using namespace metal;

kernel void mtl_path_setup(constant int* pathCount [[buffer(0)]],
                           const device mg_mtl_path* pathBuffer [[buffer(1)]],
                           device mg_mtl_path_queue* pathQueueBuffer [[buffer(2)]],
                           device mg_mtl_tile_queue* tileQueueBuffer [[buffer(3)]],
                           device atomic_int* tileQueueCount [[buffer(4)]],
                           constant int* tileSize [[buffer(5)]],
                           uint pathIndex [[thread_position_in_grid]])
{
	const device mg_mtl_path* path = &pathBuffer[pathIndex];

	int2 firstTile = int2(path->box.xy)/tileSize[0];
	int2 lastTile = max(firstTile, int2(path->box.zw)/tileSize[0]);
	int nTilesX = lastTile.x - firstTile.x + 1;
	int nTilesY = lastTile.y - firstTile.y + 1;
	int tileCount = nTilesX * nTilesY;

	int tileQueuesIndex = atomic_fetch_add_explicit(tileQueueCount, tileCount, memory_order_relaxed);

	pathQueueBuffer[pathIndex].area = int4(firstTile.x, firstTile.y, nTilesX, nTilesY);
	pathQueueBuffer[pathIndex].tileQueues = tileQueuesIndex;

	device mg_mtl_tile_queue* tileQueues = &tileQueueBuffer[tileQueuesIndex];

	for(int i=0; i<tileCount; i++)
	{
		atomic_store_explicit(&tileQueues[i].first, -1, memory_order_relaxed);
	}
}

bool mtl_is_left_of_segment(float2 p, const device mg_mtl_segment* seg)
{
	//NOTE: test is p is on the left of a curve segment.

	/*WARN: if p is outside the bounding box of segment, we still consider it left from
	        the segment if it is left of its diagonal. This is done so that we can test
	        if tile corners are on the same side of the curve during tiling (corner are
	        not necessarily inside the bounding box, even if the tile itself overlaps
	        the curve).
	        During fine rasterization, this function need to be guarded by a the following
	        check: if(p.y >= seg->box.y && p.y < seg->box.w) {...}
	*/
	bool isLeft = false;

	//NOTE: if point is left of curve bounding box, it is left of curve
	if(p.x < seg->box.x)
	{
		isLeft = true;
	}
	else if(p.x < seg->box.z)
	{
		/*NOTE: if point and curve are on opposite sides of diagonal and on the left of diagonal,
		        it is left from the curve
				otherwise if point and curve are on the same side of diagonal, do implicit test
		*/
		float alpha = (seg->box.w - seg->box.y)/(seg->box.z - seg->box.x);
		float ofs = seg->box.w - seg->box.y;
		float dx = p.x - seg->box.x;
		float dy = p.y - seg->box.y;

		if( (seg->config == MG_MTL_BR && dy > alpha*dx)
				||(seg->config == MG_MTL_TR && dy < ofs - alpha*dx))
		{
			isLeft = true;
		}
		else if(  !(seg->config == MG_MTL_TL && dy < alpha*dx)
				     && !(seg->config == MG_MTL_BL && dy > ofs - alpha*dx))
		{
			//Need implicit test, but for lines, we only have config BR or TR, so the test is always negative for now
		}
	}
	return(isLeft);
}

kernel void mtl_segment_setup(constant int* elementCount [[buffer(0)]],
                              const device mg_mtl_path_elt* elementBuffer [[buffer(1)]],
                              device atomic_int* segmentCount [[buffer(2)]],
                              device mg_mtl_segment* segmentBuffer [[buffer(3)]],
                              const device mg_mtl_path_queue* pathQueueBuffer [[buffer(4)]],
                              device mg_mtl_tile_queue* tileQueueBuffer [[buffer(5)]],
                              device mg_mtl_tile_op* tileOpBuffer [[buffer(6)]],
                              device atomic_int* tileOpCount [[buffer(7)]],
                              constant int* tileSize [[buffer(8)]],
                              uint eltIndex [[thread_position_in_grid]])
{
	const device mg_mtl_path_elt* elt = &elementBuffer[eltIndex];
	float2 p0 = elt->p[0];
	float2 p3 = elt->p[3];

	if(elt->kind == MG_MTL_LINE && p0.y != p3.y)
	{
		int segIndex = atomic_fetch_add_explicit(segmentCount, 1, memory_order_relaxed);
		device mg_mtl_segment* seg = &segmentBuffer[segIndex];

		seg->pathIndex = elt->pathIndex;
		seg->box = (vector_float4){min(p0.x, p3.x),
		                           min(p0.y, p3.y),
		                           max(p0.x, p3.x),
		                           max(p0.y, p3.y)};

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

		//NOTE: add segment index to the queues of tiles it overlaps with
		const device mg_mtl_path_queue* pathQueue = &pathQueueBuffer[seg->pathIndex];
		device mg_mtl_tile_queue* tileQueues = &tileQueueBuffer[pathQueue->tileQueues];

		int4 coveredTiles = int4(seg->box)/tileSize[0];
		int xMin = max(0, coveredTiles.x - pathQueue->area.x);
		int yMin = max(0, coveredTiles.y - pathQueue->area.y);
		int xMax = min(coveredTiles.z - pathQueue->area.x, pathQueue->area.z-1);
		int yMax = min(coveredTiles.w - pathQueue->area.y, pathQueue->area.w-1);

		for(int y = yMin; y <= yMax; y++)
		{
			for(int x = xMin ; x <= xMax; x++)
			{
				float4 tileBox = (float4){float(x + pathQueue->area.x),
				                          float(y + pathQueue->area.y),
				                          float(x + pathQueue->area.x + 1),
				                          float(y + pathQueue->area.y + 1)} * float(tileSize[0]);

				//NOTE: select two corners of tile box to test against the curve
				float2 testPoint[2] = {{tileBox.x, tileBox.y},
				                       {tileBox.z, tileBox.w}};
				if(seg->config == MG_MTL_BR || seg->config == MG_MTL_TL)
				{
					testPoint[0] = (float2){tileBox.x, tileBox.w};
					testPoint[1] = (float2){tileBox.z, tileBox.y};
				}
				bool test0 = mtl_is_left_of_segment(testPoint[0], seg);
				bool test1 = mtl_is_left_of_segment(testPoint[1], seg);

				//NOTE: the curve overlaps the tile only if test points are on opposite sides of segment
				if(test0 != test1)
				{
					int tileOpIndex = atomic_fetch_add_explicit(tileOpCount, 1, memory_order_relaxed);
					device mg_mtl_tile_op* op = &tileOpBuffer[tileOpIndex];

					op->kind = MG_MTL_OP_SEGMENT;
					op->index = segIndex;

					int tileIndex = y*pathQueue->area.z + x;
					op->next = atomic_exchange_explicit(&tileQueues[tileIndex].first, tileOpIndex, memory_order_relaxed);
				}
			}
		}
	}
}

kernel void mtl_raster(constant int* pathCount [[buffer(0)]],
                       const device mg_mtl_path* pathBuffer [[buffer(1)]],
                       constant int* segCount [[buffer(2)]],
                       const device mg_mtl_segment* segmentBuffer [[buffer(3)]],
                       const device mg_mtl_path_queue* pathQueueBuffer [[buffer(4)]],
                       const device mg_mtl_tile_queue* tileQueueBuffer [[buffer(5)]],
                       const device mg_mtl_tile_op* tileOpBuffer [[buffer(6)]],
                       constant int* tileSize [[buffer(7)]],
                       texture2d<float, access::write> outTexture [[texture(0)]],
                       uint2 threadCoord [[thread_position_in_grid]],
                       uint2 gridSize [[threads_per_grid]])
{
	int2 pixelCoord = int2(threadCoord);
	int2 tileCoord = pixelCoord / tileSize[0];

	float4 color = float4(0, 0, 0, 0);
	int currentPath = 0;
	int winding = 0;

	if( (pixelCoord.x % tileSize[0] == 0)
	  ||(pixelCoord.y % tileSize[0] == 0))
	{
		outTexture.write(float4(0, 0, 0, 1), uint2(pixelCoord));
		return;
	}

	for(int pathIndex = 0; pathIndex < pathCount[0]; pathIndex++)
	{
		const device mg_mtl_path_queue* pathQueue = &pathQueueBuffer[pathIndex];
		int2 pathTileCoord = tileCoord - pathQueue->area.xy;

		if(  pathTileCoord.x >= 0
		  && pathTileCoord.x < pathQueue->area.z
		  && pathTileCoord.y >= 0
		  && pathTileCoord.y < pathQueue->area.w)
		{
			int pathTileIndex = pathTileCoord.y * pathQueue->area.z + pathTileCoord.x;
			const device mg_mtl_tile_queue* tileQueue = &tileQueueBuffer[pathQueue->tileQueues + pathTileIndex];

			int opIndex = atomic_load_explicit(&tileQueue->first, memory_order_relaxed);
			while(opIndex != -1)
			{
				//outTexture.write(float4(0, 0, 1, 1), uint2(pixelCoord));
				//return;

				const device mg_mtl_tile_op* op = &tileOpBuffer[opIndex];

				if(op->kind == MG_MTL_OP_SEGMENT)
				{
					const device mg_mtl_segment* seg = &segmentBuffer[op->index];

					if(seg->pathIndex != currentPath)
					{
						//depending on winding number, update color
						if(winding & 1)
						{
							float4 pathColor = pathBuffer[currentPath].color;
							pathColor.rgb *= pathColor.a;
							color = color*(1-pathColor.a) + pathColor;
						}
						currentPath = seg->pathIndex;
						winding = 0;
					}

					if(pixelCoord.y >= seg->box.y && pixelCoord.y < seg->box.w)
					{
						if(pixelCoord.x < seg->box.x)
						{
							winding += seg->windingIncrement;
						}
						else if(pixelCoord.x < seg->box.z)
						{
							/*TODO: if pixel is on opposite size of diagonal as curve on the right, increment
					       			otherwise if not on same size of diagonal as curve, do implicit test
							*/
							float alpha = (seg->box.w - seg->box.y)/(seg->box.z - seg->box.x);
							float ofs = seg->box.w - seg->box.y;
							float dx = pixelCoord.x - seg->box.x;
							float dy = pixelCoord.y - seg->box.y;

							if( (seg->config == MG_MTL_BR && dy > alpha*dx)
				  			||(seg->config == MG_MTL_TR && dy < ofs - alpha*dx))
							{
								winding += seg->windingIncrement;
							}
							else if(  !(seg->config == MG_MTL_TL && dy < alpha*dx)
				       			&& !(seg->config == MG_MTL_BL && dy > ofs - alpha*dx))
							{
								//Need implicit test, but for lines, we only have config BR or TR, so the test is always negative for now
							}
						}
					}
				}
				opIndex = op->next;
			}
		}
	}

	if(winding & 1)
	{
		float4 pathColor = pathBuffer[currentPath].color;
		pathColor.rgb *= pathColor.a;
		color = color*(1-pathColor.a) + pathColor;
	}

	outTexture.write(color, uint2(pixelCoord));
}

//------------------------------------------------------------------------------------
// Blit shader
//------------------------------------------------------------------------------------
struct vs_out
{
    float4 pos [[position]];
    float2 uv;
};

vertex vs_out mtl_vertex_shader(ushort vid [[vertex_id]])
{
	vs_out out;
	out.uv = float2((vid << 1) & 2, vid & 2);
	out.pos = float4(out.uv * float2(2, -2) + float2(-1, 1), 0, 1);
	return(out);
}

fragment float4 mtl_fragment_shader(vs_out i [[stage_in]], texture2d<float> tex [[texture(0)]])
{
	constexpr sampler smp(mip_filter::nearest, mag_filter::linear, min_filter::linear);
	return(tex.sample(smp, i.uv));
}
