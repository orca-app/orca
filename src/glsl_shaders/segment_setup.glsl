
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
				tileOpBuffer.elements[tileOpIndex].windingOffsetOrCrossRight = 0;
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
					tileOpBuffer.elements[tileOpIndex].windingOffsetOrCrossRight = 1;
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

int quadratic_roots_with_det(float a, float b, float c, float det, out float r[2])
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
				if(abs(a) >= abs(c))
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

int quadratic_roots(float a, float b, float c, out float r[2])
{
	float det = square(b)/4. - a*c;
	return(quadratic_roots_with_det(a, b, c, det, r));
}

vec2 cubic_blossom(vec2 p[4], float u, float v, float w)
{
	vec2 b10 = u*p[1] + (1-u)*p[0];
	vec2 b11 = u*p[2] + (1-u)*p[1];
	vec2 b12 = u*p[3] + (1-u)*p[2];
	vec2 b20 = v*b11 + (1-v)*b10;
	vec2 b21 = v*b12 + (1-v)*b11;
	vec2 b30 = w*b21 + (1-w)*b20;
	return(b30);
}

void cubic_slice(vec2 p[4], float s0, float s1, out vec2 sp[4])
{
	/*NOTE: using blossoms to compute sub-curve control points ensure that the fourth point
	        of sub-curve (s0, s1) and the first point of sub-curve (s1, s3) match.
	        However, due to numerical errors, the evaluation of B(s=0) might not be equal to
	        p[0] (and likewise, B(s=1) might not equal p[3]).
	        We handle that case explicitly to ensure that we don't create gaps in the paths.
	*/
	sp[0] = (s0 == 0) ? p[0] : cubic_blossom(p, s0, s0, s0);
	sp[1] = cubic_blossom(p, s0, s0, s1);
	sp[2] = cubic_blossom(p, s0, s1, s1);
	sp[3] = (s1 == 1) ? p[3] : cubic_blossom(p, s1, s1, s1);
}

#define CUBIC_ERROR                0
#define CUBIC_SERPENTINE           1
#define CUBIC_CUSP                 2
#define CUBIC_CUSP_INFINITY        3
#define CUBIC_LOOP                 4
#define CUBIC_DEGENERATE_QUADRATIC 5
#define CUBIC_DEGENERATE_LINE      6

struct cubic_info
{
	int kind;
	mat4 K;
	vec2 ts[2];
	float d1;
	float d2;
	float d3;
};

cubic_info cubic_classify(vec2 c[4])
{
	cubic_info result;
	result.kind = CUBIC_ERROR;
	mat4 F;

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

	//NOTE(martin): compute the second factor of the discriminant discr(I) = d1^2*(3*d2^2 - 4*d3*d1)
	float discrFactor2 = 3.0*square(d2) - 4.0*d3*d1;

	//NOTE(martin): each following case gives the number of roots, hence the category of the parametric curve
	if(abs(d1) <= 1e-6 && abs(d2) <= 1e-6 && abs(d3) > 1e-6)
	{
		//NOTE(martin): quadratic degenerate case
		//NOTE(martin): compute quadratic curve control point, which is at p0 + 1.5*(p1-p0) = 1.5*p1 - 0.5*p0
		result.kind = CUBIC_DEGENERATE_QUADRATIC;
	}
	else if( (discrFactor2 > 0 && abs(d1) > 1e-6)
	       ||(discrFactor2 == 0 && abs(d1) > 1e-6))
	{
		//NOTE(martin): serpentine curve or cusp with inflection at infinity
		//              (these two cases are handled the same way).
		//NOTE(martin): compute the solutions (tl, sl), (tm, sm), and (tn, sn) of the inflection point equation
		float tmtl[2];
		quadratic_roots_with_det(1, -2*d2, (4./3.*d1*d3), (1./3.)*discrFactor2, tmtl);

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
		result.kind = (discrFactor2 > 0 && d1 != 0) ? CUBIC_SERPENTINE : CUBIC_CUSP;

		F = mat4(tl*tm, -sm*tl-sl*tm, sl*sm, 0,
		         cube(tl), -3*sl*square(tl), 3*square(sl)*tl, -cube(sl),
		         cube(tm), -3*sm*square(tm), 3*square(sm)*tm, -cube(sm),
		         1, 0, 0, 0);

		result.ts[0] = vec2(tm, sm);
		result.ts[1] = vec2(tl, sl);
	}
	else if(discrFactor2 < 0 && abs(d1) > 1e-6)
	{
		//NOTE(martin): loop curve
		result.kind = CUBIC_LOOP;

		float tetd[2];
		quadratic_roots_with_det(1, -2*d2, 4*(square(d2)-d1*d3), -discrFactor2, tetd);

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

		/*NOTE(martin):
			the power basis coefficients of points k,l,m,n are collected into the rows of the 4x4 matrix F:

				| td*te            td^2*te                 td*te^2                1 |
				| -se*td - sd*te   -se*td^2 - 2sd*te*td    -sd*te^2 - 2*se*td*te  0 |
				| sd*se            te*sd^2 + 2*se*td*sd    td*se^2 + 2*sd*te*se   0 |
				| 0                -sd^2*se                -sd*se^2               0 |
		*/
		F = mat4(td*te, -se*td-sd*te, sd*se, 0,
		         square(td)*te, -se*square(td)-2*sd*te*td, te*square(sd)+2*se*td*sd, -square(sd)*se,
		         td*square(te), -sd*square(te)-2*se*td*te, td*square(se)+2*sd*te*se, -sd*square(se),
		         1, 0, 0, 0);

		result.ts[0] = vec2(td, sd);
		result.ts[1] = vec2(te, se);
	}
	else if(d2 != 0)
	{
		//NOTE(martin): cusp with cusp at infinity
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
		result.kind = CUBIC_CUSP_INFINITY;

		F = mat4(tl, -sl, 0, 0,
		         cube(tl), -3*sl*square(tl), 3*square(sl)*tl, -cube(sl),
		         1, 0, 0, 0,
		         1, 0, 0, 0);

		result.ts[0] = vec2(tl, sl);
		result.ts[1] = vec2(0, 0);
	}
	else
	{
		//NOTE(martin): line or point degenerate case
		result.kind = CUBIC_DEGENERATE_LINE;
	}

	/*
			F is then multiplied by M3^(-1) on the left which yelds the bezier coefficients k, l, m, n
			at the control points.

			               | 1  0   0   0 |
				M3^(-1) =  | 1  1/3 0   0 |
				           | 1  2/3 1/3 0 |
					       | 1  1   1   1 |
	*/
	mat4 invM3 = mat4(1, 1, 1, 1,
	                  0, 1./3., 2./3., 1,
	                  0, 0, 1./3., 1,
	                  0, 0, 0, 1);

	result.K = transpose(invM3*F);

	return(result);
}

vec2 select_hull_vertex(vec2 p0, vec2 p1, vec2 p2, vec2 p3)
{
	/*NOTE: check intersection of lines (p1-p0) and (p3-p2)
		P = p0 + u(p1-p0)
		P = p2 + w(p3-p2)

		control points are inside a right triangle so we should always find an intersection
	*/
	vec2 pm;

	float det = (p1.x - p0.x)*(p3.y - p2.y) - (p1.y - p0.y)*(p3.x - p2.x);
	float sqrNorm0 = dot(p1-p0, p1-p0);
	float sqrNorm1 = dot(p2-p3, p2-p3);

	if(abs(det) < 1e-3 || sqrNorm0 < 0.1 || sqrNorm1 < 0.1)
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

void cubic_emit(cubic_info curve, vec2 p[4], float s0, float s1, vec2 sp[4], int pathIndex)
{
	int segIndex = push_segment(sp, MG_GL_CUBIC, pathIndex);

	vec2 v0 = p[0];
	vec2 v1 = p[3];
	vec2 v2;
	mat3 K;

	//TODO: haul that up in caller
	float sqrNorm0 = dot(p[1]-p[0], p[1]-p[0]);
	float sqrNorm1 = dot(p[2]-p[3], p[2]-p[3]);

	if(dot(p[0]-p[3], p[0]-p[3]) > 1e-5)
	{
		if(sqrNorm0 >= sqrNorm1)
 		{
 			v2 = p[1];
			K = mat3(curve.K[0].xyz, curve.K[3].xyz, curve.K[1].xyz);
 		}
 		else
 		{
			v2 = p[2];
			K = mat3(curve.K[0].xyz, curve.K[3].xyz, curve.K[2].xyz);
 		}
 	}
 	else
 	{
		v1 = p[1];
		v2 = p[2];
		K = mat3(curve.K[0].xyz, curve.K[1].xyz, curve.K[2].xyz);
 	}
 	//NOTE: set matrices

 	//TODO: should we compute matrix relative to a base point to avoid loss of precision
 	//      when computing barycentric matrix?

	mat3 B = barycentric_matrix(v0, v1, v2);

 	segmentBuffer.elements[segIndex].implicitMatrix = K*B;
	segmentBuffer.elements[segIndex].hullVertex = select_hull_vertex(sp[0], sp[1], sp[2], sp[3]);

  	//NOTE: compute sign flip
  	segmentBuffer.elements[segIndex].sign = 1;

  	if(  curve.kind == CUBIC_SERPENTINE
	  || curve.kind == CUBIC_CUSP)
  	{
		segmentBuffer.elements[segIndex].sign = (curve.d1 < 0)? -1 : 1;
	}
	else if(curve.kind == CUBIC_LOOP)
	{
		float d1 = curve.d1;
		float d2 = curve.d2;
		float d3 = curve.d3;

		float H0 = d3*d1-square(d2) + d1*d2*s0 - square(d1)*square(s0);
		float H1 = d3*d1-square(d2) + d1*d2*s1 - square(d1)*square(s1);
		float H = (abs(H0) > abs(H1)) ? H0 : H1;
		segmentBuffer.elements[segIndex].sign = (H*d1 > 0) ? -1 : 1;
	}

	if(sp[3].y > sp[0].y)
	{
		segmentBuffer.elements[segIndex].sign *= -1;
	}

	//NOTE: bin to tiles
	bin_to_tiles(segIndex);
}

void cubic_setup(vec2 p[4], int pathIndex)
{
	/*NOTE(martin): first convert the control points to power basis, multiplying by M3

		     | 1  0  0  0|      |p0|      |c0|
		M3 = |-3  3  0  0|, B = |p1|, C = |c1| = M3*B
		     | 3 -6  3  0|      |p2|      |c2|
		     |-1  3 -3  1|      |p3|      |c3|
	*/
	vec2 c[4] = {
		p[0],
	    3.0*(p[1] - p[0]),
	    3.0*(p[0] + p[2] - 2*p[1]),
	    3.0*(p[1] - p[2]) + p[3] - p[0]};

	//NOTE: get classification, implicit matrix, double points and inflection points
	cubic_info curve = cubic_classify(c);

	if(curve.kind == CUBIC_DEGENERATE_LINE)
	{
		vec2 l[4] = {p[0], p[3], vec2(0), vec2(0)};
		line_setup(l, pathIndex);
		return;
	}
	else if(curve.kind == CUBIC_DEGENERATE_QUADRATIC)
	{
		vec2 quadPoint = vec2(1.5*p[1].x - 0.5*p[0].x, 1.5*p[1].y - 0.5*p[0].y);
		vec2 q[4] = {p[0], quadPoint, p[3], vec2(0)};
		quadratic_setup(q, pathIndex);
		return;
	}

	//NOTE: get the roots of B'(s) = 3.c3.s^2 + 2.c2.s + c1
	float rootsX[2];
	int rootCountX = quadratic_roots(3*c[3].x, 2*c[2].x, c[1].x, rootsX);

	float rootsY[2];
	int rootCountY = quadratic_roots(3*c[3].y, 2*c[2].y, c[1].y, rootsY);

	float roots[6];
	for(int i=0; i<rootCountX; i++)
	{
		roots[i] = rootsX[i];
	}
	for(int i=0; i<rootCountY; i++)
	{
		roots[i+rootCountX] = rootsY[i];
	}

	//NOTE: add double points and inflection points to roots if finite
	int rootCount = rootCountX + rootCountY;
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

	//NOTE: for each monotonic segment, compute hull matrix and sign, and emit segment
	for(int sliceIndex=0; sliceIndex<splitCount-1; sliceIndex++)
	{
		float s0 = splits[sliceIndex];
		float s1 = splits[sliceIndex+1];
		vec2 sp[4];
		cubic_slice(p, s0, s1, sp);
		cubic_emit(curve, p, s0, s1, sp, pathIndex);
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

		case MG_GL_CUBIC:
		{
			vec2 p[4] = {elt.p[0]*scale, elt.p[1]*scale, elt.p[2]*scale, elt.p[3]*scale};
			cubic_setup(p, elt.pathIndex);
		} break;

		default:
			break;
	}
}
