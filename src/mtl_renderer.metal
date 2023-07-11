
#include<metal_stdlib>
#include<simd/simd.h>
#include<metal_simdgroup>

#include"mtl_renderer.h"

using namespace metal;


typedef struct mtl_log_context
{
	device char* buffer;
	device volatile atomic_int* offset;
	bool enabled;
} mtl_log_context;

int strlen(const constant char* msg)
{
	int count = 0;
	while(msg[count] != '\0')
	{
		count++;
	}
	return(count);
}

int strlen(const thread char* msg)
{
	int count = 0;
	while(msg[count] != '\0')
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

		for(int i=0; i<len; i++)
		{
			context.buffer[offset+i] = msg[i];
		}
		context.buffer[offset+len] = '\0';
	}
}

void mtl_log(mtl_log_context context, const thread char* msg)
{
	if(context.enabled)
	{
		int len = strlen(msg);
		int offset = atomic_fetch_add_explicit(context.offset, len+1, memory_order_relaxed);

		for(int i=0; i<len; i++)
		{
			context.buffer[offset+i] = msg[i];
		}
		context.buffer[offset+len] = '\0';
	}
}

int mtl_itoa_right_aligned(int bufSize, thread char* buffer, int64_t value, bool zeroPad)
{
	// convert value to a null-terminated string at end of buffer and returns the size
	// (excluding the final null).
	bool minus = false;
	if(value < 0)
	{
		minus = true;
		value *= -1;
	}
	buffer[bufSize-1] = '\0';
	int index = bufSize-2;
	int stop = minus ? 1 : 0;

	do
	{
		buffer[index] = '0' + (value % 10);
		index--;
		value /= 10;
	} while(value != 0 && index >= stop);

	if(zeroPad)
	{
		while(index >= stop)
		{
			buffer[index] = '0';
			index--;
		}
	}

	if(minus)
	{
		buffer[index] = '-';
		index--;
	}

	int count = bufSize - (index+1);
	return(count - 1);
}

int mtl_itoa(int bufSize, thread char* buffer, int64_t value)
{
	int count = mtl_itoa_right_aligned(bufSize, buffer, value, false);
	int start = bufSize - (count+1);

	for(int i=0; i<count+1; i++)
	{
		buffer[i] = buffer[start+i];
	}
	return(count);
}

void mtl_log_i32(mtl_log_context context, int value)
{
	char buffer[12];
	mtl_itoa(12, buffer, value);
	mtl_log(context, buffer);
}

void mtl_log_f32(mtl_log_context context, float value)
{
	bool minus = false;
	if(value < 0)
	{
		minus = true;
		value *= -1;
	}

	int64_t integral = (int64_t)value;
	int64_t decimal = (int64_t)((value - (float)integral)*1e6);

	const int bufSize = 64;
	char buffer[bufSize];
	int index = 0;
	if(minus)
	{
		buffer[index] = '-';
		index++;
	}
	index += mtl_itoa(bufSize-index, buffer+index, integral);

	if(index < bufSize && decimal)
	{
		buffer[index] = '.';
		index++;

		int width = 6;
		while(decimal % 10 == 0 && width > 0)
		{
			decimal /= 10;
			width--;
		}

		int decSize = min(bufSize-index, width+1);
		mtl_itoa_right_aligned(decSize, buffer+index, decimal, true);
	}
	buffer[bufSize-1] = '\0';
	mtl_log(context, buffer);
}

void mtl_log_point(mtl_log_context context, float2 p)
{
	mtl_log(context, "(");
	mtl_log_f32(context, p.x);
	mtl_log(context, ", ");
	mtl_log_f32(context, p.y);
	mtl_log(context, ")");
}

void log_line(thread float2* p, mtl_log_context logCtx)
{
	mtl_log(logCtx, "(");
	mtl_log_f32(logCtx, p[0].x);
	mtl_log(logCtx, ", ");
	mtl_log_f32(logCtx, p[0].y);
	mtl_log(logCtx, ") (");
	mtl_log_f32(logCtx, p[1].x);
	mtl_log(logCtx, ", ");
	mtl_log_f32(logCtx, p[1].y);
	mtl_log(logCtx, ")\n");
}

void log_quadratic_bezier(thread float2* p, mtl_log_context logCtx)
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
	mtl_log(logCtx, ")\n");
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

kernel void mtl_path_setup(constant int* pathCount [[buffer(0)]],
                           const device mg_mtl_path* pathBuffer [[buffer(1)]],
                           device mg_mtl_path_queue* pathQueueBuffer [[buffer(2)]],
                           device mg_mtl_tile_queue* tileQueueBuffer [[buffer(3)]],
                           device atomic_int* tileQueueCount [[buffer(4)]],
                           constant int* tileSize [[buffer(5)]],
                           constant float* scale [[buffer(6)]],
                           uint pathIndex [[thread_position_in_grid]])
{
	const device mg_mtl_path* path = &pathBuffer[pathIndex];


	//NOTE: we don't clip on the right, since we need those tiles to accurately compute
	//      the prefix sum of winding increments in the backprop pass.
	float4 clippedBox = {max(path->box.x, path->clip.x),
	                     max(path->box.y, path->clip.y),
	                     path->box.z,
	                     min(path->box.w, path->clip.w)};

	int2 firstTile = int2(clippedBox.xy*scale[0])/tileSize[0];
	int2 lastTile = int2(clippedBox.zw*scale[0])/tileSize[0];

	int nTilesX = max(0, lastTile.x - firstTile.x + 1);
	int nTilesY = max(0, lastTile.y - firstTile.y + 1);
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

float ccw(float2 a, float2 b, float2 c)
{
	return((b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x));
}

int mtl_side_of_segment(float2 p, const device mg_mtl_segment* seg, mtl_log_context log = {.enabled = false})
{
	int side = 0;
	if(p.y > seg->box.w || p.y <= seg->box.y)
	{
		if(p.x > seg->box.x && p.x <= seg->box.z)
		{
			if(p.y > seg->box.w)
			{
				side = (seg->config == MG_MTL_TL || seg->config == MG_MTL_BR)? -1 : 1;
			}
			else
			{
				side = (seg->config == MG_MTL_TL || seg->config == MG_MTL_BR)? 1 : -1;
			}
		}
	}
	else if(p.x > seg->box.z)
	{
		side = 1;
	}
	else if(p.x <= seg->box.x)
	{
		side = -1;
	}
	else
	{
		float2 a, b, c;
		switch(seg->config)
		{
			case MG_MTL_TL:
				a = seg->box.xy;
				b = seg->box.zw;
				break;

			case MG_MTL_BR:
				a = seg->box.zw;
				b = seg->box.xy;
				break;

			case MG_MTL_TR:
				a = seg->box.xw;
				b = seg->box.zy;
				break;

			case MG_MTL_BL:
				a = seg->box.zy;
				b = seg->box.xw;
				break;
		}
		c = seg->hullVertex;

		if(ccw(a, b, p) < 0)
		{
			// other side of the diagonal
			side = (seg->config == MG_MTL_BR || seg->config == MG_MTL_TR) ? -1 : 1;
		}
		else if(ccw(b, c, p) < 0 || ccw(c, a, p) < 0)
		{
			// same side of the diagonal, but outside curve hull
			side = (seg->config == MG_MTL_BL || seg->config == MG_MTL_TL) ? -1 : 1;
		}
		else
		{
			// inside curve hull
			switch(seg->kind)
			{
				case MG_MTL_LINE:
					side = 1;
					break;

				case MG_MTL_QUADRATIC:
				{
					float3 ph = {p.x, p.y, 1};
					float3 klm = seg->implicitMatrix * ph;
					side = ((klm.x*klm.x - klm.y)*klm.z < 0)? -1 : 1;
				} break;

				case MG_MTL_CUBIC:
				{
					float3 ph = {p.x, p.y, 1};
					float3 klm = seg->implicitMatrix * ph;
					side = (seg->sign*(klm.x*klm.x*klm.x - klm.y*klm.z) < 0)? -1 : 1;
				} break;
			}
		}
	}
	return(side);
}


typedef struct mtl_segment_setup_context
{
	device atomic_int* segmentCount;
	device mg_mtl_segment* segmentBuffer;
	const device mg_mtl_path_queue* pathQueue;
	device mg_mtl_tile_queue* tileQueues;
	device mg_mtl_tile_op* tileOpBuffer;
	device atomic_int* tileOpCount;
	int tileSize;
	mtl_log_context log;

	int pathIndex;

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

			float2 bl = {tileBox.x, tileBox.y};
			float2 br = {tileBox.z, tileBox.y};
			float2 tr = {tileBox.z, tileBox.w};
			float2 tl = {tileBox.x, tileBox.w};

			int sbl = mtl_side_of_segment(bl, seg, context->log);
			int sbr = mtl_side_of_segment(br, seg, context->log);
			int str = mtl_side_of_segment(tr, seg, context->log);
			int stl = mtl_side_of_segment(tl, seg, context->log);

			bool crossL = (stl*sbl < 0);
			bool crossR = (str*sbr < 0);
			bool crossT = (stl*str < 0);
			bool crossB = (sbl*sbr < 0);

			float2 s0, s1;
			if(seg->config == MG_MTL_TL||seg->config == MG_MTL_BR)
			{
				s0 = seg->box.xy;
				s1 = seg->box.zw;
			}
			else
			{
				s0 = seg->box.xw;
				s1 = seg->box.zy;
			}
			bool s0Inside =  s0.x >= tileBox.x
			              && s0.x < tileBox.z
			              && s0.y >= tileBox.y
			              && s0.y < tileBox.w;

			bool s1Inside =  s1.x >= tileBox.x
			              && s1.x < tileBox.z
			              && s1.y >= tileBox.y
			              && s1.y < tileBox.w;

			if(crossL || crossR || crossT || crossB || s0Inside || s1Inside)
			{
				int tileOpIndex = atomic_fetch_add_explicit(context->tileOpCount, 1, memory_order_relaxed);
				device mg_mtl_tile_op* op = &context->tileOpBuffer[tileOpIndex];

				op->kind = MG_MTL_OP_SEGMENT;
				op->index = segIndex;
				op->crossRight = false;
				op->next = -1;

				int tileIndex = y*pathArea.z + x;
				device mg_mtl_tile_queue* tile = &context->tileQueues[tileIndex];
				op->next = atomic_exchange_explicit(&tile->first, tileOpIndex, memory_order_relaxed);
				if(op->next == -1)
				{
					tile->last = tileOpIndex;
				}

				//NOTE: if the segment crosses the tile's bottom boundary, update the tile's winding offset
				if(crossB)
				{
					mtl_log(context->log, "cross bottom boundary, increment ");
					mtl_log_f32(context->log, seg->windingIncrement);
					mtl_log(context->log, "\n");
					atomic_fetch_add_explicit(&tile->windingOffset, seg->windingIncrement, memory_order_relaxed);
				}

				//NOTE: if the segment crosses the right boundary, mark it. We reuse one of the previous tests
				if(crossR)
				{
					op->crossRight = true;
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
		{
			s = p[0];
			float sqrNorm0 = length_squared(p[1]-p[0]);
			float sqrNorm1 = length_squared(p[3]-p[2]);
			if(sqrNorm0 < sqrNorm1)
			{
				c = p[2];
			}
			else
			{
				c = p[1];
			}
			e = p[3];
		} break;
	}

	int segIndex = atomic_fetch_add_explicit(context->segmentCount, 1, memory_order_relaxed);
	device mg_mtl_segment* seg = &context->segmentBuffer[segIndex];

	bool goingUp = e.y >= s.y;
	bool goingRight = e.x >= s.x;

	seg->kind = kind;
	seg->pathIndex = context->pathIndex;
	seg->windingIncrement = goingUp? 1 : -1;

	seg->box = (vector_float4){min(s.x, e.x),
	                           min(s.y, e.y),
	                           max(s.x, e.x),
	                           max(s.y, e.y)};

	float dx = c.x - seg->box.x;
	float dy = c.y - seg->box.y;
	float alpha = (seg->box.w - seg->box.y)/(seg->box.z - seg->box.x);
	float ofs = seg->box.w - seg->box.y;

	if(goingUp == goingRight)
	{
		if(seg->kind == MG_MTL_LINE)
		{
			seg->config = MG_MTL_BR;
		}
		else if(dy > alpha*dx)
		{
			seg->config = MG_MTL_TL;
		}
		else
		{
			seg->config = MG_MTL_BR;
		}
	}
	else
	{
		if(seg->kind == MG_MTL_LINE)
		{
			seg->config = MG_MTL_TR;
		}
		else if(dy < ofs - alpha*dx)
		{
			seg->config = MG_MTL_BL;
		}
		else
		{
			seg->config = MG_MTL_TR;
		}
	}
	return(seg);
}

#define square(x) ((x)*(x))
#define cube(x) ((x)*(x)*(x))

void mtl_line_setup(thread mtl_segment_setup_context* context, float2 p[2])
{
	device mg_mtl_segment* seg = mtl_segment_push(context, p, MG_MTL_LINE);
	seg->hullVertex = p[0];
	mtl_segment_bin_to_tiles(context, seg);
}

float2 mtl_quadratic_blossom(float2 p[3], float u, float v)
{
	float2 b10 = u*p[1] + (1-u)*p[0];
	float2 b11 = u*p[2] + (1-u)*p[1];
	float2 b20 = v*b11 + (1-v)*b10;
	return(b20);
}

void mtl_quadratic_slice(float2 p[3], float s0, float s1, float2 sp[3])
{
	/*NOTE: using blossoms to compute sub-curve control points ensure that the fourth point
	        of sub-curve (s0, s1) and the first point of sub-curve (s1, s3) match.
	        However, due to numerical errors, the evaluation of B(s=0) might not be equal to
	        p[0] (and likewise, B(s=1) might not equal p[3]).
	        We handle that case explicitly to ensure that we don't create gaps in the paths.
	*/
	sp[0] = (s0 == 0) ? p[0] : mtl_quadratic_blossom(p, s0, s0);
	sp[1] = mtl_quadratic_blossom(p, s0, s1);
	sp[2] = (s1 == 1) ? p[2] : mtl_quadratic_blossom(p, s1, s1);
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

matrix_float3x3 mtl_barycentric_matrix(float2 v0, float2 v1, float2 v2)
{
	float det = v0.x*(v1.y-v2.y) + v1.x*(v2.y-v0.y) + v2.x*(v0.y - v1.y);
 	matrix_float3x3 B = {{v1.y - v2.y, v2.y-v0.y, v0.y-v1.y},
 	                     {v2.x - v1.x, v0.x-v2.x, v1.x-v0.x},
 	                     {v1.x*v2.y-v2.x*v1.y, v2.x*v0.y-v0.x*v2.y, v0.x*v1.y-v1.x*v0.y}};
 	B *= (1/det);
 	return(B);
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

	seg->hullVertex = p[1];

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

/*
  diff_of_products() computes a*b-c*d with a maximum error <= 1.5 ulp

  Claude-Pierre Jeannerod, Nicolas Louvet, and Jean-Michel Muller,
  "Further Analysis of Kahan's Algorithm for the Accurate Computation
  of 2x2 Determinants". Mathematics of Computation, Vol. 82, No. 284,
  Oct. 2013, pp. 2245-2264
*/
float diff_of_products (float a, float b, float c, float d)
{
    float w = d * c;
    float e = fma(-d, c, w);
    float f = fma(a, b, -w);
    return(f + e);
}

int mtl_quadratic_roots_with_det(float a, float b, float c, float det, thread float* r, mtl_log_context log = {.enabled = false})
{
	int count = 0;

	if(a == 0)
	{
		if(b)
		{
			count = 1;
			r[0] = -c/b;
		}
	}
	else
	{
		b /= 2.0;

		if(det >= 0)
		{
			count = (det == 0) ? 1 : 2;

			if(b > 0)
			{
				float q = b + sqrt(det);
				r[0] = -c/q;
				r[1] = -q/a;
			}
			else if(b < 0)
			{
				float q = -b + sqrt(det);
				r[0] = q/a;
				r[1] = c/q;
			}
			else
			{
				float q = sqrt(-a*c);
				if(fabs(a) >= fabs(c))
				{
					r[0] = q/a;
					r[1] = -q/a;
				}
				else
				{
					r[0] = -c/q;
					r[1] = c/q;
				}
			}
		}
	}
	if(count>1 && r[0] > r[1])
	{
		float tmp = r[0];
		r[0] = r[1];
		r[1] = tmp;
	}
	return(count);
}

int mtl_quadratic_roots(float a, float b, float c, thread float* r, mtl_log_context log = {.enabled = false})
{
	//float det = diff_of_products(b, b, a, c);
	float det = square(b)/4. - a*c;
	return(mtl_quadratic_roots_with_det(a, b, c, det, r, log));
}

float2 mtl_cubic_blossom(float2 p[4], float u, float v, float w)
{
	float2 b10 = u*p[1] + (1-u)*p[0];
	float2 b11 = u*p[2] + (1-u)*p[1];
	float2 b12 = u*p[3] + (1-u)*p[2];
	float2 b20 = v*b11 + (1-v)*b10;
	float2 b21 = v*b12 + (1-v)*b11;
	float2 b30 = w*b21 + (1-w)*b20;
	return(b30);
}

void mtl_cubic_slice(float2 p[4], float s0, float s1, float2 sp[4])
{
	/*NOTE: using blossoms to compute sub-curve control points ensure that the fourth point
	        of sub-curve (s0, s1) and the first point of sub-curve (s1, s3) match.
	        However, due to numerical errors, the evaluation of B(s=0) might not be equal to
	        p[0] (and likewise, B(s=1) might not equal p[3]).
	        We handle that case explicitly to ensure that we don't create gaps in the paths.
	*/
	sp[0] = (s0 == 0) ? p[0] : mtl_cubic_blossom(p, s0, s0, s0);
	sp[1] = mtl_cubic_blossom(p, s0, s0, s1);
	sp[2] = mtl_cubic_blossom(p, s0, s1, s1);
	sp[3] = (s1 == 1) ? p[3] : mtl_cubic_blossom(p, s1, s1, s1);
}

typedef enum {
	MTL_CUBIC_ERROR,
	MTL_CUBIC_SERPENTINE,
	MTL_CUBIC_CUSP,
	MTL_CUBIC_CUSP_INFINITY,
	MTL_CUBIC_LOOP,
	MTL_CUBIC_DEGENERATE_QUADRATIC,
	MTL_CUBIC_DEGENERATE_LINE,

} mtl_cubic_kind;

typedef struct mtl_cubic_info
{
	mtl_cubic_kind kind;
	matrix_float4x4 K;
	float2 ts[2];
	float d1;
	float d2;
	float d3;

} mtl_cubic_info;

mtl_cubic_info mtl_cubic_classify(thread float2* c, mtl_log_context log = {.enabled = false})
{
	mtl_cubic_info result = {MTL_CUBIC_ERROR};
	matrix_float4x4 F;

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

		//WARN: there seems to be a mismatch between the signs of the d_i and the orientation test in the Loop-Blinn paper?
		//      flipping the sign of the d_i doesn't change the roots (and the implicit matrix), but it does change the orientation.
		//      Keeping the signs of the paper puts the interior on the left of parametric travel, unlike what's stated in the paper.
		//      this may very well be an error on my part that's cancelled by flipping the signs of the d_i though!
	*/

	float d1 = -(c[3].y*c[2].x - c[3].x*c[2].y);
	float d2 = -(c[3].x*c[1].y - c[3].y*c[1].x);
	float d3 = -(c[2].y*c[1].x - c[2].x*c[1].y);

	result.d1 = d1;
	result.d2 = d2;
	result.d3 = d3;

//	mtl_log(log, "d1 = ");
/*	mtl_log_f32(log, d1);
	mtl_log(log, ", d2 = ");
	mtl_log_f32(log, d2);
	mtl_log(log, ", d3 = ");
	mtl_log_f32(log, d3);
	mtl_log(log, "\n");
*/
	//NOTE(martin): compute the second factor of the discriminant discr(I) = d1^2*(3*d2^2 - 4*d3*d1)
	float discrFactor2 = 3.0*square(d2) - 4.0*d3*d1;

	//NOTE(martin): each following case gives the number of roots, hence the category of the parametric curve
	if(fabs(d1) <= 1e-6 && fabs(d2) <= 1e-6 && fabs(d3) > 1e-6)
	{
		//NOTE(martin): quadratic degenerate case
		//NOTE(martin): compute quadratic curve control point, which is at p0 + 1.5*(p1-p0) = 1.5*p1 - 0.5*p0
		result.kind = MTL_CUBIC_DEGENERATE_QUADRATIC;
	}
	else if( (discrFactor2 > 0 && fabs(d1) > 1e-6)
	       ||(discrFactor2 == 0 && fabs(d1) > 1e-6))
	{
		//mtl_log(log, "cusp or serpentine\n");

		//NOTE(martin): serpentine curve or cusp with inflection at infinity
		//              (these two cases are handled the same way).
		//NOTE(martin): compute the solutions (tl, sl), (tm, sm), and (tn, sn) of the inflection point equation
		float tmtl[2];
		mtl_quadratic_roots_with_det(1, -2*d2, (4./3.*d1*d3), (1./3.)*discrFactor2, tmtl);

		float tm = tmtl[0];
		float sm = 2*d1;
		float tl = tmtl[1];
		float sl = 2*d1;

		float invNorm = 1/sqrt(square(tm) + square(sm));
		tm *= invNorm;
		sm *= invNorm;

		invNorm = 1/sqrt(square(tl) + square(sl));
		tl *= invNorm;
		sl *= invNorm;

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

		result.ts[0] = (float2){tm, sm};
		result.ts[1] = (float2){tl, sl};
	}
	else if(discrFactor2 < 0 && fabs(d1) > 1e-6)
	{
//		mtl_log(log, "loop\n");

		//NOTE(martin): loop curve
		result.kind = MTL_CUBIC_LOOP;

		float tetd[2];
		mtl_quadratic_roots_with_det(1, -2*d2, 4*(square(d2)-d1*d3), -discrFactor2, tetd, log);

		float td = tetd[1];
		float sd = 2*d1;
		float te = tetd[0];
		float se = 2*d1;

		float invNorm = 1/sqrt(square(td) + square(sd));
		td *= invNorm;
		sd *= invNorm;

		invNorm = 1/sqrt(square(te) + square(se));
		te *= invNorm;
		se *= invNorm;

		//NOTE(martin): if one of the parameters (td/sd) or (te/se) is in the interval [0,1], the double point
		//              is inside the control points convex hull and would cause a shading anomaly. If this is
		//              the case, subdivide the curve at that point
/*
		mtl_log(log, "td = ");
		mtl_log_f32(log, td);
		mtl_log(log, ", sd = ");
		mtl_log_f32(log, sd);
		mtl_log(log, ", te = ");
		mtl_log_f32(log, te);
		mtl_log(log, ", se = ");
		mtl_log_f32(log, td);
		mtl_log(log, ", td/sd = ");
		mtl_log_f32(log, td/sd);
		mtl_log(log, ", te/se = ");
		mtl_log_f32(log, te/se);
		mtl_log(log, "\n");
//*/
		/*NOTE(martin):
			the power basis coefficients of points k,l,m,n are collected into the rows of the 4x4 matrix F:

				| td*te            td^2*te                 td*te^2                1 |
				| -se*td - sd*te   -se*td^2 - 2sd*te*td    -sd*te^2 - 2*se*td*te  0 |
				| sd*se            te*sd^2 + 2*se*td*sd    td*se^2 + 2*sd*te*se   0 |
				| 0                -sd^2*se                -sd*se^2               0 |
		*/
		F = (matrix_float4x4){{td*te, -se*td-sd*te, sd*se, 0},
			                     {square(td)*te, -se*square(td)-2*sd*te*td, te*square(sd)+2*se*td*sd, -square(sd)*se},
			                     {td*square(te), -sd*square(te)-2*se*td*te, td*square(se)+2*sd*te*se, -sd*square(se)},
			                     {1, 0, 0, 0}};

		result.ts[0] = (float2){td, sd};
		result.ts[1] = (float2){te, se};
	}
	else if(d2 != 0)
	{
		//NOTE(martin): cusp with cusp at infinity

//		mtl_log(log, "cusp at infinity\n");

		float tl = d3;
		float sl = 3*d2;

		float invNorm = 1/sqrt(square(tl)+square(sl));
		tl *= invNorm;
		sl *= invNorm;

		/*NOTE(martin):
			the power basis coefficients of points k,l,m,n are collected into the rows of the 4x4 matrix F:

				| tl    tl^3        1  1 |
				| -sl   -3sl*tl^2   0  0 |
				| 0     3*sl^2*tl   0  0 |
				| 0     -sl^3       0  0 |
		*/
		result.kind = MTL_CUBIC_CUSP_INFINITY;

		F = (matrix_float4x4){{tl, -sl, 0, 0},
		                      {cube(tl), -3*sl*square(tl), 3*square(sl)*tl, -cube(sl)},
		                      {1, 0, 0, 0},
		                      {1, 0, 0, 0}};

		result.ts[0] = (float2){tl, sl};
		result.ts[1] = (float2){0, 0};
	}
	else
	{
		//NOTE(martin): line or point degenerate case
		result.kind = MTL_CUBIC_DEGENERATE_LINE;
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

float2 mtl_select_hull_vertex(float2 p0, float2 p1, float2 p2, float2 p3, mtl_log_context log)
{
	/*NOTE: check intersection of lines (p1-p0) and (p3-p2)
		P = p0 + u(p1-p0)
		P = p2 + w(p3-p2)

		control points are inside a right triangle so we should always find an intersection
	*/
	float2 pm;

	float det = (p1.x - p0.x)*(p3.y - p2.y) - (p1.y - p0.y)*(p3.x - p2.x);
	float sqrNorm0 = length_squared(p1-p0);
	float sqrNorm1 = length_squared(p2-p3);

	if(fabs(det) < 1e-3 || sqrNorm0 < 0.1 || sqrNorm1 < 0.1)
	{
		if(sqrNorm0 < sqrNorm1)
		{
			pm = p2;
		}
		else
		{
			pm = p1;
		}
	}
	else
	{
		float u = ((p0.x - p2.x)*(p2.y - p3.y) - (p0.y - p2.y)*(p2.x - p3.x))/det;
		pm = p0 + u*(p1-p0);
	}
	return(pm);
}

void mtl_cubic_emit(thread mtl_segment_setup_context* context, mtl_cubic_info curve, float2 p[4], float s0, float s1, float2 sp[4])
{
	device mg_mtl_segment* seg = mtl_segment_push(context, sp, MG_MTL_CUBIC);

	float2 v0 = p[0];
	float2 v1 = p[3];
	float2 v2;
	matrix_float3x3 K;

	float sqrNorm0 = length_squared(p[1]-p[0]);
	float sqrNorm1 = length_squared(p[2]-p[3]);

	//TODO: should not be the local sub-curve, but the global curve!!!
	if(length_squared(p[0]-p[3]) > 1e-5)
	{
		if(sqrNorm0 >= sqrNorm1)
 		{
 			v2 = p[1];
			K = {curve.K[0].xyz, curve.K[3].xyz, curve.K[1].xyz};
 		}
 		else
 		{
			v2 = p[2];
			K = {curve.K[0].xyz, curve.K[3].xyz, curve.K[2].xyz};
 		}
 	}
 	else
 	{
		v1 = p[1];
		v2 = p[2];
		K = {curve.K[0].xyz, curve.K[1].xyz, curve.K[2].xyz};
 	}
 	//NOTE: set matrices

 	//TODO: should we compute matrix relative to a base point to avoid loss of precision
 	//      when computing barycentric matrix?

	matrix_float3x3 B = mtl_barycentric_matrix(v0, v1, v2);
 	seg->implicitMatrix = K*B;
	seg->hullVertex = mtl_select_hull_vertex(sp[0], sp[1], sp[2], sp[3], context->log);

  	//NOTE: compute sign flip
  	seg->sign = 1;

  	if(curve.kind == MTL_CUBIC_SERPENTINE
  	|| curve.kind == MTL_CUBIC_CUSP)
  	{
		seg->sign = (curve.d1 < 0)? -1 : 1;
	}
	else if(curve.kind == MTL_CUBIC_LOOP)
	{
		float d1 = curve.d1;
		float d2 = curve.d2;
		float d3 = curve.d3;

		float H0 = d3*d1-square(d2) + d1*d2*s0 - square(d1)*square(s0);
		float H1 = d3*d1-square(d2) + d1*d2*s1 - square(d1)*square(s1);
		float H = (abs(H0) > abs(H1)) ? H0 : H1;
		seg->sign = (H*d1 > 0) ? -1 : 1;
	}

	if(sp[3].y > sp[0].y)
	{
		seg->sign *= -1;
	}

	//NOTE: bin to tiles
	mtl_segment_bin_to_tiles(context, seg);
}

void mtl_cubic_setup(thread mtl_segment_setup_context* context, float2 p[4])
{
	/*NOTE(martin): first convert the control points to power basis, multiplying by M3

		     | 1  0  0  0|      |p0|      |c0|
		M3 = |-3  3  0  0|, B = |p1|, C = |c1| = M3*B
		     | 3 -6  3  0|      |p2|      |c2|
		     |-1  3 -3  1|      |p3|      |c3|
	*/
	float2 c[4] = {
		p[0],
	    3.0*(p[1] - p[0]),
	    3.0*(p[0] + p[2] - 2*p[1]),
	    3.0*(p[1] - p[2]) + p[3] - p[0]};

/*
	mtl_log(context->log, "bezier basis: ");
	log_cubic_bezier(p, context->log);

	mtl_log(context->log, "power basis: ");
	log_cubic_bezier(c, context->log);
*/
	//NOTE: get classification, implicit matrix, double points and inflection points
	mtl_cubic_info curve = mtl_cubic_classify(c, context->log);

	if(curve.kind == MTL_CUBIC_DEGENERATE_LINE)
	{
		float2 l[2] = {p[0], p[3]};
		mtl_line_setup(context, l);
		return;
	}
	else if(curve.kind == MTL_CUBIC_DEGENERATE_QUADRATIC)
	{
		float2 quadPoint = float2(1.5*p[1].x - 0.5*p[0].x, 1.5*p[1].y - 0.5*p[0].y);
		float2 q[3] = {p[0], quadPoint, p[3]};
		mtl_quadratic_setup(context, q);
		return;
	}

	//NOTE: get the roots of B'(s) = 3.c3.s^2 + 2.c2.s + c1
	float roots[6];
	int rootCount = mtl_quadratic_roots(3*c[3].x, 2*c[2].x, c[1].x, roots);
	rootCount += mtl_quadratic_roots(3*c[3].y, 2*c[2].y, c[1].y, roots + rootCount);

	//NOTE: add double points and inflection points to roots if finite
	for(int i=0; i<2; i++)
	{
		if(curve.ts[i].y)
		{
			roots[rootCount] = curve.ts[i].x / curve.ts[i].y;
			rootCount++;
		}
	}

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
	float splits[8];
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

	mtl_log(context->log, "monotonic segment count = ");
	mtl_log_i32(context->log, splitCount-1);
	mtl_log(context->log, "\n");

	//NOTE: for each monotonic segment, compute hull matrix and sign, and emit segment
	for(int sliceIndex=0; sliceIndex<splitCount-1; sliceIndex++)
	{
		float s0 = splits[sliceIndex];
		float s1 = splits[sliceIndex+1];
		float2 sp[4];
		mtl_cubic_slice(p, s0, s1, sp);

		/////////////////////// we should ensure that these have the same endpoints as the original curve and all endpoints are joining

		mtl_log(context->log, "monotonic slice ");
		mtl_log_i32(context->log, sliceIndex);
		mtl_log(context->log, " ( ");
		mtl_log_f32(context->log, s0);
		mtl_log(context->log, " <= s <= ");
		mtl_log_f32(context->log, s1);
		mtl_log(context->log," ): ");
		log_cubic_bezier(sp, context->log);


		mtl_cubic_emit(context, curve, p, s0, s1, sp);
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
                              constant float* scale [[buffer(9)]],

                              device char* logBuffer [[buffer(10)]],
                              device atomic_int* logOffsetBuffer [[buffer(11)]],
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
	                                      .log.offset = logOffsetBuffer,
	                                      .log.enabled = false};

	switch(elt->kind)
	{
		case MG_MTL_LINE:
		{
			float2 p[2] = {elt->p[0]*scale[0], elt->p[1]*scale[0]};
			mtl_log(setupCtx.log, "line: ");
			log_line(p, setupCtx.log);
			mtl_line_setup(&setupCtx, p);
		} break;

		case MG_MTL_QUADRATIC:
		{
			float2 p[3] = {elt->p[0]*scale[0], elt->p[1]*scale[0], elt->p[2]*scale[0]};
			mtl_log(setupCtx.log, "quadratic: ");
			log_quadratic_bezier(p, setupCtx.log);
			mtl_quadratic_setup(&setupCtx, p);
		} break;

		case MG_MTL_CUBIC:
		{
			float2 p[4] = {elt->p[0]*scale[0], elt->p[1]*scale[0], elt->p[2]*scale[0], elt->p[3]*scale[0]};
			mtl_log(setupCtx.log, "cubic: ");
			log_cubic_bezier(p, setupCtx.log);
			mtl_cubic_setup(&setupCtx, p);

		} break;
	}
}

kernel void mtl_backprop(const device mg_mtl_path_queue* pathQueueBuffer [[buffer(0)]],
                         device mg_mtl_tile_queue* tileQueueBuffer [[buffer(1)]],
                         device char* logBuffer [[buffer(2)]],
                         device atomic_int* logOffsetBuffer [[buffer(3)]],
                         uint pathIndex [[threadgroup_position_in_grid]],
                         uint localID [[thread_position_in_threadgroup]])
{
//	mtl_log_context log = {.buffer = logBuffer, .offset = logOffsetBuffer, .enabled = false};

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
                      device MTLDispatchThreadgroupsIndirectArguments* dispatchBuffer [[buffer(6)]],
                      device mg_mtl_screen_tile* screenTilesBuffer [[buffer(7)]],
                      constant int* tileSize [[buffer(8)]],
                      constant float* scale [[buffer(9)]],
                      device char* logBuffer [[buffer(10)]],
                      device atomic_int* logOffsetBuffer [[buffer(11)]],
                      uint2 threadCoord [[thread_position_in_grid]],
                      uint2 gridSize [[threads_per_grid]])
{
	int2 tileCoord = int2(threadCoord);
	int tileIndex = -1;
	device int* nextLink = 0;

/*
	mtl_log_context log = {.buffer = logBuffer,
	                       .offset = logOffsetBuffer,
	                       .enabled = true};
*/
	dispatchBuffer[0].threadgroupsPerGrid[1] = 1;
	dispatchBuffer[0].threadgroupsPerGrid[2] = 1;

	for(int pathIndex = 0; pathIndex < pathCount[0]; pathIndex++)
	{
		const device mg_mtl_path_queue* pathQueue = &pathQueueBuffer[pathIndex];
		int2 pathTileCoord = tileCoord - pathQueue->area.xy;

		const device mg_mtl_path* path = &pathBuffer[pathIndex];
		float xMax = min(path->box.z, path->clip.z);
		int tileMaxX = xMax * scale[0] / tileSize[0];
		int pathTileMaxX = tileMaxX - pathQueue->area.x;

		if(  pathTileCoord.x >= 0
		  && pathTileCoord.x <= pathTileMaxX
		  && pathTileCoord.y >= 0
		  && pathTileCoord.y < pathQueue->area.w)
		{
			if(tileIndex < 0)
			{
				tileIndex = atomic_fetch_add_explicit((device atomic_uint*)&dispatchBuffer[0].threadgroupsPerGrid[0], 1, memory_order_relaxed);
				screenTilesBuffer[tileIndex].tileCoord = uint2(tileCoord);
				nextLink = &screenTilesBuffer[tileIndex].first;
				*nextLink = -1;
			}

			int pathTileIndex = pathTileCoord.y * pathQueue->area.z + pathTileCoord.x;
			const device mg_mtl_tile_queue* tileQueue = &tileQueueBuffer[pathQueue->tileQueues + pathTileIndex];

			int windingOffset = atomic_load_explicit(&tileQueue->windingOffset, memory_order_relaxed);
			int firstOpIndex = atomic_load_explicit(&tileQueue->first, memory_order_relaxed);

			float4 tileBox = float4(tileCoord.x, tileCoord.y, tileCoord.x+1, tileCoord.y+1);
			tileBox *= tileSize[0];
			float4 clip = pathBuffer[pathIndex].clip * scale[0];

			if(  tileBox.x >= clip.z
			  || tileBox.z < clip.x
			  || tileBox.y >= clip.w
			  || tileBox.w < clip.y)
			{
				//NOTE: tile is fully outside clip, cull it
			}
			else if(firstOpIndex == -1)
			{
				if(windingOffset & 1)
				{
					//NOTE: tile is full covered. Add path start op (with winding offset).
					//      Additionally if color is opaque and tile is fully inside clip, trim tile list.
					int pathOpIndex = atomic_fetch_add_explicit(tileOpCount, 1, memory_order_relaxed);
					device mg_mtl_tile_op* pathOp = &tileOpBuffer[pathOpIndex];
					pathOp->kind = MG_MTL_OP_CLIP_FILL;
					pathOp->next = -1;
					pathOp->index = pathIndex;
					pathOp->windingOffset = windingOffset;

					*nextLink = pathOpIndex;

					if(tileBox.x >= clip.x
					  && tileBox.z < clip.z
					  && tileBox.y >= clip.y
					  && tileBox.w < clip.w)
					{
						pathOp->kind = MG_MTL_OP_FILL;

						if(pathBuffer[pathIndex].color.a == 1)
						{
							screenTilesBuffer[tileIndex].first = pathOpIndex;
						}
					}
					nextLink = &pathOp->next;
				}
				// else, tile is fully uncovered, skip path
			}
			else
			{
				//NOTE: add path start op (with winding offset)
				int startOpIndex = atomic_fetch_add_explicit(tileOpCount, 1, memory_order_relaxed);
				device mg_mtl_tile_op* startOp = &tileOpBuffer[startOpIndex];
				startOp->kind = MG_MTL_OP_START;
				startOp->next = -1;
				startOp->index = pathIndex;
				startOp->windingOffset = windingOffset;

				*nextLink = startOpIndex;
				nextLink = &startOp->next;

				//NOTE: chain remaining path ops to end of tile list
				int lastOpIndex = tileQueue->last;
				device mg_mtl_tile_op* lastOp = &tileOpBuffer[lastOpIndex];
				*nextLink = firstOpIndex;
				nextLink = &lastOp->next;


				//NOTE: add path end op
				int endOpIndex = atomic_fetch_add_explicit(tileOpCount, 1, memory_order_relaxed);
				device mg_mtl_tile_op* endOp = &tileOpBuffer[endOpIndex];
				endOp->kind = MG_MTL_OP_END;
				endOp->next = -1;
				endOp->index = pathIndex;

				*nextLink = endOpIndex;
				nextLink = &endOp->next;

			}
		}
	}
}

kernel void mtl_raster(const device mg_mtl_screen_tile* screenTilesBuffer [[buffer(0)]],
                       const device mg_mtl_tile_op* tileOpBuffer [[buffer(1)]],
                       const device mg_mtl_path* pathBuffer [[buffer(2)]],
                       const device mg_mtl_segment* segmentBuffer [[buffer(3)]],
                       constant int* tileSize [[buffer(4)]],
                       constant float* scale [[buffer(5)]],
                       constant int* sampleCountBuffer [[buffer(6)]],
                       device char* logBuffer [[buffer(7)]],
                       device atomic_int* logOffsetBuffer [[buffer(8)]],
                       constant int* useTexture [[buffer(9)]],
                       texture2d<float, access::write> outTexture [[texture(0)]],
                       texture2d<float> srcTexture [[texture(1)]],
                       uint2  threadGroupCoord [[threadgroup_position_in_grid]],
                       uint2 localCoord [[thread_position_in_threadgroup]])
{
/*
	mtl_log_context log = {.buffer = logBuffer,
	                       .offset = logOffsetBuffer,
	                       .enabled = true};
*/
	int tileIndex = int(threadGroupCoord.x);
	uint2 tileCoord = screenTilesBuffer[tileIndex].tileCoord;
	uint2 pixelCoord = tileCoord*tileSize[0] + localCoord.xy;

	int opIndex = screenTilesBuffer[tileIndex].first;

	const int MG_MTL_MAX_SAMPLE_COUNT = 8;
	float2 sampleCoords[MG_MTL_MAX_SAMPLE_COUNT];
	int sampleCount = sampleCountBuffer[0];
	float2 centerCoord = float2(pixelCoord) + float2(0.5, 0.5);

	if(sampleCount == 8)
	{
		sampleCount = 8;
		sampleCoords[0] = centerCoord + float2(1, 3)/16;
		sampleCoords[1] = centerCoord + float2(-1, -3)/16;
		sampleCoords[2] = centerCoord + float2(5, -1)/16;
		sampleCoords[3] = centerCoord + float2(-3, 5)/16;
		sampleCoords[4] = centerCoord + float2(-5, -5)/16;
		sampleCoords[5] = centerCoord + float2(-7, 1)/16;
		sampleCoords[6] = centerCoord + float2(3, -7)/16;
		sampleCoords[7] = centerCoord + float2(7, 7)/16;
	}
	else
	{
		sampleCount = 1;
		sampleCoords[0] = centerCoord;
	}

	const int MG_MTL_MAX_SRC_SAMPLE_COUNT = 4;
	const int srcSampleCount = 2;

	const float2 imgSampleCoords[MG_MTL_MAX_SRC_SAMPLE_COUNT] = {
		centerCoord + float2(-0.25, 0.25),
	    centerCoord + float2(+0.25, +0.25),
	    centerCoord + float2(+0.25, -0.25),
	    centerCoord + float2(-0.25, +0.25)};

	float4 color = {0};
	int winding[MG_MTL_MAX_SAMPLE_COUNT] = {0};

	while(opIndex != -1)
	{
		const device mg_mtl_tile_op* op = &tileOpBuffer[opIndex];
		int pathIndex = op->index;

		if(op->kind == MG_MTL_OP_START)
		{
			for(int sampleIndex=0; sampleIndex<sampleCount; sampleIndex++)
			{
				winding[sampleIndex] = op->windingOffset;
			}
		}
		else if(op->kind == MG_MTL_OP_SEGMENT)
		{
			const device mg_mtl_segment* seg = &segmentBuffer[op->index];

			for(int sampleIndex=0; sampleIndex<sampleCount; sampleIndex++)
			{
				float2 sampleCoord = sampleCoords[sampleIndex];

				if( (sampleCoord.y > seg->box.y)
				  &&(sampleCoord.y <= seg->box.w)
				  &&(mtl_side_of_segment(sampleCoord, seg) < 0))
				{
					winding[sampleIndex] += seg->windingIncrement;
				}

				if(op->crossRight)
				{
					if( (seg->config == MG_MTL_BR || seg->config == MG_MTL_TL)
					  &&(sampleCoord.y > seg->box.w))
					{
						winding[sampleIndex] += seg->windingIncrement;
					}
					else if( (seg->config == MG_MTL_BL || seg->config == MG_MTL_TR)
					       &&(sampleCoord.y > seg->box.y))
					{
						winding[sampleIndex] -= seg->windingIncrement;
					}
				}
			}
		}
		else
		{
			float4 nextColor = pathBuffer[pathIndex].color;
			nextColor.rgb *= nextColor.a;

			if(useTexture[0])
			{
				constexpr sampler smp(mip_filter::nearest, mag_filter::linear, min_filter::linear);

				float4 texColor = {0};
				for(int sampleIndex=0; sampleIndex<srcSampleCount; sampleIndex++)
				{
					float2 sampleCoord = imgSampleCoords[sampleIndex];
					float3 ph = float3(sampleCoord.xy, 1);
					float2 uv = (pathBuffer[pathIndex].uvTransform * ph).xy;

					texColor += srcTexture.sample(smp, uv);
				}
				texColor /= srcSampleCount;
				texColor.rgb *= texColor.a;
				nextColor *= texColor;
			}

			if(op->kind == MG_MTL_OP_FILL)
			{
				color = color*(1-nextColor.a) + nextColor;
			}
			else
			{
				float4 clip = pathBuffer[pathIndex].clip * scale[0];
				float coverage = 0;

				for(int sampleIndex=0; sampleIndex<sampleCount; sampleIndex++)
				{
					float2 sampleCoord = sampleCoords[sampleIndex];

					if(  sampleCoord.x >= clip.x
					  && sampleCoord.x < clip.z
					  && sampleCoord.y >= clip.y
					  && sampleCoord.y < clip.w)
					{
						bool filled = op->kind == MG_MTL_OP_CLIP_FILL
						            ||(pathBuffer[pathIndex].cmd == MG_MTL_FILL && (winding[sampleIndex] & 1))
						            ||(pathBuffer[pathIndex].cmd == MG_MTL_STROKE && (winding[sampleIndex] != 0));
						if(filled)
						{
							coverage++;
						}
					}
				}
				coverage /= sampleCount;
				color = coverage*(color*(1-nextColor.a) + nextColor) + (1-coverage)*color;
			}
		}
		opIndex = op->next;
	}

/*
	if((pixelCoord.x % tileSize[0] == 0) || (pixelCoord.y % tileSize[0] == 0))
	{
		color = float4(0, 0, 0, 1);
	}
//*/
	outTexture.write(color, pixelCoord);
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

fragment float4 mtl_fragment_shader(vs_out i [[stage_in]], texture2d<float, access::sample> tex [[texture(0)]])
{
	constexpr sampler smp(mip_filter::nearest, mag_filter::linear, min_filter::linear);
	return(tex.sample(smp, i.uv));
}
