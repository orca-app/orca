
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

precision mediump float;
layout(std430) buffer;

layout(binding = 0) coherent restrict writeonly buffer tileCounterBufferSSBO {
	uint elements[];
} tileCounterBuffer ;

void main()
{
	uint tileIndex = gl_WorkGroupID.x;
	tileCounterBuffer.elements[tileIndex] = 0u;
}
