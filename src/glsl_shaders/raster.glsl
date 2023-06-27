
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

precision mediump float;
layout(std430) buffer;

layout(binding = 0) restrict readonly buffer screenTilesBufferSSBO
{
	mg_gl_tile_queue elements[];
} screenTilesBuffer;

layout(binding = 1) restrict readonly buffer tileOpBufferSSBO
{
	mg_gl_tile_op elements[];
} tileOpBuffer;

layout(binding = 2) restrict readonly buffer pathBufferSSBO
{
	mg_gl_path elements[];
} pathBuffer;

layout(binding = 3) restrict readonly buffer segmentBufferSSBO
{
	mg_gl_segment elements[];
} segmentBuffer;


void main()
{

}
