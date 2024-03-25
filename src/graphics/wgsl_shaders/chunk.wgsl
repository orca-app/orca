
//------------------------------------------------------------------------------------------------
// Chunk pass
//------------------------------------------------------------------------------------------------

@group(0) @binding(0) var<storage, read> paths : array<oc_path>;
@group(0) @binding(1) var<uniform> pathCount : u32;
@group(0) @binding(2) var<storage, read_write> chunks : array<oc_chunk>;
@group(0) @binding(3) var<storage, read_write> chunkElements : array<oc_chunk_elt>;
@group(0) @binding(4) var<storage, read_write> chunkEltCount : atomic<u32>;
@group(0) @binding(5) var<uniform> chunkSize : u32;

@compute @workgroup_size(1) fn chunk(@builtin(num_workgroups) workGroupCount : vec3u,
                                     @builtin(workgroup_id) workGroupID : vec3u)
{
    let chunkBoxMin = vec2f(workGroupID.xy * chunkSize);
    let chunkBoxMax = chunkBoxMin + vec2f(f32(chunkSize), f32(chunkSize));

    let chunkIndex = workGroupID.y * workGroupCount.x + workGroupID.x;
    let chunk = &chunks[chunkIndex];
    (*chunk).first = -1;
    (*chunk).last = -1; // could be local only

    for(var pathIndex : u32 = 0; pathIndex < pathCount ; pathIndex++)
    {
        let pathBox : vec4f = paths[pathIndex].box;

        if(  pathBox.x < chunkBoxMax.x
          && pathBox.y < chunkBoxMax.y
          && pathBox.z >= chunkBoxMin.x
          && pathBox.w >= chunkBoxMin.y)
        {
            var eltIndex : u32 = atomicAdd(&chunkEltCount, 1u);
            let elt = &chunkElements[eltIndex];

            (*elt).pathIndex = pathIndex;

            if((*chunk).first < 0)
            {
                (*chunk).last = i32(eltIndex);
            }
            (*elt).next = (*chunk).first;
            (*chunk).first = i32(eltIndex);
        }
    }
}
