/************************************************************//**
*
*	@file: gl_canvas.c
*	@author: Martin Fouilleul
*	@date: 29/01/2023
*	@revision:
*
*****************************************************************/
#include"graphics_surface.h"
#include"macro_helpers.h"
#include"glsl_shaders.h"
#include"gl_api.h"

typedef struct mg_gl_image
{
	mg_image_data interface;
	GLuint texture;
} mg_gl_image;

enum _mg_gl_cmd {
	MG_GL_FILL,
	MG_GL_STROKE,
};
typedef int mg_gl_cmd;

typedef struct mg_gl_path
{
	float uvTransform[12];
	vec4 color;
	vec4 box;
	vec4 clip;
	mg_gl_cmd cmd;
	u8 pad[12];
} mg_gl_path;

enum _mg_gl_seg_kind{
	MG_GL_LINE = 1,
	MG_GL_QUADRATIC,
	MG_GL_CUBIC,
};
typedef int mg_gl_seg_kind;

typedef struct mg_gl_path_elt
{
	vec2 p[4];
	int pathIndex;
	int localEltIndex;
	mg_gl_seg_kind kind;
	u8 pad[4];
} mg_gl_path_elt;

enum {
	LAYOUT_PATH_SIZE = sizeof(mg_gl_path),
	LAYOUT_PATH_ELT_SIZE = sizeof(mg_gl_path_elt),
};

////////////////////////////////////////////////////////////
//NOTE: these are just here for the sizes...

typedef struct mg_gl_segment
{
	int kind;
	int pathIndex;
	int config;
	int windingIncrement;
	vec4 box;
	float hullMatrix[9];
	float implicitMatrix[9];
	float sign;
	vec2 hullVertex;
	int debugID;

} mg_gl_segment;

typedef struct mg_gl_path_queue
{
	vec4 area;
	int tileQueues;
	u8 pad[12];
} mg_gl_path_queue;

typedef struct mg_gl_tile_op
{
	int kind;
	int index;
	int next;
	bool crossRight;
	int windingOffset;

} mg_gl_tile_op;

typedef struct mg_gl_tile_queue
{
	int windingOffset;
	int first;
	int last;

} mg_gl_tile_queue;

////////////////////////////////////////////////////////////

typedef struct mg_gl_encoding_context
{
	int glEltCount;
	mg_gl_path* pathBufferData;
	mg_gl_path_elt* elementBufferData;
	int pathIndex;
	int localEltIndex;
	mg_primitive* primitive;
	vec4 pathScreenExtents;
	vec4 pathUserExtents;

} mg_gl_encoding_context;

enum {
//	MG_GL_INPUT_BUFFERS_COUNT = 3,
	MG_GL_TILE_SIZE = 16,
	MG_GL_MSAA_COUNT = 8,
};

typedef struct mg_gl_canvas_backend
{
	mg_canvas_backend interface;
	mg_wgl_surface* surface;

	GLuint vao;

	GLuint pathSetup;
	GLuint segmentSetup;
	GLuint backprop;
	GLuint merge;
	GLuint raster;
	GLuint blit;

	GLuint outTexture;

	int bufferIndex;
	//TODO buffer semaphore...

	GLuint pathBuffer;
	GLuint elementBuffer;

	GLuint segmentBuffer;
	GLuint segmentCountBuffer;
	GLuint pathQueueBuffer;
	GLuint tileQueueBuffer;
	GLuint tileQueueCountBuffer;
	GLuint tileOpBuffer;
	GLuint tileOpCountBuffer;
	GLuint screenTilesBuffer;

	GLuint dummyVertexBuffer;

	mg_gl_path* pathBufferData;
	mg_gl_path_elt* elementBufferData;

	int msaaCount;
	vec2 frameSize;

} mg_gl_canvas_backend;

static void mg_update_path_extents(vec4* extents, vec2 p)
{
	extents->x = minimum(extents->x, p.x);
	extents->y = minimum(extents->y, p.y);
	extents->z = maximum(extents->z, p.x);
	extents->w = maximum(extents->w, p.y);
}

void mg_gl_canvas_encode_element(mg_gl_encoding_context* context, mg_path_elt_type kind, vec2* p)
{
	mg_gl_path_elt* glElt = &context->elementBufferData[context->glEltCount];
	context->glEltCount++;

	glElt->pathIndex = context->pathIndex;
	int count = 0;
	switch(kind)
	{
		case MG_PATH_LINE:
			glElt->kind = MG_GL_LINE;
			count = 2;
			break;

		case MG_PATH_QUADRATIC:
			glElt->kind = MG_GL_QUADRATIC;
			count = 3;
			break;

		case MG_PATH_CUBIC:
			glElt->kind = MG_GL_CUBIC;
			count = 4;
			break;

		default:
			break;
	}

	glElt->localEltIndex = context->localEltIndex;

	for(int i=0; i<count; i++)
	{
		mg_update_path_extents(&context->pathUserExtents, p[i]);

		vec2 screenP = mg_mat2x3_mul(context->primitive->attributes.transform, p[i]);
		glElt->p[i] = (vec2){screenP.x, screenP.y};

		mg_update_path_extents(&context->pathScreenExtents, screenP);
	}
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

void mg_gl_encode_stroke_line(mg_gl_encoding_context* context, vec2* p)
{
	f32 width = context->primitive->attributes.width;

	vec2 v = {p[1].x-p[0].x, p[1].y-p[0].y};
	vec2 n = {v.y, -v.x};
	f32 norm = sqrt(n.x*n.x + n.y*n.y);
	vec2 offset = vec2_mul(0.5*width/norm, n);

	vec2 left[2] = {vec2_add(p[0], offset), vec2_add(p[1], offset)};
	vec2 right[2] = {vec2_add(p[1], vec2_mul(-1, offset)), vec2_add(p[0], vec2_mul(-1, offset))};
	vec2 joint0[2] = {vec2_add(p[0], vec2_mul(-1, offset)), vec2_add(p[0], offset)};
	vec2 joint1[2] = {vec2_add(p[1], offset), vec2_add(p[1], vec2_mul(-1, offset))};

	mg_gl_canvas_encode_element(context, MG_PATH_LINE, right);

	mg_gl_canvas_encode_element(context, MG_PATH_LINE, left);
	mg_gl_canvas_encode_element(context, MG_PATH_LINE, joint0);
	mg_gl_canvas_encode_element(context, MG_PATH_LINE, joint1);
}

enum { MG_HULL_CHECK_SAMPLE_COUNT = 5 };

void mg_gl_encode_stroke_quadratic(mg_gl_encoding_context* context, vec2* p)
{
	f32 width = context->primitive->attributes.width;
	f32 tolerance = minimum(context->primitive->attributes.tolerance, 0.5 * width);

	//NOTE: check for degenerate line case
	const f32 equalEps = 1e-3;
	if(vec2_close(p[0], p[1], equalEps))
	{
		mg_gl_encode_stroke_line(context, p+1);
		return;
	}
	else if(vec2_close(p[1], p[2], equalEps))
	{
		mg_gl_encode_stroke_line(context, p);
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
		mg_gl_encode_stroke_quadratic(context, splitLeft);
		mg_gl_encode_stroke_quadratic(context, splitRight);
	}
	else
	{
		f32 checkSamples[MG_HULL_CHECK_SAMPLE_COUNT] = {1./6, 2./6, 3./6, 4./6, 5./6};

		f32 d2LowBound = Square(0.5 * width - tolerance);
		f32 d2HighBound = Square(0.5 * width + tolerance);

		f32 maxOvershoot = 0;
		f32 maxOvershootParameter = 0;

		for(int i=0; i<MG_HULL_CHECK_SAMPLE_COUNT; i++)
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
			mg_gl_encode_stroke_quadratic(context, splitLeft);
			mg_gl_encode_stroke_quadratic(context, splitRight);
		}
		else
		{
			vec2 tmp = leftHull[0];
			leftHull[0] = leftHull[2];
			leftHull[2] = tmp;

			mg_gl_canvas_encode_element(context, MG_PATH_QUADRATIC, rightHull);
			mg_gl_canvas_encode_element(context, MG_PATH_QUADRATIC, leftHull);

			vec2 joint0[2] = {rightHull[2], leftHull[0]};
			vec2 joint1[2] = {leftHull[2], rightHull[0]};
			mg_gl_canvas_encode_element(context, MG_PATH_LINE, joint0);
			mg_gl_canvas_encode_element(context, MG_PATH_LINE, joint1);
		}
	}
}

void mg_gl_encode_stroke_cubic(mg_gl_encoding_context* context, vec2* p)
{
	f32 width = context->primitive->attributes.width;
	f32 tolerance = minimum(context->primitive->attributes.tolerance, 0.5 * width);

	//NOTE: check degenerate line cases
	f32 equalEps = 1e-3;

	if( (vec2_close(p[0], p[1], equalEps) && vec2_close(p[2], p[3], equalEps))
	  ||(vec2_close(p[0], p[1], equalEps) && vec2_close(p[1], p[2], equalEps))
	  ||(vec2_close(p[1], p[2], equalEps) && vec2_close(p[2], p[3], equalEps)))
	{
		vec2 line[2] = {p[0], p[3]};
		mg_gl_encode_stroke_line(context, line);
		return;
	}
	else if(vec2_close(p[0], p[1], equalEps) && vec2_close(p[1], p[3], equalEps))
	{
		vec2 line[2] = {p[0], vec2_add(vec2_mul(5./9, p[0]), vec2_mul(4./9, p[2]))};
		mg_gl_encode_stroke_line(context, line);
		return;
	}
	else if(vec2_close(p[0], p[2], equalEps) && vec2_close(p[2], p[3], equalEps))
	{
		vec2 line[2] = {p[0], vec2_add(vec2_mul(5./9, p[0]), vec2_mul(4./9, p[1]))};
		mg_gl_encode_stroke_line(context, line);
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
		mg_gl_encode_stroke_cubic(context, splitLeft);
		mg_gl_encode_stroke_cubic(context, splitRight);
	}
	else
	{
		f32 checkSamples[MG_HULL_CHECK_SAMPLE_COUNT] = {1./6, 2./6, 3./6, 4./6, 5./6};

		f32 d2LowBound = Square(0.5 * width - tolerance);
		f32 d2HighBound = Square(0.5 * width + tolerance);

		f32 maxOvershoot = 0;
		f32 maxOvershootParameter = 0;

		for(int i=0; i<MG_HULL_CHECK_SAMPLE_COUNT; i++)
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
			mg_gl_encode_stroke_cubic(context, splitLeft);
			mg_gl_encode_stroke_cubic(context, splitRight);
		}
		else
		{
			vec2 tmp = leftHull[0];
			leftHull[0] = leftHull[3];
			leftHull[3] = tmp;
			tmp = leftHull[1];
			leftHull[1] = leftHull[2];
			leftHull[2] = tmp;

			mg_gl_canvas_encode_element(context, MG_PATH_CUBIC, rightHull);
			mg_gl_canvas_encode_element(context, MG_PATH_CUBIC, leftHull);

			vec2 joint0[2] = {rightHull[3], leftHull[0]};
			vec2 joint1[2] = {leftHull[3], rightHull[0]};
			mg_gl_canvas_encode_element(context, MG_PATH_LINE, joint0);
			mg_gl_canvas_encode_element(context, MG_PATH_LINE, joint1);
		}
	}
}

void mg_gl_encode_stroke_element(mg_gl_encoding_context* context,
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
			mg_gl_encode_stroke_line(context, controlPoints);
			endPointIndex = 1;
			break;

		case MG_PATH_QUADRATIC:
			mg_gl_encode_stroke_quadratic(context, controlPoints);
			endPointIndex = 2;
			break;

		case MG_PATH_CUBIC:
			mg_gl_encode_stroke_cubic(context, controlPoints);
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

void mg_gl_stroke_cap(mg_gl_encoding_context* context,
                       vec2 p0,
                       vec2 direction)
{
	mg_attributes* attributes = &context->primitive->attributes;

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

	mg_gl_canvas_encode_element(context, MG_PATH_LINE, points);
	mg_gl_canvas_encode_element(context, MG_PATH_LINE, points+1);
	mg_gl_canvas_encode_element(context, MG_PATH_LINE, points+2);
	mg_gl_canvas_encode_element(context, MG_PATH_LINE, points+3);
}

void mg_gl_stroke_joint(mg_gl_encoding_context* context,
                         vec2 p0,
                         vec2 t0,
                         vec2 t1)
{
	mg_attributes* attributes = &context->primitive->attributes;

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

		mg_gl_canvas_encode_element(context, MG_PATH_LINE, points);
		mg_gl_canvas_encode_element(context, MG_PATH_LINE, points+1);
		mg_gl_canvas_encode_element(context, MG_PATH_LINE, points+2);
		mg_gl_canvas_encode_element(context, MG_PATH_LINE, points+3);
	}
	else
	{
		//NOTE(martin): add a bevel joint
		vec2 points[] = {p0,
		                 {p0.x + n0.x*halfW, p0.y + n0.y*halfW},
		                 {p0.x + n1.x*halfW, p0.y + n1.y*halfW},
		                 p0};

		mg_gl_canvas_encode_element(context, MG_PATH_LINE, points);
		mg_gl_canvas_encode_element(context, MG_PATH_LINE, points+1);
		mg_gl_canvas_encode_element(context, MG_PATH_LINE, points+2);
	}
}

u32 mg_gl_encode_stroke_subpath(mg_gl_encoding_context* context,
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

	//NOTE(martin): encode first element and compute first tangent
	mg_gl_encode_stroke_element(context, elements + startIndex, currentPoint, &startTangent, &endTangent, &endPoint);

	firstTangent = startTangent;
	previousEndTangent = endTangent;
	currentPoint = endPoint;

	//NOTE(martin): encode subsequent elements along with their joints

	mg_attributes* attributes = &context->primitive->attributes;

	u32 eltIndex = startIndex + 1;
	for(;
	    eltIndex<eltCount && elements[eltIndex].type != MG_PATH_MOVE;
	    eltIndex++)
	{
		mg_gl_encode_stroke_element(context, elements + eltIndex, currentPoint, &startTangent, &endTangent, &endPoint);

		if(attributes->joint != MG_JOINT_NONE)
		{
			mg_gl_stroke_joint(context, currentPoint, previousEndTangent, startTangent);
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
			mg_gl_stroke_joint(context, endPoint, endTangent, firstTangent);
		}
	}
	else if(attributes->cap == MG_CAP_SQUARE)
	{
		//NOTE(martin): add start and end cap
		mg_gl_stroke_cap(context, startPoint, (vec2){-startTangent.x, -startTangent.y});
		mg_gl_stroke_cap(context, endPoint, endTangent);
	}
	return(eltIndex);
}

void mg_gl_encode_stroke(mg_gl_encoding_context* context,
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
			startIndex = mg_gl_encode_stroke_subpath(context, elements, path, startIndex, startPoint);
		}
	}
}


void mg_gl_render_batch(mg_gl_canvas_backend* backend,
                        mg_wgl_surface* surface,
                        int pathCount,
                        int eltCount,
                        mg_image_data* image,
                        int tileSize,
                        int nTilesX,
                        int nTilesY,
                        vec2 viewportSize,
                        f32 scale)
{
	//NOTE: send the buffers
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// this is wrong, we should send the part we just wrote, which doesn't necessarily start at the begining of the buffer
	// or, alternatively, we should always write at the beginning of our data buffer and send from there...
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->pathBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, LAYOUT_PATH_SIZE*pathCount, backend->pathBufferData, GL_STREAM_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->elementBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, LAYOUT_PATH_ELT_SIZE*eltCount, backend->elementBufferData, GL_STREAM_DRAW);

	// --> this always bind the begining of the buffer, with the wrong pathCount/eltCount...

	//NOTE: clear counters
	int zero = 0;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->segmentCountBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), &zero, GL_DYNAMIC_COPY);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->tileQueueCountBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), &zero, GL_DYNAMIC_COPY);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->tileOpCountBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), &zero, GL_DYNAMIC_COPY);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

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
		int pathOffset = i*sizeof(mg_gl_path);
		int pathQueueOffset = i*sizeof(mg_gl_path_queue);
		int count = minimum(maxWorkGroupCount, pathCount-i);

		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, backend->pathBuffer, pathOffset, count*sizeof(mg_gl_path));
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, backend->pathQueueBuffer, pathQueueOffset, count*sizeof(mg_gl_path_queue));

		glDispatchCompute(count, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
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
		int offset = i*sizeof(mg_gl_path_elt);
		int count = minimum(maxWorkGroupCount, eltCount-i);

		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, backend->elementBuffer, offset, count*sizeof(mg_gl_path_elt));

		glDispatchCompute(count, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	//NOTE: backprop pass
	glUseProgram(backend->backprop);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, backend->tileQueueBuffer);

	for(int i=0; i<pathCount; i += maxWorkGroupCount)
	{
		int offset = i*sizeof(mg_gl_path_queue);
		int count = minimum(maxWorkGroupCount, pathCount-i);

		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, backend->pathQueueBuffer, offset, count*sizeof(mg_gl_path_queue));

		glDispatchCompute(count, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	//NOTE: merge pass
	glUseProgram(backend->merge);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, backend->pathBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, backend->pathQueueBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, backend->tileQueueBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, backend->tileOpCountBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, backend->tileOpBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, backend->screenTilesBuffer);

	glUniform1i(0, tileSize);
	glUniform1f(1, scale);
	glUniform1i(2, pathCount);

	glDispatchCompute(nTilesX, nTilesY, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	//NOTE: raster pass
	glUseProgram(backend->raster);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, backend->pathBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, backend->segmentCountBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, backend->segmentBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, backend->tileOpBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, backend->screenTilesBuffer);

	glUniform1f(0, scale);
	glUniform1i(1, backend->msaaCount);

	glBindImageTexture(0, backend->outTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

	if(image)
	{
		mg_gl_image* glImage = (mg_gl_image*)image;
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, glImage->texture);
		glUniform1ui(2, 1);
	}
	else
	{
		glUniform1ui(2, 0);
	}
	glDispatchCompute(nTilesX, nTilesY, 1);

	//NOTE: blit pass
	glUseProgram(backend->blit);
	glBindBuffer(GL_ARRAY_BUFFER, backend->dummyVertexBuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, backend->outTexture);
	glUniform1i(0, 0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	int err = glGetError();
	if(err)
	{
		log_error("gl error %i\n", err);
	}
}

/////////////////////////////////////////////////////////////////////////
//TODO
void mg_gl_canvas_resize(mg_gl_canvas_backend* backend, vec2 size);
/////////////////////////////////////////////////////////////////////////

void mg_gl_canvas_render(mg_canvas_backend* interface,
                         mg_color clearColor,
                         u32 primitiveCount,
                         mg_primitive* primitives,
                         u32 eltCount,
                         mg_path_elt* pathElements)
{
	mg_gl_canvas_backend* backend = (mg_gl_canvas_backend*)interface;

	//TODO update screen tiles buffer size
	mg_wgl_surface* surface = backend->surface;
	mp_rect frame = surface->interface.getFrame((mg_surface_data*)surface);
	vec2 contentsScaling = surface->interface.contentsScaling((mg_surface_data*)surface);
	//TODO support scaling in both axes
	f32 scale = contentsScaling.x;

	vec2 viewportSize = {frame.w * scale, frame.h * scale};
	int tileSize = MG_GL_TILE_SIZE;
	int nTilesX = (int)(frame.w * scale + tileSize - 1)/tileSize;
	int nTilesY = (int)(frame.h * scale + tileSize - 1)/tileSize;

	if(viewportSize.x != backend->frameSize.x || viewportSize.y != backend->frameSize.y)
	{
		//TODO:	mg_gl_canvas_resize(backend, viewportSize);
	}

	//NOTE: clear screen and reset input buffer offsets
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT);

	//NOTE: encode and render batches
	int pathCount = 0;
	vec2 currentPos = {0};
	mg_image currentImage = mg_image_nil();

	mg_gl_encoding_context context = {.glEltCount = 0,
	                                  .elementBufferData = backend->elementBufferData,
	                                  .pathBufferData = backend->pathBufferData };

	for(int primitiveIndex = 0; primitiveIndex < primitiveCount; primitiveIndex++)
	{
		mg_primitive* primitive = &primitives[primitiveIndex];

		if(primitiveIndex && (primitive->attributes.image.h != currentImage.h))
		{
			mg_image_data* imageData = mg_image_data_from_handle(currentImage);

			mg_gl_render_batch(backend,
			                   surface,
			                   pathCount,
			                   context.glEltCount,
			                   imageData,
			                   tileSize,
			                   nTilesX,
			                   nTilesY,
			                   viewportSize,
			                   scale);

			pathCount = 0;
			context.glEltCount = 0;
		}
		currentImage = primitive->attributes.image;

		if(primitive->path.count)
		{
			context.primitive = primitive;
			context.pathIndex = pathCount;
			context.pathScreenExtents = (vec4){FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};
			context.pathUserExtents = (vec4){FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};

			if(primitive->cmd == MG_CMD_STROKE)
			{
				mg_gl_encode_stroke(&context, pathElements + primitive->path.startIndex, &primitive->path);
			}
			else
			{
				int segCount = 0;
				for(int eltIndex = 0;
			    	(eltIndex < primitive->path.count) && (primitive->path.startIndex + eltIndex < eltCount);
			    	eltIndex++)
				{
					context.localEltIndex = segCount;

					mg_path_elt* elt = &pathElements[primitive->path.startIndex + eltIndex];

					if(elt->type != MG_PATH_MOVE)
					{
						vec2 p[4] = {currentPos, elt->p[0], elt->p[1], elt->p[2]};
						mg_gl_canvas_encode_element(&context, elt->type, p);
						segCount++;
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
			//NOTE: push path
			mg_gl_path* path = &context.pathBufferData[pathCount];
			pathCount++;

			path->cmd =	(mg_gl_cmd)primitive->cmd;

			path->box = (vec4){context.pathScreenExtents.x,
			                   context.pathScreenExtents.y,
			                   context.pathScreenExtents.z,
			                   context.pathScreenExtents.w};

			path->clip = (vec4){primitive->attributes.clip.x,
			                    primitive->attributes.clip.y,
			                    primitive->attributes.clip.x + primitive->attributes.clip.w,
			                    primitive->attributes.clip.y + primitive->attributes.clip.h};

			path->color = (vec4){primitive->attributes.color.r,
			                     primitive->attributes.color.g,
			                     primitive->attributes.color.b,
			                     primitive->attributes.color.a};

			mp_rect srcRegion = primitive->attributes.srcRegion;

			mp_rect destRegion = {context.pathUserExtents.x,
			                      context.pathUserExtents.y,
			                      context.pathUserExtents.z - context.pathUserExtents.x,
			                      context.pathUserExtents.w - context.pathUserExtents.y};

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

				//NOTE: mat3 std430 layout is an array of vec3, which are padded to _vec4_ alignment
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
			}
		}
	}

	mg_image_data* imageData = mg_image_data_from_handle(currentImage);
	mg_gl_render_batch(backend,
	                    surface,
	                    pathCount,
	                    context.glEltCount,
	                    imageData,
	                    tileSize,
	                    nTilesX,
	                    nTilesY,
	                    viewportSize,
	                    scale);

	//TODO add completion handler for rolling input buffers
}

//--------------------------------------------------------------------
// Image API
//--------------------------------------------------------------------
mg_image_data* mg_gl_canvas_image_create(mg_canvas_backend* interface, vec2 size)
{
	mg_gl_image* image = 0;

	image = malloc_type(mg_gl_image);
	if(image)
	{
		glGenTextures(1, &image->texture);
		glBindTexture(GL_TEXTURE_2D, image->texture);
//		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, size.x, size.y);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		image->interface.size = size;
	}
	return((mg_image_data*)image);
}

void mg_gl_canvas_image_destroy(mg_canvas_backend* interface, mg_image_data* imageInterface)
{
	//TODO: check that this image belongs to this context
	mg_gl_image* image = (mg_gl_image*)imageInterface;
	glDeleteTextures(1, &image->texture);
	free(image);
}

void mg_gl_canvas_image_upload_region(mg_canvas_backend* interface,
                                      mg_image_data* imageInterface,
                                      mp_rect region,
                                      u8* pixels)
{
	//TODO: check that this image belongs to this context
	mg_gl_image* image = (mg_gl_image*)imageInterface;
	glBindTexture(GL_TEXTURE_2D, image->texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, region.w, region.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

//--------------------------------------------------------------------
// Canvas setup / destroy
//--------------------------------------------------------------------

void mg_gl_canvas_destroy(mg_canvas_backend* interface)
{
	mg_gl_canvas_backend* backend = (mg_gl_canvas_backend*)interface;

	////////////////////////////////////////////////////////////////////
	//TODO
	////////////////////////////////////////////////////////////////////

	free(backend);
}

static int mg_gl_compile_shader(const char* name, GLuint shader, const char* source)
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

static int mg_gl_canvas_compile_compute_program_named(const char* name, const char* source, GLuint* outProgram)
{
	int res = 0;
	*outProgram = 0;

	GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
	GLuint program = glCreateProgram();

	res |= mg_gl_compile_shader(name, shader, source);

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
			log_error("Shader link error (%s): %.*s\n", name, size, buffer);

			res = -1;
		}
		else
		{
			*outProgram = program;
		}
	}
	return(res);
}

int mg_gl_canvas_compile_render_program_named(const char* progName,
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

	res |= mg_gl_compile_shader(vertexName, vertexShader, vertexSrc);
	res |= mg_gl_compile_shader(fragmentName, fragmentShader, fragmentSrc);

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
			log_error("Shader link error (%s): %.*s\n", progName, size, buffer);
			res = -1;
 		}
 		else
 		{
			*outProgram = program;
 		}
 	}
 	return(res);
}

#define mg_gl_canvas_compile_compute_program(src, out) \
	mg_gl_canvas_compile_compute_program_named(#src, src, out)

#define mg_gl_canvas_compile_render_program(progName, shaderSrc, vertexSrc, out) \
	mg_gl_canvas_compile_render_program_named(progName, #shaderSrc, #vertexSrc, shaderSrc, vertexSrc, out)

const u32 MG_GL_PATH_BUFFER_SIZE       = (4<<20)*sizeof(mg_gl_path),
          MG_GL_ELEMENT_BUFFER_SIZE    = (4<<20)*sizeof(mg_gl_path_elt),
          MG_GL_SEGMENT_BUFFER_SIZE    = (4<<20)*sizeof(mg_gl_segment),
          MG_GL_PATH_QUEUE_BUFFER_SIZE = (4<<20)*sizeof(mg_gl_path_queue),
          MG_GL_TILE_QUEUE_BUFFER_SIZE = (4<<20)*sizeof(mg_gl_tile_queue),
          MG_GL_TILE_OP_BUFFER_SIZE    = (4<<20)*sizeof(mg_gl_tile_op);

mg_canvas_backend* gl_canvas_backend_create(mg_wgl_surface* surface)
{
	mg_gl_canvas_backend* backend = malloc_type(mg_gl_canvas_backend);
	if(backend)
	{
		memset(backend, 0, sizeof(mg_gl_canvas_backend));
		backend->surface = surface;

		backend->msaaCount = MG_GL_MSAA_COUNT;

		//NOTE(martin): setup interface functions
		backend->interface.destroy = mg_gl_canvas_destroy;
		backend->interface.render = mg_gl_canvas_render;
		backend->interface.imageCreate = mg_gl_canvas_image_create;
		backend->interface.imageDestroy = mg_gl_canvas_image_destroy;
		backend->interface.imageUploadRegion = mg_gl_canvas_image_upload_region;

		surface->interface.prepare((mg_surface_data*)surface);

		glGenVertexArrays(1, &backend->vao);
		glBindVertexArray(backend->vao);

		//NOTE: create programs
		int err = 0;
		err |= mg_gl_canvas_compile_compute_program(glsl_path_setup, &backend->pathSetup);
		err |= mg_gl_canvas_compile_compute_program(glsl_segment_setup, &backend->segmentSetup);
		err |= mg_gl_canvas_compile_compute_program(glsl_backprop, &backend->backprop);
		err |= mg_gl_canvas_compile_compute_program(glsl_merge, &backend->merge);
		err |= mg_gl_canvas_compile_compute_program(glsl_raster, &backend->raster);
		err |= mg_gl_canvas_compile_render_program("blit", glsl_blit_vertex, glsl_blit_fragment, &backend->blit);

		if(glGetError() != GL_NO_ERROR)
		{
			err |= -1;
		}

		//NOTE: create out texture
		mp_rect frame = surface->interface.getFrame((mg_surface_data*)surface);
		vec2 scale = surface->interface.contentsScaling((mg_surface_data*)surface);

		glGenTextures(1, &backend->outTexture);
		glBindTexture(GL_TEXTURE_2D, backend->outTexture);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, frame.w*scale.x, frame.h*scale.y);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//NOTE: generate buffers
		glGenBuffers(1, &backend->dummyVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, backend->dummyVertexBuffer);

		glGenBuffers(1, &backend->pathBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->pathBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MG_GL_PATH_BUFFER_SIZE, 0, GL_DYNAMIC_COPY);

		//TODO change flags
		glGenBuffers(1, &backend->elementBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->elementBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MG_GL_ELEMENT_BUFFER_SIZE, 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->segmentBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->segmentBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MG_GL_SEGMENT_BUFFER_SIZE, 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->segmentCountBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->segmentCountBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->pathQueueBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->pathQueueBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MG_GL_PATH_QUEUE_BUFFER_SIZE, 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->tileQueueBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->tileQueueBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MG_GL_TILE_QUEUE_BUFFER_SIZE, 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->tileQueueCountBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->tileQueueCountBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->tileOpBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->tileOpBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MG_GL_TILE_OP_BUFFER_SIZE, 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->tileOpCountBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->tileOpCountBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), 0, GL_DYNAMIC_COPY);

		int tileSize = MG_GL_TILE_SIZE;
		int nTilesX = (int)(frame.w * scale.x + tileSize - 1)/tileSize;
		int nTilesY = (int)(frame.h * scale.y + tileSize - 1)/tileSize;

		glGenBuffers(1, &backend->screenTilesBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->screenTilesBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, nTilesX*nTilesY*sizeof(int), 0, GL_DYNAMIC_COPY);

		backend->pathBufferData = malloc(MG_GL_PATH_BUFFER_SIZE);
		backend->elementBufferData = malloc(MG_GL_ELEMENT_BUFFER_SIZE);

		if(err)
		{
			mg_gl_canvas_destroy((mg_canvas_backend*)backend);
			backend = 0;
		}
	}
	return((mg_canvas_backend*)backend);
}

mg_surface_data* gl_canvas_surface_create_for_window(mp_window window)
{
	mg_wgl_surface* surface = (mg_wgl_surface*)mg_wgl_surface_create_for_window(window);

	if(surface)
	{
		surface->interface.backend = gl_canvas_backend_create(surface);
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
