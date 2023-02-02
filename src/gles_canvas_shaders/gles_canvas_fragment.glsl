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
	uint elements[];
} indexBuffer ;

layout(location = 0) uniform int indexCount;
layout(location = 1) uniform vec4 clearColor;

layout(location = 0) out vec4 fragColor;

bool is_top_left(ivec2 a, ivec2 b)
{
	return( (a.y == b.y && b.x < a.x)
	      ||(b.y < a.y));
}

int orient2d(ivec2 a, ivec2 b, ivec2 p)
{
	return((b.x-a.x)*(p.y-a.y) - (b.y-a.y)*(p.x-a.x));
}

void main()
{
	float subPixelFactor = 16.;

    vec4 pixelColor = clearColor;
    vec4 currentColor = clearColor;

	ivec2 samplePoint = ivec2(gl_FragCoord.xy * subPixelFactor + vec2(0.5, 0.5));

    int currentZIndex = -1;
    int flipCount = 0;


    for(int i=0; i<indexCount; i+=3)
    {
		uint i0 = indexBuffer.elements[i];
		uint i1 = indexBuffer.elements[i+1];
		uint i2 = indexBuffer.elements[i+2];

		ivec2 p0 = ivec2(vertexBuffer.elements[i0].pos * subPixelFactor + vec2(0.5, 0.5));
		ivec2 p1 = ivec2(vertexBuffer.elements[i1].pos * subPixelFactor + vec2(0.5, 0.5));
		ivec2 p2 = ivec2(vertexBuffer.elements[i2].pos * subPixelFactor + vec2(0.5, 0.5));

		int zIndex = vertexBuffer.elements[i0].zIndex;
		vec4 color = vertexBuffer.elements[i0].color;

		//NOTE(martin): reorder triangle counter-clockwise and compute bias for each edge
		int cw = (p1 - p0).x*(p2 - p0).y - (p1 - p0).y*(p2 - p0).x;
		if(cw < 0)
		{
			uint tmpIndex = i1;
			i1 = i2;
			i2 = tmpIndex;

			ivec2 tmpPoint = p1;
			p1 = p2;
			p2 = tmpPoint;
		}

		vec4 cubic0 = vertexBuffer.elements[i0].cubic;
		vec4 cubic1 = vertexBuffer.elements[i1].cubic;
		vec4 cubic2 = vertexBuffer.elements[i2].cubic;

		int bias0 = is_top_left(p1, p2) ? 0 : -1;
		int bias1 = is_top_left(p2, p0) ? 0 : -1;
		int bias2 = is_top_left(p0, p1) ? 0 : -1;

		int w0 = orient2d(p1, p2, samplePoint);
		int w1 = orient2d(p2, p0, samplePoint);
		int w2 = orient2d(p0, p1, samplePoint);

		if((w0+bias0) >= 0 && (w1+bias1) >= 0 && (w2+bias2) >= 0)
		{
			vec4 cubic = (cubic0*float(w0) + cubic1*float(w1) + cubic2*float(w2))/(float(w0)+float(w1)+float(w2));

			float eps = 0.0001;
			if(cubic.w*(cubic.x*cubic.x*cubic.x - cubic.y*cubic.z) <= eps)
			{
				if(zIndex == currentZIndex)
				{
					flipCount++;
				}
				else
				{
					if((flipCount & 0x01) != 0)
					{
						pixelColor = currentColor;
					}
					currentColor = pixelColor*(1.-color.a) + color.a*color;
					currentZIndex = zIndex;
					flipCount = 1;
				}
			}
		}
    }
    if((flipCount & 0x01) != 0)
    {
		pixelColor = currentColor;
    }

    fragColor = pixelColor;
}
