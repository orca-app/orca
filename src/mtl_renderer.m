/************************************************************//**
*
*	@file: mtl_canvas.m
*	@author: Martin Fouilleul
*	@date: 12/07/2020
*	@revision: 24/01/2023
*
*****************************************************************/
#import<Metal/Metal.h>
#import<QuartzCore/CAMetalLayer.h>
#include<simd/simd.h>

#include"graphics_surface.h"
#include"macro_helpers.h"
#include"osx_app.h"

#include"mtl_renderer.h"

const int MG_MTL_INPUT_BUFFERS_COUNT = 3,
          MG_MTL_TILE_SIZE = 16,
          MG_MTL_MSAA_COUNT = 8;

typedef struct mg_mtl_canvas_backend
{
	mg_canvas_backend interface;
	mg_mtl_surface* surface;

	id<MTLComputePipelineState> pathPipeline;
	id<MTLComputePipelineState> segmentPipeline;
	id<MTLComputePipelineState> backpropPipeline;
	id<MTLComputePipelineState> mergePipeline;
	id<MTLComputePipelineState> rasterPipeline;
	id<MTLRenderPipelineState> blitPipeline;

	id<MTLTexture> outTexture;

	int bufferIndex;
	dispatch_semaphore_t bufferSemaphore;

	id<MTLBuffer> pathBuffer[MG_MTL_INPUT_BUFFERS_COUNT];
	id<MTLBuffer> elementBuffer[MG_MTL_INPUT_BUFFERS_COUNT];
	id<MTLBuffer> logBuffer[MG_MTL_INPUT_BUFFERS_COUNT];
	id<MTLBuffer> logOffsetBuffer[MG_MTL_INPUT_BUFFERS_COUNT];

	id<MTLBuffer> segmentCountBuffer;
	id<MTLBuffer> segmentBuffer;
	id<MTLBuffer> pathQueueBuffer;
	id<MTLBuffer> tileQueueBuffer;
	id<MTLBuffer> tileQueueCountBuffer;
	id<MTLBuffer> tileOpBuffer;
	id<MTLBuffer> tileOpCountBuffer;
	id<MTLBuffer> screenTilesBuffer;
	id<MTLBuffer> rasterDispatchBuffer;

	int msaaCount;
	vec2 frameSize;

	// encoding context
	int eltCap;
	int eltCount;
	int eltBatchStart;

	int pathCap;
	int pathCount;
	int pathBatchStart;

	mg_primitive* primitive;
	vec4 pathScreenExtents;
	vec4 pathUserExtents;

	int maxTileQueueCount;
	int maxSegmentCount;

} mg_mtl_canvas_backend;

typedef struct mg_mtl_image_data
{
	mg_image_data interface;
	id<MTLTexture> texture;
} mg_mtl_image_data;

void mg_mtl_print_log(int bufferIndex, id<MTLBuffer> logBuffer, id<MTLBuffer> logOffsetBuffer)
{
	char* log = [logBuffer contents];
	int size = *(int*)[logOffsetBuffer contents];

	if(size)
	{
		log_info("Log from buffer %i:\n", bufferIndex);

		int index = 0;
		while(index < size)
		{
			int len = strlen(log+index);
			printf("%s", log+index);
			index += (len+1);
		}
	}
}

static void mg_update_path_extents(vec4* extents, vec2 p)
{
	extents->x = minimum(extents->x, p.x);
	extents->y = minimum(extents->y, p.y);
	extents->z = maximum(extents->z, p.x);
	extents->w = maximum(extents->w, p.y);
}

id<MTLBuffer> mg_mtl_grow_input_buffer(id<MTLDevice> device, id<MTLBuffer> oldBuffer, int oldCopySize, int newSize)
{
	@autoreleasepool
	{
		MTLResourceOptions bufferOptions = MTLResourceCPUCacheModeWriteCombined
		                                 | MTLResourceStorageModeShared;

		id<MTLBuffer> newBuffer = [device newBufferWithLength: newSize options: bufferOptions];

		memcpy([newBuffer contents], [oldBuffer contents], oldCopySize);

		[oldBuffer release];
		return(newBuffer);
	}
}

void mg_mtl_canvas_encode_element(mg_mtl_canvas_backend* backend, mg_path_elt_type kind, vec2* p)
{
	int bufferIndex = backend->bufferIndex;
	int bufferCap = [backend->elementBuffer[bufferIndex] length] / sizeof(mg_mtl_path_elt);
	if(backend->eltCount >= bufferCap)
	{
		int newBufferCap = (int)(bufferCap * 1.5);
		int newBufferSize = newBufferCap * sizeof(mg_mtl_path_elt);

		log_info("growing element buffer to %i elements\n", newBufferCap);

		backend->elementBuffer[bufferIndex] = mg_mtl_grow_input_buffer(backend->surface->device,
		                                                               backend->elementBuffer[bufferIndex],
		                                                               backend->eltCount * sizeof(mg_mtl_path_elt),
		                                                               newBufferSize);
	}

	mg_mtl_path_elt* elements = (mg_mtl_path_elt*)[backend->elementBuffer[bufferIndex] contents];
	mg_mtl_path_elt* elt = &elements[backend->eltCount];
	backend->eltCount++;

	elt->pathIndex = backend->pathCount - backend->pathBatchStart;
	int count = 0;
	switch(kind)
	{
		case MG_PATH_LINE:
			backend->maxSegmentCount += 1;
			elt->kind = MG_MTL_LINE;
			count = 2;
			break;

		case MG_PATH_QUADRATIC:
			backend->maxSegmentCount += 3;
			elt->kind = MG_MTL_QUADRATIC;
			count = 3;
			break;

		case MG_PATH_CUBIC:
			backend->maxSegmentCount += 7;
			elt->kind = MG_MTL_CUBIC;
			count = 4;
			break;

		default:
			break;
	}

	for(int i=0; i<count; i++)
	{
		mg_update_path_extents(&backend->pathUserExtents, p[i]);

		vec2 screenP = mg_mat2x3_mul(backend->primitive->attributes.transform, p[i]);
		elt->p[i] = (vector_float2){screenP.x, screenP.y};

		mg_update_path_extents(&backend->pathScreenExtents, screenP);
	}
}


void mg_mtl_encode_path(mg_mtl_canvas_backend* backend, mg_primitive* primitive, float scale)
{
	int bufferIndex = backend->bufferIndex;
	int bufferCap = [backend->pathBuffer[bufferIndex] length] / sizeof(mg_mtl_path);
	if(backend->pathCount >= bufferCap)
	{
		int newBufferCap = (int)(bufferCap * 1.5);
		int newBufferSize = newBufferCap * sizeof(mg_mtl_path);

		log_info("growing path buffer to %i elements\n", newBufferCap);

		backend->pathBuffer[bufferIndex] = mg_mtl_grow_input_buffer(backend->surface->device,
		                                                               backend->pathBuffer[bufferIndex],
		                                                               backend->eltCount * sizeof(mg_mtl_path),
		                                                               newBufferSize);
	}

	mg_mtl_path* pathBufferData = (mg_mtl_path*)[backend->pathBuffer[backend->bufferIndex] contents];
	mg_mtl_path* path = &(pathBufferData[backend->pathCount]);
	backend->pathCount++;

	path->cmd =	(mg_mtl_cmd)primitive->cmd;

	path->box = (vector_float4){backend->pathScreenExtents.x,
	                            backend->pathScreenExtents.y,
	                            backend->pathScreenExtents.z,
	                            backend->pathScreenExtents.w};

	path->clip = (vector_float4){primitive->attributes.clip.x,
	                             primitive->attributes.clip.y,
	                             primitive->attributes.clip.x + primitive->attributes.clip.w,
	                             primitive->attributes.clip.y + primitive->attributes.clip.h};

	path->color = (vector_float4){primitive->attributes.color.r,
	                              primitive->attributes.color.g,
	                              primitive->attributes.color.b,
	                              primitive->attributes.color.a};

	mp_rect srcRegion = primitive->attributes.srcRegion;

	mp_rect destRegion = {backend->pathUserExtents.x,
	                      backend->pathUserExtents.y,
	                      backend->pathUserExtents.z - backend->pathUserExtents.x,
	                      backend->pathUserExtents.w - backend->pathUserExtents.y};

	if(!mg_image_is_nil(primitive->attributes.image))
	{
		vec2 texSize = mg_image_size(primitive->attributes.image);

		mg_mat2x3 srcRegionToImage = {1/texSize.x, 0, srcRegion.x/texSize.x,
		                              0, 1/texSize.y, srcRegion.y/texSize.y};

		mg_mat2x3 destRegionToSrcRegion = {srcRegion.w/destRegion.w, 0, 0,
		                                   0, srcRegion.h/destRegion.h, 0};

		mg_mat2x3 userToDestRegion = {1, 0, -destRegion.x,
		                              0, 1, -destRegion.y};

		mg_mat2x3 screenToUser = mg_mat2x3_inv(primitive->attributes.transform);

		mg_mat2x3 uvTransform = srcRegionToImage;
		uvTransform = mg_mat2x3_mul_m(uvTransform, destRegionToSrcRegion);
		uvTransform = mg_mat2x3_mul_m(uvTransform, userToDestRegion);
		uvTransform = mg_mat2x3_mul_m(uvTransform, screenToUser);

		path->uvTransform = simd_matrix(simd_make_float3(uvTransform.m[0]/scale, uvTransform.m[3]/scale, 0),
		                                simd_make_float3(uvTransform.m[1]/scale, uvTransform.m[4]/scale, 0),
		                                simd_make_float3(uvTransform.m[2], uvTransform.m[5], 1));
	}

	int nTilesX = ((path->box.z - path->box.x)*scale - 1) / MG_MTL_TILE_SIZE + 1;
	int nTilesY = ((path->box.w - path->box.y)*scale - 1) / MG_MTL_TILE_SIZE + 1;
	backend->maxTileQueueCount += (nTilesX * nTilesY);
}

bool mg_intersect_hull_legs(vec2 p0, vec2 p1, vec2 p2, vec2 p3, vec2* intersection)
{
	/*NOTE: check intersection of lines (p0-p1) and (p2-p3)

		P = p0 + u(p1-p0)
		P = p2 + w(p3-p2)
	*/
	bool found = false;

	f32 den = (p0.x - p1.x)*(p2.y - p3.y) - (p0.y - p1.y)*(p2.x - p3.x);
	if(fabs(den) > 0.0001)
	{
		f32 u = ((p0.x - p2.x)*(p2.y - p3.y) - (p0.y - p2.y)*(p2.x - p3.x))/den;
		f32 w = ((p0.x - p2.x)*(p0.y - p1.y) - (p0.y - p2.y)*(p0.x - p1.x))/den;

		intersection->x = p0.x + u*(p1.x - p0.x);
		intersection->y = p0.y + u*(p1.y - p0.y);
		found = true;
	}
	return(found);
}

bool mg_offset_hull(int count, vec2* p, vec2* result, f32 offset)
{
	//NOTE: we should have no more than two coincident points here. This means the leg between
	//      those two points can't be offset, but we can set a double point at the start of first leg,
	//      end of first leg, or we can join the first and last leg to create a missing middle one

	vec2 legs[3][2] = {0};
	bool valid[3] = {0};

	for(int i=0; i<count-1; i++)
	{
		vec2 n = {p[i].y - p[i+1].y,
	              p[i+1].x - p[i].x};

		f32 norm = sqrt(n.x*n.x + n.y*n.y);
		if(norm >= 1e-6)
		{
			n = vec2_mul(offset/norm, n);
			legs[i][0] = vec2_add(p[i], n);
			legs[i][1] = vec2_add(p[i+1], n);
			valid[i] = true;
		}
	}

	//NOTE: now we find intersections

	// first point is either the start of the first or second leg
	if(valid[0])
	{
		result[0] = legs[0][0];
	}
	else
	{
		ASSERT(valid[1]);
		result[0] = legs[1][0];
	}

	for(int i=1; i<count-1; i++)
	{
		//NOTE: we're computing the control point i, at the end of leg (i-1)

		if(!valid[i-1])
		{
			ASSERT(valid[i]);
			result[i] = legs[i][0];
		}
		else if(!valid[i])
		{
			ASSERT(valid[i-1]);
			result[i] = legs[i-1][0];
		}
		else
		{
			if(!mg_intersect_hull_legs(legs[i-1][0], legs[i-1][1], legs[i][0], legs[i][1], &result[i]))
			{
				// legs don't intersect.
				return(false);
			}
		}
	}

	if(valid[count-2])
	{
		result[count-1] = legs[count-2][1];
	}
	else
	{
		ASSERT(valid[count-3]);
		result[count-1] = legs[count-3][1];
	}

	return(true);
}

vec2 mg_quadratic_get_point(vec2 p[3], f32 t)
{
	vec2 r;

	f32 oneMt = 1-t;
	f32 oneMt2 = Square(oneMt);
	f32 t2 = Square(t);

	r.x = oneMt2*p[0].x + 2*oneMt*t*p[1].x + t2*p[2].x;
	r.y = oneMt2*p[0].y + 2*oneMt*t*p[1].y + t2*p[2].y;

	return(r);
}

void mg_quadratic_split(vec2 p[3], f32 t, vec2 outLeft[3], vec2 outRight[3])
{
	//NOTE(martin): split bezier curve p at parameter t, using De Casteljau's algorithm
	//              the q_n are the points along the hull's segments at parameter t
	//              s is the split point.

	f32 oneMt = 1-t;

	vec2 q0 = {oneMt*p[0].x + t*p[1].x,
		   oneMt*p[0].y + t*p[1].y};

	vec2 q1 = {oneMt*p[1].x + t*p[2].x,
		   oneMt*p[1].y + t*p[2].y};

	vec2 s = {oneMt*q0.x + t*q1.x,
		   oneMt*q0.y + t*q1.y};

	outLeft[0] = p[0];
	outLeft[1] = q0;
	outLeft[2] = s;

	outRight[0] = s;
	outRight[1] = q1;
	outRight[2] = p[2];
}

vec2 mg_cubic_get_point(vec2 p[4], f32 t)
{
	vec2 r;

	f32 oneMt = 1-t;
	f32 oneMt2 = Square(oneMt);
	f32 oneMt3 = oneMt2*oneMt;
	f32 t2 = Square(t);
	f32 t3 = t2*t;

	r.x = oneMt3*p[0].x + 3*oneMt2*t*p[1].x + 3*oneMt*t2*p[2].x + t3*p[3].x;
	r.y = oneMt3*p[0].y + 3*oneMt2*t*p[1].y + 3*oneMt*t2*p[2].y + t3*p[3].y;

	return(r);
}

void mg_cubic_split(vec2 p[4], f32 t, vec2 outLeft[4], vec2 outRight[4])
{
	//NOTE(martin): split bezier curve p at parameter t, using De Casteljau's algorithm
	//              the q_n are the points along the hull's segments at parameter t
	//              the r_n are the points along the (q_n, q_n+1) segments at parameter t
	//              s is the split point.

	vec2 q0 = {(1-t)*p[0].x + t*p[1].x,
	           (1-t)*p[0].y + t*p[1].y};

	vec2 q1 = {(1-t)*p[1].x + t*p[2].x,
	           (1-t)*p[1].y + t*p[2].y};

	vec2 q2 = {(1-t)*p[2].x + t*p[3].x,
	           (1-t)*p[2].y + t*p[3].y};

	vec2 r0 = {(1-t)*q0.x + t*q1.x,
	           (1-t)*q0.y + t*q1.y};

	vec2 r1 = {(1-t)*q1.x + t*q2.x,
	           (1-t)*q1.y + t*q2.y};

	vec2 s = {(1-t)*r0.x + t*r1.x,
	          (1-t)*r0.y + t*r1.y};;

	outLeft[0] = p[0];
	outLeft[1] = q0;
	outLeft[2] = r0;
	outLeft[3] = s;

	outRight[0] = s;
	outRight[1] = r1;
	outRight[2] = q2;
	outRight[3] = p[3];
}

void mg_mtl_render_stroke_line(mg_mtl_canvas_backend* backend, vec2* p)
{
	f32 width = backend->primitive->attributes.width;

	vec2 v = {p[1].x-p[0].x, p[1].y-p[0].y};
	vec2 n = {v.y, -v.x};
	f32 norm = sqrt(n.x*n.x + n.y*n.y);
	vec2 offset = vec2_mul(0.5*width/norm, n);

	vec2 left[2] = {vec2_add(p[0], offset), vec2_add(p[1], offset)};
	vec2 right[2] = {vec2_add(p[1], vec2_mul(-1, offset)), vec2_add(p[0], vec2_mul(-1, offset))};
	vec2 joint0[2] = {vec2_add(p[0], vec2_mul(-1, offset)), vec2_add(p[0], offset)};
	vec2 joint1[2] = {vec2_add(p[1], offset), vec2_add(p[1], vec2_mul(-1, offset))};

	mg_mtl_canvas_encode_element(backend, MG_PATH_LINE, right);
	mg_mtl_canvas_encode_element(backend, MG_PATH_LINE, left);
	mg_mtl_canvas_encode_element(backend, MG_PATH_LINE, joint0);
	mg_mtl_canvas_encode_element(backend, MG_PATH_LINE, joint1);
}

void mg_mtl_render_stroke_quadratic(mg_mtl_canvas_backend* backend, vec2* p)
{
	f32 width = backend->primitive->attributes.width;
	f32 tolerance = minimum(backend->primitive->attributes.tolerance, 0.5 * width);

	//NOTE: check for degenerate line case
	const f32 equalEps = 1e-3;
	if(vec2_close(p[0], p[1], equalEps))
	{
		mg_mtl_render_stroke_line(backend, p+1);
		return;
	}
	else if(vec2_close(p[1], p[2], equalEps))
	{
		mg_mtl_render_stroke_line(backend, p);
		return;
	}

	vec2 leftHull[3];
	vec2 rightHull[3];

	if(  !mg_offset_hull(3, p, leftHull, width/2)
	  || !mg_offset_hull(3, p, rightHull, -width/2))
	{
		//TODO split and recurse
		//NOTE: offsetting the hull failed, split the curve
		vec2 splitLeft[3];
		vec2 splitRight[3];
		mg_quadratic_split(p, 0.5, splitLeft, splitRight);
		mg_mtl_render_stroke_quadratic(backend, splitLeft);
		mg_mtl_render_stroke_quadratic(backend, splitRight);
	}
	else
	{
		const int CHECK_SAMPLE_COUNT = 5;
		f32 checkSamples[CHECK_SAMPLE_COUNT] = {1./6, 2./6, 3./6, 4./6, 5./6};

		f32 d2LowBound = Square(0.5 * width - tolerance);
		f32 d2HighBound = Square(0.5 * width + tolerance);

		f32 maxOvershoot = 0;
		f32 maxOvershootParameter = 0;

		for(int i=0; i<CHECK_SAMPLE_COUNT; i++)
		{
			f32 t = checkSamples[i];

			vec2 c = mg_quadratic_get_point(p, t);
			vec2 cp =  mg_quadratic_get_point(leftHull, t);
			vec2 cn =  mg_quadratic_get_point(rightHull, t);

			f32 positiveDistSquare = Square(c.x - cp.x) + Square(c.y - cp.y);
			f32 negativeDistSquare = Square(c.x - cn.x) + Square(c.y - cn.y);

			f32 positiveOvershoot = maximum(positiveDistSquare - d2HighBound, d2LowBound - positiveDistSquare);
			f32 negativeOvershoot = maximum(negativeDistSquare - d2HighBound, d2LowBound - negativeDistSquare);

			f32 overshoot = maximum(positiveOvershoot, negativeOvershoot);

			if(overshoot > maxOvershoot)
			{
				maxOvershoot = overshoot;
				maxOvershootParameter = t;
			}
		}

		if(maxOvershoot > 0)
		{
			vec2 splitLeft[3];
			vec2 splitRight[3];
			mg_quadratic_split(p, maxOvershootParameter, splitLeft, splitRight);
			mg_mtl_render_stroke_quadratic(backend, splitLeft);
			mg_mtl_render_stroke_quadratic(backend, splitRight);
		}
		else
		{
			vec2 tmp = leftHull[0];
			leftHull[0] = leftHull[2];
			leftHull[2] = tmp;

			mg_mtl_canvas_encode_element(backend, MG_PATH_QUADRATIC, rightHull);
			mg_mtl_canvas_encode_element(backend, MG_PATH_QUADRATIC, leftHull);

			vec2 joint0[2] = {rightHull[2], leftHull[0]};
			vec2 joint1[2] = {leftHull[2], rightHull[0]};
			mg_mtl_canvas_encode_element(backend, MG_PATH_LINE, joint0);
			mg_mtl_canvas_encode_element(backend, MG_PATH_LINE, joint1);
		}
	}
}

void mg_mtl_render_stroke_cubic(mg_mtl_canvas_backend* backend, vec2* p)
{
	f32 width = backend->primitive->attributes.width;
	f32 tolerance = minimum(backend->primitive->attributes.tolerance, 0.5 * width);

	//NOTE: check degenerate line cases
	f32 equalEps = 1e-3;

	if( (vec2_close(p[0], p[1], equalEps) && vec2_close(p[2], p[3], equalEps))
	  ||(vec2_close(p[0], p[1], equalEps) && vec2_close(p[1], p[2], equalEps))
	  ||(vec2_close(p[1], p[2], equalEps) && vec2_close(p[2], p[3], equalEps)))
	{
		vec2 line[2] = {p[0], p[3]};
		mg_mtl_render_stroke_line(backend, line);
		return;
	}
	else if(vec2_close(p[0], p[1], equalEps) && vec2_close(p[1], p[3], equalEps))
	{
		vec2 line[2] = {p[0], vec2_add(vec2_mul(5./9, p[0]), vec2_mul(4./9, p[2]))};
		mg_mtl_render_stroke_line(backend, line);
		return;
	}
	else if(vec2_close(p[0], p[2], equalEps) && vec2_close(p[2], p[3], equalEps))
	{
		vec2 line[2] = {p[0], vec2_add(vec2_mul(5./9, p[0]), vec2_mul(4./9, p[1]))};
		mg_mtl_render_stroke_line(backend, line);
		return;
	}

	vec2 leftHull[4];
	vec2 rightHull[4];

	if(  !mg_offset_hull(4, p, leftHull, width/2)
	  || !mg_offset_hull(4, p, rightHull, -width/2))
	{
		//TODO split and recurse
		//NOTE: offsetting the hull failed, split the curve
		vec2 splitLeft[4];
		vec2 splitRight[4];
		mg_cubic_split(p, 0.5, splitLeft, splitRight);
		mg_mtl_render_stroke_cubic(backend, splitLeft);
		mg_mtl_render_stroke_cubic(backend, splitRight);
	}
	else
	{
		const int CHECK_SAMPLE_COUNT = 5;
		f32 checkSamples[CHECK_SAMPLE_COUNT] = {1./6, 2./6, 3./6, 4./6, 5./6};

		f32 d2LowBound = Square(0.5 * width - tolerance);
		f32 d2HighBound = Square(0.5 * width + tolerance);

		f32 maxOvershoot = 0;
		f32 maxOvershootParameter = 0;

		for(int i=0; i<CHECK_SAMPLE_COUNT; i++)
		{
			f32 t = checkSamples[i];

			vec2 c = mg_cubic_get_point(p, t);
			vec2 cp =  mg_cubic_get_point(leftHull, t);
			vec2 cn =  mg_cubic_get_point(rightHull, t);

			f32 positiveDistSquare = Square(c.x - cp.x) + Square(c.y - cp.y);
			f32 negativeDistSquare = Square(c.x - cn.x) + Square(c.y - cn.y);

			f32 positiveOvershoot = maximum(positiveDistSquare - d2HighBound, d2LowBound - positiveDistSquare);
			f32 negativeOvershoot = maximum(negativeDistSquare - d2HighBound, d2LowBound - negativeDistSquare);

			f32 overshoot = maximum(positiveOvershoot, negativeOvershoot);

			if(overshoot > maxOvershoot)
			{
				maxOvershoot = overshoot;
				maxOvershootParameter = t;
			}
		}

		if(maxOvershoot > 0)
		{
			vec2 splitLeft[4];
			vec2 splitRight[4];
			mg_cubic_split(p, maxOvershootParameter, splitLeft, splitRight);
			mg_mtl_render_stroke_cubic(backend, splitLeft);
			mg_mtl_render_stroke_cubic(backend, splitRight);
		}
		else
		{
			vec2 tmp = leftHull[0];
			leftHull[0] = leftHull[3];
			leftHull[3] = tmp;
			tmp = leftHull[1];
			leftHull[1] = leftHull[2];
			leftHull[2] = tmp;

			mg_mtl_canvas_encode_element(backend, MG_PATH_CUBIC, rightHull);
			mg_mtl_canvas_encode_element(backend, MG_PATH_CUBIC, leftHull);

			vec2 joint0[2] = {rightHull[3], leftHull[0]};
			vec2 joint1[2] = {leftHull[3], rightHull[0]};
			mg_mtl_canvas_encode_element(backend, MG_PATH_LINE, joint0);
			mg_mtl_canvas_encode_element(backend, MG_PATH_LINE, joint1);
		}
	}
}

void mg_mtl_render_stroke_element(mg_mtl_canvas_backend* backend,
                                  mg_path_elt* element,
                                  vec2 currentPoint,
                                  vec2* startTangent,
                                  vec2* endTangent,
                                  vec2* endPoint)
{
	vec2 controlPoints[4] = {currentPoint, element->p[0], element->p[1], element->p[2]};
	int endPointIndex = 0;

	switch(element->type)
	{
		case MG_PATH_LINE:
			mg_mtl_render_stroke_line(backend, controlPoints);
			endPointIndex = 1;
			break;

		case MG_PATH_QUADRATIC:
			mg_mtl_render_stroke_quadratic(backend, controlPoints);
			endPointIndex = 2;
			break;

		case MG_PATH_CUBIC:
			mg_mtl_render_stroke_cubic(backend, controlPoints);
			endPointIndex = 3;
			break;

		case MG_PATH_MOVE:
			ASSERT(0, "should be unreachable");
			break;
	}

	//NOTE: ensure tangents are properly computed even in presence of coincident points
	//TODO: see if we can do this in a less hacky way

	for(int i=1; i<4; i++)
	{
		if(  controlPoints[i].x != controlPoints[0].x
		  || controlPoints[i].y != controlPoints[0].y)
		{
			*startTangent = (vec2){.x = controlPoints[i].x - controlPoints[0].x,
			                       .y = controlPoints[i].y - controlPoints[0].y};
			break;
		}
	}
	*endPoint = controlPoints[endPointIndex];

	for(int i=endPointIndex-1; i>=0; i++)
	{
		if(  controlPoints[i].x != endPoint->x
		  || controlPoints[i].y != endPoint->y)
		{
			*endTangent = (vec2){.x = endPoint->x - controlPoints[i].x,
			                     .y = endPoint->y - controlPoints[i].y};
			break;
		}
	}
	DEBUG_ASSERT(startTangent->x != 0 || startTangent->y != 0);
}

void mg_mtl_stroke_cap(mg_mtl_canvas_backend* backend,
                       vec2 p0,
                       vec2 direction)
{
	mg_attributes* attributes = &backend->primitive->attributes;

	//NOTE(martin): compute the tangent and normal vectors (multiplied by half width) at the cap point
	f32 dn = sqrt(Square(direction.x) + Square(direction.y));
	f32 alpha = 0.5 * attributes->width/dn;

	vec2 n0 = {-alpha*direction.y,
		    alpha*direction.x};

	vec2 m0 = {alpha*direction.x,
	           alpha*direction.y};

	vec2 points[] = {{p0.x + n0.x, p0.y + n0.y},
	                 {p0.x + n0.x + m0.x, p0.y + n0.y + m0.y},
	                 {p0.x - n0.x + m0.x, p0.y - n0.y + m0.y},
	                 {p0.x - n0.x, p0.y - n0.y},
	                 {p0.x + n0.x, p0.y + n0.y}};

	mg_mtl_canvas_encode_element(backend, MG_PATH_LINE, points);
	mg_mtl_canvas_encode_element(backend, MG_PATH_LINE, points+1);
	mg_mtl_canvas_encode_element(backend, MG_PATH_LINE, points+2);
	mg_mtl_canvas_encode_element(backend, MG_PATH_LINE, points+3);
}

void mg_mtl_stroke_joint(mg_mtl_canvas_backend* backend,
                         vec2 p0,
                         vec2 t0,
                         vec2 t1)
{
	mg_attributes* attributes = &backend->primitive->attributes;

	//NOTE(martin): compute the normals at the joint point
	f32 norm_t0 = sqrt(Square(t0.x) + Square(t0.y));
	f32 norm_t1 = sqrt(Square(t1.x) + Square(t1.y));

	vec2 n0 = {-t0.y, t0.x};
	n0.x /= norm_t0;
	n0.y /= norm_t0;

	vec2 n1 = {-t1.y, t1.x};
	n1.x /= norm_t1;
	n1.y /= norm_t1;

	//NOTE(martin): the sign of the cross product determines if the normals are facing outwards or inwards the angle.
	//              we flip them to face outwards if needed
	f32 crossZ = n0.x*n1.y - n0.y*n1.x;
	if(crossZ > 0)
	{
		n0.x *= -1;
		n0.y *= -1;
		n1.x *= -1;
		n1.y *= -1;
	}

	//NOTE(martin): use the same code as hull offset to find mitter point...
	/*NOTE(martin): let vector u = (n0+n1) and vector v = pIntersect - p1
		then v = u * (2*offset / norm(u)^2)
		(this can be derived from writing the pythagoras theorems in the triangles of the joint)
	*/
	f32 halfW = 0.5 * attributes->width;
	vec2 u = {n0.x + n1.x, n0.y + n1.y};
	f32 uNormSquare = u.x*u.x + u.y*u.y;
	f32 alpha = attributes->width / uNormSquare;
	vec2 v = {u.x * alpha, u.y * alpha};

	f32 excursionSquare = uNormSquare * Square(alpha - attributes->width/4);

	if(  attributes->joint == MG_JOINT_MITER
	  && excursionSquare <= Square(attributes->maxJointExcursion))
	{
		//NOTE(martin): add a mitter joint
		vec2 points[] = {p0,
		                 {p0.x + n0.x*halfW, p0.y + n0.y*halfW},
		                 {p0.x + v.x, p0.y + v.y},
		                 {p0.x + n1.x*halfW, p0.y + n1.y*halfW},
		                 p0};

		mg_mtl_canvas_encode_element(backend, MG_PATH_LINE, points);
		mg_mtl_canvas_encode_element(backend, MG_PATH_LINE, points+1);
		mg_mtl_canvas_encode_element(backend, MG_PATH_LINE, points+2);
		mg_mtl_canvas_encode_element(backend, MG_PATH_LINE, points+3);
	}
	else
	{
		//NOTE(martin): add a bevel joint
		vec2 points[] = {p0,
		                 {p0.x + n0.x*halfW, p0.y + n0.y*halfW},
		                 {p0.x + n1.x*halfW, p0.y + n1.y*halfW},
		                 p0};

		mg_mtl_canvas_encode_element(backend, MG_PATH_LINE, points);
		mg_mtl_canvas_encode_element(backend, MG_PATH_LINE, points+1);
		mg_mtl_canvas_encode_element(backend, MG_PATH_LINE, points+2);
	}
}

u32 mg_mtl_render_stroke_subpath(mg_mtl_canvas_backend* backend,
                                 mg_path_elt* elements,
                                 mg_path_descriptor* path,
                                 u32 startIndex,
                                 vec2 startPoint)
{
	u32 eltCount = path->count;
	DEBUG_ASSERT(startIndex < eltCount);

	vec2 currentPoint = startPoint;
	vec2 endPoint = {0, 0};
	vec2 previousEndTangent = {0, 0};
	vec2 firstTangent = {0, 0};
	vec2 startTangent = {0, 0};
	vec2 endTangent = {0, 0};

	//NOTE(martin): render first element and compute first tangent
	mg_mtl_render_stroke_element(backend, elements + startIndex, currentPoint, &startTangent, &endTangent, &endPoint);

	firstTangent = startTangent;
	previousEndTangent = endTangent;
	currentPoint = endPoint;

	//NOTE(martin): render subsequent elements along with their joints

	mg_attributes* attributes = &backend->primitive->attributes;

	u32 eltIndex = startIndex + 1;
	for(;
	    eltIndex<eltCount && elements[eltIndex].type != MG_PATH_MOVE;
	    eltIndex++)
	{
		mg_mtl_render_stroke_element(backend, elements + eltIndex, currentPoint, &startTangent, &endTangent, &endPoint);

		if(attributes->joint != MG_JOINT_NONE)
		{
			mg_mtl_stroke_joint(backend, currentPoint, previousEndTangent, startTangent);
		}
		previousEndTangent = endTangent;
		currentPoint = endPoint;
	}
	u32 subPathEltCount = eltIndex - startIndex;

	//NOTE(martin): draw end cap / joint. We ensure there's at least two segments to draw a closing joint
	if(  subPathEltCount > 1
	  && startPoint.x == endPoint.x
	  && startPoint.y == endPoint.y)
	{
		if(attributes->joint != MG_JOINT_NONE)
		{
			//NOTE(martin): add a closing joint if the path is closed
			mg_mtl_stroke_joint(backend, endPoint, endTangent, firstTangent);
		}
	}
	else if(attributes->cap == MG_CAP_SQUARE)
	{
		//NOTE(martin): add start and end cap
		mg_mtl_stroke_cap(backend, startPoint, (vec2){-startTangent.x, -startTangent.y});
		mg_mtl_stroke_cap(backend, endPoint, endTangent);
	}
	return(eltIndex);
}

void mg_mtl_render_stroke(mg_mtl_canvas_backend* backend,
                          mg_path_elt* elements,
                          mg_path_descriptor* path)
{
	u32 eltCount = path->count;
	DEBUG_ASSERT(eltCount);

	vec2 startPoint = path->startPoint;
	u32 startIndex = 0;

	while(startIndex < eltCount)
	{
		//NOTE(martin): eliminate leading moves
		while(startIndex < eltCount && elements[startIndex].type == MG_PATH_MOVE)
		{
			startPoint = elements[startIndex].p[0];
			startIndex++;
		}
		if(startIndex < eltCount)
		{
			startIndex = mg_mtl_render_stroke_subpath(backend, elements, path, startIndex, startPoint);
		}
	}
}


void mg_mtl_grow_buffer_if_needed(mg_mtl_canvas_backend* backend, id<MTLBuffer>* buffer, u64 wantedSize)
{
	u64 bufferSize = [(*buffer) length];
	if(bufferSize < wantedSize)
	{
		int newSize = wantedSize * 1.2;

		@autoreleasepool
		{
			 //NOTE: MTLBuffers are retained by the command buffer, so we don't risk deallocating while the buffer is in use
			[*buffer release];
			*buffer = nil;

			id<MTLDevice> device = backend->surface->device;
			MTLResourceOptions bufferOptions = MTLResourceStorageModePrivate;

			*buffer = [device newBufferWithLength: newSize options: bufferOptions];
		}
	}
}

void mg_mtl_render_batch(mg_mtl_canvas_backend* backend,
                         mg_mtl_surface* surface,
                         mg_image_data* image,
                         int tileSize,
                         int nTilesX,
                         int nTilesY,
                         vec2 viewportSize,
                         f32 scale)
{
	int pathBufferOffset = backend->pathBatchStart * sizeof(mg_mtl_path);
	int elementBufferOffset = backend->eltBatchStart * sizeof(mg_mtl_path_elt);
	int pathCount = backend->pathCount - backend->pathBatchStart;
	int eltCount = backend->eltCount - backend->eltBatchStart;

	//NOTE: update intermediate buffers sizes if needed

	mg_mtl_grow_buffer_if_needed(backend, &backend->pathQueueBuffer, pathCount * sizeof(mg_mtl_path_queue));
	mg_mtl_grow_buffer_if_needed(backend, &backend->tileQueueBuffer, backend->maxTileQueueCount * sizeof(mg_mtl_tile_queue));
	mg_mtl_grow_buffer_if_needed(backend, &backend->segmentBuffer, backend->maxSegmentCount * sizeof(mg_mtl_segment));
	mg_mtl_grow_buffer_if_needed(backend, &backend->screenTilesBuffer, nTilesX * nTilesY * sizeof(mg_mtl_screen_tile));
	mg_mtl_grow_buffer_if_needed(backend, &backend->tileOpBuffer, backend->maxSegmentCount * 30 * sizeof(mg_mtl_tile_op));

	//NOTE: encode GPU commands
	@autoreleasepool
	{
		//NOTE: clear output texture
		MTLRenderPassDescriptor* clearDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
		clearDescriptor.colorAttachments[0].texture = backend->outTexture;
		clearDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
		clearDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0, 0, 0, 0);
		clearDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

		id<MTLRenderCommandEncoder> clearEncoder = [surface->commandBuffer renderCommandEncoderWithDescriptor:clearDescriptor];
		clearEncoder.label = @"clear out texture pass";
		[clearEncoder endEncoding];

		//NOTE: clear counters
		id<MTLBlitCommandEncoder> blitEncoder = [surface->commandBuffer blitCommandEncoder];
		blitEncoder.label = @"clear counters";
		[blitEncoder fillBuffer: backend->segmentCountBuffer range: NSMakeRange(0, sizeof(int)) value: 0];
		[blitEncoder fillBuffer: backend->tileQueueCountBuffer range: NSMakeRange(0, sizeof(int)) value: 0];
		[blitEncoder fillBuffer: backend->tileOpCountBuffer range: NSMakeRange(0, sizeof(int)) value: 0];
		[blitEncoder fillBuffer: backend->rasterDispatchBuffer range: NSMakeRange(0, sizeof(MTLDispatchThreadgroupsIndirectArguments)) value: 0];
		[blitEncoder endEncoding];

		//NOTE: path setup pass
		id<MTLComputeCommandEncoder> pathEncoder = [surface->commandBuffer computeCommandEncoder];
		pathEncoder.label = @"path pass";
		[pathEncoder setComputePipelineState: backend->pathPipeline];

		int tileQueueMax = [backend->tileQueueBuffer length] / sizeof(mg_mtl_tile_queue);

		[pathEncoder setBytes:&pathCount length:sizeof(int) atIndex:0];
		[pathEncoder setBuffer:backend->pathBuffer[backend->bufferIndex] offset:pathBufferOffset atIndex:1];
		[pathEncoder setBuffer:backend->pathQueueBuffer offset:0 atIndex:2];
		[pathEncoder setBuffer:backend->tileQueueBuffer offset:0 atIndex:3];
		[pathEncoder setBuffer:backend->tileQueueCountBuffer offset:0 atIndex:4];
		[pathEncoder setBytes:&tileQueueMax length:sizeof(int) atIndex:5];
		[pathEncoder setBytes:&tileSize length:sizeof(int) atIndex:6];
		[pathEncoder setBytes:&scale length:sizeof(int) atIndex:7];

		MTLSize pathGridSize = MTLSizeMake(pathCount, 1, 1);
		MTLSize pathGroupSize = MTLSizeMake([backend->pathPipeline maxTotalThreadsPerThreadgroup], 1, 1);

		[pathEncoder dispatchThreads: pathGridSize threadsPerThreadgroup: pathGroupSize];
		[pathEncoder endEncoding];

		//NOTE: segment setup pass
		id<MTLComputeCommandEncoder> segmentEncoder = [surface->commandBuffer computeCommandEncoder];
		segmentEncoder.label = @"segment pass";
		[segmentEncoder setComputePipelineState: backend->segmentPipeline];

		int tileOpMax = [backend->tileOpBuffer length] / sizeof(mg_mtl_tile_op);
		int segmentMax = [backend->segmentBuffer length] / sizeof(mg_mtl_segment);

		[segmentEncoder setBytes:&eltCount length:sizeof(int) atIndex:0];
		[segmentEncoder setBuffer:backend->elementBuffer[backend->bufferIndex] offset:elementBufferOffset atIndex:1];
		[segmentEncoder setBuffer:backend->segmentCountBuffer offset:0 atIndex:2];
		[segmentEncoder setBuffer:backend->segmentBuffer offset:0 atIndex:3];
		[segmentEncoder setBuffer:backend->pathQueueBuffer offset:0 atIndex:4];
		[segmentEncoder setBuffer:backend->tileQueueBuffer offset:0 atIndex:5];
		[segmentEncoder setBuffer:backend->tileOpBuffer offset:0 atIndex:6];
		[segmentEncoder setBuffer:backend->tileOpCountBuffer offset:0 atIndex:7];
		[segmentEncoder setBytes:&tileOpMax length:sizeof(int) atIndex:8];
		[segmentEncoder setBytes:&segmentMax length:sizeof(int) atIndex:9];
		[segmentEncoder setBytes:&tileSize length:sizeof(int) atIndex:10];
		[segmentEncoder setBytes:&scale length:sizeof(int) atIndex:11];
		[segmentEncoder setBuffer:backend->logBuffer[backend->bufferIndex] offset:0 atIndex:12];
		[segmentEncoder setBuffer:backend->logOffsetBuffer[backend->bufferIndex] offset:0 atIndex:13];

		MTLSize segmentGridSize = MTLSizeMake(eltCount, 1, 1);
		MTLSize segmentGroupSize = MTLSizeMake([backend->segmentPipeline maxTotalThreadsPerThreadgroup], 1, 1);

		[segmentEncoder dispatchThreads: segmentGridSize threadsPerThreadgroup: segmentGroupSize];
		[segmentEncoder endEncoding];

		//NOTE: backprop pass
		id<MTLComputeCommandEncoder> backpropEncoder = [surface->commandBuffer computeCommandEncoder];
		backpropEncoder.label = @"backprop pass";
		[backpropEncoder setComputePipelineState: backend->backpropPipeline];

		[backpropEncoder setBuffer:backend->pathQueueBuffer offset:0 atIndex:0];
		[backpropEncoder setBuffer:backend->tileQueueBuffer offset:0 atIndex:1];
		[backpropEncoder setBuffer:backend->logBuffer[backend->bufferIndex] offset:0 atIndex:2];
		[backpropEncoder setBuffer:backend->logOffsetBuffer[backend->bufferIndex] offset:0 atIndex:3];

		MTLSize backpropGroupSize = MTLSizeMake([backend->backpropPipeline maxTotalThreadsPerThreadgroup], 1, 1);
		MTLSize backpropGridSize = MTLSizeMake(pathCount*backpropGroupSize.width, 1, 1);

		[backpropEncoder dispatchThreads: backpropGridSize threadsPerThreadgroup: backpropGroupSize];
		[backpropEncoder endEncoding];

		//NOTE: merge pass
		id<MTLComputeCommandEncoder> mergeEncoder = [surface->commandBuffer computeCommandEncoder];
		mergeEncoder.label = @"merge pass";
		[mergeEncoder setComputePipelineState: backend->mergePipeline];

		[mergeEncoder setBytes:&pathCount length:sizeof(int) atIndex:0];
		[mergeEncoder setBuffer:backend->pathBuffer[backend->bufferIndex] offset:pathBufferOffset atIndex:1];
		[mergeEncoder setBuffer:backend->pathQueueBuffer offset:0 atIndex:2];
		[mergeEncoder setBuffer:backend->tileQueueBuffer offset:0 atIndex:3];
		[mergeEncoder setBuffer:backend->tileOpBuffer offset:0 atIndex:4];
		[mergeEncoder setBuffer:backend->tileOpCountBuffer offset:0 atIndex:5];
		[mergeEncoder setBuffer:backend->rasterDispatchBuffer offset:0 atIndex:6];
		[mergeEncoder setBuffer:backend->screenTilesBuffer offset:0 atIndex:7];
		[mergeEncoder setBytes:&tileOpMax length:sizeof(int) atIndex:8];
		[mergeEncoder setBytes:&tileSize length:sizeof(int) atIndex:9];
		[mergeEncoder setBytes:&scale length:sizeof(float) atIndex:10];
		[mergeEncoder setBuffer:backend->logBuffer[backend->bufferIndex] offset:0 atIndex:11];
		[mergeEncoder setBuffer:backend->logOffsetBuffer[backend->bufferIndex] offset:0 atIndex:12];

		MTLSize mergeGridSize = MTLSizeMake(nTilesX, nTilesY, 1);
		MTLSize mergeGroupSize = MTLSizeMake(MG_MTL_TILE_SIZE, MG_MTL_TILE_SIZE, 1);

		[mergeEncoder dispatchThreads: mergeGridSize threadsPerThreadgroup: mergeGroupSize];
		[mergeEncoder endEncoding];

		//NOTE: raster pass
		id<MTLComputeCommandEncoder> rasterEncoder = [surface->commandBuffer computeCommandEncoder];
		rasterEncoder.label = @"raster pass";
		[rasterEncoder setComputePipelineState: backend->rasterPipeline];

		[rasterEncoder setBuffer:backend->screenTilesBuffer offset:0 atIndex:0];
		[rasterEncoder setBuffer:backend->tileOpBuffer offset:0 atIndex:1];
		[rasterEncoder setBuffer:backend->pathBuffer[backend->bufferIndex] offset:pathBufferOffset atIndex:2];
		[rasterEncoder setBuffer:backend->segmentBuffer offset:0 atIndex:3];
		[rasterEncoder setBytes:&tileSize length:sizeof(int) atIndex:4];
		[rasterEncoder setBytes:&scale length:sizeof(float) atIndex:5];
		[rasterEncoder setBytes:&backend->msaaCount length:sizeof(int) atIndex:6];
		[rasterEncoder setBuffer:backend->logBuffer[backend->bufferIndex] offset:0 atIndex:7];
		[rasterEncoder setBuffer:backend->logOffsetBuffer[backend->bufferIndex] offset:0 atIndex:8];

		[rasterEncoder setTexture:backend->outTexture atIndex:0];

		int useTexture = 0;
		if(image)
		{
			mg_mtl_image_data* mtlImage = (mg_mtl_image_data*)image;
			[rasterEncoder setTexture: mtlImage->texture atIndex: 1];
			useTexture = 1;
		}
		[rasterEncoder setBytes: &useTexture length:sizeof(int) atIndex: 9];

		MTLSize rasterGridSize = MTLSizeMake(viewportSize.x, viewportSize.y, 1);
		MTLSize rasterGroupSize = MTLSizeMake(MG_MTL_TILE_SIZE, MG_MTL_TILE_SIZE, 1);

		[rasterEncoder dispatchThreadgroupsWithIndirectBuffer: backend->rasterDispatchBuffer
		                                 indirectBufferOffset: 0
		                                 threadsPerThreadgroup: rasterGroupSize];

		[rasterEncoder endEncoding];

		//NOTE: blit pass
		MTLViewport viewport = {0, 0, viewportSize.x, viewportSize.y, 0, 1};

		MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
		renderPassDescriptor.colorAttachments[0].texture = surface->drawable.texture;
		renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
		renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

		id<MTLRenderCommandEncoder> renderEncoder = [surface->commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
		renderEncoder.label = @"blit pass";
		[renderEncoder setViewport: viewport];
		[renderEncoder setRenderPipelineState: backend->blitPipeline];
		[renderEncoder setFragmentTexture: backend->outTexture atIndex: 0];
		[renderEncoder drawPrimitives: MTLPrimitiveTypeTriangle
			 vertexStart: 0
			 vertexCount: 3 ];
		[renderEncoder endEncoding];
	}

	backend->pathBatchStart = backend->pathCount;
	backend->eltBatchStart = backend->eltCount;

	backend->maxSegmentCount = 0;
	backend->maxTileQueueCount = 0;
}

void mg_mtl_canvas_resize(mg_mtl_canvas_backend* backend, vec2 size)
{
	@autoreleasepool
	{
		if(backend->screenTilesBuffer)
		{
			[backend->screenTilesBuffer release];
			backend->screenTilesBuffer = nil;
		}
		int tileSize = MG_MTL_TILE_SIZE;
		int nTilesX = (int)(size.x + tileSize - 1)/tileSize;
		int nTilesY = (int)(size.y + tileSize - 1)/tileSize;
		MTLResourceOptions bufferOptions = MTLResourceStorageModePrivate;
		backend->screenTilesBuffer = [backend->surface->device newBufferWithLength: nTilesX*nTilesY*sizeof(mg_mtl_screen_tile)
		                                              options: bufferOptions];

		if(backend->outTexture)
	    {
			[backend->outTexture release];
			backend->outTexture = nil;
	    }
		MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
		texDesc.textureType = MTLTextureType2D;
		texDesc.storageMode = MTLStorageModePrivate;
		texDesc.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite | MTLTextureUsageRenderTarget;
		texDesc.pixelFormat = MTLPixelFormatRGBA8Unorm;
		texDesc.width = size.x;
		texDesc.height = size.y;

		backend->outTexture = [backend->surface->device newTextureWithDescriptor:texDesc];

		backend->frameSize = size;
	}
}

void mg_mtl_canvas_render(mg_canvas_backend* interface,
                          mg_color clearColor,
                          u32 primitiveCount,
                          mg_primitive* primitives,
                          u32 eltCount,
                          mg_path_elt* pathElements)
{
	mg_mtl_canvas_backend* backend = (mg_mtl_canvas_backend*)interface;

	//NOTE: update rolling input buffers
	dispatch_semaphore_wait(backend->bufferSemaphore, DISPATCH_TIME_FOREVER);
	backend->bufferIndex = (backend->bufferIndex + 1) % MG_MTL_INPUT_BUFFERS_COUNT;

	//NOTE: ensure screen tiles buffer is correct size
	mg_mtl_surface* surface = backend->surface;

	mp_rect frame = surface->interface.getFrame((mg_surface_data*)surface);

	f32 scale = surface->mtlLayer.contentsScale;
	vec2 viewportSize = {frame.w * scale, frame.h * scale};
	int tileSize = MG_MTL_TILE_SIZE;
	int nTilesX = (int)(frame.w * scale + tileSize - 1)/tileSize;
	int nTilesY = (int)(frame.h * scale + tileSize - 1)/tileSize;

	if(viewportSize.x != backend->frameSize.x || viewportSize.y != backend->frameSize.y)
	{
		mg_mtl_canvas_resize(backend, viewportSize);
	}

	//NOTE: acquire metal resources for rendering
	mg_mtl_surface_acquire_command_buffer(surface);
	mg_mtl_surface_acquire_drawable(surface);

	@autoreleasepool
	{
		//NOTE: clear log counter
		id<MTLBlitCommandEncoder> blitEncoder = [surface->commandBuffer blitCommandEncoder];
		blitEncoder.label = @"clear log counter";
		[blitEncoder fillBuffer: backend->logOffsetBuffer[backend->bufferIndex] range: NSMakeRange(0, sizeof(int)) value: 0];
		[blitEncoder endEncoding];

		//NOTE: clear screen
		MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
		renderPassDescriptor.colorAttachments[0].texture = surface->drawable.texture;
		renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
		renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

		id<MTLRenderCommandEncoder> renderEncoder = [surface->commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
		renderEncoder.label = @"clear pass";
		[renderEncoder endEncoding];
	}
	backend->pathCount = 0;
	backend->pathBatchStart = 0;
	backend->eltCount = 0;
	backend->eltBatchStart = 0;
	backend->maxSegmentCount = 0;
	backend->maxTileQueueCount = 0;

	//NOTE: encode and render batches
	vec2 currentPos = {0};

	mg_image currentImage = mg_image_nil();

	for(int primitiveIndex = 0; primitiveIndex < primitiveCount; primitiveIndex++)
	{
		mg_primitive* primitive = &primitives[primitiveIndex];

		if(primitiveIndex && (primitive->attributes.image.h != currentImage.h))
		{
			mg_image_data* imageData = mg_image_data_from_handle(currentImage);

			mg_mtl_render_batch(backend,
			                    surface,
			                    imageData,
			                    tileSize,
			                    nTilesX,
			                    nTilesY,
			                    viewportSize,
			                    scale);
		}
		currentImage = primitive->attributes.image;

		if(primitive->path.count)
		{
			backend->primitive = primitive;
			backend->pathScreenExtents = (vec4){FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};
			backend->pathUserExtents = (vec4){FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};

			if(primitive->cmd == MG_CMD_STROKE)
			{
				mg_mtl_render_stroke(backend, pathElements + primitive->path.startIndex, &primitive->path);
			}
			else
			{
				for(int eltIndex = 0;
			    	(eltIndex < primitive->path.count) && (primitive->path.startIndex + eltIndex < eltCount);
			    	eltIndex++)
				{
					mg_path_elt* elt = &pathElements[primitive->path.startIndex + eltIndex];

					if(elt->type != MG_PATH_MOVE)
					{
						vec2 p[4] = {currentPos, elt->p[0], elt->p[1], elt->p[2]};
						mg_mtl_canvas_encode_element(backend, elt->type, p);
					}
					switch(elt->type)
					{
						case MG_PATH_MOVE:
							currentPos = elt->p[0];
							break;

						case MG_PATH_LINE:
							currentPos = elt->p[0];
							break;

						case MG_PATH_QUADRATIC:
							currentPos = elt->p[1];
							break;

						case MG_PATH_CUBIC:
							currentPos = elt->p[2];
							break;
					}
				}
			}
			//NOTE: encode path
			mg_mtl_encode_path(backend, primitive, scale);
		}
	}

	mg_image_data* imageData = mg_image_data_from_handle(currentImage);

	mg_mtl_render_batch(backend,
	                    surface,
	                    imageData,
	                    tileSize,
	                    nTilesX,
	                    nTilesY,
	                    viewportSize,
	                    scale);

	@autoreleasepool
	{
		//NOTE: finalize
		[surface->commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> commandBuffer)
			{
				mg_mtl_print_log(backend->bufferIndex, backend->logBuffer[backend->bufferIndex], backend->logOffsetBuffer[backend->bufferIndex]);
				dispatch_semaphore_signal(backend->bufferSemaphore);
			}];
	}
}

void mg_mtl_canvas_destroy(mg_canvas_backend* interface)
{
	mg_mtl_canvas_backend* backend = (mg_mtl_canvas_backend*)interface;

	@autoreleasepool
	{
		[backend->pathPipeline release];
		[backend->segmentPipeline release];
		[backend->backpropPipeline release];
		[backend->mergePipeline release];
		[backend->rasterPipeline release];
		[backend->blitPipeline release];

		for(int i=0; i<MG_MTL_INPUT_BUFFERS_COUNT; i++)
		{
			[backend->pathBuffer[i] release];
			[backend->elementBuffer[i] release];
			[backend->logBuffer[i] release];
			[backend->logOffsetBuffer[i] release];
		}
		[backend->segmentCountBuffer release];
		[backend->segmentBuffer release];
		[backend->tileQueueBuffer release];
		[backend->tileQueueCountBuffer release];
		[backend->tileOpBuffer release];
		[backend->tileOpCountBuffer release];
		[backend->screenTilesBuffer release];
	}

	free(backend);
}

mg_image_data* mg_mtl_canvas_image_create(mg_canvas_backend* interface, vec2 size)
{
	mg_mtl_image_data* image = 0;
	mg_mtl_canvas_backend* backend = (mg_mtl_canvas_backend*)interface;
	mg_mtl_surface* surface = backend->surface;

	@autoreleasepool
	{
		image = malloc_type(mg_mtl_image_data);
		if(image)
		{
			MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
			texDesc.textureType = MTLTextureType2D;
			texDesc.storageMode = MTLStorageModeManaged;
			texDesc.usage = MTLTextureUsageShaderRead;
			texDesc.pixelFormat = MTLPixelFormatRGBA8Unorm;
			texDesc.width = size.x;
			texDesc.height = size.y;

			image->texture = [surface->device newTextureWithDescriptor:texDesc];
			if(image->texture != nil)
			{
				[image->texture retain];
				image->interface.size = size;
			}
			else
			{
				free(image);
				image = 0;
			}
		}
	}
	return((mg_image_data*)image);
}

void mg_mtl_canvas_image_destroy(mg_canvas_backend* backendInterface, mg_image_data* imageInterface)
{
	mg_mtl_image_data* image = (mg_mtl_image_data*)imageInterface;
	@autoreleasepool
	{
		[image->texture release];
		free(image);
	}
}

void mg_mtl_canvas_image_upload_region(mg_canvas_backend* backendInterface, mg_image_data* imageInterface, mp_rect region, u8* pixels)
{@autoreleasepool{
	mg_mtl_image_data* image = (mg_mtl_image_data*)imageInterface;
	MTLRegion mtlRegion = MTLRegionMake2D(region.x, region.y, region.w, region.h);
	[image->texture replaceRegion:mtlRegion
	                mipmapLevel:0
	                withBytes:(void*)pixels
	                bytesPerRow: 4 * region.w];
}}

const u32 MG_MTL_DEFAULT_PATH_BUFFER_LEN       = (4<<10),
          MG_MTL_DEFAULT_ELT_BUFFER_LEN        = (4<<10),

          MG_MTL_DEFAULT_SEGMENT_BUFFER_LEN    = (4<<10),
          MG_MTL_DEFAULT_PATH_QUEUE_BUFFER_LEN = (4<<10),
          MG_MTL_DEFAULT_TILE_QUEUE_BUFFER_LEN = (4<<10),
          MG_MTL_DEFAULT_TILE_OP_BUFFER_LEN    = (4<<20);

mg_canvas_backend* mtl_canvas_backend_create(mg_mtl_surface* surface)
{
	mg_mtl_canvas_backend* backend = 0;

	backend = malloc_type(mg_mtl_canvas_backend);
	memset(backend, 0, sizeof(mg_mtl_canvas_backend));

	backend->msaaCount = MG_MTL_MSAA_COUNT;
	backend->surface = surface;

	//NOTE(martin): setup interface functions
	backend->interface.destroy = mg_mtl_canvas_destroy;
	backend->interface.render = mg_mtl_canvas_render;
	backend->interface.imageCreate = mg_mtl_canvas_image_create;
	backend->interface.imageDestroy = mg_mtl_canvas_image_destroy;
	backend->interface.imageUploadRegion = mg_mtl_canvas_image_upload_region;

	@autoreleasepool{
		//NOTE: load metal library
		str8 shaderPath = path_executable_relative(mem_scratch(), STR8("mtl_renderer.metallib"));
		NSString* metalFileName = [[NSString alloc] initWithBytes: shaderPath.ptr length:shaderPath.len encoding: NSUTF8StringEncoding];
		NSError* err = 0;
		id<MTLLibrary> library = [surface->device newLibraryWithFile: metalFileName error:&err];
		if(err != nil)
		{
			const char* errStr = [[err localizedDescription] UTF8String];
			log_error("error : %s\n", errStr);
			return(0);
		}
		id<MTLFunction> pathFunction = [library newFunctionWithName:@"mtl_path_setup"];
		id<MTLFunction> segmentFunction = [library newFunctionWithName:@"mtl_segment_setup"];
		id<MTLFunction> backpropFunction = [library newFunctionWithName:@"mtl_backprop"];
		id<MTLFunction> mergeFunction = [library newFunctionWithName:@"mtl_merge"];
		id<MTLFunction> rasterFunction = [library newFunctionWithName:@"mtl_raster"];
		id<MTLFunction> vertexFunction = [library newFunctionWithName:@"mtl_vertex_shader"];
		id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"mtl_fragment_shader"];

		//NOTE: create pipelines
		NSError* error = NULL;

		backend->pathPipeline = [surface->device newComputePipelineStateWithFunction: pathFunction
		                                              error:&error];

		backend->segmentPipeline = [surface->device newComputePipelineStateWithFunction: segmentFunction
		                                                 error:&error];

		backend->backpropPipeline = [surface->device newComputePipelineStateWithFunction: backpropFunction
		                                                 error:&error];

		backend->mergePipeline = [surface->device newComputePipelineStateWithFunction: mergeFunction
		                                                 error:&error];

		backend->rasterPipeline = [surface->device newComputePipelineStateWithFunction: rasterFunction
		                                                 error:&error];

		MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
		pipelineStateDescriptor.label = @"blit pipeline";
		pipelineStateDescriptor.vertexFunction = vertexFunction;
		pipelineStateDescriptor.fragmentFunction = fragmentFunction;
		pipelineStateDescriptor.colorAttachments[0].pixelFormat = surface->mtlLayer.pixelFormat;
		pipelineStateDescriptor.colorAttachments[0].blendingEnabled = YES;
		pipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
		pipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
		pipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
		pipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
		pipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
		pipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

		backend->blitPipeline = [surface->device newRenderPipelineStateWithDescriptor: pipelineStateDescriptor error:&err];

		//NOTE: create textures
		mp_rect frame = surface->interface.getFrame((mg_surface_data*)surface);
		f32 scale = surface->mtlLayer.contentsScale;

		backend->frameSize = (vec2){frame.w*scale, frame.h*scale};

		MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
		texDesc.textureType = MTLTextureType2D;
		texDesc.storageMode = MTLStorageModePrivate;
		texDesc.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite | MTLTextureUsageRenderTarget;
		texDesc.pixelFormat = MTLPixelFormatRGBA8Unorm;
		texDesc.width = backend->frameSize.x;
		texDesc.height = backend->frameSize.y;

		backend->outTexture = [surface->device newTextureWithDescriptor:texDesc];

		//NOTE: create buffers

		backend->bufferSemaphore = dispatch_semaphore_create(MG_MTL_INPUT_BUFFERS_COUNT);
		backend->bufferIndex = 0;

		MTLResourceOptions bufferOptions = MTLResourceCPUCacheModeWriteCombined
		                                 | MTLResourceStorageModeShared;

		for(int i=0; i<MG_MTL_INPUT_BUFFERS_COUNT; i++)
		{
			backend->pathBuffer[i] = [surface->device newBufferWithLength: MG_MTL_DEFAULT_PATH_BUFFER_LEN * sizeof(mg_mtl_path)
			                                               options: bufferOptions];

			backend->elementBuffer[i] = [surface->device newBufferWithLength: MG_MTL_DEFAULT_ELT_BUFFER_LEN * sizeof(mg_mtl_path_elt)
			                                                  options: bufferOptions];
		}

		bufferOptions = MTLResourceStorageModePrivate;
		backend->segmentBuffer = [surface->device newBufferWithLength: MG_MTL_DEFAULT_SEGMENT_BUFFER_LEN * sizeof(mg_mtl_segment)
		                                               options: bufferOptions];

		backend->segmentCountBuffer = [surface->device newBufferWithLength: sizeof(int)
		                                                    options: bufferOptions];

		backend->pathQueueBuffer = [surface->device newBufferWithLength: MG_MTL_DEFAULT_PATH_QUEUE_BUFFER_LEN * sizeof(mg_mtl_path_queue)
		                                                 options: bufferOptions];

		backend->tileQueueBuffer = [surface->device newBufferWithLength: MG_MTL_DEFAULT_TILE_QUEUE_BUFFER_LEN * sizeof(mg_mtl_tile_queue)
		                                                 options: bufferOptions];

		backend->tileQueueCountBuffer = [surface->device newBufferWithLength: sizeof(int)
		                                                      options: bufferOptions];

		backend->tileOpBuffer = [surface->device newBufferWithLength: MG_MTL_DEFAULT_TILE_OP_BUFFER_LEN * sizeof(mg_mtl_tile_op)
		                                              options: bufferOptions];

		backend->tileOpCountBuffer = [surface->device newBufferWithLength: sizeof(int)
		                                                   options: bufferOptions];

		backend->rasterDispatchBuffer = [surface->device newBufferWithLength: sizeof(MTLDispatchThreadgroupsIndirectArguments)
		                                                   options: bufferOptions];

		int tileSize = MG_MTL_TILE_SIZE;
		int nTilesX = (int)(frame.w * scale + tileSize - 1)/tileSize;
		int nTilesY = (int)(frame.h * scale + tileSize - 1)/tileSize;
		backend->screenTilesBuffer = [surface->device newBufferWithLength: nTilesX*nTilesY*sizeof(mg_mtl_screen_tile)
		                                                   options: bufferOptions];

		bufferOptions = MTLResourceStorageModeShared;
		for(int i=0; i<MG_MTL_INPUT_BUFFERS_COUNT; i++)
		{
			backend->logBuffer[i] = [surface->device newBufferWithLength: 1<<20
			                                              options: bufferOptions];

			backend->logOffsetBuffer[i] = [surface->device newBufferWithLength: sizeof(int)
			                                                    options: bufferOptions];
		}
	}
	return((mg_canvas_backend*)backend);
}

mg_surface_data* mtl_canvas_surface_create_for_window(mp_window window)
{
	mg_mtl_surface* surface = (mg_mtl_surface*)mg_mtl_surface_create_for_window(window);

	if(surface)
	{
		surface->interface.backend = mtl_canvas_backend_create(surface);
		if(surface->interface.backend)
		{
			surface->interface.api = MG_CANVAS;
		}
		else
		{
			surface->interface.destroy((mg_surface_data*)surface);
			surface = 0;
		}
	}
	return((mg_surface_data*)surface);
}
