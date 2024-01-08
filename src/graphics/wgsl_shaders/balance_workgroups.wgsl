
//------------------------------------------------------------------------------------------------
// Balance workgroups
//------------------------------------------------------------------------------------------------

@group(0) @binding(0) var<storage, read_write> dispatchWorkGroups : array<u32, 3>;
@group(0) @binding(1) var<uniform> maxWorkGroupsPerDimension : u32;

@compute @workgroup_size(1) fn balance_workgroups()
{
    let totalWorkGroupCount : u32 = dispatchWorkGroups[0];

    if(totalWorkGroupCount > maxWorkGroupsPerDimension)
    {
        dispatchWorkGroups[0] = maxWorkGroupsPerDimension;
    }
    else
    {
        dispatchWorkGroups[0] = totalWorkGroupCount;
    }
    dispatchWorkGroups[1] = (totalWorkGroupCount + maxWorkGroupsPerDimension - 1) / maxWorkGroupsPerDimension;
    dispatchWorkGroups[2] = 1;
}
