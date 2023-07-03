
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

layout(rgba8, binding = 0) uniform restrict writeonly image2D outTexture;

void main()
{
	uvec2 nTiles = gl_NumWorkGroups.xy;
	uvec2 tileCoord = gl_WorkGroupID.xy;
	uint tileIndex =  tileCoord.y * nTiles.x + tileCoord.x;

	ivec2 pixelCoord = ivec2(gl_WorkGroupID.xy*uvec2(16, 16) + gl_LocalInvocationID.xy);
	vec2 sampleCoord = vec2(pixelCoord);

	int pathIndex = 0;
	int opIndex = screenTilesBuffer.elements[tileIndex];
	int winding = 0;
	vec4 color = vec4(0);

	if((pixelCoord.x % 16) == 0 || (pixelCoord.y % 16) == 0)
	{
		imageStore(outTexture, ivec2(sampleCoord), vec4(0, 0, 0, 1));
		return;
	}

	while(opIndex >= 0)
	{
		mg_gl_tile_op op = tileOpBuffer.elements[opIndex];
		opIndex = op.next;

		if(op.kind == MG_GL_OP_START)
		{
			vec4 pathColor = pathBuffer.elements[pathIndex].color;
			pathColor.rgb *= pathColor.a;

			vec4 clip = pathBuffer.elements[pathIndex].clip * scale;

			if(  sampleCoord.x >= clip.x
			  && sampleCoord.x < clip.z
			  && sampleCoord.y >= clip.y
			  && sampleCoord.y < clip.w)
			{
				bool filled = (pathBuffer.elements[pathIndex].cmd == MG_GL_FILL && ((winding & 1) != 0))
				            ||(pathBuffer.elements[pathIndex].cmd == MG_GL_STROKE && (winding != 0));
				if(filled)
				{
					vec4 nextColor = pathColor;
					color = color*(1-nextColor.a) + nextColor;
				}
				winding = op.windingOffset;
			}
			pathIndex = op.index;
		}
		else if(op.kind == MG_GL_OP_SEGMENT)
		{
			int segIndex = op.index;
			mg_gl_segment seg = segmentBuffer.elements[segIndex];

			if( (sampleCoord.y > seg.box.y)
			  &&(sampleCoord.y <= seg.box.w)
			  &&(side_of_segment(sampleCoord, seg) < 0))
			{
				winding += seg.windingIncrement;
			}

			if(op.crossRight)
			{
				if( (seg.config == MG_GL_BR || seg.config == MG_GL_TL)
				  &&(sampleCoord.y > seg.box.w))
				{
					winding += seg.windingIncrement;
				}
				else if( (seg.config == MG_GL_BL || seg.config == MG_GL_TR)
				       &&(sampleCoord.y > seg.box.y))
				{
					winding -= seg.windingIncrement;
				}
			}
		}
	}

	vec4 pathColor = pathBuffer.elements[pathIndex].color;
	pathColor.rgb *= pathColor.a;

	vec4 clip = pathBuffer.elements[pathIndex].clip * scale;

	if(  sampleCoord.x >= clip.x
	  && sampleCoord.x < clip.z
	  && sampleCoord.y >= clip.y
	  && sampleCoord.y < clip.w)
	{
		bool filled = (pathBuffer.elements[pathIndex].cmd == MG_GL_FILL && ((winding & 1) != 0))
		            ||(pathBuffer.elements[pathIndex].cmd == MG_GL_STROKE && (winding != 0));
		if(filled)
		{
			vec4 nextColor = pathColor;
			color = color*(1-nextColor.a) + nextColor;
		}
	}
	// write to texture
	imageStore(outTexture, ivec2(sampleCoord), color);
}
