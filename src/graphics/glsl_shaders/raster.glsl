
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

precision mediump float;
layout(std430) buffer;

layout(binding = 0) restrict readonly buffer pathBufferSSBO
{
    oc_gl_path elements[];
}

pathBuffer;

layout(binding = 1) restrict readonly buffer segmentBufferSSBO
{
    oc_gl_segment elements[];
}

segmentBuffer;

layout(binding = 2) restrict readonly buffer tileOpBufferSSBO
{
    oc_gl_tile_op elements[];
}

tileOpBuffer;

layout(binding = 3) restrict readonly buffer screenTilesBufferSSBO
{
    oc_gl_screen_tile elements[];
}

screenTilesBuffer;

layout(binding = 4) restrict readonly buffer screenTilesCountBufferSSBO
{
    int elements[];
}

screenTilesCountBuffer;

layout(location = 0) uniform float scale;
layout(location = 1) uniform int msaaSampleCount;
layout(location = 2) uniform int pathBufferStart;
layout(location = 3) uniform uint maxWorkGroupCount;

layout(rgba8, binding = 0) uniform restrict writeonly image2D outTexture;

layout(binding = 1) uniform sampler2D srcTexture0;
layout(binding = 2) uniform sampler2D srcTexture1;
layout(binding = 3) uniform sampler2D srcTexture2;
layout(binding = 4) uniform sampler2D srcTexture3;
layout(binding = 5) uniform sampler2D srcTexture4;
layout(binding = 6) uniform sampler2D srcTexture5;
layout(binding = 7) uniform sampler2D srcTexture6;
layout(binding = 8) uniform sampler2D srcTexture7;

void main()
{
    uint tileIndex = gl_WorkGroupID.y * maxWorkGroupCount + gl_WorkGroupID.x;

    if(tileIndex >= screenTilesCountBuffer.elements[0])
    {
        return;
    }

    uvec2 tileCoord = screenTilesBuffer.elements[tileIndex].tileCoord;
    ivec2 pixelCoord = ivec2(tileCoord * gl_WorkGroupSize.x + gl_LocalInvocationID.xy);

    vec2 centerCoord = vec2(pixelCoord) + vec2(0.5, 0.5);

    /*
	if((pixelCoord.x % 16) == 0 || (pixelCoord.y % 16) == 0)
	{
		imageStore(outTexture, pixelCoord, vec4(0, 0, 0, 1));
		return;
	}
*/
    vec2 sampleCoords[OC_GL_MAX_SAMPLE_COUNT] = {
        centerCoord + vec2(1, 3) / 16,
        centerCoord + vec2(-1, -3) / 16,
        centerCoord + vec2(5, -1) / 16,
        centerCoord + vec2(-3, 5) / 16,
        centerCoord + vec2(-5, -5) / 16,
        centerCoord + vec2(-7, 1) / 16,
        centerCoord + vec2(3, -7) / 16,
        centerCoord + vec2(7, 7) / 16
    };

    int sampleCount = msaaSampleCount;
    if(sampleCount != 8)
    {
        sampleCount = 1;
        sampleCoords[0] = centerCoord;
    }

    const int srcSampleCount = 2;

    vec2 imgSampleCoords[OC_GL_MAX_SRC_SAMPLE_COUNT] = {
        centerCoord + vec2(-0.25, 0.25),
        centerCoord + vec2(+0.25, +0.25),
        centerCoord + vec2(+0.25, -0.25),
        centerCoord + vec2(-0.25, +0.25)
    };

    vec4 color = vec4(0);
    int winding[OC_GL_MAX_SAMPLE_COUNT];

    for(int i = 0; i < sampleCount; i++)
    {
        winding[i] = 0;
    }

    int pathIndex = 0;
    int opIndex = screenTilesBuffer.elements[tileIndex].first;

    while(opIndex >= 0)
    {
        oc_gl_tile_op op = tileOpBuffer.elements[opIndex];

        if(op.kind == OC_GL_OP_START)
        {
            for(int sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
            {
                winding[sampleIndex] = op.windingOffsetOrCrossRight;
            }
        }
        else if(op.kind == OC_GL_OP_SEGMENT)
        {
            int segIndex = op.index;
            oc_gl_segment seg = segmentBuffer.elements[segIndex];

            for(int sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
            {
                vec2 sampleCoord = sampleCoords[sampleIndex];

                if((sampleCoord.y > seg.box.y)
                   && (sampleCoord.y <= seg.box.w)
                   && (side_of_segment(sampleCoord, seg) < 0))
                {
                    winding[sampleIndex] += seg.windingIncrement;
                }

                if(op.windingOffsetOrCrossRight != 0)
                {
                    if((seg.config == OC_GL_BR || seg.config == OC_GL_TL)
                       && (sampleCoord.y > seg.box.w))
                    {
                        winding[sampleIndex] += seg.windingIncrement;
                    }
                    else if((seg.config == OC_GL_BL || seg.config == OC_GL_TR)
                            && (sampleCoord.y > seg.box.y))
                    {
                        winding[sampleIndex] -= seg.windingIncrement;
                    }
                }
            }
        }
        else
        {
            int pathIndex = op.index;

            vec4 nextColor = pathBuffer.elements[pathBufferStart + pathIndex].color;
            nextColor.rgb *= nextColor.a;

            int textureID = pathBuffer.elements[pathBufferStart + pathIndex].textureID;
            if(textureID >= 0)
            {
                vec4 texColor = vec4(0);

                for(int sampleIndex = 0; sampleIndex < srcSampleCount; sampleIndex++)
                {
                    vec2 sampleCoord = imgSampleCoords[sampleIndex];
                    vec3 ph = vec3(sampleCoord.xy, 1);
                    vec2 uv = (pathBuffer.elements[pathBufferStart + pathIndex].uvTransform * ph).xy;

                    if(textureID == 0)
                    {
                        texColor += texture(srcTexture0, uv);
                    }
                    else if(textureID == 1)
                    {
                        texColor += texture(srcTexture1, uv);
                    }
                    else if(textureID == 2)
                    {
                        texColor += texture(srcTexture2, uv);
                    }
                    else if(textureID == 3)
                    {
                        texColor += texture(srcTexture3, uv);
                    }
                    else if(textureID == 4)
                    {
                        texColor += texture(srcTexture4, uv);
                    }
                    else if(textureID == 5)
                    {
                        texColor += texture(srcTexture5, uv);
                    }
                    else if(textureID == 6)
                    {
                        texColor += texture(srcTexture6, uv);
                    }
                    else if(textureID == 7)
                    {
                        texColor += texture(srcTexture7, uv);
                    }
                }
                texColor /= srcSampleCount;
                texColor.rgb *= texColor.a;
                nextColor *= texColor;
            }

            if(op.kind == OC_GL_OP_FILL)
            {
                color = color * (1 - nextColor.a) + nextColor;
            }
            else
            {
                vec4 clip = pathBuffer.elements[pathBufferStart + pathIndex].clip * scale;
                float coverage = 0;

                for(int sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
                {
                    vec2 sampleCoord = sampleCoords[sampleIndex];

                    if(sampleCoord.x >= clip.x
                       && sampleCoord.x < clip.z
                       && sampleCoord.y >= clip.y
                       && sampleCoord.y < clip.w)
                    {
                        bool filled = op.kind == OC_GL_OP_CLIP_FILL
                                   || (pathBuffer.elements[pathBufferStart + pathIndex].cmd == OC_GL_FILL
                                       && ((winding[sampleIndex] & 1) != 0))
                                   || (pathBuffer.elements[pathBufferStart + pathIndex].cmd == OC_GL_STROKE
                                       && (winding[sampleIndex] != 0));
                        if(filled)
                        {
                            coverage++;
                        }
                    }
                    winding[sampleIndex] = op.windingOffsetOrCrossRight;
                }
                coverage /= sampleCount;
                color = coverage * (color * (1 - nextColor.a) + nextColor) + (1. - coverage) * color;
            }
        }
        opIndex = op.next;
    }

    imageStore(outTexture, pixelCoord, color);
}
