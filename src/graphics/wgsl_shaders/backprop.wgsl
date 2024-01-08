
//------------------------------------------------------------------------------------------------
// Backprop pass
//------------------------------------------------------------------------------------------------

@group(0) @binding(0) var<storage, read> pathBins : array<oc_path_bin>;
@group(0) @binding(1) var<uniform> pathCount : u32;
@group(0) @binding(2) var<storage, read_write> binQueues : array<oc_bin_queue>;

var<workgroup> nextRowIndex : atomic<i32>;

@compute @workgroup_size(16) fn backprop(@builtin(num_workgroups) workGroupCount : vec3u,
                                         @builtin(workgroup_id) workGroupID : vec3u)
{
    atomicStore(&nextRowIndex, 0);
    workgroupBarrier();

    let pathIndex : u32 = workGroupID.y * workGroupCount.x + workGroupID.x;
    if(pathIndex >= pathCount)
    {
        return;
    }

    let pathBin = &pathBins[pathIndex];

    let rowSize : i32 = (*pathBin).area.z;
    let rowCount : i32 = (*pathBin).area.w;
    let binQueueStartIndex = (*pathBin).binQueues;

    var rowIndex : i32 = atomicAdd(&nextRowIndex, 1);

    while(rowIndex < rowCount)
    {
        var sum : i32 = 0;
        for(var x : i32 = rowSize - 1; x >= 0; x--)
        {
            let tileIndex : i32 = binQueueStartIndex + rowIndex * rowSize + x;
            let offset : i32 = binQueues[tileIndex].windingOffset;
            binQueues[tileIndex].windingOffset = sum;
            sum += offset;
        }
        rowIndex = atomicAdd(&nextRowIndex, 1);
    }
}
