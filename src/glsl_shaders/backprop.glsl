
layout(local_size_x = 16, local_size_y = 1, local_size_z = 1) in;

precision mediump float;
layout(std430) buffer;

layout(binding = 0) restrict readonly buffer pathQueueBufferSSBO
{
	mg_gl_path_queue elements[];
} pathQueueBuffer;

layout(binding = 1) restrict buffer tileQueueBufferSSBO
{
	mg_gl_tile_queue elements[];
} tileQueueBuffer;

layout(location = 0) uniform int pathQueueBufferStart;

shared int nextRowIndex;

void main()
{
	int pathIndex = int(gl_WorkGroupID.x);
	int localID = int(gl_LocalInvocationID.x);

	if(localID == 0)
	{
		nextRowIndex = 0;
	}
	barrier();

	int rowIndex = 0;
	mg_gl_path_queue pathQueue = pathQueueBuffer.elements[pathQueueBufferStart + pathIndex];
	int tileQueueBase = pathQueue.tileQueues;
	int rowSize = pathQueue.area.z;
	int rowCount = pathQueue.area.w;

	rowIndex = atomicAdd(nextRowIndex, 1);
	while(rowIndex < rowCount)
	{
		int sum = 0;
		for(int x = rowSize-1; x >= 0; x--)
		{
			int tileIndex = tileQueueBase + rowIndex * rowSize.x + x;
			int offset = tileQueueBuffer.elements[tileIndex].windingOffset;
			tileQueueBuffer.elements[tileIndex].windingOffset = sum;
			sum += offset;
		}
		rowIndex = atomicAdd(nextRowIndex, 1);
	}
}
