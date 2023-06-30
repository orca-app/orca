
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

precision mediump float;
layout(std430) buffer;

layout(binding = 0) restrict writeonly buffer tileQueueBufferSSBO
{
	mg_gl_tile_queue elements[];
} tileQueueBuffer;

void main()
{
	uvec2 nTiles = gl_NumWorkGroups.xy;
	uvec2 tileCoord = gl_WorkGroupID.xy;
	uint tileIndex =  tileCoord.y * nTiles.x + tileCoord.x;

	tileQueueBuffer.elements[tileIndex].windingOffset = 0;
	tileQueueBuffer.elements[tileIndex].first = -1;
	tileQueueBuffer.elements[tileIndex].last = -1;
}
