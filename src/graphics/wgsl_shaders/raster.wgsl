//------------------------------------------------------------------------------------------------
// Raster
//------------------------------------------------------------------------------------------------

const OC_WGPU_MAX_SAMPLE_COUNT : u32 = 8;
const OC_WGPU_SOURCE_SAMPLE_COUNT : u32 = 2;


@group(0) @binding(0) var<storage, read> pathBuffer : array<oc_path>;
@group(0) @binding(1) var<storage, read> segmentBuffer : array<oc_segment>;
@group(0) @binding(2) var<storage, read> tileQueues : array<oc_tile_queue>;
@group(0) @binding(3) var<storage, read> tileOpBuffer : array<oc_tile_op>;
@group(0) @binding(4) var<storage, read> msaaOffsets : array<vec2f, OC_WGPU_MAX_SAMPLE_COUNT>;
@group(0) @binding(5) var<uniform> msaaSampleCount : u32;
@group(0) @binding(6) var<uniform> tileSize : u32;
@group(0) @binding(7) var outTexture : texture_storage_2d<rgba8unorm, write>;
@group(0) @binding(8) var<uniform> debugDisplayOptions : oc_debug_display_options;

// source textures bindgroup
@group(1) @binding(0) var srcTexture0 : texture_2d<f32>;
@group(1) @binding(1) var srcTexture1 : texture_2d<f32>;
@group(1) @binding(2) var srcTexture2 : texture_2d<f32>;
@group(1) @binding(3) var srcTexture3 : texture_2d<f32>;
@group(1) @binding(4) var srcTexture4 : texture_2d<f32>;
@group(1) @binding(5) var srcTexture5 : texture_2d<f32>;
@group(1) @binding(6) var srcTexture6 : texture_2d<f32>;
@group(1) @binding(7) var srcTexture7 : texture_2d<f32>;


//NOTE: you can't sample from textures in a compute shaders (wtf?), so... we do it ourselves
fn sampleFromTexture(texture : texture_2d<f32>, uv : vec2f) -> vec4f
{
    var color : vec4f;
    if(uv.x < 0 || uv.y < 0 || uv.x > 1 || uv.y > 1)
    {
        color = vec4f(0, 0, 0, 0);
    }
    else
    {
        let p : vec2f = uv * vec2f(textureDimensions(texture));
        let tl = vec2i(p);
        let bl = tl + vec2i(0, 1);
        let tr = tl + vec2i(1, 0);
        let br = tl + vec2i(1, 1);

        let tlColor : vec4f = textureLoad(texture, tl, 0);
        let blColor : vec4f = textureLoad(texture, bl, 0);
        let trColor : vec4f = textureLoad(texture, tr, 0);
        let brColor : vec4f = textureLoad(texture, br, 0);

        let lColor : vec4f = 0.5*(tlColor + blColor);
        let rColor : vec4f = 0.5*(trColor + brColor);
        color = 0.5*(lColor + rColor);
    }
    return(color);
}

const DEBUG_COLOR_QUEUE_EMPTY = vec4f(1, 0, 1, 1);
const DEBUG_COLOR_QUEUE_NOT_EMPTY = vec4f(0, 1, 0, 1);
const DEBUG_COLOR_FILL_OP = vec4f(1, 0, 0, 1);

fn get_next_color(pathIndex : u32,
                  sourceSampleCoords : array<vec2f, OC_WGPU_SOURCE_SAMPLE_COUNT>) -> vec4f
{
    var nextColor : vec4f = pathBuffer[pathIndex].color;
    nextColor = vec4f(nextColor.rgb * nextColor.a, nextColor.a);

    let textureID : i32 = pathBuffer[pathIndex].textureID;
    if(  textureID >= 0
      && debugDisplayOptions.textureOff == 0)
    {
        var texColor = vec4f(0, 0, 0, 0);

        for(var sampleIndex : u32 = 0; sampleIndex < OC_WGPU_SOURCE_SAMPLE_COUNT; sampleIndex++)
        {
            let sampleCoord = vec3f(sourceSampleCoords[sampleIndex], 1);
            let uv : vec2f = (pathBuffer[pathIndex].uvTransform * sampleCoord).xy;

            if(textureID == 0)
            {
                texColor += sampleFromTexture(srcTexture0, uv);
            }
            else if(textureID == 1)
            {
                texColor += sampleFromTexture(srcTexture1, uv);
            }
            else if(textureID == 2)
            {
                texColor += sampleFromTexture(srcTexture2, uv);
            }
            else if(textureID == 3)
            {
                texColor += sampleFromTexture(srcTexture3, uv);
            }
            else if(textureID == 4)
            {
                texColor += sampleFromTexture(srcTexture4, uv);
            }
            else if(textureID == 5)
            {
                texColor += sampleFromTexture(srcTexture5, uv);
            }
            else if(textureID == 6)
            {
                texColor += sampleFromTexture(srcTexture6, uv);
            }
            else if(textureID == 7)
            {
                texColor += sampleFromTexture(srcTexture7, uv);
            }
        }
        texColor /= f32(OC_WGPU_SOURCE_SAMPLE_COUNT);
        texColor = vec4f(texColor.rgb*texColor.a, texColor.a);
        nextColor *= texColor;
    }
    return(nextColor);
}

@compute @workgroup_size(16, 16) fn raster(@builtin(num_workgroups) workGroupCount : vec3u,
                                           @builtin(workgroup_id) workGroupID : vec3u,
                                           @builtin(local_invocation_id) localID : vec3u)
{
    let tileQueueIndex : u32 = workGroupID.y * workGroupCount.x + workGroupID.x;
    let tileQueue = &tileQueues[tileQueueIndex];
    let pixCoord = vec2i((*tileQueue).tileCoord * tileSize + localID.xy);
    let centerCoord = vec2f(pixCoord) + vec2f(0.5, 0.5);

    let sourceSampleCoords = array<vec2f, OC_WGPU_SOURCE_SAMPLE_COUNT>(
        centerCoord + vec2f(0.5, 0.5),
        centerCoord + vec2f(-0.5, -0.5),
    );

    var sampleCoords : array<vec2f, OC_WGPU_MAX_SAMPLE_COUNT>;
    for(var sampleIndex : u32 = 0; sampleIndex < msaaSampleCount; sampleIndex++)
    {
        sampleCoords[sampleIndex] = centerCoord + msaaOffsets[sampleIndex];
    }

    var color = vec4f(0, 0, 0, 0);

    /*
    if(debugDisplayOptions.showPathArea != 0)
    {
        color = vec4f(0, 0, 0, 0.1);
    }
    */

    var opIndex : i32 = (*tileQueue).first;
    var winding = array<i32, OC_WGPU_MAX_SAMPLE_COUNT>(
        0, 0, 0, 0, 0, 0, 0, 0
    );

    /*
    if(debugDisplayOptions.debugTileQueues != 0)
    {
        if(opIndex >= 0)
        {
            color = DEBUG_COLOR_QUEUE_NOT_EMPTY;
        }
        else
        {
            color = DEBUG_COLOR_QUEUE_EMPTY;
        }
    }
    */

    while(opIndex >= 0)
    {
        var op : oc_tile_op = tileOpBuffer[opIndex];

        if(op.kind == OC_OP_START)
        {
            for(var sampleIndex : u32 = 0; sampleIndex < msaaSampleCount; sampleIndex++)
            {
                winding[sampleIndex] = op.windingOffsetOrCrossRight;
            }
        }
        else if(op.kind == OC_OP_SEGMENT)
        {
            var seg : oc_segment = segmentBuffer[op.index];

            for(var sampleIndex : u32 = 0; sampleIndex < msaaSampleCount; sampleIndex++)
            {
                let samplePos : vec2f = sampleCoords[sampleIndex];

                if((samplePos.y > seg.box.y)
                   && (samplePos.y <= seg.box.w)
                   && (side_of_segment(samplePos.xy, seg) < 0))
                {
                    winding[sampleIndex] += seg.windingIncrement;
                }

                if(op.windingOffsetOrCrossRight != 0)
                {
                    if((seg.config == OC_SEG_BR || seg.config == OC_SEG_TL)
                       && (samplePos.y > seg.box.w))
                    {
                        winding[sampleIndex] += seg.windingIncrement;
                    }
                    else if((seg.config == OC_SEG_BL || seg.config == OC_SEG_TR)
                            && (samplePos.y > seg.box.y))
                    {
                        winding[sampleIndex] -= seg.windingIncrement;
                    }
                }
            }
        }
        else
        {
            let pathIndex : u32 = op.index;
            let nextColor : vec4f = get_next_color(pathIndex, sourceSampleCoords);

            if(op.kind == OC_OP_FILL)
            {
                color = nextColor * (1 - color.a) + color;

                // if(debugDisplayOptions.debugTileQueues != 0)
                // {
                //     color = DEBUG_COLOR_FILL_OP;
                // }
            }
            else
            {
                let clip : vec4f = pathBuffer[pathIndex].clip;
                var coverage : f32 = 0;

                for(var sampleIndex : u32 = 0; sampleIndex < msaaSampleCount; sampleIndex++)
                {
                    let sampleCoord : vec2f = sampleCoords[sampleIndex];

                    //TODO: do that test when computing winding number?
                    if(sampleCoord.x >= clip.x
                      && sampleCoord.x < clip.z
                      && sampleCoord.y >= clip.y
                      && sampleCoord.y < clip.w)
                    {
                        var filled : bool = (op.kind == OC_OP_CLIP_FILL)
                                          || (pathBuffer[pathIndex].cmd == OC_CMD_FILL
                                              && ((winding[sampleIndex] & 1) != 0))
                                          || (pathBuffer[pathIndex].cmd == OC_CMD_STROKE
                                              && (winding[sampleIndex] != 0));
                        if(filled)
                        {
                            coverage += 1;
                        }
                    }
                }
                if(coverage != 0)
                {
                    coverage /= f32(msaaSampleCount);
                    color = coverage*nextColor * (1 - color.a) + color;

                    if(coverage == 1 && nextColor.a == 1)
                    {
                        break;
                    }
                }

                // if(debugDisplayOptions.showClip != 0)
                // {
                //     var coverage : f32 = 0;

                //     for(var sampleIndex : u32 = 0; sampleIndex < msaaSampleCount; sampleIndex++)
                //     {
                //         let sampleCoord : vec2f = sampleCoords[sampleIndex];

                //         if(  sampleCoord.x > clip.x - 0.5
                //           && sampleCoord.x < clip.z + 0.5
                //           && sampleCoord.y > clip.y - 0.5
                //           && sampleCoord.y < clip.w + 0.5)
                //         {
                //             if(  sampleCoord.x < clip.x + 0.5
                //               || sampleCoord.x > clip.z - 0.5
                //               || sampleCoord.y < clip.y + 0.5
                //               || sampleCoord.y > clip.w - 0.5)
                //             {
                //                 coverage +=1;
                //             }
                //         }
                //     }
                //     coverage /= f32(msaaSampleCount);
                //     color = coverage * vec4f(1,0,1,1) + (1 - coverage) * color;
                // }
            }
        }
        opIndex = op.next;
    }

    // if(  debugDisplayOptions.showTileBorders != 0
    //   && (pixCoord.x % i32(tileSize) == 0 || pixCoord.y % i32(tileSize) == 0))
    // {
    //     color = 0.5 * color + vec4f(0, 0, 0, 0.5);
    // }

    textureStore(outTexture, pixCoord, color);
}
