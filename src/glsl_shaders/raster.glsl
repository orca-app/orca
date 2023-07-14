
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

precision mediump float;
layout(std430) buffer;

layout(binding = 0) restrict readonly buffer pathBufferSSBO
{
	mg_gl_path elements[];
} pathBuffer;

layout(binding = 1) restrict readonly buffer segmentBufferSSBO
{
	mg_gl_segment elements[];
} segmentBuffer;

layout(binding = 2) restrict readonly buffer tileOpBufferSSBO
{
	mg_gl_tile_op elements[];
} tileOpBuffer;

layout(binding = 3) restrict readonly buffer screenTilesBufferSSBO
{
	int elements[];
} screenTilesBuffer;

layout(location = 0) uniform float scale;
layout(location = 1) uniform int msaaSampleCount;
layout(location = 2) uniform uint useTexture;

layout(rgba8, binding = 0) uniform restrict writeonly image2D outTexture;
layout(binding = 1) uniform sampler2D srcTexture;

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

	const int srcSampleCount = 2;

	vec2 imgSampleCoords[MG_GL_MAX_SRC_SAMPLE_COUNT] = {
		centerCoord + vec2(-0.25, 0.25),
	    centerCoord + vec2(+0.25, +0.25),
	    centerCoord + vec2(+0.25, -0.25),
	    centerCoord + vec2(-0.25, +0.25)};

	vec4 color = vec4(0);
	int winding[MG_GL_MAX_SAMPLE_COUNT];

	for(int i=0; i<sampleCount; i++)
	{
		winding[i] = 0;
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
			vec4 nextColor = pathBuffer.elements[pathIndex].color;
			nextColor.rgb *= nextColor.a;

			if(useTexture != 0)
			{
				vec4 texColor = vec4(0);
				for(int sampleIndex = 0; sampleIndex<srcSampleCount; sampleIndex++)
				{
					vec2 sampleCoord = imgSampleCoords[sampleIndex];
					vec3 ph = vec3(sampleCoord.xy, 1);
					vec2 uv = (pathBuffer.elements[pathIndex].uvTransform * ph).xy;
					texColor += texture(srcTexture, uv);
				}
				texColor /= srcSampleCount;
				texColor.rgb *= texColor.a;
				nextColor *= texColor;
			}

			float coverage = 0;

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
						coverage++;
					}
				}
				winding[sampleIndex] = op.windingOffsetOrCrossRight;
			}
			coverage /= sampleCount;
			color = coverage*(color*(1-nextColor.a) + nextColor) + (1.-coverage)*color;
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

				if(op.windingOffsetOrCrossRight != 0)
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

	vec4 nextColor = pathBuffer.elements[pathIndex].color;
	nextColor.rgb *= nextColor.a;

	if(useTexture != 0)
	{
		vec4 texColor = vec4(0);
		for(int sampleIndex = 0; sampleIndex<srcSampleCount; sampleIndex++)
		{
			vec2 sampleCoord = imgSampleCoords[sampleIndex];
			vec3 ph = vec3(sampleCoord.xy, 1);
			vec2 uv = (pathBuffer.elements[pathIndex].uvTransform * ph).xy;
			texColor += texture(srcTexture, uv);
		}
		texColor /= srcSampleCount;
		texColor.rgb *= texColor.a;
		nextColor *= texColor;
	}

	float coverage = 0;
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
				coverage++;
			}
		}
	}
	coverage /= sampleCount;
	color = coverage*(color*(1-nextColor.a) + nextColor) + (1.-coverage)*color;

	imageStore(outTexture, pixelCoord, color);
}
