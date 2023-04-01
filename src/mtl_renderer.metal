
#include<metal_stdlib>
#include<simd/simd.h>
#include<metal_simdgroup>

#include"mtl_renderer.h"

using namespace metal;


typedef struct mtl_log_context
{
	device char* buffer;
	device atomic_int* offset;
	bool enabled;
} mtl_log_context;

int strlen(const constant char* msg)
{
	int count = 0;
	while(msg[count] != 0)
	{
		count++;
	}
	return(count);
}

int strlen(const thread char* msg)
{
	int count = 0;
	while(msg[count] != 0)
	{
		count++;
	}
	return(count);
}

void mtl_log(mtl_log_context context, const constant char* msg)
{
	if(context.enabled)
	{
		int len = strlen(msg);
		int offset = atomic_fetch_add_explicit(context.offset, len+1, memory_order_relaxed);

		for(int i=0; i<len+1; i++)
		{
			context.buffer[offset+i] = msg[i];
		}
	}
}

void mtl_log(mtl_log_context context, const thread char* msg)
{
	if(context.enabled)
	{
		int len = strlen(msg);
		int offset = atomic_fetch_add_explicit(context.offset, len+1, memory_order_relaxed);

		for(int i=0; i<len+1; i++)
		{
			context.buffer[offset+i] = msg[i];
		}
	}
}

int mtl_itoa(int bufSize, thread char* buffer, thread char** start, int64_t value)
{
	bool minus = false;
	if(value < 0)
	{
		minus = true;
		value *= -1;
	}
	buffer[bufSize-1] = '\0';
	int index = bufSize-2;
	do
	{
		buffer[index] = '0' + (value % 10);
		index--;
		value /= 10;
	} while(value != 0 && index >= 1);

	if(minus)
	{
		buffer[index] = '-';
		index--;
	}

	*start = buffer+index+1;
	return(bufSize - (index+1) - 1);
}


void mtl_log_i32(mtl_log_context context, int value)
{
	char buffer[12];
	thread char* start = 0;
	mtl_itoa(12, buffer, &start, value);
	mtl_log(context, start);
}

void mtl_log_f32(mtl_log_context context, float value)
{
	int64_t integral = (int64_t)value;
	int64_t decimal = (int64_t)((value - (float)integral)*1e9);

	while(decimal && (decimal % 10 == 0))
	{
		decimal /= 10;
	}
	while(decimal > 999999)
	{
		decimal /= 10;
	}

	const int bufSize = 64;
	char buffer[bufSize];
	thread char* start = 0;
	int integralSize = mtl_itoa(bufSize, buffer, &start, integral);

	for(int i=0; i<integralSize; i++)
	{
		buffer[i] = start[i];
	}

	int decimalSize	= 0;
	if(integralSize < bufSize && decimal)
	{
		buffer[integralSize] = '.';
		integralSize++;
		decimalSize = mtl_itoa(bufSize - integralSize - 1, buffer, &start, decimal);
		for(int i=0; i<decimalSize; i++)
		{
			buffer[integralSize+i] = start[i];
		}
	}
	buffer[integralSize+decimalSize] = '\0';
	mtl_log(context, buffer);
}

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
				float3 ph = {p.x, p.y, 1};
				float3 klm = seg->implicitMatrix * ph;
				if((klm.x*klm.x - klm.y)*klm.z < 0)
				{
					isLeft = true;
				}
			}
			else if(seg->kind == MG_MTL_CUBIC)
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
				if(klm.x*klm.x*klm.x - klm.y*klm.z < 0)
				{
					isLeft = true;
				}
				//*/
			}
		}
	}
	return(isLeft);
}

typedef struct mtl_segment_setup_context
{
	int pathIndex;
	device atomic_int* segmentCount;
	device mg_mtl_segment* segmentBuffer;
	const device mg_mtl_path_queue* pathQueue;
	device mg_mtl_tile_queue* tileQueues;
	device mg_mtl_tile_op* tileOpBuffer;
	device atomic_int* tileOpCount;
	int tileSize;
	mtl_log_context log;
} mtl_segment_setup_context;

void mtl_segment_bin_to_tiles(thread mtl_segment_setup_context* context, device mg_mtl_segment* seg)
{
	//NOTE: add segment index to the queues of tiles it overlaps with
	int segIndex = seg - context->segmentBuffer;

	int tileSize = context->tileSize;
	int4 pathArea = context->pathQueue->area;
	int4 coveredTiles = int4(seg->box)/tileSize;
	int xMin = max(0, coveredTiles.x - pathArea.x);
	int yMin = max(0, coveredTiles.y - pathArea.y);
	int xMax = min(coveredTiles.z - pathArea.x, pathArea.z-1);
	int yMax = min(coveredTiles.w - pathArea.y, pathArea.w-1);

	for(int y = yMin; y <= yMax; y++)
	{
		for(int x = xMin ; x <= xMax; x++)
		{
			float4 tileBox = (float4){float(x + pathArea.x),
			                          float(y + pathArea.y),
			                          float(x + pathArea.x + 1),
			                          float(y + pathArea.y + 1)} * float(tileSize);

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
				int tileOpIndex = atomic_fetch_add_explicit(context->tileOpCount, 1, memory_order_relaxed);
				device mg_mtl_tile_op* op = &context->tileOpBuffer[tileOpIndex];

				op->kind = MG_MTL_OP_SEGMENT;
				op->index = segIndex;
				op->next = -1;

				int tileIndex = y*pathArea.z + x;
				device mg_mtl_tile_queue* tile = &context->tileQueues[tileIndex];
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
					 && seg->box.y < testPoint0.y
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

device mg_mtl_segment* mtl_segment_push(thread mtl_segment_setup_context* context, float2 p[4], mg_mtl_seg_kind kind)
{
	float2 s, e, c;

	switch(kind)
	{
		case MG_MTL_LINE:
			s = p[0];
			c = p[0];
			e = p[1];
			break;

		case MG_MTL_QUADRATIC:
			s = p[0];
			c = p[1];
			e = p[2];
			break;

		case MG_MTL_CUBIC:
			s = p[0];
			if(any(p[1] != p[0]))
			{
				c = p[1];
			}
			else
			{
				c = p[2];
			}
			e = p[3];
			break;
	}

	int segIndex = atomic_fetch_add_explicit(context->segmentCount, 1, memory_order_relaxed);
	device mg_mtl_segment* seg = &context->segmentBuffer[segIndex];

	seg->kind = kind;
	seg->pathIndex = context->pathIndex;
	seg->windingIncrement = (e.y > s.y)? 1 : -1;

	seg->box = (vector_float4){min(s.x, e.x),
	                           min(s.y, e.y),
	                           max(s.x, e.x),
	                           max(s.y, e.y)};

	float dx = c.x - seg->box.x;
	float dy = c.y - seg->box.y;
	float alpha = (seg->box.w - seg->box.y)/(seg->box.z - seg->box.x);
	float ofs = seg->box.w - seg->box.y;

	//TODO: check that it works for line segments!
	if( (e.x > s.x && e.y < s.y)
	  ||(e.x <= s.x && e.y > s.y))
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
	else if( (e.x > s.x && e.y >= s.y)
	       ||(e.x <= s.x && e.y <= s.y))
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
	return(seg);
}

#define square(x) ((x)*(x))
#define cube(x) ((x)*(x)*(x))

void mtl_line_setup(thread mtl_segment_setup_context* context, float2 p[2])
{
	device mg_mtl_segment* seg = mtl_segment_push(context, p, MG_MTL_LINE);
	mtl_segment_bin_to_tiles(context, seg);
}

void mtl_quadratic_slice(float2 p[3], float s0, float s1, float2 sp[3])
{
	//NOTE cut curve between splitPoint[i] and splitPoint[i+1]
	float sr = (s1-s0)/(1-s0);

	sp[0] = (s0-1)*(s0-1)*p[0]
	      - 2*(s0-1)*s0*p[1]
	      + s0*s0*p[2];

	sp[1] = (s0-1)*(s0-1)*(1-sr)*p[0]
	      + ((1-s0)*sr - 2*(s0-1)*(1-sr)*s0)*p[1]
	      + (s0*s0*(1-sr) + s0*sr)*p[2];

	sp[2] = (s0-1)*(s0-1)*(1-sr)*(1-sr)*p[0]
	      - 2*((s0-1)*s0*(sr-1)*(sr-1)+ (1-s0)*(sr-1)*sr)*p[1]
	      + (s0*s0*(sr-1)*(sr-1) - 2*s0*(sr-1)*sr + sr*sr)*p[2];
}

int mtl_quadratic_monotonize(float2 p[3], float splits[4])
{
	//NOTE: compute split points
	int count = 0;
	splits[0] = 0;
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
		splits[count] = r.x;
		count++;
	}
	if(r.y > 0 && r.y < 1)
	{
		splits[count] = r.y;
		count++;
	}
	splits[count] = 1;
	count++;
	return(count);
}

void mtl_quadratic_emit(thread mtl_segment_setup_context* context,
                        thread float2* p)
{
	device mg_mtl_segment* seg = mtl_segment_push(context, p, MG_MTL_QUADRATIC);

	//NOTE: compute implicit equation matrix
	float det = p[0].x*(p[1].y-p[2].y) + p[1].x*(p[2].y-p[0].y) + p[2].x*(p[0].y - p[1].y);

	float a = p[0].y - p[1].y + 0.5*(p[2].y - p[0].y);
	float b = p[1].x - p[0].x + 0.5*(p[0].x - p[2].x);
	float c = p[0].x*p[1].y - p[1].x*p[0].y + 0.5*(p[2].x*p[0].y - p[0].x*p[2].y);
	float d = p[0].y - p[1].y;
	float e = p[1].x - p[0].x;
	float f = p[0].x*p[1].y - p[1].x*p[0].y;

	float flip = (seg->config == MG_MTL_TL || seg->config == MG_MTL_BL)? -1 : 1;
	float g = flip*(p[2].x*(p[0].y - p[1].y) + p[0].x*(p[1].y - p[2].y) + p[1].x*(p[2].y - p[0].y));

	seg->implicitMatrix = (1/det)*matrix_float3x3({a, d, 0.},
	                                              {b, e, 0.},
	                                              {c, f, g});

	mtl_segment_bin_to_tiles(context, seg);
}

void mtl_quadratic_setup(thread mtl_segment_setup_context* context, thread float2* p)
{
	float splits[4];
	int splitCount = mtl_quadratic_monotonize(p, splits);

	//NOTE: produce bézier curve for each consecutive pair of roots
	for(int sliceIndex=0; sliceIndex<splitCount-1; sliceIndex++)
	{
		float2 sp[3];
		mtl_quadratic_slice(p, splits[sliceIndex], splits[sliceIndex+1], sp);
		mtl_quadratic_emit(context, sp);
	}
}

int mtl_quadratic_roots(float a, float b, float c, thread float* r)
{
	//TODO: replace by something more numerically stable
	int count = 0;
	float det = square(b) - 4*a*c;
	if(det > 0)
	{
		count = 2;
		r[0] = (-b - sqrt(det))/(2*a);
		r[1] = (-b + sqrt(det))/(2*a);
	}
	else if(det == 0)
	{
		count = 1;
		r[0] = -b/(2*a);
	}
	return(count);
}

void log_cubic_bezier(thread float2* p, mtl_log_context logCtx)
{
	mtl_log(logCtx, "(");
	mtl_log_f32(logCtx, p[0].x);
	mtl_log(logCtx, ", ");
	mtl_log_f32(logCtx, p[0].y);
	mtl_log(logCtx, ") (");
	mtl_log_f32(logCtx, p[1].x);
	mtl_log(logCtx, ", ");
	mtl_log_f32(logCtx, p[1].y);
	mtl_log(logCtx, ") (");
	mtl_log_f32(logCtx, p[2].x);
	mtl_log(logCtx, ", ");
	mtl_log_f32(logCtx, p[2].y);
	mtl_log(logCtx, ") (");
	mtl_log_f32(logCtx, p[3].x);
	mtl_log(logCtx, ", ");
	mtl_log_f32(logCtx, p[3].y);
	mtl_log(logCtx, ")\n");
}

void mtl_cubic_slice(float2 p[4], float s0, float s1, float2 sp[4])
{
	float sr = (s1 - s0)/(1-s0);

	matrix_float4x4 rightCut = {{-cube(s0-1), 0, 0, 0},
	                            {3*square(s0-1)*s0, square(s0-1), 0, 0},
	                            {-3*(s0-1)*square(s0), -2*(s0-1)*s0, 1-s0, 0},
	                            {cube(s0), square(s0), s0, 1}};

	matrix_float4x4 leftCut = {{1, 1-sr, square(sr-1), -cube(sr-1)},
	                           {0, sr, -2*(sr-1)*sr, 3*square(sr-1)*sr},
	                           {0, 0, square(sr), -3*(sr-1)*square(sr)},
	                           {0, 0, 0, cube(sr)}};

	float4 px = {p[0].x, p[1].x, p[2].x, p[3].x};
	float4 py = {p[0].y, p[1].y, p[2].y, p[3].y};

	float4 qx = leftCut*rightCut*px;
	float4 qy = leftCut*rightCut*py;

	sp[0] = float2(qx.x, qy.x);
	sp[1] = float2(qx.y, qy.y);
	sp[2] = float2(qx.z, qy.z);
	sp[3] = float2(qx.w, qy.w);
}

int mtl_cubic_monotonize(float2 p[4], float splits[8])
{
	//NOTE(martin): first convert the control points to power basis
	float2 c[4];
	c[0] = p[0];
	c[1] = 3*(p[1]-p[0]);
	c[2] = 3*(p[0] - 2*p[1] + p[2]);
	c[3] = 3*(p[1] - p[2]) + p[3] - p[0];

	//NOTE: compute the roots of the derivative
	float roots[6];
	int rootCount = mtl_quadratic_roots(3*c[3].x, 2*c[2].x, c[1].x, roots);
	rootCount += mtl_quadratic_roots(3*c[3].y, 2*c[2].y, c[1].y, roots+rootCount);

	//NOTE: compute inflection points
	rootCount += mtl_quadratic_roots(6*(c[2].x*c[3].y-c[3].x*c[2].y),
	                                 6*(c[1].x*c[3].y-c[1].y*c[3].x),
	                                 2*(c[1].x*c[2].y-c[1].y*c[2].x),
	                                 roots + rootCount);

	//NOTE: sort roots
	for(int i=1; i<rootCount; i++)
	{
		float tmp = roots[i];
		int j = i-1;
		while(j>=0 && roots[j]>tmp)
		{
			roots[j+1] = roots[j];
			j--;
		}
		roots[j+1] = tmp;
	}

	//NOTE: compute split points
	int splitCount = 0;
	splits[0] = 0;
	splitCount++;
	for(int i=0; i<rootCount; i++)
	{
		if(roots[i] > 0 && roots[i] < 1)
		{
			splits[splitCount] = roots[i];
			splitCount++;
		}
	}

	splits[splitCount] = 1;
	splitCount++;

	//NOTE: return number of split points
	return(splitCount);
}

typedef enum
{
	MTL_CUBIC_ERROR,
	MTL_CUBIC_DEGENERATE_LINE,
	MTL_CUBIC_DEGENERATE_QUADRATIC,
	MTL_CUBIC_LOOP_SPLIT,
	MTL_CUBIC_LOOP_OK,
	MTL_CUBIC_CUSP,
	MTL_CUBIC_SERPENTINE
} mtl_cubic_kind;

typedef struct mtl_cubic_info
{
	mtl_cubic_kind kind;
	matrix_float4x4 K;
	float2 quadPoint;
	float split;

} mtl_cubic_info;

mtl_cubic_info mtl_cubic_classify(thread float2* p)
{
	mtl_cubic_info result = {MTL_CUBIC_ERROR};
	matrix_float4x4 F;

	/*NOTE(martin): first convert the control points to power basis, multiplying by M3

		     | 1  0  0  0|
		M3 = |-3  3  0  0|
		     | 3 -6  3  0|
		     |-1  3 -3  1|
		ie:
		    c0 = p0
		    c1 = -3*p0 + 3*p1
		    c2 = 3*p0 - 6*p1 + 3*p2
		    c3 = -p0 + 3*p1 - 3*p2 + p3
	*/
	float2 c1 = 3.0*p[1] - 3.0*p[0];
	float2 c2 = 3.0*p[0] + 3.0*p[2] - 6.0*p[1];
	float2 c3 = 3.0*p[1] - 3.0*p[2] + p[3] - p[0];

	/*NOTE(martin):
		now, compute determinants d0, d1, d2, d3, which gives the coefficients of the
	        inflection points polynomial:

		I(t, s) = d0*t^3 - 3*d1*t^2*s + 3*d2*t*s^2 - d3*s^3

		The roots of this polynomial are the inflection points of the parametric curve, in homogeneous
		coordinates (ie we can have an inflection point at inifinity with s=0).

		         |x3 y3 w3|              |x3 y3 w3|             |x3 y3 w3|              |x2 y2 w2|
		d0 = det |x2 y2 w2|    d1 = -det |x2 y2 w2|    d2 = det |x1 y1 w1|    d3 = -det |x1 y1 w1|
		         |x1 y1 w1|              |x0 y0 w0|             |x0 y0 w0|              |x0 y0 w0|

		In our case, the pi.w equal 1 (no point at infinity), so _in_the_power_basis_, w1 = w2 = w3 = 0 and w0 = 1
		(which also means d0 = 0)
	*/
	float d1 = c3.y*c2.x - c3.x*c2.y;
	float d2 = c3.x*c1.y - c3.y*c1.x;
	float d3 = c2.y*c1.x - c2.x*c1.y;

	//NOTE(martin): compute the second factor of the discriminant discr(I) = d1^2*(3*d2^2 - 4*d3*d1)
	float discrFactor2 = 3.0*square(d2) - 4.0*d3*d1;

	//NOTE(martin): each following case gives the number of roots, hence the category of the parametric curve
	if(fabs(d1) < 0.1 && fabs(d2) < 0.1 && d3 != 0)
	{
		//NOTE(martin): quadratic degenerate case
		//NOTE(martin): compute quadratic curve control point, which is at p0 + 1.5*(p1-p0) = 1.5*p1 - 0.5*p0
		result.kind = MTL_CUBIC_DEGENERATE_QUADRATIC;
		result.quadPoint = float2(1.5*p[1].x - 0.5*p[0].x, 1.5*p[1].y - 0.5*p[0].y);
	}
	else if( (discrFactor2 > 0 && d1 != 0)
	       ||(discrFactor2 == 0 && d1 != 0))
	{
		//NOTE(martin): serpentine curve or cusp with inflection at infinity
		//              (these two cases are handled the same way).
		//NOTE(martin): compute the solutions (tl, sl), (tm, sm), and (tn, sn) of the inflection point equation
		float tl = d2 + sqrt(discrFactor2/3);
		float sl = 2*d1;
		float tm = d2 - sqrt(discrFactor2/3);
		float sm = sl;

		/*NOTE(martin):
			the power basis coefficients of points k,l,m,n are collected into the rows of the 4x4 matrix F:

				| tl*tm            tl^3        tm^3        1 |
				| -sm*tl - sl*tm   -3sl*tl^2   -3*sm*tm^2  0 |
				| sl*sm            3*sl^2*tl   3*sm^2*tm   0 |
				| 0                -sl^3       -sm^3       0 |
		*/
		result.kind = (discrFactor2 > 0 && d1 != 0) ? MTL_CUBIC_SERPENTINE : MTL_CUBIC_CUSP;

		F = (matrix_float4x4){{tl*tm, -sm*tl-sl*tm, sl*sm, 0},
		                      {cube(tl), -3*sl*square(tl), 3*square(sl)*tl, -cube(sl)},
		                      {cube(tm), -3*sm*square(tm), 3*square(sm)*tm, -cube(sm)},
		                      {1, 0, 0, 0}};

		//NOTE:  if necessary, flip sign of k and l to ensure the interior is west from the curve
		float flip = (d1 < 0)^(p[3].y < p[0].y) ? -1 : 1;
		F[0] *= flip;
		F[1] *= flip;
	}
	else if(discrFactor2 < 0 && d1 != 0)
	{
		//NOTE(martin): loop curve
		float td = d2 + sqrt(-discrFactor2);
		float sd = 2*d1;
		float te = d2 - sqrt(-discrFactor2);
		float se = sd;

		//NOTE(martin): if one of the parameters (td/sd) or (te/se) is in the interval [0,1], the double point
		//              is inside the control points convex hull and would cause a shading anomaly. If this is
		//              the case, subdivide the curve at that point

		//TODO: study edge case where td/sd ~ 1 or 0 (which causes an infinite recursion in split and fill).
		//      quick fix for now is adding a little slop in the check...

		if(sd != 0 && td/sd < 0.99 && td/sd > 0.01)
		{
			result.kind = MTL_CUBIC_LOOP_SPLIT;
			result.split = td/sd;
		}
		if(se != 0 && te/se < 0.99 && te/se > 0.01)
		{
			result.kind = MTL_CUBIC_LOOP_SPLIT;
			result.split = te/se;
		}
		else
		{
			/*NOTE(martin):
				the power basis coefficients of points k,l,m,n are collected into the rows of the 4x4 matrix F:

					| td*te            td^2*te                 td*te^2                1 |
					| -se*td - sd*te   -se*td^2 - 2sd*te*td    -sd*te^2 - 2*se*td*te  0 |
					| sd*se            te*sd^2 + 2*se*td*sd    td*se^2 + 2*sd*te*se   0 |
					| 0                -sd^2*se                -sd*se^2               0 |
			*/
			result.kind = MTL_CUBIC_LOOP_OK;

			F = (matrix_float4x4){{td*te, -se*td-sd*te, sd*se, 0},
			                      {square(td)*te, -se*square(td)-2*sd*te*td, te*square(sd)+2*se*td*sd, -square(sd)*se},
			                      {td*square(te), -sd*square(td)-2*se*td*te, td*square(se)+2*sd*te*se, -sd*square(se)},
			                      {1, 0, 0, 0}};

			//NOTE:  if necessary, flip sign of k and l to ensure the interior is west from the curve
			float H0 = 36*(d3*d1-square(d2));
			float H1 = 36*(d3*d1-square(d2) + d1*d2 - square(d1));
			float H = (abs(H0) > abs(H1)) ? H0 : H1;
			float flip = (H*d1 > 0)^(p[3].y < p[0].y) ? -1 : 1;
			F[0] *= flip;
			F[1] *= flip;
		}
	}
	else if(d1 == 0 && d2 != 0)
	{
		//NOTE(martin): cusp with cusp at infinity

		float tl = d3;
		float sl = 3*d2;

		/*NOTE(martin):
			the power basis coefficients of points k,l,m,n are collected into the rows of the 4x4 matrix F:

				| tl    tl^3        1  1 |
				| -sl   -3sl*tl^2   0  0 |
				| 0     3*sl^2*tl   0  0 |
				| 0     -sl^3       0  0 |
		*/
		result.kind = MTL_CUBIC_CUSP;

		F = (matrix_float4x4){{tl, -sl, 0, 0},
		                      {cube(tl), -3*sl*square(tl), 3*square(sl)*tl, -cube(sl)},
		                      {1, 0, 0, 0},
		                      {1, 0, 0, 0}};

		//NOTE:  if necessary, flip sign of k and l to ensure the interior is west from the curve
		float flip = (p[3].y > p[0].y) ? -1 : 1;
		F[0] *= flip;
		F[1] *= flip;
	}
	else if(d1 == 0 && d2 == 0 && d3 == 0)
	{
		//NOTE(martin): line or point degenerate case
		result.kind = MTL_CUBIC_DEGENERATE_LINE;
	}
	else
	{
		//TODO(martin): handle error ? put some epsilon slack on the conditions ?
		result.kind = MTL_CUBIC_ERROR;
	}

	/*
			F is then multiplied by M3^(-1) on the left which yelds the bezier coefficients k, l, m, n
			at the control points.

			               | 1  0   0   0 |
				M3^(-1) =  | 1  1/3 0   0 |
				           | 1  2/3 1/3 0 |
					       | 1  1   1   1 |
	*/
	matrix_float4x4 invM3 = {{1, 1, 1, 1},
	                         {0, 1./3., 2./3., 1},
	                         {0, 0, 1./3., 1},
	                         {0, 0, 0, 1}};

	result.K = transpose(invM3*F);

	return(result);
}

void mtl_cubic_emit(thread mtl_segment_setup_context* context, float2 p[4], mtl_cubic_info info)
{
	device mg_mtl_segment* seg = mtl_segment_push(context, p, MG_MTL_CUBIC);

	float2 v0 = p[0];
	float2 v1 = p[3];
	float2 v2;
	matrix_float3x3 K;

 	if(any(p[0] != p[1]))
 	{
 		v2 = p[1];
		K = {info.K[0].xyz, info.K[3].xyz, info.K[1].xyz};
 	}
 	else
 	{
		v2 = p[2];
		K = {info.K[0].xyz, info.K[3].xyz, info.K[2].xyz};
 	}

 	//NOTE: compute barycentric matrix
	float det = v0.x*(v1.y-v2.y) + v1.x*(v2.y-v0.y) + v2.x*(v0.y - v1.y);
 	matrix_float3x3 B = {{v1.y - v2.y, v2.y-v0.y, v0.y-v1.y},
 	                     {v2.x - v1.x, v0.x-v2.x, v1.x-v0.x},
 	                     {v1.x*v2.y-v2.x*v1.y, v2.x*v0.y-v0.x*v2.y, v0.x*v1.y-v1.x*v0.y}};
 	B *= (1/det);

 	//NOTE: set implicit matrix and bin segment
 	seg->implicitMatrix = K*B;
	mtl_segment_bin_to_tiles(context, seg);
}

void mtl_cubic_setup(thread mtl_segment_setup_context* context, float2 p[4])
{
	float splits[8];
	int splitCount = mtl_cubic_monotonize(p, splits);

	//NOTE: produce bézier curve for each consecutive pair of roots
	for(int sliceIndex=0; sliceIndex<splitCount-1; sliceIndex++)
	{
		float2 sp[4];
		mtl_cubic_slice(p, splits[sliceIndex], splits[sliceIndex+1], sp);

		mtl_cubic_info curve = mtl_cubic_classify(sp);
		switch(curve.kind)
		{
			case MTL_CUBIC_ERROR:
				mtl_log(context->log, "cubic curve classification error\n");
				break;

			case MTL_CUBIC_DEGENERATE_LINE:
			{
				float2 l[2] = {p[0], p[1]};
				mtl_line_setup(context, l);
			} break;

			case MTL_CUBIC_DEGENERATE_QUADRATIC:
			{
				float2 q[3] = {p[0], curve.quadPoint, p[3]};
				mtl_quadratic_setup(context, q);
			} break;

			case MTL_CUBIC_LOOP_SPLIT:
			{
				//NOTE: split and reclassify, check that we have a valid loop and emit
				float2 ssp[8];
				mtl_cubic_slice(sp, 0, curve.split, ssp);
				mtl_cubic_slice(sp, curve.split, 1, ssp+4);

				for(int i=0; i<2; i++)
				{
					curve = mtl_cubic_classify(ssp + 4*i);

					if(curve.kind != MTL_CUBIC_LOOP_OK)
					{
						mtl_log(context->log, "loop split left error\n");
					}
					else
					{
						mtl_cubic_emit(context, ssp + 4*i, curve);
					}

				}
			} break;

			case MTL_CUBIC_LOOP_OK:
			case MTL_CUBIC_CUSP:
			case MTL_CUBIC_SERPENTINE:
			{
				if(sliceIndex == 2)
				{
					log_cubic_bezier(sp, context->log);
				}
				mtl_cubic_emit(context, sp, curve);
			} break;
		}
	}
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

                              device char* logBuffer [[buffer(9)]],
                              device atomic_int* logOffsetBuffer [[buffer(10)]],
                              uint eltIndex [[thread_position_in_grid]])
{
	const device mg_mtl_path_elt* elt = &elementBuffer[eltIndex];

	const device mg_mtl_path_queue* pathQueue = &pathQueueBuffer[elt->pathIndex];
	device mg_mtl_tile_queue* tileQueues = &tileQueueBuffer[pathQueue->tileQueues];
	mtl_segment_setup_context setupCtx = {.pathIndex = elt->pathIndex,
	                                      .segmentCount = segmentCount,
	                                      .segmentBuffer = segmentBuffer,
	                                      .pathQueue = pathQueue,
	                                      .tileQueues = tileQueues,
	                                      .tileOpBuffer = tileOpBuffer,
	                                      .tileOpCount = tileOpCount,
	                                      .tileSize = tileSize[0],
	                                      .log.buffer = logBuffer,
	                                      .log.offset = logOffsetBuffer};

	switch(elt->kind)
	{
		case MG_MTL_LINE:
		{
			float2 p[2] = {elt->p[0], elt->p[1]};
			mtl_line_setup(&setupCtx, p);
		} break;

		case MG_MTL_QUADRATIC:
		{
			float2 p[3] = {elt->p[0], elt->p[1], elt->p[2]};
			mtl_quadratic_setup(&setupCtx, p);
		} break;

		case MG_MTL_CUBIC:
		{
			float2 p[4] = {elt->p[0], elt->p[1], elt->p[2], elt->p[3]};
			mtl_cubic_setup(&setupCtx, p);

		} break;
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
