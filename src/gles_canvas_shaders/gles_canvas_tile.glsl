#version 310 es
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

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

layout(binding = 0) buffer vertexBufferSSBO {
	vertex elements[];
} vertexBuffer ;

layout(binding = 1) buffer indexBufferSSBO {
	uint elements[];
} indexBuffer ;

layout(binding = 2) coherent buffer tileCounterBufferSSBO {
	uint elements[];
} tileCounterBuffer ;

layout(binding = 3) coherent buffer tileArrayBufferSSBO {
	uint elements[];
} tileArrayBuffer ;

layout(location = 0) uniform uint indexCount;
layout(location = 1) uniform uvec2 tileCount;
layout(location = 2) uniform uint tileSize;
layout(location = 3) uniform uint tileArraySize;

void main()
{
	uint tileIndex = gl_WorkGroupID.y * tileCount.x + gl_WorkGroupID.x;
	uint tileArrayOffset = tileArraySize * tileIndex;

	vec4 tileBox = 	vec4(gl_WorkGroupID.x * tileSize,
	                     gl_WorkGroupID.y * tileSize,
	                     gl_WorkGroupID.x * tileSize + tileSize,
	                     gl_WorkGroupID.y * tileSize + tileSize);

	uint tileCounter = 0u;

	for(uint triangleIndex = 0u; triangleIndex < indexCount; triangleIndex += 3u)
	{
		uint i0 = indexBuffer.elements[triangleIndex];
		uint i1 = indexBuffer.elements[triangleIndex+1u];
		uint i2 = indexBuffer.elements[triangleIndex+2u];

		vec2 p0 = vertexBuffer.elements[i0].pos;
		vec2 p1 = vertexBuffer.elements[i1].pos;
		vec2 p2 = vertexBuffer.elements[i2].pos;

		vec4 bbox = vec4(min(min(p0.x, p1.x), p2.x),
		                 min(min(p0.y, p1.y), p2.y),
		                 max(max(p0.x, p1.x), p2.x),
		                 max(max(p0.y, p1.y), p2.y));

		if(!(  bbox.x > tileBox.z
		    || bbox.z < tileBox.x
		    || bbox.y > tileBox.w
		    || bbox.w < tileBox.y))
		{
			tileArrayBuffer.elements[tileArrayOffset + tileCounter] = triangleIndex;
			tileCounter++;
		}
	}
	tileCounterBuffer.elements[tileIndex] = tileCounter;

//	tileCounterBuffer.elements[tileIndex] = 1u;
}
