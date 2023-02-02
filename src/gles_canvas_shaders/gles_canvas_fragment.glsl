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
	const float subPixelFactor = 16.;
	const int sampleCount = 8;

	ivec2 centerPoint = ivec2(round(gl_FragCoord.xy * subPixelFactor));
	ivec2 samplePoints[sampleCount] = ivec2[sampleCount](centerPoint + ivec2(1, 3),
	                                                     centerPoint + ivec2(-1, -3),
	                                                     centerPoint + ivec2(5, -1),
	                                                     centerPoint + ivec2(-3, 5),
	                                                     centerPoint + ivec2(-5, -5),
	                                                     centerPoint + ivec2(-7, 1),
	                                                     centerPoint + ivec2(3, -7),
	                                                     centerPoint + ivec2(7, 7));

	vec4 sampleColor[sampleCount];
	vec4 currentColor[sampleCount];
    int currentZIndex[sampleCount];
    int flipCount[sampleCount];

    for(int i=0; i<sampleCount; i++)
    {
		currentZIndex[i] = -1;
		flipCount[i] = 0;
		sampleColor[i] = clearColor;
		currentColor[i] = clearColor;
    }

    for(int triangleIndex=0; triangleIndex<indexCount; triangleIndex+=3)
    {
		uint i0 = indexBuffer.elements[triangleIndex];
		uint i1 = indexBuffer.elements[triangleIndex+1];
		uint i2 = indexBuffer.elements[triangleIndex+2];

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

		for(int sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
		{
			ivec2 samplePoint = samplePoints[sampleIndex];

			int w0 = orient2d(p1, p2, samplePoint);
			int w1 = orient2d(p2, p0, samplePoint);
			int w2 = orient2d(p0, p1, samplePoint);

			if((w0+bias0) >= 0 && (w1+bias1) >= 0 && (w2+bias2) >= 0)
			{
				vec4 cubic = (cubic0*float(w0) + cubic1*float(w1) + cubic2*float(w2))/(float(w0)+float(w1)+float(w2));

				float eps = 0.0001;
				if(cubic.w*(cubic.x*cubic.x*cubic.x - cubic.y*cubic.z) <= eps)
				{
					if(zIndex == currentZIndex[sampleIndex])
					{
						flipCount[sampleIndex]++;
					}
					else
					{
						if((flipCount[sampleIndex] & 0x01) != 0)
						{
							sampleColor[sampleIndex] = currentColor[sampleIndex];
						}
						currentColor[sampleIndex] = sampleColor[sampleIndex]*(1.-color.a) + color.a*color;
						currentZIndex[sampleIndex] = zIndex;
						flipCount[sampleIndex] = 1;
					}
				}
			}
		}
    }
    vec4 pixelColor = vec4(0);
    for(int sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
    {
    	if((flipCount[sampleIndex] & 0x01) != 0)
    	{
			sampleColor[sampleIndex] = currentColor[sampleIndex];
    	}
    	pixelColor += sampleColor[sampleIndex];
	}
    fragColor = pixelColor/float(sampleCount);
}
