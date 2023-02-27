
layout(std430) buffer;

struct vertex {
	vec4 cubic;
	vec2 pos;
	int shapeIndex;
};

struct shape {
	vec4 color;
	vec4 clip;
	float uvTransform[6];
};
