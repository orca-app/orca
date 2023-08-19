/************************************************************/ /**
*
*	@file: mtl_renderer.h
*	@author: Martin Fouilleul
*	@date: 01/08/2022
*	@revision:
*
*****************************************************************/
#ifndef __MTL_RENDERER_H_
#define __MTL_RENDERER_H_

#include <simd/simd.h>

typedef enum
{
    OC_MTL_FILL,
    OC_MTL_STROKE,
} oc_mtl_cmd;

typedef struct oc_mtl_path
{
    oc_mtl_cmd cmd;
    matrix_float3x3 uvTransform;
    vector_float4 color;
    vector_float4 box;
    vector_float4 clip;
    int texture;
} oc_mtl_path;

typedef enum
{
    OC_MTL_LINE = 1,
    OC_MTL_QUADRATIC,
    OC_MTL_CUBIC,
} oc_mtl_seg_kind;

typedef struct oc_mtl_path_elt
{
    int pathIndex;
    oc_mtl_seg_kind kind;
    vector_float2 p[4];
} oc_mtl_path_elt;

typedef enum
{
    OC_MTL_BL, // curve on bottom left
    OC_MTL_BR, // curve on bottom right
    OC_MTL_TL, // curve on top left
    OC_MTL_TR  // curve on top right
} oc_mtl_seg_config;

typedef struct oc_mtl_segment
{
    oc_mtl_seg_kind kind;
    int pathIndex;
    oc_mtl_seg_config config; //TODO pack these
    int windingIncrement;
    vector_float4 box;
    matrix_float3x3 implicitMatrix;
    float sign;
    vector_float2 hullVertex;
    int debugID;

} oc_mtl_segment;

typedef struct oc_mtl_path_queue
{
    vector_int4 area;
    int tileQueues;
} oc_mtl_path_queue;

#ifdef __METAL_VERSION__
using namespace metal;
#endif

typedef enum
{
    OC_MTL_OP_FILL,
    OC_MTL_OP_CLIP_FILL,
    OC_MTL_OP_START,
    OC_MTL_OP_END,
    OC_MTL_OP_SEGMENT
} oc_mtl_tile_op_kind;

typedef struct oc_mtl_tile_op
{
    oc_mtl_tile_op_kind kind;
    int index;
    int next;

    union
    {
        bool crossRight;
        int windingOffset;
    };

} oc_mtl_tile_op;

typedef struct oc_mtl_tile_queue
{
    atomic_int windingOffset;
    atomic_int first;
    int last;

} oc_mtl_tile_queue;

typedef struct oc_mtl_screen_tile
{
    vector_uint2 tileCoord;
    int first;

} oc_mtl_screen_tile;

enum
{
    OC_MTL_MAX_IMAGES_PER_BATCH = 30
};

#endif //__MTL_RENDERER_H_
