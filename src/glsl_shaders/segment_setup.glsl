
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

precision mediump float;
layout(std430) buffer;

layout(binding = 0) restrict readonly buffer elementBufferSSBO
{
	mg_gl_path_elt elements[];
} elementBuffer;

layout(binding = 1) restrict buffer segmentCountBufferSSBO
{
	int elements[];
} segmentCountBuffer;

layout(binding = 2) restrict writeonly buffer segmentBufferSSBO
{
	mg_gl_segment elements[];
} segmentBuffer;

layout(location = 0) uniform float scale;


int push_segment(in vec2 p[4], int kind)
{
	int segIndex = atomicAdd(segmentCountBuffer.elements[0], 1);

	vec2 s = p[0];
	vec2 c = p[0];
	vec2 e = p[1];

	bool goingUp = e.y >= s.y;
	bool goingRight = e.x >= s.x;

	vec4 box = vec4(min(s.x, e.x),
	                min(s.y, e.y),
	                max(s.x, e.x),
	                max(s.y, e.y));

	segmentBuffer.elements[segIndex].kind = kind;
	segmentBuffer.elements[segIndex].pathIndex = 0; ///
	segmentBuffer.elements[segIndex].windingIncrement = goingUp ? 1 : -1;
	segmentBuffer.elements[segIndex].box = box;

	float dx = c.x - box.x;
	float dy = c.y - box.y;
	float alpha = (box.w - box.y)/(box.z - box.x);
	float ofs = box.w - box.y;

	if(goingUp == goingRight)
	{
		if(kind == MG_GL_LINE)
		{
			segmentBuffer.elements[segIndex].config = MG_GL_BR;
		}
		else if(dy > alpha*dx)
		{
			segmentBuffer.elements[segIndex].config = MG_GL_TL;
		}
		else
		{
			segmentBuffer.elements[segIndex].config = MG_GL_BR;
		}
	}
	else
	{
		if(kind == MG_GL_LINE)
		{
			segmentBuffer.elements[segIndex].config = MG_GL_TR;
		}
		else if(dy < ofs - alpha*dx)
		{
			segmentBuffer.elements[segIndex].config = MG_GL_BL;
		}
		else
		{
			segmentBuffer.elements[segIndex].config = MG_GL_TR;
		}
	}

	return(segIndex);
}

void line_setup(vec2 p[4])
{
	int segIndex = push_segment(p, MG_GL_LINE);
	segmentBuffer.elements[segIndex].hullVertex = p[0];

	//TODO later bin to tiles
}

void main()
{
	int eltIndex = int(gl_WorkGroupID.x);

	mg_gl_path_elt elt = elementBuffer.elements[eltIndex];

	switch(elt.kind)
	{
		case MG_GL_LINE:
		{
			vec2 p[4] = {elt.p[0]*scale, elt.p[1]*scale, vec2(0), vec2(0)};
			line_setup(p);
		} break;

		default:
			break;
	}
}
