
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
		tileQueues[i].last = -1;
		atomic_store_explicit(&tileQueues[i].windingOffset, 0, memory_order_relaxed);
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
			//NOTE: for lines, we only have config BR or TR, so the test is always negative

			if(seg->kind == MG_MTL_QUADRATIC)
			{
				/*
				//DEBUG: behave as a straight line segment
				if((seg->config == MG_MTL_BL || seg->config == MG_MTL_TL))
				{
					isLeft = true;
				}
				/*/

				float3 ph = {p.x, p.y, 1};
				float3 klm = seg->implicitMatrix * ph;

				if((klm.x*klm.x - klm.y)*klm.z < 0)
				{
					isLeft = true;
				}
				//*/
			}
		}
	}
	return(isLeft);
}

void mtl_bin_segment_to_tiles(int segIndex,
                              device mg_mtl_segment* seg,
                              const device mg_mtl_path_queue* pathQueueBuffer,
                              device mg_mtl_tile_queue* tileQueueBuffer,
                              device mg_mtl_tile_op* tileOpBuffer,
                              device atomic_int* tileOpCount,
                              int tileSize)
{
	//NOTE: add segment index to the queues of tiles it overlaps with
	const device mg_mtl_path_queue* pathQueue = &pathQueueBuffer[seg->pathIndex];
	device mg_mtl_tile_queue* tileQueues = &tileQueueBuffer[pathQueue->tileQueues];

	int4 coveredTiles = int4(seg->box)/tileSize;
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
			                          float(y + pathQueue->area.y + 1)} * float(tileSize);

			//NOTE: select two corners of tile box to test against the curve
			float2 testPoint0;
			float2 testPoint1;
			if(seg->config == MG_MTL_BL || seg->config == MG_MTL_TR)
			{
				testPoint0 = (float2){tileBox.x, tileBox.y},
				testPoint1 = (float2){tileBox.z, tileBox.w};
			}
			else
			{
				testPoint0 = (float2){tileBox.z, tileBox.y};
				testPoint1 = (float2){tileBox.x, tileBox.w};
			}
			bool test0 = mtl_is_left_of_segment(testPoint0, seg);
			bool test1 = mtl_is_left_of_segment(testPoint1, seg);

			//NOTE: the curve overlaps the tile only if test points are on opposite sides of segment
			if(test0 != test1)
			{
				int tileOpIndex = atomic_fetch_add_explicit(tileOpCount, 1, memory_order_relaxed);
				device mg_mtl_tile_op* op = &tileOpBuffer[tileOpIndex];

				op->kind = MG_MTL_OP_SEGMENT;
				op->index = segIndex;
				op->next = -1;

				int tileIndex = y*pathQueue->area.z + x;
				device mg_mtl_tile_queue* tile = &tileQueues[tileIndex];
				op->next = atomic_exchange_explicit(&tile->first, tileOpIndex, memory_order_relaxed);
				if(op->next == -1)
				{
					tile->last = tileOpIndex;
				}

				//NOTE: if the segment crosses the tile's bottom boundary, update the tile's winding offset
				//      testPoint0 is always a bottom point. We select the other one and check if they are on
				//      opposite sides of the curve.
				//      We also need to check that the endpoints of the curve are on opposite sides of the bottom
				//      boundary.
				float2 testPoint3;
				if(seg->config == MG_MTL_BL || seg->config == MG_MTL_TR)
				{
					testPoint3 = (float2){tileBox.z, tileBox.y};
				}
				else
				{
					testPoint3 = (float2){tileBox.x, tileBox.y};
				}
				bool test3 = mtl_is_left_of_segment(testPoint3, seg);

				if(  test0 != test3
					 && seg->box.y <= testPoint0.y
					 && seg->box.w > testPoint0.y)
				{
					atomic_fetch_add_explicit(&tile->windingOffset, seg->windingIncrement, memory_order_relaxed);
				}

				//NOTE: if the segment crosses the right boundary, mark it. We reuse one of the previous tests
				float2 top = {tileBox.z, tileBox.w};
				bool testTop = mtl_is_left_of_segment(top, seg);
				bool testBottom = (seg->config == MG_MTL_BL || seg->config == MG_MTL_TR)? test3 : test0;

				if(testTop != testBottom
					 && seg->box.x <= top.x
					 && seg->box.z > top.x)
				{
					op->crossRight = true;
				}
				else
				{
					op->crossRight = false;
				}
			}
		}
	}
}

int mtl_quadratic_monotonize(float2 p[3], float2 sp[9])
{
	//NOTE: compute split points
	int count = 0;
	float splitPoints[4];
	splitPoints[0] = 0;
	count++;

	float2 r = (p[0] - p[1])/(p[2] - 2*p[1] + p[0]);
	if(r.x > r.y)
	{
		float tmp = r.x;
		r.x = r.y;
		r.y = tmp;
	}
	if(r.x > 0 && r.x < 1)
	{
		splitPoints[count] = r.x;
		count++;
	}
	if(r.y > 0 && r.y < 1)
	{
		splitPoints[count] = r.y;
		count++;
	}
	splitPoints[count] = 1;
	count++;

	for(int i=0; i<count-1; i++)
	{
		//NOTE cut curve between splitPoint[i] and splitPoint[i+1]
		float z0 = splitPoints[i];
		float z1 = splitPoints[i+1];
		float zr = (z1-z0)/(1-z0);

		float2 q0 = (z0-1)*(z0-1)*p[0]
		          - 2*(z0-1)*z0*p[1]
		          + z0*z0*p[2];

		float2 q1 = (z0-1)*(z0-1)*(1-zr)*p[0]
		          + ((1-z0)*zr - 2*(z0-1)*(1-zr)*z0)*p[1]
		          + (z0*z0*(1-zr) + z0*zr)*p[2];

		float2 q2 = (z0-1)*(z0-1)*(1-zr)*(1-zr)*p[0]
		          - 2*((z0-1)*z0*(zr-1)*(zr-1)+ (1-z0)*(zr-1)*zr)*p[1]
		          + (z0*z0*(zr-1)*(zr-1) - 2*z0*(zr-1)*zr + zr*zr)*p[2];

		sp[3*i] = q0;
		sp[3*i+1] = q1;
		sp[3*i+2] = q2;
	}
	return(count-1);
}

void mtl_setup_monotonic_quadratic(thread float2* p,
                                   int pathIndex,
                                   device atomic_int* segmentCount,
                                   device mg_mtl_segment* segmentBuffer,
                                   const device mg_mtl_path_queue* pathQueueBuffer,
                                   device mg_mtl_tile_queue* tileQueueBuffer,
                                   device mg_mtl_tile_op* tileOpBuffer,
                                   device atomic_int* tileOpCount,
                                   int tileSize,
                                   int debugID)
{
	//TODO: collapse with other segment kinds
	int segIndex = atomic_fetch_add_explicit(segmentCount, 1, memory_order_relaxed);
	device mg_mtl_segment* seg = &segmentBuffer[segIndex];


	seg->debugID = debugID;

	seg->kind = MG_MTL_QUADRATIC;
	seg->pathIndex = pathIndex;
	seg->box = (vector_float4){min(p[0].x, p[2].x),
	                           min(p[0].y, p[2].y),
	                           max(p[0].x, p[2].x),
	                           max(p[0].y, p[2].y)};

	float dx = p[1].x - seg->box.x;
	float dy = p[1].y - seg->box.y;
	float alpha = (seg->box.w - seg->box.y)/(seg->box.z - seg->box.x);
	float ofs = seg->box.w - seg->box.y;

	if( (p[2].x > p[0].x && p[2].y < p[0].y)
		 ||(p[2].x <= p[0].x && p[2].y > p[0].y))
	{
		if(dy < ofs - alpha*dx)
		{
			seg->config = MG_MTL_BL;
		}
		else
		{
			seg->config = MG_MTL_TR;
		}
	}
	else if( (p[2].x > p[0].x && p[2].y >= p[0].y)
	          ||(p[2].x <= p[0].x && p[2].y <= p[0].y))
	{
		//NOTE: it is important to include horizontal segments here, so that the mtl_is_left_of_segment() test
		//      becomes x > seg->box.x, in order to correctly detect right-crossing horizontal segments
		if(dy > alpha*dx)
		{
			seg->config = MG_MTL_TL;
		}
		else
		{
			seg->config = MG_MTL_BR;
		}
	}
	seg->windingIncrement = (p[2].y > p[0].y)? 1 : -1;

	//NOTE: compute implicit equation matrix

	float det = p[0].x*(p[1].y-p[2].y) + p[1].x*(p[2].y-p[0].y) + p[2].x*(p[0].y - p[1].y);

	float a = p[0].y - p[1].y + 0.5*(p[2].y - p[0].y);
	float b = p[1].x - p[0].x + 0.5*(p[0].x - p[2].x);
	float c = p[0].x*p[1].y - p[1].x*p[0].y + 0.5*(p[2].x*p[0].y - p[0].x*p[2].y);
	float d = p[0].y - p[1].y;
	float e = p[1].x - p[0].x;
	float f = p[0].x*p[1].y - p[1].x*p[0].y;

	float flip = (seg->config == MG_MTL_TL || seg->config == MG_MTL_BL)? -1 : 1;
/*
	seg->implicitMatrix = (1/det)*matrix_float3x3({flip*a, flip*d, a},
	                                              {flip*b, flip*e, b},
	                                              {flip*c, flip*f, c});
*/
	float g = flip*(p[2].x*(p[0].y - p[1].y) + p[0].x*(p[1].y - p[2].y) + p[1].x*(p[2].y - p[0].y));

	seg->implicitMatrix = (1/det)*matrix_float3x3({a, d, 0.},
	                                              {b, e, 0.},
	                                              {c, f, g});


	mtl_bin_segment_to_tiles(segIndex, seg, pathQueueBuffer, tileQueueBuffer, tileOpBuffer, tileOpCount, tileSize);
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

	if(elt->kind == MG_MTL_LINE)
	{
		int segIndex = atomic_fetch_add_explicit(segmentCount, 1, memory_order_relaxed);
		device mg_mtl_segment* seg = &segmentBuffer[segIndex];

		seg->debugID = 0;
		seg->kind = elt->kind;
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
		else if( (p3.x > p0.x && p3.y >= p0.y)
	           ||(p3.x <= p0.x && p3.y <= p0.y))
		{
			//NOTE: it is important to include horizontal segments here, so that the mtl_is_left_of_segment() test
			//      becomes x > seg->box.x, in order to correctly detect right-crossing horizontal segments
			seg->config = MG_MTL_BR;
		}

		seg->windingIncrement = (p3.y > p0.y)? 1 : -1;

		mtl_bin_segment_to_tiles(segIndex, seg, pathQueueBuffer, tileQueueBuffer, tileOpBuffer, tileOpCount, tileSize[0]);
	}
	else if(elt->kind == MG_MTL_QUADRATIC)
	{
		//NOTE: Quadratic has at most two split points (ie 3 monotonic sub curves)
		float2 p[3] = {elt->p[0], elt->p[1], elt->p[3]};
		float2 sp[9];

		int count = mtl_quadratic_monotonize(p, sp);

		for(int i=0; i<count; i++)
		{
			mtl_setup_monotonic_quadratic(&sp[3*i],
			                              elt->pathIndex,
			                              segmentCount,
			                              segmentBuffer,
			                              pathQueueBuffer,
			                              tileQueueBuffer,
			                              tileOpBuffer,
			                              tileOpCount,
			                              tileSize[0],
			                              1000 + count*10 + i);
		}
	}
}

kernel void mtl_backprop(const device mg_mtl_path_queue* pathQueueBuffer [[buffer(0)]],
                         device mg_mtl_tile_queue* tileQueueBuffer [[buffer(1)]],
                         uint pathIndex [[threadgroup_position_in_grid]],
                         uint localID [[thread_position_in_threadgroup]])
{
	threadgroup atomic_int nextRowIndex;
	if(localID == 0)
	{
		atomic_store_explicit(&nextRowIndex, 0, memory_order_relaxed);
	}
	threadgroup_barrier(mem_flags::mem_threadgroup);

	int rowIndex = 0;
	const device mg_mtl_path_queue* pathQueue = &pathQueueBuffer[pathIndex];
	device mg_mtl_tile_queue* tiles = &tileQueueBuffer[pathQueue->tileQueues];
	int rowSize = pathQueue->area.z;
	int rowCount = pathQueue->area.w;

	rowIndex = atomic_fetch_add_explicit(&nextRowIndex, 1, memory_order_relaxed);
	while(rowIndex < rowCount)
	{
		device mg_mtl_tile_queue* row = &tiles[rowIndex * rowSize];
		int sum = 0;
		for(int x = rowSize-1; x >= 0; x--)
		{
			device mg_mtl_tile_queue* tile = &row[x];
			int offset = *(device int*)&tile->windingOffset;
			*(device int*)(&tile->windingOffset) = sum;
			sum += offset;
		}
		rowIndex = atomic_fetch_add_explicit(&nextRowIndex, 1, memory_order_relaxed);
	}
}

kernel void mtl_merge(constant int* pathCount [[buffer(0)]],
                      const device mg_mtl_path* pathBuffer [[buffer(1)]],
                      const device mg_mtl_path_queue* pathQueueBuffer [[buffer(2)]],
                      const device mg_mtl_tile_queue* tileQueueBuffer [[buffer(3)]],
                      device mg_mtl_tile_op* tileOpBuffer [[buffer(4)]],
                      device atomic_int* tileOpCount [[buffer(5)]],
                      device int* screenTilesBuffer [[buffer(6)]],
                      uint2 threadCoord [[thread_position_in_grid]],
                      uint2 gridSize [[threads_per_grid]])
{
	int2 tileCoord = int2(threadCoord);
	int tileIndex = tileCoord.y * gridSize.x + tileCoord.x;
	device int* nextLink = &screenTilesBuffer[tileIndex];
	*nextLink = -1;

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

			int windingOffset = atomic_load_explicit(&tileQueue->windingOffset, memory_order_relaxed);
			int firstOpIndex = atomic_load_explicit(&tileQueue->first, memory_order_relaxed);

			if(firstOpIndex == -1)
			{
				if(windingOffset & 1)
				{
					//NOTE: tile is full covered. Add path start op (with winding offset).
					//      Additionally if color is opaque, trim tile list.
					int pathOpIndex = atomic_fetch_add_explicit(tileOpCount, 1, memory_order_relaxed);
					device mg_mtl_tile_op* pathOp = &tileOpBuffer[pathOpIndex];
					pathOp->kind = MG_MTL_OP_START;
					pathOp->next = -1;
					pathOp->index = pathIndex;
					pathOp->windingOffset = windingOffset;

					if(pathBuffer[pathIndex].color.a == 1)
					{
						screenTilesBuffer[tileIndex] = pathOpIndex;
					}
					else
					{
						*nextLink = pathOpIndex;
					}
					nextLink = &pathOp->next;
				}
				// else, tile is fully uncovered, skip path
			}
			else
			{
				//NOTE: add path start op (with winding offset)
				int pathOpIndex = atomic_fetch_add_explicit(tileOpCount, 1, memory_order_relaxed);
				device mg_mtl_tile_op* pathOp = &tileOpBuffer[pathOpIndex];
				pathOp->kind = MG_MTL_OP_START;
				pathOp->next = -1;
				pathOp->index = pathIndex;
				pathOp->windingOffset = windingOffset;

				*nextLink = pathOpIndex;
				nextLink = &pathOp->next;

				//NOTE: chain remaining path ops to end of tile list
				int lastOpIndex = tileQueue->last;
				device mg_mtl_tile_op* lastOp = &tileOpBuffer[lastOpIndex];
				*nextLink = firstOpIndex;
				nextLink = &lastOp->next;
			}
		}
	}
}

kernel void mtl_raster(const device int* screenTilesBuffer [[buffer(0)]],
                       const device mg_mtl_tile_op* tileOpBuffer [[buffer(1)]],
                       const device mg_mtl_path* pathBuffer [[buffer(2)]],
                       const device mg_mtl_segment* segmentBuffer [[buffer(3)]],
                       constant int* tileSize [[buffer(4)]],
                       texture2d<float, access::write> outTexture [[texture(0)]],
                       uint2 threadCoord [[thread_position_in_grid]],
                       uint2 gridSize [[threads_per_grid]])
{
	int2 pixelCoord = int2(threadCoord);
	int2 tileCoord = pixelCoord / tileSize[0];
	int nTilesX = (int(gridSize.x) + tileSize[0] - 1)/tileSize[0];
	int tileIndex = tileCoord.y * nTilesX + tileCoord.x;

	if( (pixelCoord.x % tileSize[0] == 0)
	  ||(pixelCoord.y % tileSize[0] == 0))
	{
		outTexture.write(float4(0, 0, 0, 1), uint2(pixelCoord));
		return;
	}

	float4 color = float4(0, 0, 0, 0);
	int pathIndex = 0;
	int winding = 0;
	int opIndex = screenTilesBuffer[tileIndex];

	while(opIndex != -1)
	{
		const device mg_mtl_tile_op* op = &tileOpBuffer[opIndex];

		if(op->kind == MG_MTL_OP_START)
		{
			if(winding & 1)
			{
				float4 pathColor = pathBuffer[pathIndex].color;
				pathColor.rgb *= pathColor.a;
				color = color*(1-pathColor.a) + pathColor;
			}
			pathIndex = op->index;
			winding = op->windingOffset;
		}
		else if(op->kind == MG_MTL_OP_SEGMENT)
		{
			const device mg_mtl_segment* seg = &segmentBuffer[op->index];


			/*
			if(seg->kind == MG_MTL_LINE && op->crossRight)
			{
				outTexture.write(float4(1, 0, 0, 1), uint2(pixelCoord));
				return;
			}
			*/

			if(pixelCoord.y >= seg->box.y && pixelCoord.y < seg->box.w)
			{
				if(mtl_is_left_of_segment(float2(pixelCoord), seg))
				{
					winding += seg->windingIncrement;
				}
			}

			if(op->crossRight)
			{
				if( (seg->config == MG_MTL_BR || seg->config == MG_MTL_TL)
						&&(pixelCoord.y >= seg->box.w))
				{
					winding += seg->windingIncrement;
				}
				else if( (seg->config == MG_MTL_BL || seg->config == MG_MTL_TR)
						     &&(pixelCoord.y >= seg->box.y))
				{
					winding -= seg->windingIncrement;
				}
			}
		}
		opIndex = op->next;
	}
	if(winding & 1)
	{
		float4 pathColor = pathBuffer[pathIndex].color;
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
