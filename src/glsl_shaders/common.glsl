
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
	int localEltIndex;
	int kind;
};

struct mg_gl_segment
{
	int kind;
	int pathIndex;
	int config; //TODO pack these
	int windingIncrement;
	vec4 box;
	mat3 hullMatrix;
	mat3 implicitMatrix;
	float sign;
	vec2 hullVertex;
	int debugID;
};

struct mg_gl_path_queue
{
	ivec4 area;
	int tileQueues;
};

struct mg_gl_tile_op
{
	int kind;
	int index;
	int next;
	bool crossRight;
	int windingOffset;
};

struct mg_gl_tile_queue
{
	int windingOffset;
	int first;
	int last;
};
