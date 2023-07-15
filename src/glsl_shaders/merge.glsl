
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
	mg_gl_screen_tile elements[];
} screenTilesBuffer;

layout(binding = 6) coherent restrict buffer dispatchBufferSSBO
{
	mg_gl_dispatch_indirect_command elements[];
} dispatchBuffer;


layout(location = 0) uniform int tileSize;
layout(location = 1) uniform float scale;
layout(location = 2) uniform int pathCount;
layout(location = 3) uniform int cullSolidTiles;

void main()
{
	ivec2 tileCoord = ivec2(gl_WorkGroupID.xy);
	int tileIndex = -1;

	int lastOpIndex = -1;

	dispatchBuffer.elements[0].num_groups_y = 1;
	dispatchBuffer.elements[0].num_groups_z = 1;

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
			if(tileIndex < 0)
			{
				tileIndex = int(atomicAdd(dispatchBuffer.elements[0].num_groups_x, 1));
				screenTilesBuffer.elements[tileIndex].tileCoord = uvec2(tileCoord);
				screenTilesBuffer.elements[tileIndex].first = -1;
			}

			int pathTileIndex = pathQueue.tileQueues + pathTileCoord.y * pathQueue.area.z + pathTileCoord.x;
			mg_gl_tile_queue tileQueue = tileQueueBuffer.elements[pathTileIndex];

			int windingOffset = tileQueue.windingOffset;
			int firstOpIndex = tileQueue.first;

			vec4 tileBox = vec4(tileCoord.x, tileCoord.y, tileCoord.x+1, tileCoord.y+1);
			tileBox *= tileSize;
			vec4 clip = pathBuffer.elements[pathIndex].clip * scale;

			if(  tileBox.x >= clip.z
			  || tileBox.z < clip.x
			  || tileBox.y >= clip.w
			  || tileBox.w < clip.y)
			{
				//NOTE: tile is fully outside clip, cull it
				//TODO: move that test up
			}
			else if(firstOpIndex == -1)
			{
				if((windingOffset & 1) != 0)
				{
					//NOTE: tile is full covered. Add path start op (with winding offset).
					//      Additionally if color is opaque and tile is fully inside clip, trim tile list.
					int pathOpIndex = atomicAdd(tileOpCountBuffer.elements[0], 1);

					tileOpBuffer.elements[pathOpIndex].kind = MG_GL_OP_CLIP_FILL;
					tileOpBuffer.elements[pathOpIndex].next = -1;
					tileOpBuffer.elements[pathOpIndex].index = pathIndex;
					tileOpBuffer.elements[pathOpIndex].windingOffsetOrCrossRight = windingOffset;

					if(lastOpIndex < 0)
					{
						screenTilesBuffer.elements[tileIndex].first = pathOpIndex;
					}
					else
					{
						tileOpBuffer.elements[lastOpIndex].next = pathOpIndex;
					}

					if(  tileBox.x >= clip.x
					  && tileBox.z < clip.z
					  && tileBox.y >= clip.y
					  && tileBox.w < clip.w)
					{
						tileOpBuffer.elements[pathOpIndex].kind = MG_GL_OP_FILL;

						if(  pathBuffer.elements[pathIndex].color.a == 1
						  && cullSolidTiles != 0)
						{
							screenTilesBuffer.elements[tileIndex].first = pathOpIndex;
						}
					}
					lastOpIndex = pathOpIndex;
				}
				// else, tile is fully uncovered, skip path
			}
			else
			{
				//NOTE: add path start op (with winding offset)
				int startOpIndex = atomicAdd(tileOpCountBuffer.elements[0], 1);

				tileOpBuffer.elements[startOpIndex].kind = MG_GL_OP_START;
				tileOpBuffer.elements[startOpIndex].next = -1;
				tileOpBuffer.elements[startOpIndex].index = pathIndex;
				tileOpBuffer.elements[startOpIndex].windingOffsetOrCrossRight = windingOffset;

				if(lastOpIndex < 0)
				{
					screenTilesBuffer.elements[tileIndex].first = startOpIndex;
				}
				else
				{
					tileOpBuffer.elements[lastOpIndex].next = startOpIndex;
				}
				lastOpIndex = startOpIndex;

				//NOTE: chain remaining path ops to end of tile list
				tileOpBuffer.elements[lastOpIndex].next = firstOpIndex;
				lastOpIndex = tileQueue.last;

				//NOTE: add path end op
				int endOpIndex = atomicAdd(tileOpCountBuffer.elements[0], 1);

				tileOpBuffer.elements[endOpIndex].kind = MG_GL_OP_END;
				tileOpBuffer.elements[endOpIndex].next = -1;
				tileOpBuffer.elements[endOpIndex].index = pathIndex;
				tileOpBuffer.elements[endOpIndex].windingOffsetOrCrossRight = windingOffset;

				if(lastOpIndex < 0)
				{
					screenTilesBuffer.elements[tileIndex].first = endOpIndex;
				}
				else
				{
					tileOpBuffer.elements[lastOpIndex].next = endOpIndex;
				}
				lastOpIndex = endOpIndex;
			}
		}
	}
}
