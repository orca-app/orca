
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

layout(binding = 3) restrict buffer pathQueueBufferSSBO
{
	mg_gl_path_queue elements[];
} pathQueueBuffer;

layout(binding = 4) coherent restrict buffer tileQueueBufferSSBO
{
	mg_gl_tile_queue elements[];
} tileQueueBuffer;

layout(binding = 5) coherent restrict buffer tileOpCountBufferSSBO
{
	int elements[];
} tileOpCountBuffer;

layout(binding = 6) restrict buffer tileOpBufferSSBO
{
	mg_gl_tile_op elements[];
} tileOpBuffer;

layout(location = 0) uniform float scale;
layout(location = 1) uniform uint tileSize;

void bin_to_tiles(int segIndex)
{
	//NOTE: add segment index to the queues of tiles it overlaps with
	const mg_gl_segment seg = segmentBuffer.elements[segIndex];
	const mg_gl_path_queue pathQueue = pathQueueBuffer.elements[seg.pathIndex];

	ivec4 pathArea = pathQueue.area;
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

				int tileQueueIndex = pathQueue.tileQueues + y*pathArea.z + x;

				tileOpBuffer.elements[tileOpIndex].next = atomicExchange(tileQueueBuffer.elements[tileQueueIndex].first, tileOpIndex);
				if(tileOpBuffer.elements[tileOpIndex].next == -1)
				{
					tileQueueBuffer.elements[tileQueueIndex].last = tileOpIndex;
				}

				//NOTE: if the segment crosses the tile's bottom boundary, update the tile's winding offset
				if(crossB)
				{
					atomicAdd(tileQueueBuffer.elements[tileQueueIndex].windingOffset, seg.windingIncrement);
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

int push_segment(in vec2 p[4], int kind, int pathIndex)
{
	int segIndex = atomicAdd(segmentCountBuffer.elements[0], 1);

	vec2 s, c, e;

	switch(kind)
	{
		case MG_GL_LINE:
			s = p[0];
			c = p[0];
			e = p[1];
			break;

		case MG_GL_QUADRATIC:
			s = p[0];
			c = p[1];
			e = p[2];
			break;

		case MG_GL_CUBIC:
		{
			s = p[0];
			float sqrNorm0 = dot(p[1]-p[0], p[1]-p[0]);
			float sqrNorm1 = dot(p[3]-p[2], p[3]-p[2]);
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

	bool goingUp = e.y >= s.y;
	bool goingRight = e.x >= s.x;

	vec4 box = vec4(min(s.x, e.x),
	                min(s.y, e.y),
	                max(s.x, e.x),
	                max(s.y, e.y));

	segmentBuffer.elements[segIndex].kind = kind;
	segmentBuffer.elements[segIndex].pathIndex = pathIndex;
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

#define square(x) ((x)*(x))
#define cube(x) ((x)*(x)*(x))

void line_setup(vec2 p[4], int pathIndex)
{
	int segIndex = push_segment(p, MG_GL_LINE, pathIndex);
	segmentBuffer.elements[segIndex].hullVertex = p[0];

	bin_to_tiles(segIndex);
}

vec2 quadratic_blossom(vec2 p[4], float u, float v)
{
	vec2 b10 = u*p[1] + (1-u)*p[0];
	vec2 b11 = u*p[2] + (1-u)*p[1];
	vec2 b20 = v*b11 + (1-v)*b10;
	return(b20);
}

void quadratic_slice(vec2 p[4], float s0, float s1, out vec2 sp[4])
{
	/*NOTE: using blossoms to compute sub-curve control points ensure that the fourth point
	        of sub-curve (s0, s1) and the first point of sub-curve (s1, s3) match.
	        However, due to numerical errors, the evaluation of B(s=0) might not be equal to
	        p[0] (and likewise, B(s=1) might not equal p[3]).
	        We handle that case explicitly to ensure that we don't create gaps in the paths.
	*/
	sp[0] = (s0 == 0) ? p[0] : quadratic_blossom(p, s0, s0);
	sp[1] = quadratic_blossom(p, s0, s1);
	sp[2] = (s1 == 1) ? p[2] : quadratic_blossom(p, s1, s1);
}

int quadratic_monotonize(vec2 p[4], out float splits[4])
{
	//NOTE: compute split points
	int count = 0;
	splits[0] = 0;
	count++;

	vec2 r = (p[0] - p[1])/(p[2] - 2*p[1] + p[0]);
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

mat3 barycentric_matrix(vec2 v0, vec2 v1, vec2 v2)
{
	float det = v0.x*(v1.y-v2.y) + v1.x*(v2.y-v0.y) + v2.x*(v0.y - v1.y);
	mat3 B = {{v1.y - v2.y, v2.y-v0.y, v0.y-v1.y},
 	            {v2.x - v1.x, v0.x-v2.x, v1.x-v0.x},
 	            {v1.x*v2.y-v2.x*v1.y, v2.x*v0.y-v0.x*v2.y, v0.x*v1.y-v1.x*v0.y}};
 	B *= (1/det);
 	return(B);
}

void quadratic_emit(vec2 p[4], int pathIndex)
{
	int segIndex = push_segment(p, MG_GL_QUADRATIC, pathIndex);

	//NOTE: compute implicit equation matrix
	float det = p[0].x*(p[1].y-p[2].y) + p[1].x*(p[2].y-p[0].y) + p[2].x*(p[0].y - p[1].y);

	float a = p[0].y - p[1].y + 0.5*(p[2].y - p[0].y);
	float b = p[1].x - p[0].x + 0.5*(p[0].x - p[2].x);
	float c = p[0].x*p[1].y - p[1].x*p[0].y + 0.5*(p[2].x*p[0].y - p[0].x*p[2].y);
	float d = p[0].y - p[1].y;
	float e = p[1].x - p[0].x;
	float f = p[0].x*p[1].y - p[1].x*p[0].y;

	float flip = (  segmentBuffer.elements[segIndex].config == MG_GL_TL
	             || segmentBuffer.elements[segIndex].config == MG_GL_BL)? -1 : 1;

	float g = flip*(p[2].x*(p[0].y - p[1].y) + p[0].x*(p[1].y - p[2].y) + p[1].x*(p[2].y - p[0].y));

	segmentBuffer.elements[segIndex].implicitMatrix = (1/det)*mat3(a, d, 0.,
	                                                               b, e, 0.,
	                                                               c, f, g);
	segmentBuffer.elements[segIndex].hullVertex = p[1];

	bin_to_tiles(segIndex);
}

void quadratic_setup(vec2 p[4], int pathIndex)
{
	float splits[4];
	int splitCount = quadratic_monotonize(p, splits);

	//NOTE: produce bézier curve for each consecutive pair of roots
	for(int sliceIndex=0; sliceIndex<splitCount-1; sliceIndex++)
	{
		vec2 sp[4];
		quadratic_slice(p, splits[sliceIndex], splits[sliceIndex+1], sp);
		quadratic_emit(sp, pathIndex);
	}
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
			line_setup(p, elt.pathIndex);
		} break;

		case MG_GL_QUADRATIC:
		{
			vec2 p[4] = {elt.p[0]*scale, elt.p[1]*scale, elt.p[2]*scale, vec2(0)};
			quadratic_setup(p, elt.pathIndex);
		} break;

		default:
			break;
	}
}
