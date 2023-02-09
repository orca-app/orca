
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

precision mediump float;

layout(binding = 0) restrict readonly buffer vertexBufferSSBO {
	vertex elements[];
} vertexBuffer ;

layout(binding = 1) restrict readonly buffer shapeBufferSSBO {
	shape elements[];
} shapeBuffer ;

layout(binding = 2) restrict readonly buffer indexBufferSSBO {
	uint elements[];
} indexBuffer ;

layout(binding = 3) coherent readonly restrict buffer tileCounterBufferSSBO {
	uint elements[];
} tileCounterBuffer ;

layout(binding = 4) coherent restrict buffer tileArrayBufferSSBO {
	uint elements[];
} tileArrayBuffer ;

layout(location = 0) uniform uint indexCount;
layout(location = 1) uniform uvec2 tileCount;
layout(location = 2) uniform uint tileSize;
layout(location = 3) uniform uint tileArraySize;

int get_shape_index(uint tileArrayOffset, uint tileArrayIndex)
{
	uint triangleIndex = tileArrayBuffer.elements[tileArrayOffset + tileArrayIndex];
	uint i0 = indexBuffer.elements[triangleIndex];
	int shapeIndex = vertexBuffer.elements[i0].shapeIndex;
	return(shapeIndex);
}

void main()
{
	uint tileIndex = gl_WorkGroupID.x;
	uint tileArrayOffset = tileArraySize * tileIndex;
	uint tileArrayCount = min(tileCounterBuffer.elements[tileIndex], tileArraySize);

	for(uint tileArrayIndex=1u; tileArrayIndex < tileArrayCount; tileArrayIndex++)
	{
		for(uint sortIndex = tileArrayIndex; sortIndex > 0u; sortIndex--)
		{
			int shapeIndex = get_shape_index(tileArrayOffset, sortIndex);
			int prevShapeIndex = get_shape_index(tileArrayOffset, sortIndex-1u);

			if(shapeIndex >= prevShapeIndex)
			{
				break;
			}
			uint tmp = tileArrayBuffer.elements[tileArrayOffset + sortIndex];
			tileArrayBuffer.elements[tileArrayOffset + sortIndex] = tileArrayBuffer.elements[tileArrayOffset + sortIndex - 1u];
			tileArrayBuffer.elements[tileArrayOffset + sortIndex - 1u] = tmp;
		}
	}
}
