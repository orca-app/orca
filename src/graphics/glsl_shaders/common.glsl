
layout(std430) buffer;

// command
#define OC_GL_FILL 0
#define OC_GL_STROKE 1

// element kind
#define OC_GL_LINE 1
#define OC_GL_QUADRATIC 2
#define OC_GL_CUBIC 3

// curve config
#define OC_GL_BL 1  /* curve on bottom left */
#define OC_GL_BR 2 /* curve on bottom right */
#define OC_GL_TL 3 /* curve on top left */
#define	OC_GL_TR 4 /* curve on top right */

// Operations
#define OC_GL_OP_FILL      0
#define OC_GL_OP_CLIP_FILL 1
#define OC_GL_OP_START     2
#define OC_GL_OP_END       3
#define OC_GL_OP_SEGMENT   4

// MSAA
#define OC_GL_MAX_SAMPLE_COUNT 8
#define OC_GL_MAX_SRC_SAMPLE_COUNT 4

struct oc_gl_path
{
	mat3 uvTransform;
	vec4 color;
	vec4 box;
	vec4 clip;
	int cmd;
	int textureID;
};

struct oc_gl_path_elt
{
	vec2 p[4];
	int pathIndex;
	int kind;
};

struct oc_gl_segment
{
	int kind;
	int pathIndex;
	int config;
	int windingIncrement;
	vec4 box;
	mat3 implicitMatrix;
	vec2 hullVertex;
	float sign;
};

struct oc_gl_path_queue
{
	ivec4 area;
	int tileQueues;
};

struct oc_gl_tile_op
{
	int kind;
	int next;
	int index;
	int windingOffsetOrCrossRight;
};

struct oc_gl_tile_queue
{
	int windingOffset;
	int first;
	int last;
};

struct oc_gl_screen_tile
{
	uvec2 tileCoord;
	int first;
};

struct oc_gl_dispatch_indirect_command
{
	uint  num_groups_x;
	uint  num_groups_y;
	uint  num_groups_z;
};

float ccw(vec2 a, vec2 b, vec2 c)
{
	return((b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x));
}

int side_of_segment(vec2 p, oc_gl_segment seg)
{
	int side = 0;
	if(p.y > seg.box.w || p.y <= seg.box.y)
	{
		if(p.x > seg.box.x && p.x <= seg.box.z)
		{
			if(p.y > seg.box.w)
			{
				side = (seg.config == OC_GL_TL || seg.config == OC_GL_BR)? -1 : 1;
			}
			else
			{
				side = (seg.config == OC_GL_TL || seg.config == OC_GL_BR)? 1 : -1;
			}
		}
	}
	else if(p.x > seg.box.z)
	{
		side = 1;
	}
	else if(p.x <= seg.box.x)
	{
		side = -1;
	}
	else
	{
		vec2 a, b, c;
		switch(seg.config)
		{
			case OC_GL_TL:
				a = seg.box.xy;
				b = seg.box.zw;
				break;

			case OC_GL_BR:
				a = seg.box.zw;
				b = seg.box.xy;
				break;

			case OC_GL_TR:
				a = seg.box.xw;
				b = seg.box.zy;
				break;

			case OC_GL_BL:
				a = seg.box.zy;
				b = seg.box.xw;
				break;
		}
		c = seg.hullVertex;

		if(ccw(a, b, p) < 0)
		{
			// other side of the diagonal
			side = (seg.config == OC_GL_BR || seg.config == OC_GL_TR) ? -1 : 1;
		}
		else if(ccw(b, c, p) < 0 || ccw(c, a, p) < 0)
		{
			// same side of the diagonal, but outside curve hull
			side = (seg.config == OC_GL_BL || seg.config == OC_GL_TL) ? -1 : 1;
		}
		else
		{
			// inside curve hull
			switch(seg.kind)
			{
				case OC_GL_LINE:
					side = 1;
					break;

				case OC_GL_QUADRATIC:
				{
					vec3 ph = {p.x, p.y, 1};
					vec3 klm = seg.implicitMatrix * ph;
					side = ((klm.x*klm.x - klm.y)*klm.z < 0)? -1 : 1;
				} break;

				case OC_GL_CUBIC:
				{
					vec3 ph = {p.x, p.y, 1};
					vec3 klm = seg.implicitMatrix * ph;
					side = (seg.sign*(klm.x*klm.x*klm.x - klm.y*klm.z) < 0)? -1 : 1;
				} break;
			}
		}
	}
	return(side);
}
