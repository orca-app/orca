/************************************************************//**
*
*	@file: mtl_renderer.h
*	@author: Martin Fouilleul
*	@date: 01/08/2022
*	@revision:
*
*****************************************************************/
#ifndef __MTL_RENDERER_H_
#define __MTL_RENDERER_H_

#include<simd/simd.h>

typedef enum {
	MG_MTL_FILL,
	MG_MTL_STROKE,
} mg_mtl_cmd;

typedef struct mg_mtl_path
{
	mg_mtl_cmd cmd;
	matrix_float3x3 uvTransform;
	vector_float4 color;
	vector_float4 box;
} mg_mtl_path;

typedef enum {
	MG_MTL_LINE = 1,
	MG_MTL_QUADRATIC,
	MG_MTL_CUBIC,
} mg_mtl_seg_kind;

typedef struct mg_mtl_path_elt
{
	int pathIndex;
	int localEltIndex;
	mg_mtl_seg_kind kind;
	vector_float2 p[4];
} mg_mtl_path_elt;

typedef enum {
	MG_MTL_BL, // curve on bottom left
	MG_MTL_BR, // curve on bottom right
	MG_MTL_TL, // curve on top left
	MG_MTL_TR  // curve on top right
} mg_mtl_seg_config;

typedef struct mg_mtl_segment
{
	mg_mtl_seg_kind kind;
	int pathIndex;
	mg_mtl_seg_config config; //TODO pack these
	int windingIncrement;
	vector_float4 box;
	matrix_float3x3 hullMatrix;
	matrix_float3x3 implicitMatrix;
	float sign;
	vector_float2 hullVertex;
	int debugID;

} mg_mtl_segment;

typedef struct mg_mtl_path_queue
{
	vector_int4 area;
	int tileQueues;
} mg_mtl_path_queue;

#ifdef __METAL_VERSION__
	using namespace metal;
#endif

typedef enum { MG_MTL_OP_START,
               MG_MTL_OP_SEGMENT } mg_mtl_tile_op_kind;

typedef struct mg_mtl_tile_op
{
	mg_mtl_tile_op_kind kind;
	int index;
	int next;
	union
	{
		bool crossRight;
		int windingOffset;
	};

} mg_mtl_tile_op;

typedef struct mg_mtl_tile_queue
{
	atomic_int windingOffset;
	atomic_int first;
	int last;

} mg_mtl_tile_queue;

#endif //__MTL_RENDERER_H_
