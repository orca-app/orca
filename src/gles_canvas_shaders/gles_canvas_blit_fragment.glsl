#version 430

precision mediump float;

in vec2 uv;
out vec4 fragColor;

layout(location=0) uniform sampler2D tex;

void main()
{
	fragColor = texture(tex, uv);
}
