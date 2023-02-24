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

#define RENDERER_TILE_BUFFER_SIZE 4096
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
	vector_float4 box;
	vector_float2 uv;    // texture coordinates?

} mg_shape;

typedef struct mg_triangle_data
{
	uint i0;
	uint i1;
	uint i2;
	uint shapeIndex;

	vector_float2 p0;
	vector_float2 p1;
	vector_float2 p2;

	int bias0;
	int bias1;
	int bias2;

} mg_triangle_data;

#endif //__MTL_RENDERER_H_
