#version 430
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

precision mediump float;
//precision mediump image2D;

layout(std430) buffer;

struct vertex {
	vec4 cubic;
	vec2 pos;
	int zIndex;
};

struct shape {
	vec4 color;
	vec4 clip;
	vec2 uv;
};

layout(binding = 0) restrict readonly buffer vertexBufferSSBO {
	vertex elements[];
} vertexBuffer ;

layout(binding = 1) restrict readonly buffer shapeBufferSSBO {
	shape elements[];
} shapeBuffer ;

layout(binding = 2) restrict readonly buffer indexBufferSSBO {
	uint elements[];
} indexBuffer ;

layout(binding = 3) restrict readonly buffer tileCounterBufferSSBO {
	uint elements[];
} tileCounterBuffer ;

layout(binding = 4) restrict readonly buffer tileArrayBufferSSBO {
	uint elements[];
} tileArrayBuffer ;

layout(location = 0) uniform uint indexCount;
layout(location = 1) uniform uvec2 tileCount;
layout(location = 2) uniform uint tileSize;
layout(location = 3) uniform uint tileArraySize;
layout(rgba8, binding = 0) uniform restrict writeonly image2D outTexture;


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
	ivec2 pixelCoord = ivec2(gl_WorkGroupID.xy*uvec2(16, 16) + gl_LocalInvocationID.xy);
	uvec2 tileCoord = uvec2(pixelCoord) / tileSize;
	uint tileIndex =  tileCoord.y * tileCount.x + tileCoord.x;
	uint tileCounter = tileCounterBuffer.elements[tileIndex];

	const float subPixelFactor = 16.;
	ivec2 centerPoint = ivec2((vec2(pixelCoord) + vec2(0.5, 0.5)) * subPixelFactor);

//*
	const int sampleCount = 8;
	ivec2 samplePoints[sampleCount] = ivec2[sampleCount](centerPoint + ivec2(1, 3),
	                                                     centerPoint + ivec2(-1, -3),
	                                                     centerPoint + ivec2(5, -1),
	                                                     centerPoint + ivec2(-3, 5),
	                                                     centerPoint + ivec2(-5, -5),
	                                                     centerPoint + ivec2(-7, 1),
	                                                     centerPoint + ivec2(3, -7),
	                                                     centerPoint + ivec2(7, 7));
/*/
	const int sampleCount = 4;
	ivec2 samplePoints[sampleCount] = ivec2[sampleCount](centerPoint + ivec2(-2, 6),
	                                                     centerPoint + ivec2(6, 2),
	                                                     centerPoint + ivec2(-6, -2),
	                                                     centerPoint + ivec2(2, -6));
//*/
	//DEBUG
/*
	{
		vec4 fragColor = vec4(0);

		if( pixelCoord.x % 16 == 0
	  	  ||pixelCoord.y % 16 == 0)
		{
			fragColor = vec4(0, 0, 0, 1);
		}
		else if(tileCounterBuffer.elements[tileIndex] == 0xffffu)
		{
			fragColor = vec4(1, 0, 1, 1);
		}
		else if(tileCounter != 0u)
		{
			fragColor = vec4(0, 1, 0, 1);
		}
		else
		{
			fragColor = vec4(1, 0, 0, 1);
		}
		imageStore(outTexture, pixelCoord, fragColor);
		return;
	}
//*/
	//----

	vec4 sampleColor[sampleCount];
	vec4 currentColor[sampleCount];
    int currentZIndex[sampleCount];
    int flipCount[sampleCount];

    for(int i=0; i<sampleCount; i++)
    {
		currentZIndex[i] = -1;
		flipCount[i] = 0;
		sampleColor[i] = vec4(0, 0, 0, 0);
		currentColor[i] = vec4(0, 0, 0, 0);
    }

    for(uint tileArrayIndex=0u; tileArrayIndex < tileCounter; tileArrayIndex++)
    {
    	uint triangleIndex = tileArrayBuffer.elements[tileArraySize * tileIndex + tileArrayIndex];

		uint i0 = indexBuffer.elements[triangleIndex];
		uint i1 = indexBuffer.elements[triangleIndex+1u];
		uint i2 = indexBuffer.elements[triangleIndex+2u];

		ivec2 p0 = ivec2((vertexBuffer.elements[i0].pos) * subPixelFactor);
		ivec2 p1 = ivec2((vertexBuffer.elements[i1].pos) * subPixelFactor);
		ivec2 p2 = ivec2((vertexBuffer.elements[i2].pos) * subPixelFactor);

		int zIndex = vertexBuffer.elements[i0].zIndex;
		vec4 color = shapeBuffer.elements[zIndex].color;
		ivec4 clip = ivec4(round((shapeBuffer.elements[zIndex].clip + vec4(0.5, 0.5, 0.5, 0.5)) * subPixelFactor));

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

			if(  samplePoint.x < clip.x
			  || samplePoint.x > clip.z
			  || samplePoint.y < clip.y
			  || samplePoint.y > clip.w)
			{
				continue;
			}

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

	imageStore(outTexture, pixelCoord, pixelColor/float(sampleCount));
}
