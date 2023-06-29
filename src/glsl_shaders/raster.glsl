
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

precision mediump float;
layout(std430) buffer;

layout(binding = 0) restrict readonly buffer pathBufferSSBO
{
	mg_gl_path elements[];
} pathBuffer;

layout(binding = 1) restrict readonly buffer segmentBufferCountSSBO
{
	int elements[];
} segmentCountBuffer;

layout(binding = 2) restrict readonly buffer segmentBufferSSBO
{
	mg_gl_segment elements[];
} segmentBuffer;

//layout(location = 0) uniform uint tileSize; // this has to be commented until it's effectively used!!
//layout(location = 0) uniform float scale;

layout(rgba8, binding = 0) uniform restrict writeonly image2D outTexture;

float ccw(vec2 a, vec2 b, vec2 c)
{
	return((b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x));
}

int side_of_segment(vec2 p, mg_gl_segment seg)
{
	int side = 0;
	if(p.y > seg.box.w || p.y <= seg.box.y)
	{
		if(p.x > seg.box.x && p.x <= seg.box.z)
		{
			if(p.y > seg.box.w)
			{
				side = (seg.config == MG_GL_TL || seg.config == MG_GL_BR)? -1 : 1;
			}
			else
			{
				side = (seg.config == MG_GL_TL || seg.config == MG_GL_BR)? 1 : -1;
			}
		}
	}
	else if(p.x > seg.box.z)
	{
		side = 1;
	}
	else if(p.x <= seg.box.x)
	{
		side = -1;
	}
	else
	{
		vec2 a, b, c;
		switch(seg.config)
		{
			case MG_GL_TL:
				a = seg.box.xy;
				b = seg.box.zw;
				break;

			case MG_GL_BR:
				a = seg.box.zw;
				b = seg.box.xy;
				break;

			case MG_GL_TR:
				a = seg.box.xw;
				b = seg.box.zy;
				break;

			case MG_GL_BL:
				a = seg.box.zy;
				b = seg.box.xw;
				break;
		}
		c = seg.hullVertex;

		if(ccw(a, b, p) < 0)
		{
			// other side of the diagonal
			side = (seg.config == MG_GL_BR || seg.config == MG_GL_TR) ? -1 : 1;
		}
		else if(ccw(b, c, p) < 0 || ccw(c, a, p) < 0)
		{
			// same side of the diagonal, but outside curve hull
			side = (seg.config == MG_GL_BL || seg.config == MG_GL_TL) ? -1 : 1;
		}
		else
		{
			// inside curve hull
			switch(seg.kind)
			{
				case MG_GL_LINE:
					side = 1;
					break;

				case MG_GL_QUADRATIC:
				{
					vec3 ph = {p.x, p.y, 1};
					vec3 klm = seg.implicitMatrix * ph;
					side = ((klm.x*klm.x - klm.y)*klm.z < 0)? -1 : 1;
				} break;

				case MG_GL_CUBIC:
				{
					vec3 ph = {p.x, p.y, 1};
					vec3 klm = seg.implicitMatrix * ph;
					side = (seg.sign*(klm.x*klm.x*klm.x - klm.y*klm.z) < 0)? -1 : 1;
				} break;
			}
		}
	}
	return(side);
}

void main()
{
	int segCount = segmentCountBuffer.elements[0];
	vec2 sampleCoord = vec2(gl_WorkGroupID.xy*uvec2(16, 16) + gl_LocalInvocationID.xy);

	int winding = 0;

	for(int segIndex=0; segIndex<segCount; segIndex++)
	{
		mg_gl_segment seg = segmentBuffer.elements[segIndex];

		if( (sampleCoord.y > seg.box.y)
		  &&(sampleCoord.y <= seg.box.w)
		  &&(side_of_segment(sampleCoord, seg) < 0))
		{
			winding += seg.windingIncrement;
		}

		/*
		if(op->crossRight)
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
		*/
	}

	int pathIndex = 0;

//	vec4 clip = pathBuffer.elements[pathIndex].clip * scale;

/*	if(  sampleCoord.x >= clip.x
	  && sampleCoord.x < clip.z
	  && sampleCoord.y >= clip.y
	  && sampleCoord.y < clip.w)
*/	{
		/*
		bool filled = (pathBuffer[pathIndex].cmd == MG_GL_FILL && (winding[sampleIndex] & 1))
		            ||(pathBuffer[pathIndex].cmd == MG_GL_STROKE && (winding[sampleIndex] != 0));
		*/
		bool filled = (winding & 1) != 0;
		if(filled)
		{
			// write to texture
			imageStore(outTexture, ivec2(sampleCoord), vec4(1, 0, 0, 1));
		}
	}
}
