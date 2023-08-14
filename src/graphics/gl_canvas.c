/************************************************************//**
*
*	@file: gl_canvas.c
*	@author: Martin Fouilleul
*	@date: 29/01/2023
*	@revision:
*
*****************************************************************/
#include"graphics_surface.h"
#include"util/macros.h"
#include"glsl_shaders.h"
#include"gl_api.h"

typedef struct oc_gl_image
{
	oc_image_data interface;
	GLuint texture;
} oc_gl_image;

enum oc_gl_cmd_enum {
	OC_GL_FILL,
	OC_GL_STROKE,
};
typedef int oc_gl_cmd;

typedef struct oc_gl_path
{
	float uvTransform[12];
	oc_vec4 color;
	oc_vec4 box;
	oc_vec4 clip;
	oc_gl_cmd cmd;
	int textureID;
	u8 pad[8];
} oc_gl_path;

enum oc_gl_seg_kind_enum
{
	OC_GL_LINE = 1,
	OC_GL_QUADRATIC,
	OC_GL_CUBIC,
};
typedef int oc_gl_seg_kind;

typedef struct oc_gl_path_elt
{
	oc_vec2 p[4];
	int pathIndex;
	oc_gl_seg_kind kind;

} oc_gl_path_elt;

typedef struct oc_gl_dispatch_indirect_command
{
	u32  num_groups_x;
	u32  num_groups_y;
	u32  num_groups_z;

} oc_gl_dispatch_indirect_command;
////////////////////////////////////////////////////////////
//NOTE: these are just here for the sizes...

#define OC_GL_LAYOUT_FIRST(name, type) \
	OC_GL_##name##_OFFSET = 0, \
	OC_GL_##name##_SIZE = OC_GL_##type##_SIZE,

#define OC_GL_LAYOUT_NEXT(name, type, prev) \
	OC_GL_##name##_OFFSET = oc_align_up_pow2(OC_GL_##prev##_OFFSET + OC_GL_##prev##_SIZE, OC_GL_##type##_ALIGN), \
	OC_GL_##name##_SIZE = OC_GL_##type##_SIZE,

#define OC_GL_LAYOUT_SIZE(name, last, maxAlignType) \
	OC_GL_##name##_ALIGN = oc_align_up_pow2(OC_GL_##maxAlignType##_ALIGN, OC_GL_VEC4_ALIGN), \
	OC_GL_##name##_SIZE = oc_align_up_pow2(OC_GL_##last##_OFFSET + OC_GL_##last##_SIZE, OC_GL_##name##_ALIGN),

enum
{
	OC_GL_I32_SIZE = sizeof(i32),
	OC_GL_I32_ALIGN = sizeof(i32),
	OC_GL_F32_SIZE = sizeof(f32),
	OC_GL_F32_ALIGN = sizeof(f32),
	OC_GL_VEC2_SIZE = 2*sizeof(f32),
	OC_GL_VEC2_ALIGN = 2*sizeof(f32),
	OC_GL_VEC3_SIZE = 4*sizeof(f32),
	OC_GL_VEC3_ALIGN = 4*sizeof(f32),
	OC_GL_VEC4_SIZE = 4*sizeof(f32),
	OC_GL_VEC4_ALIGN = 4*sizeof(f32),
	OC_GL_MAT3_SIZE = 3*3*OC_GL_VEC3_SIZE,
	OC_GL_MAT3_ALIGN = OC_GL_VEC3_ALIGN,

	OC_GL_LAYOUT_FIRST(SEGMENT_KIND, I32)
	OC_GL_LAYOUT_NEXT(SEGMENT_PATH_INDEX, I32, SEGMENT_KIND)
	OC_GL_LAYOUT_NEXT(SEGMENT_CONFIG, I32, SEGMENT_PATH_INDEX)
	OC_GL_LAYOUT_NEXT(SEGMENT_WINDING, I32, SEGMENT_CONFIG)
	OC_GL_LAYOUT_NEXT(SEGMENT_BOX, VEC4, SEGMENT_WINDING)
	OC_GL_LAYOUT_NEXT(SEGMENT_IMPLICIT_MATRIX, MAT3, SEGMENT_BOX)
	OC_GL_LAYOUT_NEXT(SEGMENT_HULL_VERTEX, VEC2, SEGMENT_IMPLICIT_MATRIX)
	OC_GL_LAYOUT_NEXT(SEGMENT_SIGN, F32, SEGMENT_HULL_VERTEX)
	OC_GL_LAYOUT_SIZE(SEGMENT, SEGMENT_SIGN, MAT3)

	OC_GL_LAYOUT_FIRST(PATH_QUEUE_AREA, VEC4)
	OC_GL_LAYOUT_NEXT(PATH_QUEUE_TILE_QUEUES, I32, PATH_QUEUE_AREA)
	OC_GL_LAYOUT_SIZE(PATH_QUEUE, PATH_QUEUE_TILE_QUEUES, VEC4)

	OC_GL_LAYOUT_FIRST(TILE_OP_KIND, I32)
	OC_GL_LAYOUT_NEXT(TILE_OP_NEXT, I32, TILE_OP_KIND)
	OC_GL_LAYOUT_NEXT(TILE_OP_INDEX, I32, TILE_OP_NEXT)
	OC_GL_LAYOUT_NEXT(TILE_OP_WINDING, I32, TILE_OP_INDEX)
	OC_GL_LAYOUT_SIZE(TILE_OP, TILE_OP_WINDING, I32)

	OC_GL_LAYOUT_FIRST(TILE_QUEUE_WINDING, I32)
	OC_GL_LAYOUT_NEXT(TILE_QUEUE_FIRST, I32, TILE_QUEUE_WINDING)
	OC_GL_LAYOUT_NEXT(TILE_QUEUE_LAST, I32, TILE_QUEUE_FIRST)
	OC_GL_LAYOUT_SIZE(TILE_QUEUE, TILE_QUEUE_LAST, I32)

	OC_GL_LAYOUT_FIRST(SCREEN_TILE_COORD, VEC2)
	OC_GL_LAYOUT_NEXT(SCREEN_TILE_FIRST, I32, SCREEN_TILE_COORD)
	OC_GL_LAYOUT_SIZE(SCREEN_TILE, SCREEN_TILE_FIRST, VEC2)
};

enum {
	OC_GL_INPUT_BUFFERS_COUNT = 3,
	OC_GL_TILE_SIZE = 16,
	OC_GL_MSAA_COUNT = 8,
	OC_GL_MAX_IMAGES_PER_BATCH = 8,
};

typedef struct oc_gl_mapped_buffer
{
	GLuint buffer;
	int size;
	char* contents;
} oc_gl_mapped_buffer;

typedef struct oc_gl_canvas_backend
{
	oc_canvas_backend interface;
	oc_wgl_surface* surface;

	int msaaCount;
	oc_vec2 frameSize;

	// gl stuff
	GLuint vao;

	GLuint pathSetup;
	GLuint segmentSetup;
	GLuint backprop;
	GLuint merge;
	GLuint balanceWorkgroups;
	GLuint raster;
	GLuint blit;

	GLuint outTexture;

	int bufferIndex;
	GLsync bufferSync[OC_GL_INPUT_BUFFERS_COUNT];
	oc_gl_mapped_buffer pathBuffer[OC_GL_INPUT_BUFFERS_COUNT];
	oc_gl_mapped_buffer elementBuffer[OC_GL_INPUT_BUFFERS_COUNT];

	GLuint segmentBuffer;
	GLuint segmentCountBuffer;
	GLuint pathQueueBuffer;
	GLuint tileQueueBuffer;
	GLuint tileQueueCountBuffer;
	GLuint tileOpBuffer;
	GLuint tileOpCountBuffer;
	GLuint screenTilesBuffer;
	GLuint screenTilesCountBuffer;
	GLuint rasterDispatchBuffer;
	GLuint dummyVertexBuffer;

	//encoding context
	int pathCount;
	int eltCount;

	int pathBatchStart;
	int eltBatchStart;

	oc_primitive* primitive;
	oc_vec4 pathScreenExtents;
	oc_vec4 pathUserExtents;

	int maxTileQueueCount;
	int maxSegmentCount;

	int currentImageIndex;
} oc_gl_canvas_backend;

static void oc_update_path_extents(oc_vec4* extents, oc_vec2 p)
{
	extents->x = oc_min(extents->x, p.x);
	extents->y = oc_min(extents->y, p.y);
	extents->z = oc_max(extents->z, p.x);
	extents->w = oc_max(extents->w, p.y);
}

void oc_gl_grow_input_buffer(oc_gl_mapped_buffer* buffer, int copyStart, int copySize, int newSize)
{
	oc_gl_mapped_buffer newBuffer = {0};
	newBuffer.size = newSize;
	glGenBuffers(1, &newBuffer.buffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, newBuffer.buffer);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, newBuffer.size, 0, GL_MAP_WRITE_BIT|GL_MAP_PERSISTENT_BIT);
	newBuffer.contents = glMapBufferRange(GL_SHADER_STORAGE_BUFFER,
	                                      0,
	                                      newBuffer.size,
	                                       GL_MAP_WRITE_BIT
	                                      |GL_MAP_PERSISTENT_BIT
	                                      |GL_MAP_FLUSH_EXPLICIT_BIT);

	memcpy(newBuffer.contents + copyStart, buffer->contents + copyStart, copySize);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer->buffer);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glDeleteBuffers(1, &buffer->buffer);

	*buffer = newBuffer;
}

void oc_gl_canvas_encode_element(oc_gl_canvas_backend* backend, oc_path_elt_type kind, oc_vec2* p)
{
	int bufferIndex = backend->bufferIndex;
	int bufferCap = backend->elementBuffer[bufferIndex].size / sizeof(oc_gl_path_elt);
	if(backend->eltCount >= bufferCap)
	{
		int newBufferCap = (int)(bufferCap * 1.5);
		int newBufferSize = newBufferCap * sizeof(oc_gl_path_elt);

		oc_log_info("growing element buffer to %i elements\n", newBufferCap);

		oc_gl_grow_input_buffer(&backend->elementBuffer[bufferIndex],
		                        backend->eltBatchStart * sizeof(oc_gl_path_elt),
		                        backend->eltCount * sizeof(oc_gl_path_elt),
		                        newBufferSize);
	}

	oc_gl_path_elt* elementData = (oc_gl_path_elt*)backend->elementBuffer[bufferIndex].contents;
	oc_gl_path_elt* elt = &elementData[backend->eltCount];
	backend->eltCount++;

	elt->pathIndex = backend->pathCount - backend->pathBatchStart;
	int count = 0;
	switch(kind)
	{
		case OC_PATH_LINE:
			backend->maxSegmentCount += 1;
			elt->kind = OC_GL_LINE;
			count = 2;
			break;

		case OC_PATH_QUADRATIC:
			backend->maxSegmentCount += 3;
			elt->kind = OC_GL_QUADRATIC;
			count = 3;
			break;

		case OC_PATH_CUBIC:
			backend->maxSegmentCount += 7;
			elt->kind = OC_GL_CUBIC;
			count = 4;
			break;

		default:
			break;
	}

	for(int i=0; i<count; i++)
	{
		oc_update_path_extents(&backend->pathUserExtents, p[i]);

		oc_vec2 screenP = oc_mat2x3_mul(backend->primitive->attributes.transform, p[i]);
		elt->p[i] = (oc_vec2){screenP.x, screenP.y};

		oc_update_path_extents(&backend->pathScreenExtents, screenP);
	}
}

void oc_gl_canvas_encode_path(oc_gl_canvas_backend* backend, oc_primitive* primitive, f32 scale)
{
	int bufferIndex = backend->bufferIndex;
	int bufferCap = backend->pathBuffer[bufferIndex].size / sizeof(oc_gl_path);
	if(backend->pathCount >= bufferCap)
	{
		int newBufferCap = (int)(bufferCap * 1.5);
		int newBufferSize = newBufferCap * sizeof(oc_gl_path);

		oc_log_info("growing path buffer to %i elements\n", newBufferCap);

		oc_gl_grow_input_buffer(&backend->pathBuffer[bufferIndex],
		                        backend->pathBatchStart * sizeof(oc_gl_path),
		                        backend->eltCount * sizeof(oc_gl_path),
		                        newBufferSize);
	}

	oc_gl_path* pathData = (oc_gl_path*)backend->pathBuffer[backend->bufferIndex].contents;
	oc_gl_path* path = &pathData[backend->pathCount];
	backend->pathCount++;

	path->cmd =	(oc_gl_cmd)primitive->cmd;

	path->box = (oc_vec4){
		backend->pathScreenExtents.x,
		backend->pathScreenExtents.y,
		backend->pathScreenExtents.z,
		backend->pathScreenExtents.w};

	path->clip = (oc_vec4){
		primitive->attributes.clip.x,
		primitive->attributes.clip.y,
		primitive->attributes.clip.x + primitive->attributes.clip.w,
		primitive->attributes.clip.y + primitive->attributes.clip.h};

	path->color = (oc_vec4){
		primitive->attributes.color.r,
		primitive->attributes.color.g,
		primitive->attributes.color.b,
		primitive->attributes.color.a};

	oc_rect srcRegion = primitive->attributes.srcRegion;

	oc_rect destRegion = {
		backend->pathUserExtents.x,
		backend->pathUserExtents.y,
		backend->pathUserExtents.z - backend->pathUserExtents.x,
		backend->pathUserExtents.w - backend->pathUserExtents.y};

	if(!oc_image_is_nil(primitive->attributes.image))
	{
		oc_vec2 texSize = oc_image_size(primitive->attributes.image);

		oc_mat2x3 srcRegionToImage = {
			1/texSize.x, 0, srcRegion.x/texSize.x,
			0, 1/texSize.y, srcRegion.y/texSize.y};

		oc_mat2x3 destRegionToSrcRegion = {
			srcRegion.w/destRegion.w, 0, 0,
			0, srcRegion.h/destRegion.h, 0};

		oc_mat2x3 userToDestRegion = {
			1, 0, -destRegion.x,
			0, 1, -destRegion.y};

		oc_mat2x3 screenToUser = oc_mat2x3_inv(primitive->attributes.transform);

		oc_mat2x3 uvTransform = srcRegionToImage;
		uvTransform = oc_mat2x3_mul_m(uvTransform, destRegionToSrcRegion);
		uvTransform = oc_mat2x3_mul_m(uvTransform, userToDestRegion);
		uvTransform = oc_mat2x3_mul_m(uvTransform, screenToUser);

		//NOTE: mat3 std430 layout is an array of oc_vec3, which are padded to _oc_vec4_ alignment
		path->uvTransform[0] = uvTransform.m[0]/scale;
		path->uvTransform[1] = uvTransform.m[3]/scale;
		path->uvTransform[2] = 0;
		path->uvTransform[3] = 0;
		path->uvTransform[4] = uvTransform.m[1]/scale;
		path->uvTransform[5] = uvTransform.m[4]/scale;
		path->uvTransform[6] = 0;
		path->uvTransform[7] = 0;
		path->uvTransform[8] = uvTransform.m[2];
		path->uvTransform[9] = uvTransform.m[5];
		path->uvTransform[10] = 1;
		path->uvTransform[11] = 0;

		path->textureID = backend->currentImageIndex;
	}
	else
	{
		path->textureID = -1;
	}

	int firstTileX = path->box.x*scale / OC_GL_TILE_SIZE;
	int firstTileY = path->box.y*scale / OC_GL_TILE_SIZE;
	int lastTileX = path->box.z*scale / OC_GL_TILE_SIZE;
	int lastTileY = path->box.w*scale / OC_GL_TILE_SIZE;

	int nTilesX = lastTileX - firstTileX + 1;
	int nTilesY = lastTileY - firstTileY + 1;

	backend->maxTileQueueCount += (nTilesX * nTilesY);
}

static bool oc_intersect_hull_legs(oc_vec2 p0, oc_vec2 p1, oc_vec2 p2, oc_vec2 p3, oc_vec2* intersection)
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

static bool oc_offset_hull(int count, oc_vec2* p, oc_vec2* result, f32 offset)
{
	//NOTE: we should have no more than two coincident points here. This means the leg between
	//      those two points can't be offset, but we can set a double point at the start of first leg,
	//      end of first leg, or we can join the first and last leg to create a missing middle one

	oc_vec2 legs[3][2] = {0};
	bool valid[3] = {0};

	for(int i=0; i<count-1; i++)
	{
		oc_vec2 n = {p[i].y - p[i+1].y,
	              p[i+1].x - p[i].x};

		f32 norm = sqrt(n.x*n.x + n.y*n.y);
		if(norm >= 1e-6)
		{
			n = oc_vec2_mul(offset/norm, n);
			legs[i][0] = oc_vec2_add(p[i], n);
			legs[i][1] = oc_vec2_add(p[i+1], n);
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
		OC_ASSERT(valid[1]);
		result[0] = legs[1][0];
	}

	for(int i=1; i<count-1; i++)
	{
		//NOTE: we're computing the control point i, at the end of leg (i-1)

		if(!valid[i-1])
		{
			OC_ASSERT(valid[i]);
			result[i] = legs[i][0];
		}
		else if(!valid[i])
		{
			OC_ASSERT(valid[i-1]);
			result[i] = legs[i-1][0];
		}
		else
		{
			if(!oc_intersect_hull_legs(legs[i-1][0], legs[i-1][1], legs[i][0], legs[i][1], &result[i]))
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
		OC_ASSERT(valid[count-3]);
		result[count-1] = legs[count-3][1];
	}

	return(true);
}

static oc_vec2 oc_quadratic_get_point(oc_vec2 p[3], f32 t)
{
	oc_vec2 r;

	f32 oneMt = 1-t;
	f32 oneMt2 = oc_square(oneMt);
	f32 t2 = oc_square(t);

	r.x = oneMt2*p[0].x + 2*oneMt*t*p[1].x + t2*p[2].x;
	r.y = oneMt2*p[0].y + 2*oneMt*t*p[1].y + t2*p[2].y;

	return(r);
}

static void oc_quadratic_split(oc_vec2 p[3], f32 t, oc_vec2 outLeft[3], oc_vec2 outRight[3])
{
	//NOTE(martin): split bezier curve p at parameter t, using De Casteljau's algorithm
	//              the q_n are the points along the hull's segments at parameter t
	//              s is the split point.

	f32 oneMt = 1-t;

	oc_vec2 q0 = {oneMt*p[0].x + t*p[1].x,
		   oneMt*p[0].y + t*p[1].y};

	oc_vec2 q1 = {oneMt*p[1].x + t*p[2].x,
		   oneMt*p[1].y + t*p[2].y};

	oc_vec2 s = {oneMt*q0.x + t*q1.x,
		   oneMt*q0.y + t*q1.y};

	outLeft[0] = p[0];
	outLeft[1] = q0;
	outLeft[2] = s;

	outRight[0] = s;
	outRight[1] = q1;
	outRight[2] = p[2];
}

static oc_vec2 oc_cubic_get_point(oc_vec2 p[4], f32 t)
{
	oc_vec2 r;

	f32 oneMt = 1-t;
	f32 oneMt2 = oc_square(oneMt);
	f32 oneMt3 = oneMt2*oneMt;
	f32 t2 = oc_square(t);
	f32 t3 = t2*t;

	r.x = oneMt3*p[0].x + 3*oneMt2*t*p[1].x + 3*oneMt*t2*p[2].x + t3*p[3].x;
	r.y = oneMt3*p[0].y + 3*oneMt2*t*p[1].y + 3*oneMt*t2*p[2].y + t3*p[3].y;

	return(r);
}

static void oc_cubic_split(oc_vec2 p[4], f32 t, oc_vec2 outLeft[4], oc_vec2 outRight[4])
{
	//NOTE(martin): split bezier curve p at parameter t, using De Casteljau's algorithm
	//              the q_n are the points along the hull's segments at parameter t
	//              the r_n are the points along the (q_n, q_n+1) segments at parameter t
	//              s is the split point.

	oc_vec2 q0 = {(1-t)*p[0].x + t*p[1].x,
	           (1-t)*p[0].y + t*p[1].y};

	oc_vec2 q1 = {(1-t)*p[1].x + t*p[2].x,
	           (1-t)*p[1].y + t*p[2].y};

	oc_vec2 q2 = {(1-t)*p[2].x + t*p[3].x,
	           (1-t)*p[2].y + t*p[3].y};

	oc_vec2 r0 = {(1-t)*q0.x + t*q1.x,
	           (1-t)*q0.y + t*q1.y};

	oc_vec2 r1 = {(1-t)*q1.x + t*q2.x,
	           (1-t)*q1.y + t*q2.y};

	oc_vec2 s = {(1-t)*r0.x + t*r1.x,
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

void oc_gl_encode_stroke_line(oc_gl_canvas_backend* backend, oc_vec2* p)
{
	f32 width = backend->primitive->attributes.width;

	oc_vec2 v = {p[1].x-p[0].x, p[1].y-p[0].y};
	oc_vec2 n = {v.y, -v.x};
	f32 norm = sqrt(n.x*n.x + n.y*n.y);
	oc_vec2 offset = oc_vec2_mul(0.5*width/norm, n);

	oc_vec2 left[2] = {oc_vec2_add(p[0], offset), oc_vec2_add(p[1], offset)};
	oc_vec2 right[2] = {oc_vec2_add(p[1], oc_vec2_mul(-1, offset)), oc_vec2_add(p[0], oc_vec2_mul(-1, offset))};
	oc_vec2 joint0[2] = {oc_vec2_add(p[0], oc_vec2_mul(-1, offset)), oc_vec2_add(p[0], offset)};
	oc_vec2 joint1[2] = {oc_vec2_add(p[1], offset), oc_vec2_add(p[1], oc_vec2_mul(-1, offset))};

	oc_gl_canvas_encode_element(backend, OC_PATH_LINE, right);

	oc_gl_canvas_encode_element(backend, OC_PATH_LINE, left);
	oc_gl_canvas_encode_element(backend, OC_PATH_LINE, joint0);
	oc_gl_canvas_encode_element(backend, OC_PATH_LINE, joint1);
}

enum { OC_HULL_CHECK_SAMPLE_COUNT = 5 };

void oc_gl_encode_stroke_quadratic(oc_gl_canvas_backend* backend, oc_vec2* p)
{
	f32 width = backend->primitive->attributes.width;
	f32 tolerance = oc_min(backend->primitive->attributes.tolerance, 0.5 * width);

	//NOTE: check for degenerate line case
	const f32 equalEps = 1e-3;
	if(oc_vec2_close(p[0], p[1], equalEps))
	{
		oc_gl_encode_stroke_line(backend, p+1);
		return;
	}
	else if(oc_vec2_close(p[1], p[2], equalEps))
	{
		oc_gl_encode_stroke_line(backend, p);
		return;
	}

	oc_vec2 leftHull[3];
	oc_vec2 rightHull[3];

	if(  !oc_offset_hull(3, p, leftHull, width/2)
	  || !oc_offset_hull(3, p, rightHull, -width/2))
	{
		//TODO split and recurse
		//NOTE: offsetting the hull failed, split the curve
		oc_vec2 splitLeft[3];
		oc_vec2 splitRight[3];
		oc_quadratic_split(p, 0.5, splitLeft, splitRight);
		oc_gl_encode_stroke_quadratic(backend, splitLeft);
		oc_gl_encode_stroke_quadratic(backend, splitRight);
	}
	else
	{
		f32 checkSamples[OC_HULL_CHECK_SAMPLE_COUNT] = {1./6, 2./6, 3./6, 4./6, 5./6};

		f32 d2LowBound = oc_square(0.5 * width - tolerance);
		f32 d2HighBound = oc_square(0.5 * width + tolerance);

		f32 maxOvershoot = 0;
		f32 maxOvershootParameter = 0;

		for(int i=0; i<OC_HULL_CHECK_SAMPLE_COUNT; i++)
		{
			f32 t = checkSamples[i];

			oc_vec2 c = oc_quadratic_get_point(p, t);
			oc_vec2 cp =  oc_quadratic_get_point(leftHull, t);
			oc_vec2 cn =  oc_quadratic_get_point(rightHull, t);

			f32 positiveDistSquare = oc_square(c.x - cp.x) + oc_square(c.y - cp.y);
			f32 negativeDistSquare = oc_square(c.x - cn.x) + oc_square(c.y - cn.y);

			f32 positiveOvershoot = oc_max(positiveDistSquare - d2HighBound, d2LowBound - positiveDistSquare);
			f32 negativeOvershoot = oc_max(negativeDistSquare - d2HighBound, d2LowBound - negativeDistSquare);

			f32 overshoot = oc_max(positiveOvershoot, negativeOvershoot);

			if(overshoot > maxOvershoot)
			{
				maxOvershoot = overshoot;
				maxOvershootParameter = t;
			}
		}

		if(maxOvershoot > 0)
		{
			oc_vec2 splitLeft[3];
			oc_vec2 splitRight[3];
			oc_quadratic_split(p, maxOvershootParameter, splitLeft, splitRight);
			oc_gl_encode_stroke_quadratic(backend, splitLeft);
			oc_gl_encode_stroke_quadratic(backend, splitRight);
		}
		else
		{
			oc_vec2 tmp = leftHull[0];
			leftHull[0] = leftHull[2];
			leftHull[2] = tmp;

			oc_gl_canvas_encode_element(backend, OC_PATH_QUADRATIC, rightHull);
			oc_gl_canvas_encode_element(backend, OC_PATH_QUADRATIC, leftHull);

			oc_vec2 joint0[2] = {rightHull[2], leftHull[0]};
			oc_vec2 joint1[2] = {leftHull[2], rightHull[0]};
			oc_gl_canvas_encode_element(backend, OC_PATH_LINE, joint0);
			oc_gl_canvas_encode_element(backend, OC_PATH_LINE, joint1);
		}
	}
}

void oc_gl_encode_stroke_cubic(oc_gl_canvas_backend* backend, oc_vec2* p)
{
	f32 width = backend->primitive->attributes.width;
	f32 tolerance = oc_min(backend->primitive->attributes.tolerance, 0.5 * width);

	//NOTE: check degenerate line cases
	f32 equalEps = 1e-3;

	if( (oc_vec2_close(p[0], p[1], equalEps) && oc_vec2_close(p[2], p[3], equalEps))
	  ||(oc_vec2_close(p[0], p[1], equalEps) && oc_vec2_close(p[1], p[2], equalEps))
	  ||(oc_vec2_close(p[1], p[2], equalEps) && oc_vec2_close(p[2], p[3], equalEps)))
	{
		oc_vec2 line[2] = {p[0], p[3]};
		oc_gl_encode_stroke_line(backend, line);
		return;
	}
	else if(oc_vec2_close(p[0], p[1], equalEps) && oc_vec2_close(p[1], p[3], equalEps))
	{
		oc_vec2 line[2] = {p[0], oc_vec2_add(oc_vec2_mul(5./9, p[0]), oc_vec2_mul(4./9, p[2]))};
		oc_gl_encode_stroke_line(backend, line);
		return;
	}
	else if(oc_vec2_close(p[0], p[2], equalEps) && oc_vec2_close(p[2], p[3], equalEps))
	{
		oc_vec2 line[2] = {p[0], oc_vec2_add(oc_vec2_mul(5./9, p[0]), oc_vec2_mul(4./9, p[1]))};
		oc_gl_encode_stroke_line(backend, line);
		return;
	}

	oc_vec2 leftHull[4];
	oc_vec2 rightHull[4];

	if(  !oc_offset_hull(4, p, leftHull, width/2)
	  || !oc_offset_hull(4, p, rightHull, -width/2))
	{
		//TODO split and recurse
		//NOTE: offsetting the hull failed, split the curve
		oc_vec2 splitLeft[4];
		oc_vec2 splitRight[4];
		oc_cubic_split(p, 0.5, splitLeft, splitRight);
		oc_gl_encode_stroke_cubic(backend, splitLeft);
		oc_gl_encode_stroke_cubic(backend, splitRight);
	}
	else
	{
		f32 checkSamples[OC_HULL_CHECK_SAMPLE_COUNT] = {1./6, 2./6, 3./6, 4./6, 5./6};

		f32 d2LowBound = oc_square(0.5 * width - tolerance);
		f32 d2HighBound = oc_square(0.5 * width + tolerance);

		f32 maxOvershoot = 0;
		f32 maxOvershootParameter = 0;

		for(int i=0; i<OC_HULL_CHECK_SAMPLE_COUNT; i++)
		{
			f32 t = checkSamples[i];

			oc_vec2 c = oc_cubic_get_point(p, t);
			oc_vec2 cp =  oc_cubic_get_point(leftHull, t);
			oc_vec2 cn =  oc_cubic_get_point(rightHull, t);

			f32 positiveDistSquare = oc_square(c.x - cp.x) + oc_square(c.y - cp.y);
			f32 negativeDistSquare = oc_square(c.x - cn.x) + oc_square(c.y - cn.y);

			f32 positiveOvershoot = oc_max(positiveDistSquare - d2HighBound, d2LowBound - positiveDistSquare);
			f32 negativeOvershoot = oc_max(negativeDistSquare - d2HighBound, d2LowBound - negativeDistSquare);

			f32 overshoot = oc_max(positiveOvershoot, negativeOvershoot);

			if(overshoot > maxOvershoot)
			{
				maxOvershoot = overshoot;
				maxOvershootParameter = t;
			}
		}

		if(maxOvershoot > 0)
		{
			oc_vec2 splitLeft[4];
			oc_vec2 splitRight[4];
			oc_cubic_split(p, maxOvershootParameter, splitLeft, splitRight);
			oc_gl_encode_stroke_cubic(backend, splitLeft);
			oc_gl_encode_stroke_cubic(backend, splitRight);
		}
		else
		{
			oc_vec2 tmp = leftHull[0];
			leftHull[0] = leftHull[3];
			leftHull[3] = tmp;
			tmp = leftHull[1];
			leftHull[1] = leftHull[2];
			leftHull[2] = tmp;

			oc_gl_canvas_encode_element(backend, OC_PATH_CUBIC, rightHull);
			oc_gl_canvas_encode_element(backend, OC_PATH_CUBIC, leftHull);

			oc_vec2 joint0[2] = {rightHull[3], leftHull[0]};
			oc_vec2 joint1[2] = {leftHull[3], rightHull[0]};
			oc_gl_canvas_encode_element(backend, OC_PATH_LINE, joint0);
			oc_gl_canvas_encode_element(backend, OC_PATH_LINE, joint1);
		}
	}
}

void oc_gl_encode_stroke_element(oc_gl_canvas_backend* backend,
                                  oc_path_elt* element,
                                  oc_vec2 currentPoint,
                                  oc_vec2* startTangent,
                                  oc_vec2* endTangent,
                                  oc_vec2* endPoint)
{
	oc_vec2 controlPoints[4] = {currentPoint, element->p[0], element->p[1], element->p[2]};
	int endPointIndex = 0;

	switch(element->type)
	{
		case OC_PATH_LINE:
			oc_gl_encode_stroke_line(backend, controlPoints);
			endPointIndex = 1;
			break;

		case OC_PATH_QUADRATIC:
			oc_gl_encode_stroke_quadratic(backend, controlPoints);
			endPointIndex = 2;
			break;

		case OC_PATH_CUBIC:
			oc_gl_encode_stroke_cubic(backend, controlPoints);
			endPointIndex = 3;
			break;

		case OC_PATH_MOVE:
			OC_ASSERT(0, "should be unreachable");
			break;
	}

	//NOTE: ensure tangents are properly computed even in presence of coincident points
	//TODO: see if we can do this in a less hacky way

	for(int i=1; i<4; i++)
	{
		if(  controlPoints[i].x != controlPoints[0].x
		  || controlPoints[i].y != controlPoints[0].y)
		{
			*startTangent = (oc_vec2){.x = controlPoints[i].x - controlPoints[0].x,
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
			*endTangent = (oc_vec2){.x = endPoint->x - controlPoints[i].x,
			                     .y = endPoint->y - controlPoints[i].y};
			break;
		}
	}
	OC_DEBUG_ASSERT(startTangent->x != 0 || startTangent->y != 0);
}

void oc_gl_stroke_cap(oc_gl_canvas_backend* backend,
                       oc_vec2 p0,
                       oc_vec2 direction)
{
	oc_attributes* attributes = &backend->primitive->attributes;

	//NOTE(martin): compute the tangent and normal vectors (multiplied by half width) at the cap point
	f32 dn = sqrt(oc_square(direction.x) + oc_square(direction.y));
	f32 alpha = 0.5 * attributes->width/dn;

	oc_vec2 n0 = {-alpha*direction.y,
		    alpha*direction.x};

	oc_vec2 m0 = {alpha*direction.x,
	           alpha*direction.y};

	oc_vec2 points[] = {{p0.x + n0.x, p0.y + n0.y},
	                 {p0.x + n0.x + m0.x, p0.y + n0.y + m0.y},
	                 {p0.x - n0.x + m0.x, p0.y - n0.y + m0.y},
	                 {p0.x - n0.x, p0.y - n0.y},
	                 {p0.x + n0.x, p0.y + n0.y}};

	oc_gl_canvas_encode_element(backend, OC_PATH_LINE, points);
	oc_gl_canvas_encode_element(backend, OC_PATH_LINE, points+1);
	oc_gl_canvas_encode_element(backend, OC_PATH_LINE, points+2);
	oc_gl_canvas_encode_element(backend, OC_PATH_LINE, points+3);
}

void oc_gl_stroke_joint(oc_gl_canvas_backend* backend,
                         oc_vec2 p0,
                         oc_vec2 t0,
                         oc_vec2 t1)
{
	oc_attributes* attributes = &backend->primitive->attributes;

	//NOTE(martin): compute the normals at the joint point
	f32 norm_t0 = sqrt(oc_square(t0.x) + oc_square(t0.y));
	f32 norm_t1 = sqrt(oc_square(t1.x) + oc_square(t1.y));

	oc_vec2 n0 = {-t0.y, t0.x};
	n0.x /= norm_t0;
	n0.y /= norm_t0;

	oc_vec2 n1 = {-t1.y, t1.x};
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
	oc_vec2 u = {n0.x + n1.x, n0.y + n1.y};
	f32 uNormSquare = u.x*u.x + u.y*u.y;
	f32 alpha = attributes->width / uNormSquare;
	oc_vec2 v = {u.x * alpha, u.y * alpha};

	f32 excursionSquare = uNormSquare * oc_square(alpha - attributes->width/4);

	if(  attributes->joint == OC_JOINT_MITER
	  && excursionSquare <= oc_square(attributes->maxJointExcursion))
	{
		//NOTE(martin): add a mitter joint
		oc_vec2 points[] = {p0,
		                 {p0.x + n0.x*halfW, p0.y + n0.y*halfW},
		                 {p0.x + v.x, p0.y + v.y},
		                 {p0.x + n1.x*halfW, p0.y + n1.y*halfW},
		                 p0};

		oc_gl_canvas_encode_element(backend, OC_PATH_LINE, points);
		oc_gl_canvas_encode_element(backend, OC_PATH_LINE, points+1);
		oc_gl_canvas_encode_element(backend, OC_PATH_LINE, points+2);
		oc_gl_canvas_encode_element(backend, OC_PATH_LINE, points+3);
	}
	else
	{
		//NOTE(martin): add a bevel joint
		oc_vec2 points[] = {p0,
		                 {p0.x + n0.x*halfW, p0.y + n0.y*halfW},
		                 {p0.x + n1.x*halfW, p0.y + n1.y*halfW},
		                 p0};

		oc_gl_canvas_encode_element(backend, OC_PATH_LINE, points);
		oc_gl_canvas_encode_element(backend, OC_PATH_LINE, points+1);
		oc_gl_canvas_encode_element(backend, OC_PATH_LINE, points+2);
	}
}

u32 oc_gl_encode_stroke_subpath(oc_gl_canvas_backend* backend,
                                 oc_path_elt* elements,
                                 oc_path_descriptor* path,
                                 u32 startIndex,
                                 oc_vec2 startPoint)
{
	u32 eltCount = path->count;
	OC_DEBUG_ASSERT(startIndex < eltCount);

	oc_vec2 currentPoint = startPoint;
	oc_vec2 endPoint = {0, 0};
	oc_vec2 previousEndTangent = {0, 0};
	oc_vec2 firstTangent = {0, 0};
	oc_vec2 startTangent = {0, 0};
	oc_vec2 endTangent = {0, 0};

	//NOTE(martin): encode first element and compute first tangent
	oc_gl_encode_stroke_element(backend, elements + startIndex, currentPoint, &startTangent, &endTangent, &endPoint);

	firstTangent = startTangent;
	previousEndTangent = endTangent;
	currentPoint = endPoint;

	//NOTE(martin): encode subsequent elements along with their joints

	oc_attributes* attributes = &backend->primitive->attributes;

	u32 eltIndex = startIndex + 1;
	for(;
	    eltIndex<eltCount && elements[eltIndex].type != OC_PATH_MOVE;
	    eltIndex++)
	{
		oc_gl_encode_stroke_element(backend, elements + eltIndex, currentPoint, &startTangent, &endTangent, &endPoint);

		if(attributes->joint != OC_JOINT_NONE)
		{
			oc_gl_stroke_joint(backend, currentPoint, previousEndTangent, startTangent);
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
		if(attributes->joint != OC_JOINT_NONE)
		{
			//NOTE(martin): add a closing joint if the path is closed
			oc_gl_stroke_joint(backend, endPoint, endTangent, firstTangent);
		}
	}
	else if(attributes->cap == OC_CAP_SQUARE)
	{
		//NOTE(martin): add start and end cap
		oc_gl_stroke_cap(backend, startPoint, (oc_vec2){-startTangent.x, -startTangent.y});
		oc_gl_stroke_cap(backend, endPoint, endTangent);
	}
	return(eltIndex);
}

void oc_gl_encode_stroke(oc_gl_canvas_backend* backend,
                          oc_path_elt* elements,
                          oc_path_descriptor* path)
{
	u32 eltCount = path->count;
	OC_DEBUG_ASSERT(eltCount);

	oc_vec2 startPoint = path->startPoint;
	u32 startIndex = 0;

	while(startIndex < eltCount)
	{
		//NOTE(martin): eliminate leading moves
		while(startIndex < eltCount && elements[startIndex].type == OC_PATH_MOVE)
		{
			startPoint = elements[startIndex].p[0];
			startIndex++;
		}
		if(startIndex < eltCount)
		{
			startIndex = oc_gl_encode_stroke_subpath(backend, elements, path, startIndex, startPoint);
		}
	}
}

void oc_gl_grow_buffer_if_needed(GLuint buffer, i32 wantedSize, const char* name)
{
	i32 oldSize = 0;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
	glGetBufferParameteriv(GL_SHADER_STORAGE_BUFFER, GL_BUFFER_SIZE, &oldSize);

	if(oldSize < wantedSize)
	{
		oc_log_info("growing %s buffer\n", name);

		int newSize = wantedSize * 1.2;

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, newSize, 0, GL_DYNAMIC_COPY);
	}
}



void oc_gl_render_batch(oc_gl_canvas_backend* backend,
                        oc_wgl_surface* surface,
                        oc_image* images,
                        int tileSize,
                        int nTilesX,
                        int nTilesY,
                        oc_vec2 viewportSize,
                        f32 scale)
{
	GLuint pathBuffer = backend->pathBuffer[backend->bufferIndex].buffer;
	GLuint elementBuffer = backend->elementBuffer[backend->bufferIndex].buffer;

	int pathBufferOffset = backend->pathBatchStart * sizeof(oc_gl_path);
	int elementBufferOffset = backend->eltBatchStart * sizeof(oc_gl_path_elt);
	int pathCount = backend->pathCount - backend->pathBatchStart;
	int eltCount = backend->eltCount - backend->eltBatchStart;

	if(!pathCount || !eltCount)
	{
		return;
	}

	//NOTE: update intermediate buffers size if needed
	//TODO: compute correct sizes

	oc_gl_grow_buffer_if_needed(backend->pathQueueBuffer, pathCount * OC_GL_PATH_QUEUE_SIZE, "path queues");
	oc_gl_grow_buffer_if_needed(backend->tileQueueBuffer, backend->maxTileQueueCount * OC_GL_TILE_QUEUE_SIZE, "tile queues");
	oc_gl_grow_buffer_if_needed(backend->segmentBuffer, backend->maxSegmentCount * OC_GL_SEGMENT_SIZE, "segments");
	oc_gl_grow_buffer_if_needed(backend->screenTilesBuffer, nTilesX * nTilesY * OC_GL_SCREEN_TILE_SIZE, "screen tiles");
	oc_gl_grow_buffer_if_needed(backend->tileOpBuffer, backend->maxSegmentCount * 30 * OC_GL_TILE_OP_SIZE, "tile ops");

	//NOTE: make the buffers visible to gl
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, pathBuffer);
	glFlushMappedBufferRange(GL_SHADER_STORAGE_BUFFER, pathBufferOffset, pathCount*sizeof(oc_gl_path));

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, elementBuffer);
	glFlushMappedBufferRange(GL_SHADER_STORAGE_BUFFER, elementBufferOffset, eltCount*sizeof(oc_gl_path_elt));

	//NOTE: clear out texture
	u8 clearColor[4] = {0};
	glClearTexImage(backend->outTexture, 0, GL_RGBA, GL_BYTE, clearColor);

	//NOTE: clear counters
	int zero = 0;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->segmentCountBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), &zero, GL_DYNAMIC_COPY);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->tileQueueCountBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), &zero, GL_DYNAMIC_COPY);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->tileOpCountBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), &zero, GL_DYNAMIC_COPY);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->rasterDispatchBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(oc_gl_dispatch_indirect_command), &zero, GL_DYNAMIC_COPY);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->screenTilesCountBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), &zero, GL_DYNAMIC_COPY);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	int err = glGetError();
	if(err)
	{
		oc_log_error("gl error %i\n", err);
	}

	//NOTE: path setup pass
	int maxWorkGroupCount = 0;
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxWorkGroupCount);
	//NOTE: glDispatchCompute errors if work group count is greater _or equal_ to GL_MAX_COMPUTE_WORK_GROUP_COUNT
	//      so the maximum _allowed_ group count is one less.
	maxWorkGroupCount--;

	glUseProgram(backend->pathSetup);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, backend->tileQueueCountBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, backend->tileQueueBuffer);

	glUniform1i(0, tileSize);
	glUniform1f(1, scale);

	for(int i=0; i<pathCount; i += maxWorkGroupCount)
	{
		int count = oc_min(maxWorkGroupCount, pathCount-i);

		glUniform1i(2, backend->pathBatchStart + i);
		glUniform1i(3, i);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pathBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, backend->pathQueueBuffer);

		glDispatchCompute(count, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	if(!err)
	{
		err = glGetError();
		if(err)
		{
			oc_log_error("gl error %i\n", err);
		}
	}

	//NOTE: segment setup pass
	glUseProgram(backend->segmentSetup);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, backend->segmentCountBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, backend->segmentBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, backend->pathQueueBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, backend->tileQueueBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, backend->tileOpCountBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, backend->tileOpBuffer);

	glUniform1f(0, scale);
	glUniform1ui(1, tileSize);

	for(int i=0; i<eltCount; i += maxWorkGroupCount)
	{
		int offset = elementBufferOffset + i*sizeof(oc_gl_path_elt);
		int count = oc_min(maxWorkGroupCount, eltCount-i);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, elementBuffer);

		glUniform1i(2, (backend->eltBatchStart + i));

		glDispatchCompute(count, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	if(!err)
	{
		err = glGetError();
		if(err)
		{
			oc_log_error("gl error %i\n", err);
		}
	}

	//NOTE: backprop pass
	glUseProgram(backend->backprop);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, backend->tileQueueBuffer);

	for(int i=0; i<pathCount; i += maxWorkGroupCount)
	{
		int count = oc_min(maxWorkGroupCount, pathCount-i);

		glUniform1i(0, i);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, backend->pathQueueBuffer);

		glDispatchCompute(count, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	if(!err)
	{
		err = glGetError();
		if(err)
		{
			oc_log_error("gl error %i\n", err);
		}
	}

	//NOTE: merge pass
	glUseProgram(backend->merge);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pathBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, backend->pathQueueBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, backend->tileQueueBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, backend->tileOpCountBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, backend->tileOpBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, backend->screenTilesBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, backend->screenTilesCountBuffer);

	glUniform1i(0, tileSize);
	glUniform1f(1, scale);
	glUniform1i(2, pathCount);
	glUniform1i(3, backend->pathBatchStart);

	glDispatchCompute(nTilesX, nTilesY, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	if(!err)
	{
		err = glGetError();
		if(err)
		{
			oc_log_error("gl error %i\n", err);
		}
	}

	//NOTE: balance work groups
	glUseProgram(backend->balanceWorkgroups);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, backend->screenTilesCountBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, backend->rasterDispatchBuffer);
	glUniform1ui(0, maxWorkGroupCount);

	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	//NOTE: raster pass
	glUseProgram(backend->raster);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pathBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, backend->segmentBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, backend->tileOpBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, backend->screenTilesBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, backend->screenTilesCountBuffer);

	glUniform1f(0, scale);
	glUniform1i(1, backend->msaaCount);

	glBindImageTexture(0, backend->outTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

	for(int i=0; i<OC_GL_MAX_IMAGES_PER_BATCH; i++)
	{
		if(images[i].h)
		{
			oc_gl_image* image = (oc_gl_image*)oc_image_data_from_handle(images[i]);
			if(image)
			{
				glActiveTexture(GL_TEXTURE1+i);
				glBindTexture(GL_TEXTURE_2D, image->texture);
			}
		}
	}

	glUniform1i(2, backend->pathBatchStart);
	glUniform1ui(3, maxWorkGroupCount);

	glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, backend->rasterDispatchBuffer);
	glDispatchComputeIndirect(0);

	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

	if(!err)
	{
		err = glGetError();
		if(err)
		{
			oc_log_error("gl error %i\n", err);
		}
	}

	//NOTE: blit pass
	glUseProgram(backend->blit);
	glBindBuffer(GL_ARRAY_BUFFER, backend->dummyVertexBuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, backend->outTexture);
	glUniform1i(0, 0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	if(!err)
	{
		err = glGetError();
		if(err)
		{
			oc_log_error("gl error %i\n", err);
		}
	}

	backend->pathBatchStart = backend->pathCount;
	backend->eltBatchStart = backend->eltCount;

	backend->maxSegmentCount = 0;
	backend->maxTileQueueCount = 0;
}

void oc_gl_canvas_resize(oc_gl_canvas_backend* backend, oc_vec2 size)
{
	int tileSize = OC_GL_TILE_SIZE;
	int nTilesX = (int)(size.x + tileSize - 1)/tileSize;
	int nTilesY = (int)(size.y + tileSize - 1)/tileSize;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->screenTilesBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, nTilesX*nTilesY*OC_GL_SCREEN_TILE_SIZE, 0, GL_DYNAMIC_COPY);

	if(backend->outTexture)
	{
		//NOTE: do we need to explicitly glDeleteTextures()?
		glDeleteTextures(1, &backend->outTexture);
		glGenTextures(1, &backend->outTexture);
		glBindTexture(GL_TEXTURE_2D, backend->outTexture);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, size.x, size.y);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	backend->frameSize = size;
}

void oc_gl_canvas_render(oc_canvas_backend* interface,
                         oc_color clearColor,
                         u32 primitiveCount,
                         oc_primitive* primitives,
                         u32 eltCount,
                         oc_path_elt* pathElements)
{
	oc_gl_canvas_backend* backend = (oc_gl_canvas_backend*)interface;

	//NOTE: roll input buffers
	backend->bufferIndex = (backend->bufferIndex + 1) % OC_GL_INPUT_BUFFERS_COUNT;
	if(backend->bufferSync[backend->bufferIndex] != 0)
	{
		glClientWaitSync(backend->bufferSync[backend->bufferIndex], GL_SYNC_FLUSH_COMMANDS_BIT, 0xffffffff);
		glDeleteSync(backend->bufferSync[backend->bufferIndex]);
		backend->bufferSync[backend->bufferIndex] = 0;
	}

	//NOTE update screen tiles buffer size
	oc_wgl_surface* surface = backend->surface;
	oc_vec2 surfaceSize = surface->interface.getSize((oc_surface_data*)surface);
	oc_vec2 contentsScaling = surface->interface.contentsScaling((oc_surface_data*)surface);
	//TODO support scaling in both axes?
	f32 scale = contentsScaling.x;

	oc_vec2 viewportSize = {surfaceSize.x * scale, surfaceSize.y * scale};
	int tileSize = OC_GL_TILE_SIZE;
	int nTilesX = (int)(viewportSize.x + tileSize - 1)/tileSize;
	int nTilesY = (int)(viewportSize.y + tileSize - 1)/tileSize;

	if(viewportSize.x != backend->frameSize.x || viewportSize.y != backend->frameSize.y)
	{
		oc_gl_canvas_resize(backend, viewportSize);
	}

	glViewport(0, 0, viewportSize.x, viewportSize.y);

	//NOTE: clear screen and reset input buffer offsets
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT);

	backend->pathCount = 0;
	backend->pathBatchStart = 0;
	backend->eltCount = 0;
	backend->eltBatchStart = 0;
	backend->maxSegmentCount = 0;
	backend->maxTileQueueCount = 0;

	//NOTE: encode and render batches
	oc_vec2 currentPos = {0};
	oc_image images[OC_GL_MAX_IMAGES_PER_BATCH] = {0};
	int imageCount = 0;
	backend->eltCount = 0;

	for(int primitiveIndex = 0; primitiveIndex < primitiveCount; primitiveIndex++)
	{
		oc_primitive* primitive = &primitives[primitiveIndex];

		if(primitive->attributes.image.h != 0)
		{
			backend->currentImageIndex = -1;
			for(int i=0; i<imageCount; i++)
			{
				if(images[i].h == primitive->attributes.image.h)
				{
					backend->currentImageIndex = i;
				}
			}
			if(backend->currentImageIndex <= 0)
			{
				if(imageCount<OC_GL_MAX_IMAGES_PER_BATCH)
				{
					images[imageCount] = primitive->attributes.image;
					backend->currentImageIndex = imageCount;
					imageCount++;
				}
				else
				{
					oc_gl_render_batch(backend,
					                    surface,
					                    images,
					                    tileSize,
					                    nTilesX,
					                    nTilesY,
					                    viewportSize,
					                    scale);

					images[0] = primitive->attributes.image;
					backend->currentImageIndex = 0;
					imageCount = 1;
				}
			}
		}
		else
		{
			backend->currentImageIndex = -1;
		}

		if(primitive->path.count)
		{
			backend->primitive = primitive;
			backend->pathScreenExtents = (oc_vec4){FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};
			backend->pathUserExtents = (oc_vec4){FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};

			if(primitive->cmd == OC_CMD_STROKE)
			{
				oc_gl_encode_stroke(backend, pathElements + primitive->path.startIndex, &primitive->path);
			}
			else
			{
				int segCount = 0;
				for(int eltIndex = 0;
			    	(eltIndex < primitive->path.count) && (primitive->path.startIndex + eltIndex < eltCount);
			    	eltIndex++)
				{
					oc_path_elt* elt = &pathElements[primitive->path.startIndex + eltIndex];

					if(elt->type != OC_PATH_MOVE)
					{
						oc_vec2 p[4] = {currentPos, elt->p[0], elt->p[1], elt->p[2]};
						oc_gl_canvas_encode_element(backend, elt->type, p);
						segCount++;
					}
					switch(elt->type)
					{
						case OC_PATH_MOVE:
							currentPos = elt->p[0];
							break;

						case OC_PATH_LINE:
							currentPos = elt->p[0];
							break;

						case OC_PATH_QUADRATIC:
							currentPos = elt->p[1];
							break;

						case OC_PATH_CUBIC:
							currentPos = elt->p[2];
							break;
					}
				}
			}
			//NOTE: push path
			oc_gl_canvas_encode_path(backend, primitive, scale);
		}
	}

	oc_gl_render_batch(backend,
	                    surface,
	                    images,
	                    tileSize,
	                    nTilesX,
	                    nTilesY,
	                    viewportSize,
	                    scale);

	//NOTE: add fence for rolling input buffers
	backend->bufferSync[backend->bufferIndex] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

//--------------------------------------------------------------------
// Image API
//--------------------------------------------------------------------
oc_image_data* oc_gl_canvas_image_create(oc_canvas_backend* interface, oc_vec2 size)
{
	oc_gl_image* image = 0;

	image = oc_malloc_type(oc_gl_image);
	if(image)
	{
		glGenTextures(1, &image->texture);
		glBindTexture(GL_TEXTURE_2D, image->texture);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, size.x, size.y);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		image->interface.size = size;
	}
	return((oc_image_data*)image);
}

void oc_gl_canvas_image_destroy(oc_canvas_backend* interface, oc_image_data* imageInterface)
{
	//TODO: check that this image belongs to this backend
	oc_gl_image* image = (oc_gl_image*)imageInterface;
	glDeleteTextures(1, &image->texture);
	free(image);
}

void oc_gl_canvas_image_upload_region(oc_canvas_backend* interface,
                                      oc_image_data* imageInterface,
                                      oc_rect region,
                                      u8* pixels)
{
	//TODO: check that this image belongs to this backend
	oc_gl_image* image = (oc_gl_image*)imageInterface;
	glBindTexture(GL_TEXTURE_2D, image->texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, region.x, region.y, region.w, region.h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

//--------------------------------------------------------------------
// Canvas setup / destroy
//--------------------------------------------------------------------

void oc_gl_canvas_destroy(oc_canvas_backend* interface)
{
	oc_gl_canvas_backend* backend = (oc_gl_canvas_backend*)interface;

	////////////////////////////////////////////////////////////////////
	//TODO
	////////////////////////////////////////////////////////////////////

	free(backend);
}

static int oc_gl_compile_shader(const char* name, GLuint shader, const char* source)
{
	int res = 0;

	const char* sources[3] = {"#version 430", glsl_common, source};

	glShaderSource(shader, 3, sources, 0);
	glCompileShader(shader);

	int status = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if(!status)
	{
		char buffer[256];
		int size = 0;
		glGetShaderInfoLog(shader, 256, &size, buffer);
		printf("Shader compile error (%s): %.*s\n", name, size, buffer);
		res = -1;
	}
	return(res);
}

static int oc_gl_canvas_compile_compute_program_named(const char* name, const char* source, GLuint* outProgram)
{
	int res = 0;
	*outProgram = 0;

	GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
	GLuint program = glCreateProgram();

	res |= oc_gl_compile_shader(name, shader, source);

	if(!res)
	{
		glAttachShader(program, shader);
		glLinkProgram(program);

		int status = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &status);
		if(!status)
		{
			char buffer[256];
			int size = 0;
			glGetProgramInfoLog(program, 256, &size, buffer);
			oc_log_error("Shader link error (%s): %.*s\n", name, size, buffer);

			res = -1;
		}
		else
		{
			*outProgram = program;
		}
	}
	return(res);
}

int oc_gl_canvas_compile_render_program_named(const char* progName,
                                              const char* vertexName,
                                              const char* fragmentName,
                                              const char* vertexSrc,
                                              const char* fragmentSrc,
                                              GLuint* outProgram)
{
	int res = 0;
	*outProgram = 0;

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint program = glCreateProgram();

	res |= oc_gl_compile_shader(vertexName, vertexShader, vertexSrc);
	res |= oc_gl_compile_shader(fragmentName, fragmentShader, fragmentSrc);

	if(!res)
	{
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);

		int status = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &status);
		if(!status)
		{
			char buffer[256];
			int size = 0;
			glGetProgramInfoLog(program, 256, &size, buffer);
			oc_log_error("Shader link error (%s): %.*s\n", progName, size, buffer);
			res = -1;
 		}
 		else
 		{
			*outProgram = program;
 		}
 	}
 	return(res);
}

#define oc_gl_canvas_compile_compute_program(src, out) \
	oc_gl_canvas_compile_compute_program_named(#src, src, out)

#define oc_gl_canvas_compile_render_program(progName, shaderSrc, vertexSrc, out) \
	oc_gl_canvas_compile_render_program_named(progName, #shaderSrc, #vertexSrc, shaderSrc, vertexSrc, out)

const u32 OC_GL_PATH_BUFFER_SIZE       = (4<<10)*sizeof(oc_gl_path),
          OC_GL_ELEMENT_BUFFER_SIZE    = (4<<12)*sizeof(oc_gl_path_elt),
          OC_GL_SEGMENT_BUFFER_SIZE    = (4<<10)*OC_GL_SEGMENT_SIZE,
          OC_GL_PATH_QUEUE_BUFFER_SIZE = (4<<10)*OC_GL_PATH_QUEUE_SIZE,
          OC_GL_TILE_QUEUE_BUFFER_SIZE = (4<<10)*OC_GL_TILE_QUEUE_SIZE,
          OC_GL_TILE_OP_BUFFER_SIZE    = (4<<20)*OC_GL_TILE_OP_SIZE;

oc_canvas_backend* oc_gl_canvas_backend_create(oc_wgl_surface* surface)
{
	oc_gl_canvas_backend* backend = oc_malloc_type(oc_gl_canvas_backend);
	if(backend)
	{
		memset(backend, 0, sizeof(oc_gl_canvas_backend));
		backend->surface = surface;

		backend->msaaCount = OC_GL_MSAA_COUNT;

		//NOTE(martin): setup interface functions
		backend->interface.destroy = oc_gl_canvas_destroy;
		backend->interface.render = oc_gl_canvas_render;
		backend->interface.imageCreate = oc_gl_canvas_image_create;
		backend->interface.imageDestroy = oc_gl_canvas_image_destroy;
		backend->interface.imageUploadRegion = oc_gl_canvas_image_upload_region;

		surface->interface.prepare((oc_surface_data*)surface);

		glGenVertexArrays(1, &backend->vao);
		glBindVertexArray(backend->vao);

		//NOTE: create programs
		int err = 0;
		err |= oc_gl_canvas_compile_compute_program(glsl_path_setup, &backend->pathSetup);
		err |= oc_gl_canvas_compile_compute_program(glsl_segment_setup, &backend->segmentSetup);
		err |= oc_gl_canvas_compile_compute_program(glsl_backprop, &backend->backprop);
		err |= oc_gl_canvas_compile_compute_program(glsl_merge, &backend->merge);
		err |= oc_gl_canvas_compile_compute_program(glsl_balance_workgroups, &backend->balanceWorkgroups);
		err |= oc_gl_canvas_compile_compute_program(glsl_raster, &backend->raster);
		err |= oc_gl_canvas_compile_render_program("blit", glsl_blit_vertex, glsl_blit_fragment, &backend->blit);

		if(glGetError() != GL_NO_ERROR)
		{
			err |= -1;
		}

		//NOTE: create out texture
		oc_vec2 size = surface->interface.getSize((oc_surface_data*)surface);
		oc_vec2 scale = surface->interface.contentsScaling((oc_surface_data*)surface);

		backend->frameSize = (oc_vec2){size.x * scale.x, size.y * scale.y};

		glGenTextures(1, &backend->outTexture);
		glBindTexture(GL_TEXTURE_2D, backend->outTexture);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, backend->frameSize.x, backend->frameSize.y);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//NOTE: generate buffers
		glGenBuffers(1, &backend->dummyVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, backend->dummyVertexBuffer);

		for(int i=0; i<OC_GL_INPUT_BUFFERS_COUNT; i++)
		{
			glGenBuffers(1, &backend->pathBuffer[i].buffer);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->pathBuffer[i].buffer);
			glBufferStorage(GL_SHADER_STORAGE_BUFFER, OC_GL_PATH_BUFFER_SIZE, 0, GL_MAP_WRITE_BIT|GL_MAP_PERSISTENT_BIT);
			backend->pathBuffer[i].size = OC_GL_PATH_BUFFER_SIZE;
			backend->pathBuffer[i].contents = glMapBufferRange(GL_SHADER_STORAGE_BUFFER,
			                                                   0,
			                                                   OC_GL_PATH_BUFFER_SIZE,
			                                                    GL_MAP_WRITE_BIT
			                                                   |GL_MAP_PERSISTENT_BIT
			                                                   |GL_MAP_FLUSH_EXPLICIT_BIT);

			glGenBuffers(1, &backend->elementBuffer[i].buffer);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->elementBuffer[i].buffer);
			glBufferStorage(GL_SHADER_STORAGE_BUFFER, OC_GL_ELEMENT_BUFFER_SIZE, 0, GL_MAP_WRITE_BIT|GL_MAP_PERSISTENT_BIT);
			backend->elementBuffer[i].size = OC_GL_ELEMENT_BUFFER_SIZE;
			backend->elementBuffer[i].contents = glMapBufferRange(GL_SHADER_STORAGE_BUFFER,
			                                                   0,
			                                                   OC_GL_ELEMENT_BUFFER_SIZE,
			                                                    GL_MAP_WRITE_BIT
			                                                   |GL_MAP_PERSISTENT_BIT
			                                                   |GL_MAP_FLUSH_EXPLICIT_BIT);
		}

		glGenBuffers(1, &backend->segmentBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->segmentBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, OC_GL_SEGMENT_BUFFER_SIZE, 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->segmentCountBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->segmentCountBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->pathQueueBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->pathQueueBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, OC_GL_PATH_QUEUE_BUFFER_SIZE, 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->tileQueueBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->tileQueueBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, OC_GL_TILE_QUEUE_BUFFER_SIZE, 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->tileQueueCountBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->tileQueueCountBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->tileOpBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->tileOpBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, OC_GL_TILE_OP_BUFFER_SIZE, 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->tileOpCountBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->tileOpCountBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), 0, GL_DYNAMIC_COPY);

		int tileSize = OC_GL_TILE_SIZE;
		int nTilesX = (int)(backend->frameSize.x + tileSize - 1)/tileSize;
		int nTilesY = (int)(backend->frameSize.y + tileSize - 1)/tileSize;

		glGenBuffers(1, &backend->screenTilesBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->screenTilesBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, nTilesX*nTilesY*OC_GL_SCREEN_TILE_SIZE, 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->screenTilesCountBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->screenTilesCountBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->rasterDispatchBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->rasterDispatchBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(oc_gl_dispatch_indirect_command), 0, GL_DYNAMIC_COPY);

		if(err)
		{
			oc_gl_canvas_destroy((oc_canvas_backend*)backend);
			backend = 0;
		}
	}
	return((oc_canvas_backend*)backend);
}

oc_surface_data* oc_gl_canvas_surface_create_for_window(oc_window window)
{
	oc_wgl_surface* surface = (oc_wgl_surface*)oc_wgl_surface_create_for_window(window);

	if(surface)
	{
		surface->interface.backend = oc_gl_canvas_backend_create(surface);
		if(surface->interface.backend)
		{
			surface->interface.api = OC_CANVAS;
		}
		else
		{
			surface->interface.destroy((oc_surface_data*)surface);
			surface = 0;
		}
	}
	return((oc_surface_data*)surface);
}
