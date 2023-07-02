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

typedef struct mg_gl_canvas_backend
{
	mg_canvas_backend interface;
	mg_wgl_surface* surface;

	mp_rect clip;
	mg_mat2x3 transform;
	mg_image image;
	mp_rect srcRegion;
	mg_color clearColor;

	u32 nextShapeIndex;
	u32 vertexCount;
	u32 indexCount;

	vec4 shapeExtents;
	vec4 shapeScreenExtents;

	mg_vertex_layout vertexLayout;

	GLuint vao;
	GLuint dummyVertexBuffer;
	GLuint vertexBuffer;
	GLuint shapeBuffer;
	GLuint indexBuffer;
	GLuint tileCounterBuffer;
	GLuint tileArrayBuffer;
	GLuint clearCounterProgram;
	GLuint tileProgram;
	GLuint sortProgram;
	GLuint drawProgram;
	GLuint blitProgram;

	GLuint outTexture;

	char* indexMapping;
	char* vertexMapping;
	char* shapeMapping;

} mg_gl_canvas_backend;

typedef struct mg_gl_image
{
	mg_image_data interface;

	GLuint textureID;
} mg_gl_image;

//NOTE: debugger
typedef struct debug_vertex
{
	vec4 cubic;
	vec2 pos;
	int shapeIndex;
	u8 pad[4];
} debug_vertex;

typedef struct debug_shape
{
	vec4 color;
	vec4 clip;
	vec2 uv;
	u8 pad[8];
} debug_shape;


//--------------------------------------------------------------------
// Primitives encoding
//--------------------------------------------------------------------
void mg_reset_shape_index(mg_gl_canvas_backend* backend)
{
	backend->nextShapeIndex = 0;
	backend->shapeExtents = (vec4){FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};
}

void mg_finalize_shape(mg_gl_canvas_backend* backend)
{
	if(backend->nextShapeIndex)
	{
		//NOTE: set shape's uv transform for the _current_ shape
		vec2 texSize = mg_image_size(backend->image);

		mp_rect srcRegion = backend->srcRegion;

		mp_rect destRegion = {backend->shapeExtents.x,
		                      backend->shapeExtents.y,
		                      backend->shapeExtents.z - backend->shapeExtents.x,
		                      backend->shapeExtents.w - backend->shapeExtents.y};

		mg_mat2x3 srcRegionToImage = {1/texSize.x, 0,           srcRegion.x/texSize.x,
		                                0,           1/texSize.y, srcRegion.y/texSize.y};
		mg_mat2x3 destRegionToSrcRegion = {srcRegion.w/destRegion.w, 0,                        0,
		                                   0,                        srcRegion.h/destRegion.h, 0};
		mg_mat2x3 userToDestRegion = {1, 0, -destRegion.x,
		                              0, 1, -destRegion.y};

		mg_mat2x3 screenToUser = mg_mat2x3_inv(backend->transform);

		mg_mat2x3 uvTransform = srcRegionToImage;
		uvTransform = mg_mat2x3_mul_m(uvTransform, destRegionToSrcRegion);
		uvTransform = mg_mat2x3_mul_m(uvTransform, userToDestRegion);
		uvTransform = mg_mat2x3_mul_m(uvTransform, screenToUser);

		int index = backend->nextShapeIndex-1;
		mg_vertex_layout* layout = &backend->vertexLayout;
		*(mg_mat2x3*)(layout->uvTransformBuffer + index*layout->uvTransformStride) = uvTransform;

		//TODO: transform extents before clipping
		mp_rect clip = {maximum(backend->clip.x, backend->shapeScreenExtents.x),
		                maximum(backend->clip.y, backend->shapeScreenExtents.y),
		                minimum(backend->clip.x + backend->clip.w, backend->shapeScreenExtents.z),
		                minimum(backend->clip.y + backend->clip.h, backend->shapeScreenExtents.w)};

		*(mp_rect*)(((char*)layout->clipBuffer) + index*layout->clipStride) = clip;
	}
}

u32 mg_next_shape(mg_gl_canvas_backend* backend, mg_attributes* attributes)
{
	mg_finalize_shape(backend);

	backend->clip = attributes->clip;
	backend->transform = attributes->transform;
	backend->srcRegion = attributes->srcRegion;
	backend->shapeExtents = (vec4){FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};
	backend->shapeScreenExtents = (vec4){FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};

	mg_vertex_layout* layout = &backend->vertexLayout;
	int index = backend->nextShapeIndex;
	backend->nextShapeIndex++;

	*(mg_color*)(((char*)layout->colorBuffer) + index*layout->colorStride) = attributes->color;

	return(index);
}

//TODO(martin): rename with something more explicit
u32 mg_vertices_base_index(mg_gl_canvas_backend* backend)
{
	return(backend->vertexCount);
}

int* mg_reserve_indices(mg_gl_canvas_backend* backend, u32 indexCount)
{
	mg_vertex_layout* layout = &backend->vertexLayout;

	//TODO: do something here...
	ASSERT(backend->indexCount + indexCount < layout->maxIndexCount);

	int* base = ((int*)layout->indexBuffer) + backend->indexCount;
	backend->indexCount += indexCount;
	return(base);
}

void mg_push_vertex_cubic(mg_gl_canvas_backend* backend, vec2 pos, vec4 cubic)
{
	backend->shapeExtents.x = minimum(backend->shapeExtents.x, pos.x);
	backend->shapeExtents.y = minimum(backend->shapeExtents.y, pos.y);
	backend->shapeExtents.z = maximum(backend->shapeExtents.z, pos.x);
	backend->shapeExtents.w = maximum(backend->shapeExtents.w, pos.y);

	vec2 screenPos = mg_mat2x3_mul(backend->transform, pos);

	backend->shapeScreenExtents.x = minimum(backend->shapeScreenExtents.x, screenPos.x);
	backend->shapeScreenExtents.y = minimum(backend->shapeScreenExtents.y, screenPos.y);
	backend->shapeScreenExtents.z = maximum(backend->shapeScreenExtents.z, screenPos.x);
	backend->shapeScreenExtents.w = maximum(backend->shapeScreenExtents.w, screenPos.y);

	mg_vertex_layout* layout = &backend->vertexLayout;
	ASSERT(backend->vertexCount < layout->maxVertexCount);
	ASSERT(backend->nextShapeIndex > 0);

	int shapeIndex = maximum(0, backend->nextShapeIndex-1);
	u32 index = backend->vertexCount;
	backend->vertexCount++;

	*(vec2*)(((char*)layout->posBuffer) + index*layout->posStride) = screenPos;
	*(vec4*)(((char*)layout->cubicBuffer) + index*layout->cubicStride) = cubic;
	*(u32*)(((char*)layout->shapeIndexBuffer) + index*layout->shapeIndexStride) = shapeIndex;
}

void mg_push_vertex(mg_gl_canvas_backend* backend, vec2 pos)
{
	mg_push_vertex_cubic(backend, pos, (vec4){1, 1, 1, 1});
}
//-----------------------------------------------------------------------------------------------------------
// Path Filling
//-----------------------------------------------------------------------------------------------------------
//NOTE(martin): forward declarations
void mg_render_fill_cubic(mg_gl_canvas_backend* backend, vec2 p[4]);

//NOTE(martin): quadratics filling

void mg_render_fill_quadratic(mg_gl_canvas_backend* backend, vec2 p[3])
{
	u32 baseIndex = mg_vertices_base_index(backend);

	i32* indices = mg_reserve_indices(backend, 3);

	mg_push_vertex_cubic(backend, (vec2){p[0].x, p[0].y}, (vec4){0, 0, 0, 1});
	mg_push_vertex_cubic(backend, (vec2){p[1].x, p[1].y}, (vec4){0.5, 0, 0.5, 1});
	mg_push_vertex_cubic(backend, (vec2){p[2].x, p[2].y}, (vec4){1, 1, 1, 1});

	indices[0] = baseIndex + 0;
	indices[1] = baseIndex + 1;
	indices[2] = baseIndex + 2;
}

//NOTE(martin): cubic filling

void mg_split_and_fill_cubic(mg_gl_canvas_backend* backend, vec2 p[4], f32 tSplit)
{
	int subVertexCount = 0;
	int subIndexCount = 0;

	f32 OneMinusTSplit = 1-tSplit;

	vec2 q0 = {OneMinusTSplit*p[0].x + tSplit*p[1].x,
		   OneMinusTSplit*p[0].y + tSplit*p[1].y};

	vec2 q1 = {OneMinusTSplit*p[1].x + tSplit*p[2].x,
		   OneMinusTSplit*p[1].y + tSplit*p[2].y};

	vec2 q2 = {OneMinusTSplit*p[2].x + tSplit*p[3].x,
		   OneMinusTSplit*p[2].y + tSplit*p[3].y};

	vec2 r0 = {OneMinusTSplit*q0.x + tSplit*q1.x,
		   OneMinusTSplit*q0.y + tSplit*q1.y};

	vec2 r1 = {OneMinusTSplit*q1.x + tSplit*q2.x,
		   OneMinusTSplit*q1.y + tSplit*q2.y};

	vec2 split = {OneMinusTSplit*r0.x + tSplit*r1.x,
		     OneMinusTSplit*r0.y + tSplit*r1.y};;

	vec2 subPointsLow[4] = {p[0], q0, r0, split};
	vec2 subPointsHigh[4] = {split, r1, q2, p[3]};

	//NOTE(martin): add base triangle
	u32 baseIndex = mg_vertices_base_index(backend);
	i32* indices = mg_reserve_indices(backend, 3);

	mg_push_vertex(backend, (vec2){p[0].x, p[0].y});
	mg_push_vertex(backend, (vec2){split.x, split.y});
	mg_push_vertex(backend, (vec2){p[3].x, p[3].y});

	indices[0] = baseIndex + 0;
	indices[1] = baseIndex + 1;
	indices[2] = baseIndex + 2;

	mg_render_fill_cubic(backend, subPointsLow);
	mg_render_fill_cubic(backend, subPointsHigh);
}

int mg_cubic_outside_test(vec4 c)
{
	int res = (c.x*c.x*c.x - c.y*c.z < 0) ? -1 : 1;
	return(res);
}

void mg_render_fill_cubic(mg_gl_canvas_backend* backend, vec2 p[4])
{
	vec4 testCoords[4];

	/*NOTE(martin): first convert the control points to power basis, multiplying by M3

		     | 1  0  0  0|
		M3 = |-3  3  0  0|
		     | 3 -6  3  0|
		     |-1  3 -3  1|
		ie:
		    c0 = p0
		    c1 = -3*p0 + 3*p1
		    c2 = 3*p0 - 6*p1 + 3*p2
		    c3 = -p0 + 3*p1 - 3*p2 + p3
	*/
	f32 c1x = 3.0*p[1].x - 3.0*p[0].x;
	f32 c1y = 3.0*p[1].y - 3.0*p[0].y;

	f32 c2x = 3.0*p[0].x + 3.0*p[2].x - 6.0*p[1].x;
	f32 c2y = 3.0*p[0].y + 3.0*p[2].y - 6.0*p[1].y;

	f32 c3x = 3.0*p[1].x - 3.0*p[2].x + p[3].x - p[0].x;
	f32 c3y = 3.0*p[1].y - 3.0*p[2].y + p[3].y - p[0].y;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//TODO(martin): we shouldn't need scaling here since now we're doing our shader math in fixed point?
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	c1x /= 10;
	c1y /= 10;
	c2x /= 10;
	c2y /= 10;
	c3x /= 10;
	c3y /= 10;

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
	*/

	f32 d1 = c3y*c2x - c3x*c2y;
	f32 d2 = c3x*c1y - c3y*c1x;
	f32 d3 = c2y*c1x - c2x*c1y;

	//NOTE(martin): compute the second factor of the discriminant discr(I) = d1^2*(3*d2^2 - 4*d3*d1)
	f32 discrFactor2 = 3.0*Square(d2) - 4.0*d3*d1;

	//NOTE(martin): each following case gives the number of roots, hence the category of the parametric curve
	if(fabs(d1) < 0.1 && fabs(d2) < 0.1 && d3 != 0)
	{
		//NOTE(martin): quadratic degenerate case
		//NOTE(martin): compute quadratic curve control point, which is at p0 + 1.5*(p1-p0) = 1.5*p1 - 0.5*p0
		vec2 quadControlPoints[3] = { p[0],
		                             {1.5*p[1].x - 0.5*p[0].x, 1.5*p[1].y - 0.5*p[0].y},
				  	     p[3]};

		mg_render_fill_quadratic(backend, quadControlPoints);
		return;
	}
	else if( (discrFactor2 > 0 && d1 != 0)
	  ||(discrFactor2 == 0 && d1 != 0))
	{
		//NOTE(martin): serpentine curve or cusp with inflection at infinity
		//              (these two cases are handled the same way).
		//NOTE(martin): compute the solutions (tl, sl), (tm, sm), and (tn, sn) of the inflection point equation
		f32 tl = d2 + sqrt(discrFactor2/3);
		f32 sl = 2*d1;
		f32 tm = d2 - sqrt(discrFactor2/3);
		f32 sm = sl;

		/*NOTE(martin):
			the power basis coefficients of points k,l,m,n are collected into the rows of the 4x4 matrix F:

				| tl*tm            tl^3        tm^3        1 |
				| -sm*tl - sl*tm   -3sl*tl^2   -3*sm*tm^2  0 |
				| sl*sm            3*sl^2*tl   3*sm^2*tm   0 |
				| 0                -sl^3       -sm^3       0 |

			This matrix is then multiplied by M3^(-1) on the left which yelds the bezier coefficients of k, l, m, n
			which are assigned as a 4D image coordinates to control points.


			                   | 1  0   0   0 |
				M3^(-1) =  | 1  1/3 0   0 |
				           | 1  2/3 1/3 0 |
					   | 1  1   1   1 |
		*/
		testCoords[0].x = tl*tm;
		testCoords[0].y = Cube(tl);
		testCoords[0].z = Cube(tm);

		testCoords[1].x = tl*tm - (sm*tl + sl*tm)/3;
		testCoords[1].y = Cube(tl) - sl*Square(tl);
		testCoords[1].z = Cube(tm) - sm*Square(tm);

		testCoords[2].x = tl*tm - (sm*tl + sl*tm)*2/3 + sl*sm/3;
		testCoords[2].y = Cube(tl) - 2*sl*Square(tl) + Square(sl)*tl;
		testCoords[2].z = Cube(tm) - 2*sm*Square(tm) + Square(sm)*tm;

		testCoords[3].x = tl*tm - (sm*tl + sl*tm) + sl*sm;
		testCoords[3].y = Cube(tl) - 3*sl*Square(tl) + 3*Square(sl)*tl - Cube(sl);
		testCoords[3].z = Cube(tm) - 3*sm*Square(tm) + 3*Square(sm)*tm - Cube(sm);
	}
	else if(discrFactor2 < 0 && d1 != 0)
	{
		//NOTE(martin): loop curve
		f32 td = d2 + sqrt(-discrFactor2);
		f32 sd = 2*d1;
		f32 te = d2 - sqrt(-discrFactor2);
		f32 se = sd;

		//NOTE(martin): if one of the parameters (td/sd) or (te/se) is in the interval [0,1], the double point
		//              is inside the control points convex hull and would cause a shading anomaly. If this is
		//              the case, subdivide the curve at that point

		//TODO: study edge case where td/sd ~ 1 or 0 (which causes an infinite recursion in split and fill).
		//      quick fix for now is adding a little slop in the check...

		if(sd != 0 && td/sd < 0.99 && td/sd > 0.01)
		{
			mg_split_and_fill_cubic(backend, p, td/sd);
			return;
		}
		if(se != 0 && te/se < 0.99 && te/se > 0.01)
		{
			mg_split_and_fill_cubic(backend, p, te/se);
			return;
		}

		/*NOTE(martin):
			the power basis coefficients of points k,l,m,n are collected into the rows of the 4x4 matrix F:

				| td*te            td^2*te                 td*te^2                1 |
				| -se*td - sd*te   -se*td^2 - 2sd*te*td    -sd*te^2 - 2*se*td*te  0 |
				| sd*se            te*sd^2 + 2*se*td*sd    td*se^2 + 2*sd*te*se   0 |
				| 0                -sd^2*se                -sd*se^2               0 |

			This matrix is then multiplied by M3^(-1) on the left which yelds the bezier coefficients of k, l, m, n
			which are assigned as a 4D image coordinates to control points.


			                   | 1  0   0   0 |
				M3^(-1) =  | 1  1/3 0   0 |
				           | 1  2/3 1/3 0 |
					   | 1  1   1   1 |
		*/
		testCoords[0].x = td*te;
		testCoords[0].y = Square(td)*te;
		testCoords[0].z = td*Square(te);

		testCoords[1].x = td*te - (se*td + sd*te)/3.0;
		testCoords[1].y = Square(td)*te - (se*Square(td) + 2.*sd*te*td)/3.0;
		testCoords[1].z = td*Square(te) - (sd*Square(te) + 2*se*td*te)/3.0;

		testCoords[2].x = td*te - 2.0*(se*td + sd*te)/3.0 + sd*se/3.0;
		testCoords[2].y = Square(td)*te - 2.0*(se*Square(td) + 2.0*sd*te*td)/3.0 + (te*Square(sd) + 2.0*se*td*sd)/3.0;
		testCoords[2].z = td*Square(te) - 2.0*(sd*Square(te) + 2.0*se*td*te)/3.0 + (td*Square(se) + 2.0*sd*te*se)/3.0;

		testCoords[3].x = td*te - (se*td + sd*te) + sd*se;
		testCoords[3].y = Square(td)*te - (se*Square(td) + 2.0*sd*te*td) + (te*Square(sd) + 2.0*se*td*sd) - Square(sd)*se;
		testCoords[3].z = td*Square(te) - (sd*Square(te) + 2.0*se*td*te) + (td*Square(se) + 2.0*sd*te*se) - sd*Square(se);
	}
	else if(d1 == 0 && d2 != 0)
	{
		//NOTE(martin): cusp with cusp at infinity

		f32 tl = d3;
		f32 sl = 3*d2;

		/*NOTE(martin):
			the power basis coefficients of points k,l,m,n are collected into the rows of the 4x4 matrix F:

				| tl    tl^3        1  1 |
				| -sl   -3sl*tl^2   0  0 |
				| 0     3*sl^2*tl   0  0 |
				| 0     -sl^3       0  0 |

			This matrix is then multiplied by M3^(-1) on the left which yelds the bezier coefficients of k, l, m, n
			which are assigned as a 4D image coordinates to control points.


			                   | 1  0   0   0 |
				M3^(-1) =  | 1  1/3 0   0 |
				           | 1  2/3 1/3 0 |
					   | 1  1   1   1 |
		*/

		testCoords[0].x = tl;
		testCoords[0].y = Cube(tl);
		testCoords[0].z = 1;

		testCoords[1].x = tl - sl/3;
		testCoords[1].y = Cube(tl) - sl*Square(tl);
		testCoords[1].z = 1;

		testCoords[2].x = tl - sl*2/3;
		testCoords[2].y = Cube(tl) - 2*sl*Square(tl) + Square(sl)*tl;
		testCoords[2].z = 1;

		testCoords[3].x = tl - sl;
		testCoords[3].y = Cube(tl) - 3*sl*Square(tl) + 3*Square(sl)*tl - Cube(sl);
		testCoords[3].z = 1;
	}
	else if(d1 == 0 && d2 == 0 && d3 == 0)
	{
		//NOTE(martin): line or point degenerate case, ignored
		return;
	}
	else
	{
		//TODO(martin): handle error ? put some epsilon slack on the conditions ?
		ASSERT(0, "not implemented yet !");
		return;
	}

	//NOTE(martin): compute convex hull indices using Gift wrapping / Jarvis' march algorithm
	int convexHullIndices[4];
	int leftMostPointIndex = 0;

	for(int i=0; i<4; i++)
	{
		if(p[i].x < p[leftMostPointIndex].x)
		{
			leftMostPointIndex = i;
		}
	}
	int currentPointIndex = leftMostPointIndex;
	int i=0;
	int convexHullCount = 0;

	do
	{
		convexHullIndices[i] = currentPointIndex;
		convexHullCount++;
		int bestGuessIndex = 0;

		for(int j=0; j<4; j++)
		{
			vec2 bestGuessEdge = {.x = p[bestGuessIndex].x - p[currentPointIndex].x,
		                              .y = p[bestGuessIndex].y - p[currentPointIndex].y};

			vec2 nextGuessEdge = {.x = p[j].x - p[currentPointIndex].x,
		                              .y = p[j].y - p[currentPointIndex].y};

			//NOTE(martin): if control point j is on the right of current edge, it is a best guess
			//              (in case of colinearity we choose the point which is farthest from the current point)

			f32 crossProduct = bestGuessEdge.x*nextGuessEdge.y - bestGuessEdge.y*nextGuessEdge.x;

			if(  bestGuessIndex == currentPointIndex
			  || crossProduct < 0)
			{
				bestGuessIndex = j;
			}
			else if(crossProduct == 0)
			{

				//NOTE(martin): if vectors v1, v2 are colinear and distinct, and ||v1|| > ||v2||,
				//               either abs(v1.x) > abs(v2.x) or abs(v1.y) > abs(v2.y)
				//               so we don't actually need to compute their norm to select the greatest
				//               (and if v1 and v2 are equal we don't have to update our best guess.)

				//TODO(martin): in case of colinearity we should rather select the edge that has the greatest dot product with last edge ??

				if(fabs(nextGuessEdge.x) > fabs(bestGuessEdge.x)
				  || fabs(nextGuessEdge.y) > fabs(bestGuessEdge.y))
				{
					bestGuessIndex = j;
				}
			}
		}
		i++;
		currentPointIndex = bestGuessIndex;

	} while(currentPointIndex != leftMostPointIndex && i<4);

	//NOTE(martin): triangulation and inside/outside tests. In the shader, the outside is defined by s*(k^3 - lm) > 0
	//              ie the 4th coordinate s flips the inside/outside test.
	//              We affect s such that the covered are is between the curve and the line joining p0 and p3.

	//TODO: quick fix, maybe later cull degenerate hulls beforehand
	if(convexHullCount <= 2)
	{
		//NOTE(martin): if convex hull has only two point, we have a degenerate cubic that displays nothing.
		return;
	}
	else if(convexHullCount == 3)
	{
		/*NOTE(martin):
			We have 3 case here:
				1) Endpoints are coincidents. We push on triangle, and test an intermediate point for orientation.
				2) The point not on the hull is an endpoint. We push two triangle (p0, p3, p1) and (p0, p3, p2). We test the intermediate
				points to know if we must flip the orientation of the curve.
				3) The point not on the hull is an intermediate point: we emit one triangle. We test the intermediate point on the hull
				to know if we must flip the orientation of the curve.
		*/
		if(  p[0].x == p[3].x
		  && p[0].y == p[3].y)
		{
			//NOTE: case 1: endpoints are coincidents
			int outsideTest = mg_cubic_outside_test(testCoords[1]);

			//NOTE: push triangle
			u32 baseIndex = mg_vertices_base_index(backend);
			i32* indices = mg_reserve_indices(backend, 3);

			mg_push_vertex_cubic(backend, p[0], (vec4){vec4_expand_xyz(testCoords[0]), outsideTest});
			mg_push_vertex_cubic(backend, p[1], (vec4){vec4_expand_xyz(testCoords[1]), outsideTest});
			mg_push_vertex_cubic(backend, p[2], (vec4){vec4_expand_xyz(testCoords[2]), outsideTest});

			for(int i=0; i<3; i++)
			{
				indices[i] = baseIndex + i;
			}
		}
		else
		{
			//NOTE: find point not on the hull
			int insidePointIndex = -1;
			{
				bool present[4] = {0};
				for(int i=0; i<3; i++)
				{
					present[convexHullIndices[i]] = true;
				}
				for(int i=0; i<4; i++)
				{
					if(!present[i])
					{
						insidePointIndex = i;
						break;
					}
				}
			}
			DEBUG_ASSERT(insidePointIndex >= 0 && insidePointIndex < 4);

			if(insidePointIndex == 0 || insidePointIndex == 3)
			{
				//NOTE: case 2: the point inside the hull is an endpoint

				int outsideTest0 = mg_cubic_outside_test(testCoords[1]);
				int outsideTest1 = mg_cubic_outside_test(testCoords[2]);

				//NOTE: push triangles
				u32 baseIndex = mg_vertices_base_index(backend);
				i32* indices = mg_reserve_indices(backend, 6);

				mg_push_vertex_cubic(backend, p[0], (vec4){vec4_expand_xyz(testCoords[0]), outsideTest0});
				mg_push_vertex_cubic(backend, p[3], (vec4){vec4_expand_xyz(testCoords[3]), outsideTest0});
				mg_push_vertex_cubic(backend, p[1], (vec4){vec4_expand_xyz(testCoords[1]), outsideTest0});
				mg_push_vertex_cubic(backend, p[0], (vec4){vec4_expand_xyz(testCoords[0]), outsideTest1});
				mg_push_vertex_cubic(backend, p[3], (vec4){vec4_expand_xyz(testCoords[3]), outsideTest1});
				mg_push_vertex_cubic(backend, p[2], (vec4){vec4_expand_xyz(testCoords[2]), outsideTest1});

				for(int i=0; i<6; i++)
				{
					indices[i] = baseIndex + i;
				}
			}
			else
			{
				int testIndex = (insidePointIndex == 1) ? 2 : 1;
				int outsideTest = mg_cubic_outside_test(testCoords[testIndex]);

				//NOTE: push triangle
				u32 baseIndex = mg_vertices_base_index(backend);
				i32* indices = mg_reserve_indices(backend, 3);

				for(int i=0; i<3; i++)
				{
					mg_push_vertex_cubic(backend,
					                     p[convexHullIndices[i]],
					                     (vec4){vec4_expand_xyz(testCoords[convexHullIndices[i]]),
					                     outsideTest});
				}

				for(int i=0; i<3; i++)
				{
					indices[i] = baseIndex + i;
				}
			}
		}
	}
	else
	{
		DEBUG_ASSERT(convexHullCount == 4);
		/*NOTE(martin):
			We build a fan from the hull, starting from an endpoint. For each triangle, we test the vertex that is an intermediate
			control point for orientation
		*/
		int endPointIndex = -1;
		for(int i=0; i<4; i++)
		{
			if(convexHullIndices[i] == 0 || convexHullIndices[i] == 3)
			{
				endPointIndex = i;
				break;
			}
		}
		ASSERT(endPointIndex >= 0);

		int fanIndices[6] = {convexHullIndices[endPointIndex],
		                     convexHullIndices[(endPointIndex + 1)%4],
		                     convexHullIndices[(endPointIndex + 2)%4],
		                     convexHullIndices[endPointIndex],
		                     convexHullIndices[(endPointIndex + 2)%4],
		                     convexHullIndices[(endPointIndex + 3)%4]};

		//NOTE: fan indices on the hull are (0,1,2)(0,2,3). So if the 3rd vertex of the hull is an intermediate point it works
		//      as a test vertex for both triangles. Otherwise, the test vertices on the fan are 1 and 5.
		int outsideTest0 = 1;
		int outsideTest1 = 1;

		if( fanIndices[2] == 1
		  ||fanIndices[2] == 2)
		{
			outsideTest0 = outsideTest1 = mg_cubic_outside_test(testCoords[fanIndices[2]]);
		}
		else
		{
			DEBUG_ASSERT(fanIndices[1] == 1 || fanIndices[1] == 2);
			DEBUG_ASSERT(fanIndices[5] == 1 || fanIndices[5] == 2);

			outsideTest0 = mg_cubic_outside_test(testCoords[fanIndices[1]]);
			outsideTest1 = mg_cubic_outside_test(testCoords[fanIndices[5]]);
		}

		//NOTE: push triangles
		u32 baseIndex = mg_vertices_base_index(backend);
		i32* indices = mg_reserve_indices(backend, 6);

		for(int i=0; i<3; i++)
		{
			mg_push_vertex_cubic(backend, p[fanIndices[i]], (vec4){vec4_expand_xyz(testCoords[fanIndices[i]]), outsideTest0});
		}
		for(int i=0; i<3; i++)
		{
			mg_push_vertex_cubic(backend, p[fanIndices[i+3]], (vec4){vec4_expand_xyz(testCoords[fanIndices[i+3]]), outsideTest1});
		}

		for(int i=0; i<6; i++)
		{
			indices[i] = baseIndex + i;
		}
	}
}

//NOTE(martin): global path fill

void mg_render_fill(mg_gl_canvas_backend* backend, mg_path_elt* elements, mg_path_descriptor* path)
{
	u32 eltCount = path->count;
	vec2 startPoint = path->startPoint;
	vec2 endPoint = path->startPoint;
	vec2 currentPoint = path->startPoint;

	for(int eltIndex=0; eltIndex<eltCount; eltIndex++)
	{
		mg_path_elt* elt = &(elements[eltIndex]);

		vec2 controlPoints[4] = {currentPoint, elt->p[0], elt->p[1], elt->p[2]};

		switch(elt->type)
		{
			case MG_PATH_MOVE:
			{
				startPoint = elt->p[0];
				endPoint = elt->p[0];
				currentPoint = endPoint;
				continue;
			} break;

			case MG_PATH_LINE:
			{
				endPoint = controlPoints[1];
			} break;

			case MG_PATH_QUADRATIC:
			{
				mg_render_fill_quadratic(backend, controlPoints);
				endPoint = controlPoints[2];

			} break;

			case MG_PATH_CUBIC:
			{
				mg_render_fill_cubic(backend, controlPoints);
				endPoint = controlPoints[3];
			} break;
		}

		//NOTE(martin): now fill interior triangle
		u32 baseIndex = mg_vertices_base_index(backend);
		int* indices = mg_reserve_indices(backend, 3);

		mg_push_vertex(backend, startPoint);
		mg_push_vertex(backend, currentPoint);
		mg_push_vertex(backend, endPoint);

		indices[0] = baseIndex;
		indices[1] = baseIndex + 1;
		indices[2] = baseIndex + 2;

		currentPoint = endPoint;
	}
}

//-----------------------------------------------------------------------------------------------------------
// Path Stroking
//-----------------------------------------------------------------------------------------------------------

void mg_render_stroke_line(mg_gl_canvas_backend* backend, vec2 p[2], mg_attributes* attributes)
{
	//NOTE(martin): get normals multiplied by halfWidth
	f32 halfW = attributes->width/2;

	vec2 n0 = {p[0].y - p[1].y,
		   p[1].x - p[0].x};
	f32 norm0 = sqrt(n0.x*n0.x + n0.y*n0.y);
	n0.x *= halfW/norm0;
	n0.y *= halfW/norm0;

	u32 baseIndex = mg_vertices_base_index(backend);
	i32* indices = mg_reserve_indices(backend, 6);

	mg_push_vertex(backend, (vec2){p[0].x + n0.x, p[0].y + n0.y});
	mg_push_vertex(backend, (vec2){p[1].x + n0.x, p[1].y + n0.y});
	mg_push_vertex(backend, (vec2){p[1].x - n0.x, p[1].y - n0.y});
	mg_push_vertex(backend, (vec2){p[0].x - n0.x, p[0].y - n0.y});

	indices[0] = baseIndex;
	indices[1] = baseIndex + 1;
	indices[2] = baseIndex + 2;
	indices[3] = baseIndex;
	indices[4] = baseIndex + 2;
	indices[5] = baseIndex + 3;
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


void mg_render_stroke_quadratic(mg_gl_canvas_backend* backend, vec2 p[3], mg_attributes* attributes)
{
	//NOTE: check for degenerate line case
	const f32 equalEps = 1e-3;
	if(vec2_close(p[0], p[1], equalEps))
	{
		mg_render_stroke_line(backend, p+1, attributes);
		return;
	}
	else if(vec2_close(p[1], p[2], equalEps))
	{
		mg_render_stroke_line(backend, p, attributes);
		return;
	}

	#define CHECK_SAMPLE_COUNT 5
	f32 checkSamples[CHECK_SAMPLE_COUNT] = {1./6, 2./6, 3./6, 4./6, 5./6};

	vec2 positiveOffsetHull[3];
	vec2 negativeOffsetHull[3];

	if( !mg_offset_hull(3, p, positiveOffsetHull, 0.5 * attributes->width)
	  ||!mg_offset_hull(3, p, negativeOffsetHull, -0.5 * attributes->width))
	{
		//NOTE: offsetting the hull failed, split the curve
		vec2 splitLeft[3];
		vec2 splitRight[3];
		mg_quadratic_split(p, 0.5, splitLeft, splitRight);
		mg_render_stroke_quadratic(backend, splitLeft, attributes);
		mg_render_stroke_quadratic(backend, splitRight, attributes);
	}
	else
	{
		//NOTE(martin): the distance d between the offset curve and the path must be between w/2-tolerance and w/2+tolerance
		//              thus, by constraining tolerance to be at most, 0.5*width, we can rewrite this condition like this:
		//
		//              (w/2-tolerance)^2 < d^2 < (w/2+tolerance)^2
		//
		//		we compute the maximum overshoot outside these bounds and split the curve at the corresponding parameter

		//TODO: maybe refactor by using tolerance in the _check_, not in the computation of the overshoot
		f32 tolerance = minimum(attributes->tolerance, 0.5 * attributes->width);
		f32 d2LowBound = Square(0.5 * attributes->width - attributes->tolerance);
		f32 d2HighBound = Square(0.5 * attributes->width + attributes->tolerance);

		f32 maxOvershoot = 0;
		f32 maxOvershootParameter = 0;

		for(int i=0; i<CHECK_SAMPLE_COUNT; i++)
		{
			f32 t = checkSamples[i];

			vec2 c = mg_quadratic_get_point(p, t);
			vec2 cp =  mg_quadratic_get_point(positiveOffsetHull, t);
			vec2 cn =  mg_quadratic_get_point(negativeOffsetHull, t);

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
			mg_render_stroke_quadratic(backend, splitLeft, attributes);
			mg_render_stroke_quadratic(backend, splitRight, attributes);
		}
		else
		{
			//NOTE(martin): push the actual fill commands for the offset contour

			mg_next_shape(backend, attributes);

			mg_render_fill_quadratic(backend, positiveOffsetHull);
			mg_render_fill_quadratic(backend, negativeOffsetHull);

			//NOTE(martin):	add base triangles
			u32 baseIndex = mg_vertices_base_index(backend);
			i32* indices = mg_reserve_indices(backend, 6);

			mg_push_vertex(backend, positiveOffsetHull[0]);
			mg_push_vertex(backend, positiveOffsetHull[2]);
			mg_push_vertex(backend, negativeOffsetHull[2]);
			mg_push_vertex(backend, negativeOffsetHull[0]);

			indices[0] = baseIndex + 0;
			indices[1] = baseIndex + 1;
			indices[2] = baseIndex + 2;
			indices[3] = baseIndex + 0;
			indices[4] = baseIndex + 2;
			indices[5] = baseIndex + 3;
		}
	}
	#undef CHECK_SAMPLE_COUNT
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

void mg_render_stroke_cubic(mg_gl_canvas_backend* backend, vec2 p[4], mg_attributes* attributes)
{
	//NOTE: check degenerate line cases
	f32 equalEps = 1e-3;

	if( (vec2_close(p[0], p[1], equalEps) && vec2_close(p[2], p[3], equalEps))
	  ||(vec2_close(p[0], p[1], equalEps) && vec2_close(p[1], p[2], equalEps))
	  ||(vec2_close(p[1], p[2], equalEps) && vec2_close(p[2], p[3], equalEps)))
	{
		vec2 line[2] = {p[0], p[3]};
		mg_render_stroke_line(backend, line, attributes);
		return;
	}
	else if(vec2_close(p[0], p[1], equalEps) && vec2_close(p[1], p[3], equalEps))
	{
		vec2 line[2] = {p[0], vec2_add(vec2_mul(5./9, p[0]), vec2_mul(4./9, p[2]))};
		mg_render_stroke_line(backend, line, attributes);
		return;
	}
	else if(vec2_close(p[0], p[2], equalEps) && vec2_close(p[2], p[3], equalEps))
	{
		vec2 line[2] = {p[0], vec2_add(vec2_mul(5./9, p[0]), vec2_mul(4./9, p[1]))};
		mg_render_stroke_line(backend, line, attributes);
		return;
	}

	#define CHECK_SAMPLE_COUNT 5
	f32 checkSamples[CHECK_SAMPLE_COUNT] = {1./6, 2./6, 3./6, 4./6, 5./6};

	vec2 positiveOffsetHull[4];
	vec2 negativeOffsetHull[4];

	if(  !mg_offset_hull(4, p, positiveOffsetHull, 0.5 * attributes->width)
	  || !mg_offset_hull(4, p, negativeOffsetHull, -0.5 * attributes->width))
	{
		vec2 splitLeft[4];
		vec2 splitRight[4];
		mg_cubic_split(p, 0.5, splitLeft, splitRight);
		mg_render_stroke_cubic(backend, splitLeft, attributes);
		mg_render_stroke_cubic(backend, splitRight, attributes);
		return;
	}

	//NOTE(martin): the distance d between the offset curve and the path must be between w/2-tolerance and w/2+tolerance
	//              thus, by constraining tolerance to be at most, 0.5*width, we can rewrite this condition like this:
	//
	//              (w/2-tolerance)^2 < d^2 < (w/2+tolerance)^2
	//
	//		we compute the maximum overshoot outside these bounds and split the curve at the corresponding parameter

	//TODO: maybe refactor by using tolerance in the _check_, not in the computation of the overshoot
	f32 tolerance = minimum(attributes->tolerance, 0.5 * attributes->width);
	f32 d2LowBound = Square(0.5 * attributes->width - attributes->tolerance);
	f32 d2HighBound = Square(0.5 * attributes->width + attributes->tolerance);

	f32 maxOvershoot = 0;
	f32 maxOvershootParameter = 0;

	for(int i=0; i<CHECK_SAMPLE_COUNT; i++)
	{
		f32 t = checkSamples[i];

		vec2 c = mg_cubic_get_point(p, t);
		vec2 cp =  mg_cubic_get_point(positiveOffsetHull, t);
		vec2 cn =  mg_cubic_get_point(negativeOffsetHull, t);

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
		mg_render_stroke_cubic(backend, splitLeft, attributes);
		mg_render_stroke_cubic(backend, splitRight, attributes);

		//TODO: render joint between the split curves
	}
	else
	{
		//NOTE(martin): push the actual fill commands for the offset contour
		mg_next_shape(backend, attributes);

		mg_render_fill_cubic(backend, positiveOffsetHull);
		mg_render_fill_cubic(backend, negativeOffsetHull);

		//NOTE(martin):	add base triangles
		u32 baseIndex = mg_vertices_base_index(backend);
		i32* indices = mg_reserve_indices(backend, 6);

		mg_push_vertex(backend, positiveOffsetHull[0]);
		mg_push_vertex(backend, positiveOffsetHull[3]);
		mg_push_vertex(backend, negativeOffsetHull[3]);
		mg_push_vertex(backend, negativeOffsetHull[0]);

		indices[0] = baseIndex + 0;
		indices[1] = baseIndex + 1;
		indices[2] = baseIndex + 2;
		indices[3] = baseIndex + 0;
		indices[4] = baseIndex + 2;
		indices[5] = baseIndex + 3;
	}
	#undef CHECK_SAMPLE_COUNT
}

void mg_stroke_cap(mg_gl_canvas_backend* backend, vec2 p0, vec2 direction, mg_attributes* attributes)
{
	//NOTE(martin): compute the tangent and normal vectors (multiplied by half width) at the cap point

	f32 dn = sqrt(Square(direction.x) + Square(direction.y));
	f32 alpha = 0.5 * attributes->width/dn;

	vec2 n0 = {-alpha*direction.y,
		    alpha*direction.x};

	vec2 m0 = {alpha*direction.x,
	           alpha*direction.y};

	mg_next_shape(backend, attributes);

	u32 baseIndex = mg_vertices_base_index(backend);
	i32* indices = mg_reserve_indices(backend, 6);

	mg_push_vertex(backend, (vec2){p0.x + n0.x, p0.y + n0.y});
	mg_push_vertex(backend, (vec2){p0.x + n0.x + m0.x, p0.y + n0.y + m0.y});
	mg_push_vertex(backend, (vec2){p0.x - n0.x + m0.x, p0.y - n0.y + m0.y});
	mg_push_vertex(backend, (vec2){p0.x - n0.x, p0.y - n0.y});

	indices[0] = baseIndex;
	indices[1] = baseIndex + 1;
	indices[2] = baseIndex + 2;
	indices[3] = baseIndex;
	indices[4] = baseIndex + 2;
	indices[5] = baseIndex + 3;
}

void mg_stroke_joint(mg_gl_canvas_backend* backend,
                              vec2 p0,
			      vec2 t0,
			      vec2 t1,
			      mg_attributes* attributes)
{
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

	mg_next_shape(backend, attributes);

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
		vec2 mitterPoint = {p0.x + v.x, p0.y + v.y};

		u32 baseIndex = mg_vertices_base_index(backend);
		i32* indices = mg_reserve_indices(backend, 6);

		mg_push_vertex(backend, p0);
		mg_push_vertex(backend, (vec2){p0.x + n0.x*halfW, p0.y + n0.y*halfW});
		mg_push_vertex(backend, mitterPoint);
		mg_push_vertex(backend, (vec2){p0.x + n1.x*halfW, p0.y + n1.y*halfW});

		indices[0] = baseIndex;
		indices[1] = baseIndex + 1;
		indices[2] = baseIndex + 2;
		indices[3] = baseIndex;
		indices[4] = baseIndex + 2;
		indices[5] = baseIndex + 3;
	}
	else
	{
		//NOTE(martin): add a bevel joint
		u32 baseIndex = mg_vertices_base_index(backend);
		i32* indices = mg_reserve_indices(backend, 3);

		mg_push_vertex(backend, p0);
		mg_push_vertex(backend, (vec2){p0.x + n0.x*halfW, p0.y + n0.y*halfW});
		mg_push_vertex(backend, (vec2){p0.x + n1.x*halfW, p0.y + n1.y*halfW});

		DEBUG_ASSERT(!isnan(n0.x) && !isnan(n0.y) && !isnan(n1.x) && !isnan(n1.y));

		indices[0] = baseIndex;
		indices[1] = baseIndex + 1;
		indices[2] = baseIndex + 2;
	}
}

void mg_render_stroke_element(mg_gl_canvas_backend* backend,
                                      mg_path_elt* element,
				      mg_attributes* attributes,
				      vec2 currentPoint,
				      vec2* startTangent,
				      vec2* endTangent,
				      vec2* endPoint)
{
	vec2 controlPoints[4] = {currentPoint, element->p[0], element->p[1], element->p[2]};
	int endPointIndex = 0;
	mg_next_shape(backend, attributes);

	switch(element->type)
	{
		case MG_PATH_LINE:
			mg_render_stroke_line(backend, controlPoints, attributes);
			endPointIndex = 1;
			break;

		case MG_PATH_QUADRATIC:
			mg_render_stroke_quadratic(backend, controlPoints, attributes);
			endPointIndex = 2;
			break;

		case MG_PATH_CUBIC:
			mg_render_stroke_cubic(backend, controlPoints, attributes);
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

u32 mg_render_stroke_subpath(mg_gl_canvas_backend* backend,
                             mg_path_elt* elements,
                             mg_path_descriptor* path,
                             mg_attributes* attributes,
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
	mg_render_stroke_element(backend, elements + startIndex, attributes, currentPoint, &startTangent, &endTangent, &endPoint);

	firstTangent = startTangent;
	previousEndTangent = endTangent;
	currentPoint = endPoint;

	//NOTE(martin): render subsequent elements along with their joints
	u32 eltIndex = startIndex + 1;
	for(;
	    eltIndex<eltCount && elements[eltIndex].type != MG_PATH_MOVE;
	    eltIndex++)
	{
		mg_render_stroke_element(backend, elements + eltIndex, attributes, currentPoint, &startTangent, &endTangent, &endPoint);

		if(attributes->joint != MG_JOINT_NONE)
		{
			mg_stroke_joint(backend, currentPoint, previousEndTangent, startTangent, attributes);
		}
		previousEndTangent = endTangent;
		currentPoint = endPoint;
	}
	u32 subPathEltCount = eltIndex - (startIndex+1);

	//NOTE(martin): draw end cap / joint. We ensure there's at least two segments to draw a closing joint
	if(  subPathEltCount > 1
	  && startPoint.x == endPoint.x
	  && startPoint.y == endPoint.y)
	{
		if(attributes->joint != MG_JOINT_NONE)
		{
			//NOTE(martin): add a closing joint if the path is closed
			mg_stroke_joint(backend, endPoint, endTangent, firstTangent, attributes);
		}
	}
	else if(attributes->cap == MG_CAP_SQUARE)
	{
		//NOTE(martin): add start and end cap
		mg_stroke_cap(backend, startPoint, (vec2){-startTangent.x, -startTangent.y}, attributes);
		mg_stroke_cap(backend, endPoint, startTangent, attributes);
	}

	return(eltIndex);
}


void mg_render_stroke(mg_gl_canvas_backend* backend,
                              mg_path_elt* elements,
			      mg_path_descriptor* path,
			      mg_attributes* attributes)
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
			startIndex = mg_render_stroke_subpath(backend, elements, path, attributes, startIndex, startPoint);
		}
	}
}

//--------------------------------------------------------------------
// GL dispatch
//--------------------------------------------------------------------
#define LayoutNext(prevName, prevType, nextType) \
	AlignUpOnPow2(_cat3_(LAYOUT_, prevName, _OFFSET)+_cat3_(LAYOUT_, prevType, _SIZE), _cat3_(LAYOUT_, nextType, _ALIGN))

enum {
	LAYOUT_VEC2_SIZE = 8,
	LAYOUT_VEC2_ALIGN = 8,
	LAYOUT_VEC4_SIZE = 16,
	LAYOUT_VEC4_ALIGN = 16,
	LAYOUT_INT_SIZE = 4,
	LAYOUT_INT_ALIGN = 4,
	LAYOUT_MAT2x3_SIZE = sizeof(float)*6,
	LAYOUT_MAT2x3_ALIGN = 4,

	LAYOUT_CUBIC_OFFSET = 0,
	LAYOUT_POS_OFFSET = LayoutNext(CUBIC, VEC4, VEC2),
	LAYOUT_ZINDEX_OFFSET = LayoutNext(POS, VEC2, INT),
	LAYOUT_VERTEX_ALIGN = 16,
	LAYOUT_VERTEX_SIZE = LayoutNext(ZINDEX, INT, VERTEX),

	LAYOUT_COLOR_OFFSET = 0,
	LAYOUT_CLIP_OFFSET = LayoutNext(COLOR, VEC4, VEC4),
	LAYOUT_UV_TRANSFORM_OFFSET = LayoutNext(CLIP, VEC4, MAT2x3),
	LAYOUT_SHAPE_ALIGN = 16,
	LAYOUT_SHAPE_SIZE = LayoutNext(UV_TRANSFORM, MAT2x3, SHAPE),

	MG_GL_CANVAS_MAX_BUFFER_LENGTH = 1<<20,
	MG_GL_CANVAS_MAX_SHAPE_BUFFER_SIZE = LAYOUT_SHAPE_SIZE * MG_GL_CANVAS_MAX_BUFFER_LENGTH,
	MG_GL_CANVAS_MAX_VERTEX_BUFFER_SIZE = LAYOUT_VERTEX_SIZE * MG_GL_CANVAS_MAX_BUFFER_LENGTH,
	MG_GL_CANVAS_MAX_INDEX_BUFFER_SIZE = LAYOUT_INT_SIZE * MG_GL_CANVAS_MAX_BUFFER_LENGTH,

	//TODO: actually size this dynamically
	MG_GL_CANVAS_MAX_TILE_COUNT = 65536, //NOTE: this allows for 256*256 tiles (e.g. 4096*4096 pixels)
	MG_GL_CANVAS_TILE_COUNTER_BUFFER_SIZE = LAYOUT_INT_SIZE * MG_GL_CANVAS_MAX_TILE_COUNT,

	MG_GL_CANVAS_TILE_ARRAY_LENGTH = 1<<10, // max overlapping triangles per tiles
	MG_GL_CANVAS_TILE_ARRAY_BUFFER_SIZE = LAYOUT_INT_SIZE * MG_GL_CANVAS_MAX_TILE_COUNT * MG_GL_CANVAS_TILE_ARRAY_LENGTH,
};

void mg_gl_canvas_update_vertex_layout(mg_gl_canvas_backend* backend)
{
	backend->vertexLayout = (mg_vertex_layout){
		    .maxVertexCount = MG_GL_CANVAS_MAX_BUFFER_LENGTH,
	        .maxIndexCount = MG_GL_CANVAS_MAX_BUFFER_LENGTH,
	        .posBuffer = backend->vertexMapping + LAYOUT_POS_OFFSET,
	        .posStride = LAYOUT_VERTEX_SIZE,
	        .cubicBuffer = backend->vertexMapping + LAYOUT_CUBIC_OFFSET,
	        .cubicStride = LAYOUT_VERTEX_SIZE,
	        .shapeIndexBuffer = backend->vertexMapping + LAYOUT_ZINDEX_OFFSET,
	        .shapeIndexStride = LAYOUT_VERTEX_SIZE,

	        .colorBuffer = backend->shapeMapping + LAYOUT_COLOR_OFFSET,
	        .colorStride = LAYOUT_SHAPE_SIZE,
	        .clipBuffer = backend->shapeMapping + LAYOUT_CLIP_OFFSET,
	        .clipStride = LAYOUT_SHAPE_SIZE,
	        .uvTransformBuffer = backend->shapeMapping + LAYOUT_UV_TRANSFORM_OFFSET,
	        .uvTransformStride = LAYOUT_SHAPE_SIZE,

	        .indexBuffer = backend->indexMapping,
	        .indexStride = LAYOUT_INT_SIZE};
}

void mg_gl_send_buffers(mg_gl_canvas_backend* backend, int shapeCount, int vertexCount, int indexCount)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->vertexBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, LAYOUT_VERTEX_SIZE*vertexCount, backend->vertexMapping, GL_STREAM_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->shapeBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, LAYOUT_SHAPE_SIZE*shapeCount, backend->shapeMapping, GL_STREAM_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->indexBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, LAYOUT_INT_SIZE*indexCount, backend->indexMapping, GL_STREAM_DRAW);
}

void mg_gl_canvas_begin(mg_gl_canvas_backend* backend)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(backend->clearColor.r, backend->clearColor.g, backend->clearColor.b, backend->clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void mg_gl_canvas_end(mg_gl_canvas_backend* backend)
{
	//NOTE: nothing to do here...
}

void mg_gl_canvas_clear(mg_canvas_backend* interface, mg_color clearColor)
{
	mg_gl_canvas_backend* backend = (mg_gl_canvas_backend*)interface;

	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void mg_gl_canvas_draw_batch(mg_gl_canvas_backend* backend, mg_image_data* imageInterface, u32 shapeCount, u32 vertexCount, u32 indexCount)
{
	mg_finalize_shape(backend);

/*NOTE: if we want debug_vertex while debugging, the following ensures the struct def doesn't get stripped away
	debug_vertex vertex;
	debug_shape shape;
	printf("foo %p, bar %p\n", &vertex, &shape);
//*/
	mg_gl_send_buffers(backend, shapeCount, vertexCount, indexCount);

	mg_wgl_surface* surface = backend->surface;

	mp_rect frame = surface->interface.getFrame((mg_surface_data*)surface);
	vec2 contentsScaling = surface->interface.contentsScaling((mg_surface_data*)surface);

	const int tileSize = 16;
	const int tileCountX = (frame.w*contentsScaling.x + tileSize - 1)/tileSize;
	const int tileCountY = (frame.h*contentsScaling.y + tileSize - 1)/tileSize;
	const int tileArrayLength = MG_GL_CANVAS_TILE_ARRAY_LENGTH;

	//TODO: ensure there's enough space in tile buffer

	//NOTE: first clear counters
	glUseProgram(backend->clearCounterProgram);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, backend->tileCounterBuffer);
	glDispatchCompute(tileCountX*tileCountY, 1, 1);

	//NOTE: we first distribute triangles into tiles:

	glUseProgram(backend->tileProgram);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, backend->vertexBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, backend->shapeBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, backend->indexBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, backend->tileCounterBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, backend->tileArrayBuffer);

	glUniform1ui(0, indexCount);
	glUniform2ui(1, tileCountX, tileCountY);
	glUniform1ui(2, tileSize);
	glUniform1ui(3, tileArrayLength);
	glUniform2f(4, contentsScaling.x, contentsScaling.y);

	u32 threadCount = indexCount/3;
	glDispatchCompute((threadCount + 255)/256, 1, 1);

	//NOTE: next we sort triangles in each tile
	glUseProgram(backend->sortProgram);

	glUniform1ui(0, indexCount);
	glUniform2ui(1, tileCountX, tileCountY);
	glUniform1ui(2, tileSize);
	glUniform1ui(3, tileArrayLength);

	glDispatchCompute(tileCountX * tileCountY, 1, 1);

	//NOTE: then we fire the drawing shader that will select only triangles in its tile
	glUseProgram(backend->drawProgram);

	glBindImageTexture(0, backend->outTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

	glUniform1ui(0, indexCount);
	glUniform2ui(1, tileCountX, tileCountY);
	glUniform1ui(2, tileSize);
	glUniform1ui(3, tileArrayLength);
	glUniform2f(4, contentsScaling.x, contentsScaling.y);

	if(imageInterface)
	{
		//TODO: make sure this image belongs to that context
		mg_gl_image* image = (mg_gl_image*)imageInterface;
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, image->textureID);
		glUniform1ui(5, 1);
	}
	else
	{
		glUniform1ui(5, 0);
	}

	glDispatchCompute(tileCountX, tileCountY, 1);

	//NOTE: now blit out texture to surface
	glUseProgram(backend->blitProgram);
	glBindBuffer(GL_ARRAY_BUFFER, backend->dummyVertexBuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, backend->outTexture);
	glUniform1i(0, 0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	mg_gl_canvas_update_vertex_layout(backend);

	mg_reset_shape_index(backend);

	backend->vertexCount = 0;
	backend->indexCount = 0;
}


void mg_gl_canvas_render(mg_canvas_backend* interface,
                         mg_color clearColor,
                         u32 primitiveCount,
                         mg_primitive* primitives,
                         u32 eltCount,
                         mg_path_elt* pathElements)
{
	mg_gl_canvas_backend* backend = (mg_gl_canvas_backend*)interface;

	u32 nextIndex = 0;

	mg_reset_shape_index(backend);

	backend->clip = (mp_rect){-FLT_MAX/2, -FLT_MAX/2, FLT_MAX, FLT_MAX};
	backend->image = mg_image_nil();
	backend->clearColor = clearColor;
	mg_gl_canvas_begin(backend);

	for(int i=0; i<primitiveCount; i++)
	{
		if(nextIndex >= primitiveCount)
		{
			log_error("invalid location '%i' in graphics command buffer would cause an overrun\n", nextIndex);
			break;
		}
		mg_primitive* primitive = &(primitives[nextIndex]);
		nextIndex++;

		if(i && primitive->attributes.image.h != backend->image.h)
		{
			mg_image_data* imageData = mg_image_data_from_handle(backend->image);
			mg_gl_canvas_draw_batch(backend, imageData, backend->nextShapeIndex, backend->vertexCount, backend->indexCount);
		}
		backend->image = primitive->attributes.image;

		switch(primitive->cmd)
		{
			case MG_CMD_FILL:
				{
				mg_next_shape(backend, &primitive->attributes);
				mg_render_fill(backend,
						       pathElements + primitive->path.startIndex,
						       &primitive->path);
			} break;

			case MG_CMD_STROKE:
			{
				mg_render_stroke(backend,
							 pathElements + primitive->path.startIndex,
							 &primitive->path,
							 &primitive->attributes);
			} break;

			case MG_CMD_JUMP:
			{
				if(primitive->jump == ~0)
				{
					//NOTE(martin): normal end of stream marker
					goto exit_command_loop;
				}
				else if(primitive->jump >= primitiveCount)
				{
					log_error("invalid jump location '%i' in graphics command buffer\n", primitive->jump);
					goto exit_command_loop;
				}
				else
				{
					nextIndex = primitive->jump;
				}
			} break;
		}
	}
	exit_command_loop: ;

	mg_image_data* imageData = mg_image_data_from_handle(backend->image);
	mg_gl_canvas_draw_batch(backend, imageData, backend->nextShapeIndex, backend->vertexCount, backend->indexCount);

	mg_gl_canvas_end(backend);

	//NOTE(martin): clear buffers
	backend->vertexCount = 0;
	backend->indexCount = 0;
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
		glGenTextures(1, &image->textureID);
		glBindTexture(GL_TEXTURE_2D, image->textureID);
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
	glDeleteTextures(1, &image->textureID);
	free(image);
}

void mg_gl_canvas_image_upload_region(mg_canvas_backend* interface,
                                      mg_image_data* imageInterface,
                                      mp_rect region,
                                      u8* pixels)
{
	//TODO: check that this image belongs to this context
	mg_gl_image* image = (mg_gl_image*)imageInterface;
	glBindTexture(GL_TEXTURE_2D, image->textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, region.w, region.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

//--------------------------------------------------------------------
// Canvas setup / destroy
//--------------------------------------------------------------------

void mg_gl_canvas_destroy(mg_canvas_backend* interface)
{
	mg_gl_canvas_backend* backend = (mg_gl_canvas_backend*)interface;

	glDeleteTextures(1, &backend->outTexture);

	glDeleteBuffers(1, &backend->dummyVertexBuffer);
	glDeleteBuffers(1, &backend->vertexBuffer);
	glDeleteBuffers(1, &backend->shapeBuffer);
	glDeleteBuffers(1, &backend->indexBuffer);
	glDeleteBuffers(1, &backend->tileCounterBuffer);
	glDeleteBuffers(1, &backend->tileArrayBuffer);

	glDeleteVertexArrays(1, &backend->vao);

	free(backend->shapeMapping);
	free(backend->vertexMapping);
	free(backend->indexMapping);
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

mg_canvas_backend* gl_canvas_backend_create(mg_wgl_surface* surface)
{
	mg_gl_canvas_backend* backend = malloc_type(mg_gl_canvas_backend);
	if(backend)
	{
		memset(backend, 0, sizeof(mg_gl_canvas_backend));
		backend->surface = surface;

		//NOTE(martin): setup interface functions
		backend->interface.destroy = mg_gl_canvas_destroy;
		backend->interface.render = mg_gl_canvas_render;
		backend->interface.imageCreate = mg_gl_canvas_image_create;
		backend->interface.imageDestroy = mg_gl_canvas_image_destroy;
		backend->interface.imageUploadRegion = mg_gl_canvas_image_upload_region;

		surface->interface.prepare((mg_surface_data*)surface);

		glGenVertexArrays(1, &backend->vao);
		glBindVertexArray(backend->vao);

		glGenBuffers(1, &backend->dummyVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, backend->dummyVertexBuffer);

		glGenBuffers(1, &backend->vertexBuffer);
		glGenBuffers(1, &backend->shapeBuffer);
		glGenBuffers(1, &backend->indexBuffer);

		glGenBuffers(1, &backend->tileCounterBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->tileCounterBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MG_GL_CANVAS_TILE_COUNTER_BUFFER_SIZE, 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->tileArrayBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->tileArrayBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MG_GL_CANVAS_TILE_ARRAY_BUFFER_SIZE, 0, GL_DYNAMIC_COPY);

		mp_rect frame = surface->interface.getFrame((mg_surface_data*)surface);
		vec2 contentsScaling = surface->interface.contentsScaling((mg_surface_data*)surface);

		glGenTextures(1, &backend->outTexture);
		glBindTexture(GL_TEXTURE_2D, backend->outTexture);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, frame.w*contentsScaling.x, frame.h*contentsScaling.y);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//NOTE: create programs
		int err = 0;
		err |= mg_gl_canvas_compile_compute_program(glsl_clear_counters, &backend->clearCounterProgram);
		err |= mg_gl_canvas_compile_compute_program(glsl_tile, &backend->tileProgram);
		err |= mg_gl_canvas_compile_compute_program(glsl_sort, &backend->sortProgram);
		err |= mg_gl_canvas_compile_compute_program(glsl_draw, &backend->drawProgram);
		err |= mg_gl_canvas_compile_render_program("blit", glsl_blit_vertex, glsl_blit_fragment, &backend->blitProgram);

		if(glGetError() != GL_NO_ERROR)
		{
			err |= -1;
		}

		backend->shapeMapping = malloc_array(char, MG_GL_CANVAS_MAX_SHAPE_BUFFER_SIZE);
		backend->vertexMapping = malloc_array(char, MG_GL_CANVAS_MAX_VERTEX_BUFFER_SIZE);
		backend->indexMapping = malloc_array(char, MG_GL_CANVAS_MAX_INDEX_BUFFER_SIZE);

		if(  !backend->shapeMapping
		  || !backend->shapeMapping
		  || !backend->shapeMapping)
		{
			err |= -1;
		}

		if(err)
		{
			mg_gl_canvas_destroy((mg_canvas_backend*)backend);
			backend = 0;
		}
		else
		{
			mg_gl_canvas_update_vertex_layout(backend);
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
