/************************************************************//**
*
*	@file: mtl_shader.h
*	@author: Martin Fouilleul
*	@date: 01/08/2022
*	@revision:
*
*****************************************************************/
#ifndef __MTL_RENDERER_H_
#define __MTL_RENDERER_H_

#include<simd/simd.h>

#define RENDERER_TILE_BUFFER_SIZE 8192
#define RENDERER_TILE_SIZE 16
#define RENDERER_MAX_TILES 65536

#define RENDERER_DEBUG_TILE_VISITED 0xf00d
#define RENDERER_DEBUG_TILE_BUFFER_OVERFLOW 0xdead

typedef struct mg_vertex
{
	vector_float4 cubic; // canonical implicit curve space coordinates
	vector_float2 pos;   // position
	int shapeIndex;
} mg_vertex;

typedef struct mg_shape
{
	vector_float4 color;
	vector_float4 clip;
	float uvTransform[6];

} mg_shape;

typedef struct mg_triangle_data
{
	matrix_float3x3 uvTransform;
	vector_float4 color;

	vector_float4 cubic0;
	vector_float4 cubic1;
	vector_float4 cubic2;

	vector_int4 box;

	vector_int2 p0;
	vector_int2 p1;
	vector_int2 p2;

	int bias0;
	int bias1;
	int bias2;

	int cw;

	int shapeIndex;

} mg_triangle_data;

#ifndef __METAL_VERSION__
#define device
#else
using namespace metal;
#endif

typedef struct mg_tile_elt
{
	int triangleIndex;
	int next;
} mg_tile_elt;

typedef struct mg_tile
{
	vector_float4 color;
	atomic_int firstElt;
	atomic_int eltCount;
	atomic_int partial;
	atomic_int flipCount;

} mg_tile;

typedef struct mg_shape_queue
{
	vector_int4 area;
	int tiles;
} mg_shape_queue;

#ifndef __METAL_VERSION__
#undef device
#endif


#endif //__MTL_RENDERER_H_
