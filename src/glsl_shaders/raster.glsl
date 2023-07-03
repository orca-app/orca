
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

precision mediump float;
layout(std430) buffer;

layout(binding = 0) restrict readonly buffer pathBufferSSBO
{
	mg_gl_path elements[];
} pathBuffer;

layout(binding = 1) restrict readonly buffer segmentCountBufferSSBO
{
	int elements[];
} segmentCountBuffer;

layout(binding = 2) restrict readonly buffer segmentBufferSSBO
{
	mg_gl_segment elements[];
} segmentBuffer;

layout(binding = 3) restrict readonly buffer tileOpBufferSSBO
{
	mg_gl_tile_op elements[];
} tileOpBuffer;

layout(binding = 4) restrict readonly buffer screenTilesBufferSSBO
{
	int elements[];
} screenTilesBuffer;

layout(location = 0) uniform float scale;
layout(location = 1) uniform int msaaSampleCount;

layout(rgba8, binding = 0) uniform restrict writeonly image2D outTexture;


void main()
{
	uvec2 nTiles = gl_NumWorkGroups.xy;
	uvec2 tileCoord = gl_WorkGroupID.xy;
	uint tileIndex =  tileCoord.y * nTiles.x + tileCoord.x;

	ivec2 pixelCoord = ivec2(gl_WorkGroupID.xy*uvec2(16, 16) + gl_LocalInvocationID.xy);
	vec2 centerCoord = vec2(pixelCoord) + vec2(0.5, 0.5);

	/*
	if((pixelCoord.x % 16) == 0 || (pixelCoord.y % 16) == 0)
	{
		imageStore(outTexture, pixelCoord, vec4(0, 0, 0, 1));
		return;
	}
	*/

	vec2 sampleCoords[MG_GL_MAX_SAMPLE_COUNT] = {
		centerCoord + vec2(1, 3)/16,
		centerCoord + vec2(-1, -3)/16,
		centerCoord + vec2(5, -1)/16,
		centerCoord + vec2(-3, 5)/16,
		centerCoord + vec2(-5, -5)/16,
		centerCoord + vec2(-7, 1)/16,
		centerCoord + vec2(3, -7)/16,
		centerCoord + vec2(7, 7)/16
	};

	int sampleCount = msaaSampleCount;
	if(sampleCount != 8)
	{
		sampleCount = 1;
		sampleCoords[0] = centerCoord;
	}

	vec4 color[MG_GL_MAX_SAMPLE_COUNT];
	int winding[MG_GL_MAX_SAMPLE_COUNT];

	for(int i=0; i<sampleCount; i++)
	{
		winding[i] = 0;
		color[i] = vec4(0);
	}

	int pathIndex = 0;
	int opIndex = screenTilesBuffer.elements[tileIndex];

	while(opIndex >= 0)
	{
		mg_gl_tile_op op = tileOpBuffer.elements[opIndex];
		opIndex = op.next;

		if(op.kind == MG_GL_OP_START)
		{
			vec4 clip = pathBuffer.elements[pathIndex].clip * scale;
			vec4 pathColor = pathBuffer.elements[pathIndex].color;
			pathColor.rgb *= pathColor.a;

			for(int sampleIndex = 0; sampleIndex<sampleCount; sampleIndex++)
			{
				vec2 sampleCoord = sampleCoords[sampleIndex];

				if(  sampleCoord.x >= clip.x
				  && sampleCoord.x < clip.z
				  && sampleCoord.y >= clip.y
				  && sampleCoord.y < clip.w)
				{
					bool filled = (pathBuffer.elements[pathIndex].cmd == MG_GL_FILL && ((winding[sampleIndex] & 1) != 0))
					            ||(pathBuffer.elements[pathIndex].cmd == MG_GL_STROKE && (winding[sampleIndex] != 0));
					if(filled)
					{
						vec4 nextColor = pathColor;
						color[sampleIndex] = color[sampleIndex]*(1-nextColor.a) + nextColor;
					}
				}
				winding[sampleIndex] = op.windingOffset;
			}
			pathIndex = op.index;
		}
		else if(op.kind == MG_GL_OP_SEGMENT)
		{
			int segIndex = op.index;
			mg_gl_segment seg = segmentBuffer.elements[segIndex];

			for(int sampleIndex=0; sampleIndex<sampleCount; sampleIndex++)
			{
				vec2 sampleCoord = sampleCoords[sampleIndex];

				if( (sampleCoord.y > seg.box.y)
				  &&(sampleCoord.y <= seg.box.w)
				  &&(side_of_segment(sampleCoord, seg) < 0))
				{
					winding[sampleIndex] += seg.windingIncrement;
				}

				if(op.crossRight)
				{
					if( (seg.config == MG_GL_BR || seg.config == MG_GL_TL)
					  &&(sampleCoord.y > seg.box.w))
					{
						winding[sampleIndex] += seg.windingIncrement;
					}
					else if( (seg.config == MG_GL_BL || seg.config == MG_GL_TR)
					       &&(sampleCoord.y > seg.box.y))
					{
						winding[sampleIndex] -= seg.windingIncrement;
					}
				}
			}
		}
	}

	vec4 clip = pathBuffer.elements[pathIndex].clip * scale;

	vec4 pixelColor = vec4(0);
	vec4 pathColor = pathBuffer.elements[pathIndex].color;
	pathColor.rgb *= pathColor.a;

	for(int sampleIndex=0; sampleIndex<sampleCount; sampleIndex++)
	{
		vec2 sampleCoord = sampleCoords[sampleIndex];

		if(  sampleCoord.x >= clip.x
		  && sampleCoord.x < clip.z
		  && sampleCoord.y >= clip.y
		  && sampleCoord.y < clip.w)
		{
			bool filled = (pathBuffer.elements[pathIndex].cmd == MG_GL_FILL && ((winding[sampleIndex] & 1) != 0))
			            ||(pathBuffer.elements[pathIndex].cmd == MG_GL_STROKE && (winding[sampleIndex] != 0));
			if(filled)
			{
				vec4 nextColor = pathColor;
				color[sampleIndex] = color[sampleIndex]*(1-nextColor.a) + nextColor;
			}
		}
		pixelColor += color[sampleIndex];
	}
	pixelColor /= sampleCount;

	imageStore(outTexture, pixelCoord, pixelColor);
}
