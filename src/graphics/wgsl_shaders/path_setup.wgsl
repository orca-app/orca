
//------------------------------------------------------------------------------------------------
// Path setup
//------------------------------------------------------------------------------------------------

@group(0) @binding(0) var<storage, read> pathBuffer : array<oc_path>;
@group(0) @binding(1) var<uniform> pathCount : u32;
@group(0) @binding(2) var<storage, read_write> pathBins : array<oc_path_bin>;
@group(0) @binding(3) var<storage, read_write> binQueueBuffer : array<oc_bin_queue>;
@group(0) @binding(4) var<storage, read_write> binQueueCount : atomic<u32>;
@group(0) @binding(5) var<uniform> tileSize : i32;

@compute @workgroup_size(16, 16) fn path_setup(@builtin(num_workgroups) workGroupCount : vec3u,
                                               @builtin(workgroup_id) workGroupID : vec3u,
                                               @builtin(local_invocation_id) localID : vec3u)
{
    let invocationsPerWorkGroups : u32 = 16*16;
    let workGroupIndex : u32 = workGroupID.y*workGroupCount.x + workGroupID.x;
    let pathIndex : u32 = workGroupIndex * invocationsPerWorkGroups + localID.y*16 + localID.x;

    if(pathIndex >= pathCount)
    {
        return;
    }

    let path = pathBuffer[pathIndex];

    //NOTE: we don't clip on the right, since we need those tiles to accurately compute
    //      the prefix sum of winding increments in the backprop pass.

    let clippedBox = vec4f(max(path.box.x, path.clip.x),
                           max(path.box.y, path.clip.y),
                           path.box.z,
                           min(path.box.w, path.clip.w));

    let firstTile = vec2i(clippedBox.xy) / tileSize;
    let lastTile = vec2i(clippedBox.zw) / tileSize;

    let nTilesX : i32 = max(0, lastTile.x - firstTile.x + 1);
    let nTilesY : i32 = max(0, lastTile.y - firstTile.y + 1);
    let tileCount : u32 = u32(nTilesX * nTilesY);

    let binQueuesIndex : u32 = atomicAdd(&binQueueCount, tileCount);

    if(binQueuesIndex + tileCount >= arrayLength(&binQueueBuffer))
    {
        pathBins[pathIndex].area = vec4i(0, 0, 0, 0);
        pathBins[pathIndex].binQueues = -1;
    }
    else
    {
        pathBins[pathIndex].area = vec4i(firstTile.x, firstTile.y, nTilesX, nTilesY);
        pathBins[pathIndex].binQueues = i32(binQueuesIndex);

        for(var i : u32 = 0; i < tileCount; i++)
        {
            binQueueBuffer[binQueuesIndex + i].first = -1;
            binQueueBuffer[binQueuesIndex + i].last = -1;
            binQueueBuffer[binQueuesIndex + i].windingOffset = 0;
        }
    }
}
