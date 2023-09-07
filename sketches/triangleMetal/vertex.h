/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#ifndef __VERTEX_H__
#define __VERTEX_H__

#include <simd/simd.h>

typedef struct
{
    vector_float2 pos;
    vector_float4 col;
} my_vertex;

typedef enum
{
    vertexInputIndexVertices = 0,
    vertexInputIndexViewportSize = 1
} vertexInputIndex;

#endif //__VERTEX_H__
