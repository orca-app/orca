#version 310 es
layout(local_size_x = 512, local_size_y = 1, local_size_z = 1) in;

precision mediump float;
layout(std430) buffer;

struct vertex {
	vec2 pos;
	vec4 cubic;
	vec2 uv;
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

layout(binding = 2) coherent restrict buffer tileCounterBufferSSBO {
	uint elements[];
} tileCounterBuffer ;

layout(binding = 3) coherent restrict writeonly buffer tileArrayBufferSSBO {
	uint elements[];
} tileArrayBuffer ;

layout(location = 0) uniform uint indexCount;
layout(location = 1) uniform uvec2 tileCount;
layout(location = 2) uniform uint tileSize;
layout(location = 3) uniform uint tileArraySize;

void main()
{
	uint triangleIndex = (gl_WorkGroupID.x*gl_WorkGroupSize.x + gl_LocalInvocationIndex) * 3u;
	if(triangleIndex >= indexCount)
	{
		return;
	}

	uint i0 = indexBuffer.elements[triangleIndex];
	uint i1 = indexBuffer.elements[triangleIndex+1u];
	uint i2 = indexBuffer.elements[triangleIndex+2u];

	vec2 p0 = vertexBuffer.elements[i0].pos;
	vec2 p1 = vertexBuffer.elements[i1].pos;
	vec2 p2 = vertexBuffer.elements[i2].pos;

	vec4 fbox = vec4(min(min(p0.x, p1.x), p2.x),
		             min(min(p0.y, p1.y), p2.y),
		             max(max(p0.x, p1.x), p2.x),
		             max(max(p0.y, p1.y), p2.y));

	uvec4 box = uvec4(floor(fbox))/tileSize;

	uint xMin = max(0u, box.x);
	uint yMin = max(0u, box.y);
	uint xMax = min(box.z, tileCount.x - 1u);
	uint yMax = min(box.w, tileCount.y - 1u);

	for(uint y = yMin; y <= yMax; y++)
	{
		for(uint x = xMin ; x <= xMax; x++)
		{
			uint tileIndex = y*tileCount.x + x;
			uint tileCounter = atomicAdd(tileCounterBuffer.elements[tileIndex], 1u);
			tileArrayBuffer.elements[tileArraySize*tileIndex + tileCounter] = triangleIndex;
		}
	}
}
