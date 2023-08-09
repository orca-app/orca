
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

precision mediump float;
layout(std430) buffer;

layout(binding = 0) restrict readonly buffer pathBufferSSBO
{
	mg_gl_path elements[];
} pathBuffer;

layout(binding = 1) restrict writeonly buffer pathQueueBufferSSBO
{
	mg_gl_path_queue elements[];
} pathQueueBuffer;

layout(binding = 2) coherent restrict buffer tileQueueCountBufferSSBO
{
	int elements[];
} tileQueueCountBuffer;

layout(binding = 3) restrict writeonly buffer tileQueueBufferSSBO
{
	mg_gl_tile_queue elements[];
} tileQueueBuffer;

layout(location = 0) uniform int tileSize;
layout(location = 1) uniform float scale;
layout(location = 2) uniform int pathBufferStart;
layout(location = 3) uniform int pathQueueBufferStart;

void main()
{
	uint pathIndex = gl_WorkGroupID.x;
	const mg_gl_path path = pathBuffer.elements[pathIndex + pathBufferStart];

	//NOTE: we don't clip on the right, since we need those tiles to accurately compute
	//      the prefix sum of winding increments in the backprop pass.
	vec4 clippedBox = vec4(max(path.box.x, path.clip.x),
	                       max(path.box.y, path.clip.y),
	                       path.box.z,
	                       min(path.box.w, path.clip.w));

	ivec2 firstTile = ivec2(clippedBox.xy*scale)/tileSize;
	ivec2 lastTile = ivec2(clippedBox.zw*scale)/tileSize;

	int nTilesX = max(0, lastTile.x - firstTile.x + 1);
	int nTilesY = max(0, lastTile.y - firstTile.y + 1);
	int tileCount = nTilesX * nTilesY;

	int tileQueuesIndex = atomicAdd(tileQueueCountBuffer.elements[0], tileCount);

	if(tileQueuesIndex + tileCount >= tileQueueBuffer.elements.length())
	{
		pathQueueBuffer.elements[pathIndex].area = ivec4(0);
		pathQueueBuffer.elements[pathIndex].tileQueues = 0;
	}
	else
	{
		pathQueueBuffer.elements[pathQueueBufferStart + pathIndex].area = ivec4(firstTile.x, firstTile.y, nTilesX, nTilesY);
		pathQueueBuffer.elements[pathQueueBufferStart + pathIndex].tileQueues = tileQueuesIndex;

		for(int i=0; i<tileCount; i++)
		{
			tileQueueBuffer.elements[tileQueuesIndex + i].first = -1;
			tileQueueBuffer.elements[tileQueuesIndex + i].last = -1;
			tileQueueBuffer.elements[tileQueuesIndex + i].windingOffset = 0;
		}
	}
}
