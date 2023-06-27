
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

precision mediump float;
layout(std430) buffer;

layout(binding = 0) restrict readonly buffer elementBufferSSBO
{
	mg_gl_path_elt elements[];
} elementBuffer;

layout(binding = 1) restrict buffer segmentCountBufferSSBO
{
	int elements[];
} segmentCountBuffer;

layout(binding = 2) restrict writeonly buffer segmentBufferSSBO
{
	mg_gl_segment elements[];
} segmentBuffer;

layout(binding = 3) restrict readonly buffer pathQueueBufferSSBO
{
	mg_gl_path_queue elements[];
} pathQueueBuffer;

layout(binding = 4) restrict readonly buffer tileQueueBufferSSBO
{
	mg_gl_tile_queue elements[];
} tileQueueBuffer;

layout(binding = 5) restrict readonly buffer tileOpBufferSSBO
{
	mg_gl_tile_op elements[];
} tileOpBuffer;

layout(binding = 6) restrict readonly buffer tileOpCountBufferSSBO
{
	int elements[];
} tileOpCountBuffer;


void main()
{

}
