
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

precision mediump float;
layout(std430) buffer;

layout(binding = 0) restrict readonly buffer elementBufferSSBO
{
	mg_gl_path_elt elements[];
} elementBuffer;

layout(binding = 1) coherent restrict buffer segmentCountBufferSSBO
{
	int elements[];
} segmentCountBuffer;

layout(binding = 2) restrict buffer segmentBufferSSBO
{
	mg_gl_segment elements[];
} segmentBuffer;

layout(binding = 3) coherent restrict buffer tileOpCountBufferSSBO
{
	int elements[];
} tileOpCountBuffer;

layout(binding = 4) restrict buffer tileOpBufferSSBO
{
	mg_gl_tile_op elements[];
} tileOpBuffer;

layout(binding = 5) coherent restrict buffer tileQueuesBufferSSBO
{
	mg_gl_tile_queue elements[];
} tileQueuesBuffer;

layout(location = 0) uniform float scale;
layout(location = 1) uniform uint tileSize;
layout(location = 2) uniform ivec2 nTiles;

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

void bin_to_tiles(int segIndex)
{
	//NOTE: add segment index to the queues of tiles it overlaps with
	const mg_gl_segment seg = segmentBuffer.elements[segIndex];

	ivec4 pathArea = ivec4(0, 0, nTiles.x, nTiles.y);

	ivec4 coveredTiles = ivec4(seg.box)/int(tileSize);
	int xMin = max(0, coveredTiles.x - pathArea.x);
	int yMin = max(0, coveredTiles.y - pathArea.y);
	int xMax = min(coveredTiles.z - pathArea.x, pathArea.z-1);
	int yMax = min(coveredTiles.w - pathArea.y, pathArea.w-1);

	for(int y = yMin; y <= yMax; y++)
	{
		for(int x = xMin ; x <= xMax; x++)
		{
			vec4 tileBox = vec4(float(x + pathArea.x),
			                    float(y + pathArea.y),
			                    float(x + pathArea.x + 1),
			                    float(y + pathArea.y + 1)) * float(tileSize);

			vec2 bl = {tileBox.x, tileBox.y};
			vec2 br = {tileBox.z, tileBox.y};
			vec2 tr = {tileBox.z, tileBox.w};
			vec2 tl = {tileBox.x, tileBox.w};

			int sbl = side_of_segment(bl, seg);
			int sbr = side_of_segment(br, seg);
			int str = side_of_segment(tr, seg);
			int stl = side_of_segment(tl, seg);

			bool crossL = (stl*sbl < 0);
			bool crossR = (str*sbr < 0);
			bool crossT = (stl*str < 0);
			bool crossB = (sbl*sbr < 0);

			vec2 s0, s1;
			if(seg.config == MG_GL_TL||seg.config == MG_GL_BR)
			{
				s0 = seg.box.xy;
				s1 = seg.box.zw;
			}
			else
			{
				s0 = seg.box.xw;
				s1 = seg.box.zy;
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
				int tileOpIndex = atomicAdd(tileOpCountBuffer.elements[0], 1);

				tileOpBuffer.elements[tileOpIndex].kind = MG_GL_OP_SEGMENT;
				tileOpBuffer.elements[tileOpIndex].index = segIndex;
				tileOpBuffer.elements[tileOpIndex].crossRight = false;
				tileOpBuffer.elements[tileOpIndex].next = -1;

				int tileIndex = y*pathArea.z + x;

				tileOpBuffer.elements[tileOpIndex].next = atomicExchange(tileQueuesBuffer.elements[tileIndex].first, tileOpIndex);
				if(tileOpBuffer.elements[tileOpIndex].next == -1)
				{
					tileQueuesBuffer.elements[tileIndex].last = tileOpIndex;
				}

				//NOTE: if the segment crosses the tile's bottom boundary, update the tile's winding offset
				if(crossB)
				{
					atomicAdd(tileQueuesBuffer.elements[tileIndex].windingOffset, seg.windingIncrement);
				}

				//NOTE: if the segment crosses the right boundary, mark it.
				if(crossR)
				{
					tileOpBuffer.elements[tileOpIndex].crossRight = true;
				}
			}
		}
	}
}

void line_setup(vec2 p[4])
{
	int segIndex = push_segment(p, MG_GL_LINE);
	segmentBuffer.elements[segIndex].hullVertex = p[0];

	bin_to_tiles(segIndex);
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
