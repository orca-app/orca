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
} mg_mtl_cmd;

typedef struct mg_mtl_path
{
	mg_mtl_cmd cmd;
	vector_float4 color;
	vector_float4 box;

} mg_mtl_path;

typedef enum {
	MG_MTL_LINE = 1,
} mg_mtl_seg_kind;

typedef struct mg_mtl_path_elt
{
	int pathIndex;
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
	int pathIndex;
	mg_mtl_seg_config config; //TODO pack these
	int windingIncrement;
	vector_float4 box;
} mg_mtl_segment;

typedef struct mg_mtl_path_queue
{
	vector_int4 area;
	int tileQueues;
} mg_mtl_path_queue;

#ifdef __METAL_VERSION__
	using namespace metal;
#endif

typedef enum { MG_MTL_OP_SEGMENT } mg_mtl_tile_op_kind;

typedef struct mg_mtl_tile_op
{
	mg_mtl_tile_op_kind kind;
	int index;
	int next;
} mg_mtl_tile_op;

typedef struct mg_mtl_tile_queue
{
	atomic_int first;

} mg_mtl_tile_queue;

#endif //__MTL_RENDERER_H_
