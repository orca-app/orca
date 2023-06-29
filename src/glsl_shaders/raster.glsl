
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

precision mediump float;
layout(std430) buffer;

layout(binding = 0) restrict readonly buffer pathBufferSSBO
{
	mg_gl_path elements[];
} pathBuffer;

layout(binding = 1) restrict readonly buffer elementBufferSSBO
{
	mg_gl_path_elt elements[];
} elementBuffer;

//layout(location = 0) uniform uint tileSize; // this has to be commented until it's effectively used!!
layout(location = 1) uniform float scale;
layout(location = 2) uniform uint eltCount;

layout(rgba8, binding = 0) uniform restrict writeonly image2D outTexture;

void init_segment(in vec2 p[4], int kind, out mg_gl_segment seg)
{
	vec2 s = p[0];
	vec2 c = p[0];
	vec2 e = p[1];

	bool goingUp = e.y >= s.y;
	bool goingRight = e.x >= s.x;

	seg.kind = kind;
	seg.pathIndex = 0; ///
	seg.windingIncrement = goingUp ? 1 : -1;
	seg.box = vec4(min(s.x, e.x),
	               min(s.y, e.y),
	               max(s.x, e.x),
	               max(s.y, e.y));

	float dx = c.x - seg.box.x;
	float dy = c.y - seg.box.y;
	float alpha = (seg.box.w - seg.box.y)/(seg.box.z - seg.box.x);
	float ofs = seg.box.w - seg.box.y;

	if(goingUp == goingRight)
	{
		if(seg.kind == MG_GL_LINE)
		{
			seg.config = MG_GL_BR;
		}
		else if(dy > alpha*dx)
		{
			seg.config = MG_GL_TL;
		}
		else
		{
			seg.config = MG_GL_BR;
		}
	}
	else
	{
		if(seg.kind == MG_GL_LINE)
		{
			seg.config = MG_GL_TR;
		}
		else if(dy < ofs - alpha*dx)
		{
			seg.config = MG_GL_BL;
		}
		else
		{
			seg.config = MG_GL_TR;
		}
	}
}

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
	vec2 sampleCoord = vec2(gl_WorkGroupID.xy*uvec2(16, 16) + gl_LocalInvocationID.xy);

	int winding = 0;

	for(int i=0; i<eltCount; i++)
	{
		mg_gl_path_elt elt = elementBuffer.elements[i];

		switch(elt.kind)
		{
			case MG_GL_LINE:
			{
				vec2 p[4] = {elt.p[0]*scale, elt.p[1]*scale, vec2(0), vec2(0)};
				mg_gl_segment seg;
				init_segment(p, MG_GL_LINE, seg);
				seg.hullVertex = p[0];

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

			} break;

			case MG_GL_QUADRATIC:
			case MG_GL_CUBIC:
				break;
		}
	}

	int pathIndex = 0;
	vec4 clip = pathBuffer.elements[pathIndex].clip * scale;

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
