
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

precision mediump float;
layout(std430) buffer;

layout(binding = 0) coherent restrict readonly buffer screenTilesCountBufferSSBO
{
	int elements[];
} screenTilesCountBuffer;

layout(binding = 1) coherent restrict writeonly buffer dispatchBufferSSBO
{
	mg_gl_dispatch_indirect_command elements[];
} dispatchBuffer;


layout(location = 0) uniform uint maxWorkGroupCount;

void main()
{
	uint totalWorkGroupCount = screenTilesCountBuffer.elements[0];

	dispatchBuffer.elements[0].num_groups_x = totalWorkGroupCount > maxWorkGroupCount ? maxWorkGroupCount : totalWorkGroupCount;
	dispatchBuffer.elements[0].num_groups_y = (totalWorkGroupCount + maxWorkGroupCount - 1) / maxWorkGroupCount;
	dispatchBuffer.elements[0].num_groups_z = 1;
}
