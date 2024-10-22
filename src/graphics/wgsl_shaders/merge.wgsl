
//------------------------------------------------------------------------------------------------
// Merge pass
//------------------------------------------------------------------------------------------------

@group(0) @binding(0) var<storage, read> pathBuffer : array<oc_path>;
@group(0) @binding(1) var<uniform> pathCount : u32;

@group(0) @binding(2) var<storage, read> chunks : array<oc_chunk>;
@group(0) @binding(3) var<storage, read> chunkElements : array<oc_chunk_elt>;

@group(0) @binding(4) var<storage, read> pathBins : array<oc_path_bin>;
@group(0) @binding(5) var<storage, read> binQueues : array<oc_bin_queue>;
@group(0) @binding(6) var<storage, read> binQueueCount : u32;

@group(0) @binding(7) var<storage, read_write> tileOpBuffer : array<oc_tile_op>;
@group(0) @binding(8) var<storage, read_write> tileOpCount : atomic<u32>;

@group(0) @binding(9) var<storage, read_write> tileQueues : array<oc_tile_queue>;
@group(0) @binding(10) var<storage, read_write> tileCount : atomic<u32>;

@group(0) @binding(11) var<uniform> tileSize : u32;
@group(0) @binding(12) var<uniform> chunkSize : u32;

@compute @workgroup_size(1) fn merge(@builtin(num_workgroups) workGroupCount : vec3u,
                                     @builtin(global_invocation_id) gid : vec3u)
{
    let tilesPerChunk : u32 = chunkSize / u32(tileSize);
    let tileCoord : vec2u = gid.xy;
    let chunkCoord : vec2u = tileCoord / tilesPerChunk;
    let chunkCount : vec2u = (workGroupCount.xy + vec2u(tilesPerChunk - 1, tilesPerChunk - 1)) / tilesPerChunk;
    let chunkIndex : u32 = chunkCoord.y * chunkCount.x + chunkCoord.x;

    var tileIndex : i32 = -1;
    var lastOpIndex : i32 = -1;

    for(var chunkEltIndex = chunks[chunkIndex].first;
        chunkEltIndex >= 0;
        chunkEltIndex = chunkElements[chunkEltIndex].next)
    {
        let pathIndex = chunkElements[chunkEltIndex].pathIndex;
        let pathBin = pathBins[pathIndex];
        let binCoord : vec2i = vec2i(tileCoord) - pathBin.area.xy;

        let pathBox : vec4f = pathBuffer[pathIndex].box;
        let clip : vec4f = pathBuffer[pathIndex].clip;
        let xMax = min(pathBox.z, clip.z);

        let tileMax : i32 = i32(xMax) / i32(tileSize);
        let pathTileMax : i32 = tileMax - pathBin.area.x;

        if(binCoord.x >= 0
           && binCoord.x <= pathTileMax
           && binCoord.y >= 0
           && binCoord.y < pathBin.area.w)
        {
            if(tileIndex < 0)
            {
                tileIndex = i32(atomicAdd(&tileCount, 1u));
                tileQueues[tileIndex].tileCoord = tileCoord;
                tileQueues[tileIndex].first = -1;
            }

            let binQueueIndex : i32 = pathBin.binQueues + binCoord.y * pathBin.area.z + binCoord.x;
            let binQueue = &binQueues[binQueueIndex];

            let windingOffset : i32 = (*binQueue).windingOffset;
            let firstOpIndex : i32 = (*binQueue).first;

            var tileBox = vec4f(f32(tileCoord.x), f32(tileCoord.y), f32(tileCoord.x + 1), f32(tileCoord.y + 1));
            tileBox *= f32(tileSize);

            if(tileBox.x >= clip.z
               || tileBox.z < clip.x
               || tileBox.y >= clip.w
               || tileBox.w < clip.y)
            {
                //NOTE: tile is fully outside clip, cull it
                //TODO: move that test up
            }
            else if(firstOpIndex == -1)
            {
                //NOTE: This bin queue has no ops. This means the tile is fully inside or fully outside the path
                if( ((pathBuffer[pathIndex].fillRule == OC_FILL_EVEN_ODD) && ((windingOffset & 1) != 0))
                  ||((pathBuffer[pathIndex].fillRule == OC_FILL_NON_ZERO) && (windingOffset != 0)))
                {
                    //NOTE: tile is full covered. Add fill op (with winding offset).
                    //      Additionally if color is opaque and tile is fully inside clip, trim tile list.
                    let opIndex = atomicAdd(&tileOpCount, 1u);

                    if(opIndex >= arrayLength(&tileOpBuffer))
                    {
                        //TODO: signal there's not enough tile ops
                        return;
                    }

                    tileOpBuffer[opIndex].kind = OC_OP_CLIP_FILL;
                    tileOpBuffer[opIndex].next = -1;
                    tileOpBuffer[opIndex].index = pathIndex;
                    tileOpBuffer[opIndex].windingOffsetOrCrossRight = windingOffset;

                    if(lastOpIndex < 0)
                    {
                        tileQueues[tileIndex].first = i32(opIndex);
                    }
                    else
                    {
                        tileOpBuffer[lastOpIndex].next = i32(opIndex);
                    }

                    if(tileBox.x >= clip.x
                       && tileBox.z < clip.z
                       && tileBox.y >= clip.y
                       && tileBox.w < clip.w)
                    {
                        tileOpBuffer[opIndex].kind = OC_OP_FILL;

                        if(pathBuffer[pathIndex].colors[0].a == 1
                           && pathBuffer[pathIndex].textureID < 0)
                        {
                            //NOTE(martin): the tile is fully opaque, so no need to continue
                            // traversing lower-Z paths
                            return;
                        }
                    }
                    lastOpIndex = i32(opIndex);
                }
                // else, tile is fully uncovered, skip path
            }
            else
            {
                //NOTE: add path start op (with winding offset)
                let startOpIndex : u32 = atomicAdd(&tileOpCount, 1u);
                if(startOpIndex >= arrayLength(&tileOpBuffer))
                {
                    //TODO signal there's not enough tile ops
                    return;
                }

                tileOpBuffer[startOpIndex].kind = OC_OP_START;
                tileOpBuffer[startOpIndex].next = -1;
                tileOpBuffer[startOpIndex].index = pathIndex;
                tileOpBuffer[startOpIndex].windingOffsetOrCrossRight = windingOffset;

                if(lastOpIndex < 0)
                {
                    tileQueues[tileIndex].first = i32(startOpIndex);
                }
                else
                {
                    tileOpBuffer[lastOpIndex].next = i32(startOpIndex);
                }
                lastOpIndex = i32(startOpIndex);

                //NOTE: chain remaining path ops to end of tile list
                tileOpBuffer[lastOpIndex].next = firstOpIndex;
                lastOpIndex = (*binQueue).last;

                //NOTE: add path end op
                let endOpIndex : u32 = atomicAdd(&tileOpCount, 1u);
                if(endOpIndex >= arrayLength(&tileOpBuffer))
                {
                    //TODO signal there's not enough tile ops
                    return;
                }

                tileOpBuffer[endOpIndex].kind = OC_OP_END;
                tileOpBuffer[endOpIndex].next = -1;
                tileOpBuffer[endOpIndex].index = pathIndex;
                //tileOpBuffer[endOpIndex].windingOffsetOrCrossRight = windingOffset;

                if(lastOpIndex < 0)
                {
                    tileQueues[tileIndex].first = i32(endOpIndex);
                }
                else
                {
                    tileOpBuffer[lastOpIndex].next = i32(endOpIndex);
                }
                lastOpIndex = i32(endOpIndex);
            }
        }
    }
}
