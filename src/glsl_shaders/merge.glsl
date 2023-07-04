
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

precision mediump float;
layout(std430) buffer;

layout(binding = 0) restrict readonly buffer pathBufferSSBO
{
	mg_gl_path elements[];
} pathBuffer;

layout(binding = 1) restrict readonly buffer pathQueueBufferSSBO
{
	mg_gl_path_queue elements[];
} pathQueueBuffer;

layout(binding = 2) restrict readonly buffer tileQueueBufferSSBO
{
	mg_gl_tile_queue elements[];
} tileQueueBuffer;

layout(binding = 3) coherent restrict buffer tileOpCountBufferSSBO
{
	int elements[];
} tileOpCountBuffer;

layout(binding = 4) restrict buffer tileOpBufferSSBO
{
	mg_gl_tile_op elements[];
} tileOpBuffer;

layout(binding = 5) restrict writeonly buffer screenTilesBufferSSBO
{
	int elements[];
} screenTilesBuffer;

layout(location = 0) uniform int tileSize;
layout(location = 1) uniform float scale;
layout(location = 2) uniform int pathCount;
layout(location = 3) uniform int cullSolidTiles;

void main()
{
	ivec2 nTiles = ivec2(gl_NumWorkGroups.xy);
	ivec2 tileCoord = ivec2(gl_WorkGroupID.xy);
	int tileIndex = tileCoord.y * nTiles.x + tileCoord.x;

	screenTilesBuffer.elements[tileIndex] = -1;
	int lastOpIndex = -1;

	for(int pathIndex = 0; pathIndex < pathCount; pathIndex++)
	{
		mg_gl_path_queue pathQueue = pathQueueBuffer.elements[pathIndex];
		ivec2 pathTileCoord = tileCoord - pathQueue.area.xy;

		vec4 pathBox = pathBuffer.elements[pathIndex].box;
		vec4 pathClip = pathBuffer.elements[pathIndex].clip;

		float xMax = min(pathBox.z, pathClip.z);
		int tileMax = int(xMax * scale) / tileSize;
		int pathTileMax = tileMax - pathQueue.area.x;

		if(  pathTileCoord.x >= 0
		  && pathTileCoord.x <= pathTileMax
		  && pathTileCoord.y >= 0
		  && pathTileCoord.y < pathQueue.area.w)
		{
			int pathTileIndex = pathQueue.tileQueues + pathTileCoord.y * pathQueue.area.z + pathTileCoord.x;
			mg_gl_tile_queue tileQueue = tileQueueBuffer.elements[pathTileIndex];

			int windingOffset = tileQueue.windingOffset;
			int firstOpIndex = tileQueue.first;

			if(firstOpIndex == -1)
			{
				if((windingOffset & 1) != 0)
				{
					//NOTE: tile is full covered. Add path start op (with winding offset).
					//      Additionally if color is opaque and tile is fully inside clip, trim tile list.
					int pathOpIndex = atomicAdd(tileOpCountBuffer.elements[0], 1);

					tileOpBuffer.elements[pathOpIndex].kind = MG_GL_OP_START;
					tileOpBuffer.elements[pathOpIndex].next = -1;
					tileOpBuffer.elements[pathOpIndex].index = pathIndex;
					tileOpBuffer.elements[pathOpIndex].windingOffset = windingOffset;

					vec4 clip = pathBuffer.elements[pathIndex].clip * scale;
					vec4 tileBox = vec4(tileCoord.x, tileCoord.y, tileCoord.x+1, tileCoord.y+1);
					tileBox *= tileSize;

					if( lastOpIndex < 0
					  ||(pathBuffer.elements[pathIndex].color.a == 1
					    && cullSolidTiles != 0
					    && tileBox.x >= clip.x
					    && tileBox.z < clip.z
					    && tileBox.y >= clip.y
					    && tileBox.w < clip.w))
					{
						screenTilesBuffer.elements[tileIndex] = pathOpIndex;
					}
					else
					{
						tileOpBuffer.elements[lastOpIndex].next = pathOpIndex;
					}
					lastOpIndex = pathOpIndex;
				}
				// else, tile is fully uncovered, skip path
			}
			else
			{
				//NOTE: add path start op (with winding offset)
				int pathOpIndex = atomicAdd(tileOpCountBuffer.elements[0], 1);

				tileOpBuffer.elements[pathOpIndex].kind = MG_GL_OP_START;
				tileOpBuffer.elements[pathOpIndex].next = -1;
				tileOpBuffer.elements[pathOpIndex].index = pathIndex;
				tileOpBuffer.elements[pathOpIndex].windingOffset = windingOffset;

				if(lastOpIndex < 0)
				{
					screenTilesBuffer.elements[tileIndex] = pathOpIndex;
				}
				else
				{
					tileOpBuffer.elements[lastOpIndex].next = pathOpIndex;
				}
				lastOpIndex = pathOpIndex;

				//NOTE: chain remaining path ops to end of tile list
				tileOpBuffer.elements[lastOpIndex].next = firstOpIndex;
				lastOpIndex = tileQueue.last;
			}
		}
	}
}
