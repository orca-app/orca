
layout(std430) buffer;

// command
#define MG_GL_FILL 0
#define MG_GL_STROKE 1

// element kind
#define MG_GL_LINE 1
#define MG_GL_QUADRATIC 2
#define MG_GL_CUBIC 3

// curve config
#define MG_GL_BL 1  /* curve on bottom left */
#define MG_GL_BR 2 /* curve on bottom right */
#define MG_GL_TL 3 /* curve on top left */
#define	MG_GL_TR 4 /* curve on top right */

// Operations
#define MG_GL_OP_START 0
#define MG_GL_OP_SEGMENT 1

// MSAA
#define MG_GL_MAX_SAMPLE_COUNT 8
#define MG_GL_MAX_SRC_SAMPLE_COUNT 4

struct mg_gl_path
{
	mat3 uvTransform;
	vec4 color;
	vec4 box;
	vec4 clip;
	int cmd;
};

struct mg_gl_path_elt
{
	vec2 p[4];
	int pathIndex;
	int kind;
};

struct mg_gl_segment
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

struct mg_gl_path_queue
{
	ivec4 area;
	int tileQueues;
};

struct mg_gl_tile_op
{
	int kind;
	int next;
	int index;
	int windingOffsetOrCrossRight;
};

struct mg_gl_tile_queue
{
	int windingOffset;
	int first;
	int last;
};

float ccw(vec2 a, vec2 b, vec2 c)
{
	return((b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x));
}

int side_of_segment(vec2 p, mg_gl_segment seg)
{
	int side = 0;
	if(p.y > seg.box.w || p.y <= seg.box.y)
	{
		if(p.x > seg.box.x && p.x <= seg.box.z)
		{
			if(p.y > seg.box.w)
			{
				side = (seg.config == MG_GL_TL || seg.config == MG_GL_BR)? -1 : 1;
			}
			else
			{
				side = (seg.config == MG_GL_TL || seg.config == MG_GL_BR)? 1 : -1;
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
			case MG_GL_TL:
				a = seg.box.xy;
				b = seg.box.zw;
				break;

			case MG_GL_BR:
				a = seg.box.zw;
				b = seg.box.xy;
				break;

			case MG_GL_TR:
				a = seg.box.xw;
				b = seg.box.zy;
				break;

			case MG_GL_BL:
				a = seg.box.zy;
				b = seg.box.xw;
				break;
		}
		c = seg.hullVertex;

		if(ccw(a, b, p) < 0)
		{
			// other side of the diagonal
			side = (seg.config == MG_GL_BR || seg.config == MG_GL_TR) ? -1 : 1;
		}
		else if(ccw(b, c, p) < 0 || ccw(c, a, p) < 0)
		{
			// same side of the diagonal, but outside curve hull
			side = (seg.config == MG_GL_BL || seg.config == MG_GL_TL) ? -1 : 1;
		}
		else
		{
			// inside curve hull
			switch(seg.kind)
			{
				case MG_GL_LINE:
					side = 1;
					break;

				case MG_GL_QUADRATIC:
				{
					vec3 ph = {p.x, p.y, 1};
					vec3 klm = seg.implicitMatrix * ph;
					side = ((klm.x*klm.x - klm.y)*klm.z < 0)? -1 : 1;
				} break;

				case MG_GL_CUBIC:
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
