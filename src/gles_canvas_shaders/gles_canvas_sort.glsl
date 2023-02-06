#version 310 es
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

precision mediump float;
layout(std430) buffer;

struct vertex {
	vec2 pos;
	vec2 uv;
	vec4 cubic;
	vec4 color;
	vec4 clip;
	int zIndex;
};

layout(binding = 0) restrict readonly buffer vertexBufferSSBO {
	vertex elements[];
} vertexBuffer ;

layout(binding = 1) restrict readonly buffer indexBufferSSBO {
	uint elements[];
} indexBuffer ;

layout(binding = 2) coherent readonly restrict buffer tileCounterBufferSSBO {
	uint elements[];
} tileCounterBuffer ;

layout(binding = 3) coherent restrict buffer tileArrayBufferSSBO {
	uint elements[];
} tileArrayBuffer ;

layout(location = 0) uniform uint indexCount;
layout(location = 1) uniform uvec2 tileCount;
layout(location = 2) uniform uint tileSize;
layout(location = 3) uniform uint tileArraySize;

int get_zindex(uint tileArrayOffset, uint tileArrayIndex)
{
	uint triangleIndex = tileArrayBuffer.elements[tileArrayOffset + tileArrayIndex];
	uint i0 = indexBuffer.elements[triangleIndex];
	int zIndex = vertexBuffer.elements[i0].zIndex;
	return(zIndex);
}

void main()
{
	uint tileIndex = gl_WorkGroupID.x;
	uint tileArrayOffset = tileArraySize * tileIndex;
	uint tileArrayCount = tileCounterBuffer.elements[tileIndex];

	for(uint tileArrayIndex=1u; tileArrayIndex < tileArrayCount; tileArrayIndex++)
	{
		for(uint sortIndex = tileArrayIndex; sortIndex > 0u; sortIndex--)
		{
			int zIndex = get_zindex(tileArrayOffset, sortIndex);
			int prevZIndex = get_zindex(tileArrayOffset, sortIndex-1u);

			if(zIndex >= prevZIndex)
			{
				break;
			}
			uint tmp = tileArrayBuffer.elements[tileArrayOffset + sortIndex];
			tileArrayBuffer.elements[tileArrayOffset + sortIndex] = tileArrayBuffer.elements[tileArrayOffset + sortIndex - 1u];
			tileArrayBuffer.elements[tileArrayOffset + sortIndex - 1u] = tmp;
		}
	}

	//DEBUG
	/*
	int prevZIndex = -1;
	for(uint tileArrayIndex=1u; tileArrayIndex < tileArrayCount; tileArrayIndex++)
	{
		int zIndex = get_zindex(tileArrayOffset, tileArrayIndex);

		if(zIndex < prevZIndex)
		{
			tileCounterBuffer.elements[tileIndex] = 0xffffu;
			break;
		}
		prevZIndex = zIndex;
	}
	//*/
}
