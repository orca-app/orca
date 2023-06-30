
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

precision mediump float;
layout(std430) buffer;

layout(binding = 0) restrict buffer tileQueueBufferSSBO
{
	mg_gl_tile_queue elements[];
} tileQueueBuffer;

layout(location = 0) uniform ivec2 nTiles;

void main()
{
	int rowIndex = int(gl_WorkGroupID.x);

	int sum = 0;
	for(int x = nTiles.x-1; x >= 0; x--)
	{
		int tileIndex = rowIndex * nTiles.x + x;
		int offset = tileQueueBuffer.elements[tileIndex].windingOffset;
		tileQueueBuffer.elements[tileIndex].windingOffset = sum;
		sum += offset;
	}
}
