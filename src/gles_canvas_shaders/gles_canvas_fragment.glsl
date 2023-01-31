#version 310 es

precision mediump float;
layout(std430) buffer;

struct vertex {
	vec2 pos;
	vec4 cubic;
	vec2 uv;
	vec4 color;
	vec4 clip;
	int zIndex;
};

layout(binding = 0) buffer vertexBufferSSBO {
	vertex elements[];
} vertexBuffer ;

layout(binding = 1) buffer indexBufferSSBO {
	vec2 elements[];
} indexBuffer ;

layout(location = 0) uniform int indexCount;
layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = vec4(0.0, 1.0, 0.0, 1.0);
}
