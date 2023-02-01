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
layout(location = 0) out vec4 fragColor;

bool is_top_left(vec2 a, vec2 b)
{
	return( (a.y == b.y && b.x < a.x)
	      ||(b.y < a.y));
}

float orient2d(vec2 a, vec2 b, vec2 c)
{
	//////////////////////////////////////////////////////////////////////////////////////////
	//TODO(martin): FIX this. This is a **horrible** quick hack to fix the precision issues
	//              arising when a, b, and c are close. But it degrades when a, c, and c
	//              are big. The proper solution is to change the expression to avoid
	//              precision loss but I'm too busy/lazy to do it now.
	//////////////////////////////////////////////////////////////////////////////////////////
	a *= 10.;
	b *= 10.;
	c *= 10.;
	return((b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x));
}

void main()
{
    vec4 pixelColor = vec4(0.0, 1.0, 0.0, 1.0);
    vec4 currentColor = vec4(0., 0., 0., 1.0);

	vec2 samplePoint = gl_FragCoord.xy;

    int currentZIndex = -1;
    int flipCount = 0;


    for(int i=0; i<indexCount; i+=3)
    {
		uint i0 = indexBuffer.elements[i];
		uint i1 = indexBuffer.elements[i+1];
		uint i2 = indexBuffer.elements[i+2];

		vec2 p0 = vertexBuffer.elements[i0].pos;
		vec2 p1 = vertexBuffer.elements[i1].pos;
		vec2 p2 = vertexBuffer.elements[i2].pos;

		int zIndex = vertexBuffer.elements[i0].zIndex;
		vec4 color = vertexBuffer.elements[i0].color;

		//NOTE(martin): reorder triangle counter-clockwise and compute bias for each edge
		float cw = (p1 - p0).x*(p2 - p0).y - (p1 - p0).y*(p2 - p0).x;
		if(cw < 0.)
		{
			uint tmpIndex = i1;
			i1 = i2;
			i2 = tmpIndex;

			vec2 tmpPoint = p1;
			p1 = p2;
			p2 = tmpPoint;
		}

		vec4 cubic0 = vertexBuffer.elements[i0].cubic;
		vec4 cubic1 = vertexBuffer.elements[i1].cubic;
		vec4 cubic2 = vertexBuffer.elements[i2].cubic;

		int bias0 = is_top_left(p1, p2) ? 0 : -1;
		int bias1 = is_top_left(p2, p0) ? 0 : -1;
		int bias2 = is_top_left(p0, p1) ? 0 : -1;

		float w0 = orient2d(p1, p2, samplePoint);
		float w1 = orient2d(p2, p0, samplePoint);
		float w2 = orient2d(p0, p1, samplePoint);

		if((int(w0)+bias0) >= 0 && (int(w1)+bias1) >= 0 && (int(w2)+bias2) >= 0)
		{
			//TODO check cubic
			vec4 cubic = (cubic0*w0 + cubic1*w1 + cubic2*w2)/(w0+w1+w2);

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
