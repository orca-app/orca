
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

layout(binding = 2) restrict writeonly buffer tileQueueBufferSSBO
{
	mg_gl_tile_queue elements[];
} tileQueueBuffer;

layout(binding = 3) restrict writeonly buffer tileQueueCountBufferSSBO
{
	int elements[];
} tileQueueCountBuffer;

void main()
{

}
