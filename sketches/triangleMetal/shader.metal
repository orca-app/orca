/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include<metal_stdlib>
#include<simd/simd.h>

using namespace metal;

#import "vertex.h"

typedef unsigned int uint;

typedef struct
{
	float4 clipSpacePos [[position]];
	float4 color;

} rasterizer_data;

vertex rasterizer_data VertexShader(uint vertexID [[vertex_id]],
                                    constant my_vertex* vertices [[buffer(vertexInputIndexVertices)]],
				    constant vector_uint2* viewportSizePointer [[buffer(vertexInputIndexViewportSize)]])
{
	float2 pixelPos = vertices[vertexID].pos.xy;
	float2 viewportSize = vector_float2(*viewportSizePointer);

	rasterizer_data out;
	out.clipSpacePos = vector_float4(0, 0, 0, 1);
	out.clipSpacePos.xy = pixelPos/(viewportSize/2);
	out.color = vertices[vertexID].col;

	return(out);
}

fragment float4 FragmentShader(rasterizer_data in [[stage_in]])
{
	return(in.color);
}
