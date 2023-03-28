
#include<metal_stdlib>
#include<simd/simd.h>
#include<metal_simdgroup>

#include"mtl_renderer.h"

using namespace metal;

kernel void mtl_raster(constant int* pathCount [[buffer(0)]],
                       const device mg_mtl_path* pathBuffer [[buffer(1)]],
                       constant int* segCount [[buffer(2)]],
                       const device mg_mtl_segment* segmentBuffer [[buffer(3)]],
                       texture2d<float, access::write> outTexture [[texture(0)]],
                       uint2 threadCoord [[thread_position_in_grid]])
{
	int2 pixelCoord = int2(threadCoord);

	float4 color = float4(0, 0, 0, 0);
	int currentPath = 0;
	int winding = 0;

	for(int segIndex = 0; segIndex < segCount[0]; segIndex++)
	{
		const device mg_mtl_segment* seg = &segmentBuffer[segIndex];

		if(seg->pathIndex != currentPath)
		{
			//depending on winding number, update color
			if(winding & 1)
			{
				float4 pathColor = pathBuffer[currentPath].color;
				pathColor.rgb *= pathColor.a;
				color = color*(1-pathColor.a) + pathColor;
			}
			currentPath = seg->pathIndex;
			winding = 0;
		}

		if(pixelCoord.y >= seg->box.y && pixelCoord.y < seg->box.w)
		{
			if(pixelCoord.x < seg->box.x)
			{
				winding += seg->windingIncrement;
			}
			else if(pixelCoord.x < seg->box.z)
			{
				/*TODO: if pixel is on opposite size of diagonal as curve on the right, increment
					       otherwise if not on same size of diagonal as curve, do implicit test
				*/
				float alpha = (seg->box.w - seg->box.y)/(seg->box.z - seg->box.x);
				float ofs = seg->box.w - seg->box.y;
				float dx = pixelCoord.x - seg->box.x;
				float dy = pixelCoord.y - seg->box.y;

				if( (seg->config == MG_MTL_BR && dy > alpha*dx)
				  ||(seg->config == MG_MTL_TR && dy < ofs - alpha*dx))
				{
					winding += seg->windingIncrement;
				}
				else if(  !(seg->config == MG_MTL_TL && dy < alpha*dx)
				       && !(seg->config == MG_MTL_BL && dy > ofs - alpha*dx))
				{
					//Need implicit test, but for lines, we only have config BR or TR, so the test is always negative for now
				}
			}
		}
	}

	if(winding & 1)
	{
		float4 pathColor = pathBuffer[currentPath].color;
		pathColor.rgb *= pathColor.a;
		color = color*(1-pathColor.a) + pathColor;
	}

	outTexture.write(color, uint2(pixelCoord));
}

//------------------------------------------------------------------------------------
// Blit shader
//------------------------------------------------------------------------------------
struct vs_out
{
    float4 pos [[position]];
    float2 uv;
};

vertex vs_out mtl_vertex_shader(ushort vid [[vertex_id]])
{
	vs_out out;
	out.uv = float2((vid << 1) & 2, vid & 2);
	out.pos = float4(out.uv * float2(2, -2) + float2(-1, 1), 0, 1);
	return(out);
}

fragment float4 mtl_fragment_shader(vs_out i [[stage_in]], texture2d<float> tex [[texture(0)]])
{
	constexpr sampler smp(mip_filter::nearest, mag_filter::linear, min_filter::linear);
	return(tex.sample(smp, i.uv));
}
