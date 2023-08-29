/************************************************************/ /**
*
*	@file: mtl_canvas.m
*	@author: Martin Fouilleul
*	@date: 12/07/2020
*	@revision: 24/01/2023
*
*****************************************************************/
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include <simd/simd.h>

#include "app/osx_app.h"
#include "graphics_surface.h"
#include "util/macros.h"

#include "mtl_renderer.h"

const int OC_MTL_INPUT_BUFFERS_COUNT = 3,
          OC_MTL_TILE_SIZE = 16,
          OC_MTL_MSAA_COUNT = 8;

typedef struct oc_mtl_canvas_backend
{
    oc_canvas_backend interface;
    oc_mtl_surface* surface;

    id<MTLComputePipelineState> pathPipeline;
    id<MTLComputePipelineState> segmentPipeline;
    id<MTLComputePipelineState> backpropPipeline;
    id<MTLComputePipelineState> mergePipeline;
    id<MTLComputePipelineState> rasterPipeline;
    id<MTLRenderPipelineState> blitPipeline;

    id<MTLTexture> outTexture;

    int bufferIndex;
    dispatch_semaphore_t bufferSemaphore;

    id<MTLBuffer> pathBuffer[OC_MTL_INPUT_BUFFERS_COUNT];
    id<MTLBuffer> elementBuffer[OC_MTL_INPUT_BUFFERS_COUNT];
    id<MTLBuffer> logBuffer[OC_MTL_INPUT_BUFFERS_COUNT];
    id<MTLBuffer> logOffsetBuffer[OC_MTL_INPUT_BUFFERS_COUNT];

    id<MTLBuffer> segmentCountBuffer;
    id<MTLBuffer> segmentBuffer;
    id<MTLBuffer> pathQueueBuffer;
    id<MTLBuffer> tileQueueBuffer;
    id<MTLBuffer> tileQueueCountBuffer;
    id<MTLBuffer> tileOpBuffer;
    id<MTLBuffer> tileOpCountBuffer;
    id<MTLBuffer> screenTilesBuffer;
    id<MTLBuffer> rasterDispatchBuffer;

    int msaaCount;
    oc_vec2 frameSize;

    // encoding context
    int eltCap;
    int eltCount;
    int eltBatchStart;

    int pathCap;
    int pathCount;
    int pathBatchStart;

    oc_primitive* primitive;
    oc_vec4 pathScreenExtents;
    oc_vec4 pathUserExtents;

    int maxTileQueueCount;
    int maxSegmentCount;

    int currentImageIndex;

} oc_mtl_canvas_backend;

typedef struct oc_mtl_image_data
{
    oc_image_data interface;
    id<MTLTexture> texture;
} oc_mtl_image_data;

void oc_mtl_print_log(int bufferIndex, id<MTLBuffer> logBuffer, id<MTLBuffer> logOffsetBuffer)
{
    char* log = [logBuffer contents];
    int size = *(int*)[logOffsetBuffer contents];

    if(size)
    {
        oc_log_info("Log from buffer %i:\n", bufferIndex);

        int index = 0;
        while(index < size)
        {
            int len = strlen(log + index);
            printf("%s", log + index);
            index += (len + 1);
        }
    }
}

static void oc_mtl_update_path_extents(oc_vec4* extents, oc_vec2 p)
{
    extents->x = oc_min(extents->x, p.x);
    extents->y = oc_min(extents->y, p.y);
    extents->z = oc_max(extents->z, p.x);
    extents->w = oc_max(extents->w, p.y);
}

id<MTLBuffer> oc_mtl_grow_input_buffer(id<MTLDevice> device, id<MTLBuffer> oldBuffer, int oldCopySize, int newSize)
{
    @autoreleasepool
    {
        MTLResourceOptions bufferOptions = MTLResourceCPUCacheModeWriteCombined
                                         | MTLResourceStorageModeShared;

        id<MTLBuffer> newBuffer = [device newBufferWithLength:newSize options:bufferOptions];

        memcpy([newBuffer contents], [oldBuffer contents], oldCopySize);

        [oldBuffer release];
        return (newBuffer);
    }
}

void oc_mtl_canvas_encode_element(oc_mtl_canvas_backend* backend, oc_path_elt_type kind, oc_vec2* p)
{
    int bufferIndex = backend->bufferIndex;
    int bufferCap = [backend->elementBuffer[bufferIndex] length] / sizeof(oc_mtl_path_elt);
    if(backend->eltCount >= bufferCap)
    {
        int newBufferCap = (int)(bufferCap * 1.5);
        int newBufferSize = newBufferCap * sizeof(oc_mtl_path_elt);

        oc_log_info("growing element buffer to %i elements\n", newBufferCap);

        backend->elementBuffer[bufferIndex] = oc_mtl_grow_input_buffer(backend->surface->device,
                                                                       backend->elementBuffer[bufferIndex],
                                                                       backend->eltCount * sizeof(oc_mtl_path_elt),
                                                                       newBufferSize);
    }

    oc_mtl_path_elt* elements = (oc_mtl_path_elt*)[backend->elementBuffer[bufferIndex] contents];
    oc_mtl_path_elt* elt = &elements[backend->eltCount];
    backend->eltCount++;

    elt->pathIndex = backend->pathCount - backend->pathBatchStart;
    int count = 0;
    switch(kind)
    {
        case OC_PATH_LINE:
            backend->maxSegmentCount += 1;
            elt->kind = OC_MTL_LINE;
            count = 2;
            break;

        case OC_PATH_QUADRATIC:
            backend->maxSegmentCount += 3;
            elt->kind = OC_MTL_QUADRATIC;
            count = 3;
            break;

        case OC_PATH_CUBIC:
            backend->maxSegmentCount += 7;
            elt->kind = OC_MTL_CUBIC;
            count = 4;
            break;

        default:
            break;
    }

    for(int i = 0; i < count; i++)
    {
        oc_mtl_update_path_extents(&backend->pathUserExtents, p[i]);

        oc_vec2 screenP = oc_mat2x3_mul(backend->primitive->attributes.transform, p[i]);
        elt->p[i] = (vector_float2){ screenP.x, screenP.y };

        oc_mtl_update_path_extents(&backend->pathScreenExtents, screenP);
    }
}

void oc_mtl_encode_path(oc_mtl_canvas_backend* backend, oc_primitive* primitive, float scale)
{
    int bufferIndex = backend->bufferIndex;
    int bufferCap = [backend->pathBuffer[bufferIndex] length] / sizeof(oc_mtl_path);
    if(backend->pathCount >= bufferCap)
    {
        int newBufferCap = (int)(bufferCap * 1.5);
        int newBufferSize = newBufferCap * sizeof(oc_mtl_path);

        oc_log_info("growing path buffer to %i elements\n", newBufferCap);

        backend->pathBuffer[bufferIndex] = oc_mtl_grow_input_buffer(backend->surface->device,
                                                                    backend->pathBuffer[bufferIndex],
                                                                    backend->eltCount * sizeof(oc_mtl_path),
                                                                    newBufferSize);
    }

    oc_mtl_path* pathBufferData = (oc_mtl_path*)[backend->pathBuffer[backend->bufferIndex] contents];
    oc_mtl_path* path = &(pathBufferData[backend->pathCount]);
    backend->pathCount++;

    path->cmd = (oc_mtl_cmd)primitive->cmd;

    path->box = (vector_float4){ backend->pathScreenExtents.x,
                                 backend->pathScreenExtents.y,
                                 backend->pathScreenExtents.z,
                                 backend->pathScreenExtents.w };

    path->clip = (vector_float4){ primitive->attributes.clip.x,
                                  primitive->attributes.clip.y,
                                  primitive->attributes.clip.x + primitive->attributes.clip.w,
                                  primitive->attributes.clip.y + primitive->attributes.clip.h };

    path->color = (vector_float4){ primitive->attributes.color.r,
                                   primitive->attributes.color.g,
                                   primitive->attributes.color.b,
                                   primitive->attributes.color.a };

    oc_rect srcRegion = primitive->attributes.srcRegion;

    oc_rect destRegion = { backend->pathUserExtents.x,
                           backend->pathUserExtents.y,
                           backend->pathUserExtents.z - backend->pathUserExtents.x,
                           backend->pathUserExtents.w - backend->pathUserExtents.y };

    if(!oc_image_is_nil(primitive->attributes.image))
    {
        oc_vec2 texSize = oc_image_size(primitive->attributes.image);

        oc_mat2x3 srcRegionToImage = { 1 / texSize.x, 0, srcRegion.x / texSize.x,
                                       0, 1 / texSize.y, srcRegion.y / texSize.y };

        oc_mat2x3 destRegionToSrcRegion = { srcRegion.w / destRegion.w, 0, 0,
                                            0, srcRegion.h / destRegion.h, 0 };

        oc_mat2x3 userToDestRegion = { 1, 0, -destRegion.x,
                                       0, 1, -destRegion.y };

        oc_mat2x3 screenToUser = oc_mat2x3_inv(primitive->attributes.transform);

        oc_mat2x3 uvTransform = srcRegionToImage;
        uvTransform = oc_mat2x3_mul_m(uvTransform, destRegionToSrcRegion);
        uvTransform = oc_mat2x3_mul_m(uvTransform, userToDestRegion);
        uvTransform = oc_mat2x3_mul_m(uvTransform, screenToUser);

        path->uvTransform = simd_matrix(simd_make_float3(uvTransform.m[0] / scale, uvTransform.m[3] / scale, 0),
                                        simd_make_float3(uvTransform.m[1] / scale, uvTransform.m[4] / scale, 0),
                                        simd_make_float3(uvTransform.m[2], uvTransform.m[5], 1));
    }
    path->texture = backend->currentImageIndex;

    int firstTileX = path->box.x * scale / OC_MTL_TILE_SIZE;
    int firstTileY = path->box.y * scale / OC_MTL_TILE_SIZE;
    int lastTileX = path->box.z * scale / OC_MTL_TILE_SIZE;
    int lastTileY = path->box.w * scale / OC_MTL_TILE_SIZE;

    int nTilesX = lastTileX - firstTileX + 1;
    int nTilesY = lastTileY - firstTileY + 1;

    backend->maxTileQueueCount += (nTilesX * nTilesY);
}

static bool oc_intersect_hull_legs(oc_vec2 p0, oc_vec2 p1, oc_vec2 p2, oc_vec2 p3, oc_vec2* intersection)
{
    /*NOTE: check intersection of lines (p0-p1) and (p2-p3)

		P = p0 + u(p1-p0)
		P = p2 + w(p3-p2)
	*/
    bool found = false;

    f32 den = (p0.x - p1.x) * (p2.y - p3.y) - (p0.y - p1.y) * (p2.x - p3.x);
    if(fabs(den) > 0.0001)
    {
        f32 u = ((p0.x - p2.x) * (p2.y - p3.y) - (p0.y - p2.y) * (p2.x - p3.x)) / den;
        f32 w = ((p0.x - p2.x) * (p0.y - p1.y) - (p0.y - p2.y) * (p0.x - p1.x)) / den;

        intersection->x = p0.x + u * (p1.x - p0.x);
        intersection->y = p0.y + u * (p1.y - p0.y);
        found = true;
    }
    return (found);
}

static bool oc_offset_hull(int count, oc_vec2* p, oc_vec2* result, f32 offset)
{
    //NOTE: we should have no more than two coincident points here. This means the leg between
    //      those two points can't be offset, but we can set a double point at the start of first leg,
    //      end of first leg, or we can join the first and last leg to create a missing middle one

    oc_vec2 legs[3][2] = { 0 };
    bool valid[3] = { 0 };

    for(int i = 0; i < count - 1; i++)
    {
        oc_vec2 n = { p[i].y - p[i + 1].y,
                      p[i + 1].x - p[i].x };

        f32 norm = sqrt(n.x * n.x + n.y * n.y);
        if(norm >= 1e-6)
        {
            n = oc_vec2_mul(offset / norm, n);
            legs[i][0] = oc_vec2_add(p[i], n);
            legs[i][1] = oc_vec2_add(p[i + 1], n);
            valid[i] = true;
        }
    }

    //NOTE: now we find intersections

    // first point is either the start of the first or second leg
    if(valid[0])
    {
        result[0] = legs[0][0];
    }
    else
    {
        OC_ASSERT(valid[1]);
        result[0] = legs[1][0];
    }

    for(int i = 1; i < count - 1; i++)
    {
        //NOTE: we're computing the control point i, at the end of leg (i-1)

        if(!valid[i - 1])
        {
            OC_ASSERT(valid[i]);
            result[i] = legs[i][0];
        }
        else if(!valid[i])
        {
            OC_ASSERT(valid[i - 1]);
            result[i] = legs[i - 1][0];
        }
        else
        {
            if(!oc_intersect_hull_legs(legs[i - 1][0], legs[i - 1][1], legs[i][0], legs[i][1], &result[i]))
            {
                // legs don't intersect.
                return (false);
            }
        }
    }

    if(valid[count - 2])
    {
        result[count - 1] = legs[count - 2][1];
    }
    else
    {
        OC_ASSERT(valid[count - 3]);
        result[count - 1] = legs[count - 3][1];
    }

    return (true);
}

static oc_vec2 oc_quadratic_get_point(oc_vec2 p[3], f32 t)
{
    oc_vec2 r;

    f32 oneMt = 1 - t;
    f32 oneMt2 = oc_square(oneMt);
    f32 t2 = oc_square(t);

    r.x = oneMt2 * p[0].x + 2 * oneMt * t * p[1].x + t2 * p[2].x;
    r.y = oneMt2 * p[0].y + 2 * oneMt * t * p[1].y + t2 * p[2].y;

    return (r);
}

static void oc_quadratic_split(oc_vec2 p[3], f32 t, oc_vec2 outLeft[3], oc_vec2 outRight[3])
{
    //NOTE(martin): split bezier curve p at parameter t, using De Casteljau's algorithm
    //              the q_n are the points along the hull's segments at parameter t
    //              s is the split point.

    f32 oneMt = 1 - t;

    oc_vec2 q0 = { oneMt * p[0].x + t * p[1].x,
                   oneMt * p[0].y + t * p[1].y };

    oc_vec2 q1 = { oneMt * p[1].x + t * p[2].x,
                   oneMt * p[1].y + t * p[2].y };

    oc_vec2 s = { oneMt * q0.x + t * q1.x,
                  oneMt * q0.y + t * q1.y };

    outLeft[0] = p[0];
    outLeft[1] = q0;
    outLeft[2] = s;

    outRight[0] = s;
    outRight[1] = q1;
    outRight[2] = p[2];
}

static oc_vec2 oc_cubic_get_point(oc_vec2 p[4], f32 t)
{
    oc_vec2 r;

    f32 oneMt = 1 - t;
    f32 oneMt2 = oc_square(oneMt);
    f32 oneMt3 = oneMt2 * oneMt;
    f32 t2 = oc_square(t);
    f32 t3 = t2 * t;

    r.x = oneMt3 * p[0].x + 3 * oneMt2 * t * p[1].x + 3 * oneMt * t2 * p[2].x + t3 * p[3].x;
    r.y = oneMt3 * p[0].y + 3 * oneMt2 * t * p[1].y + 3 * oneMt * t2 * p[2].y + t3 * p[3].y;

    return (r);
}

static void oc_cubic_split(oc_vec2 p[4], f32 t, oc_vec2 outLeft[4], oc_vec2 outRight[4])
{
    //NOTE(martin): split bezier curve p at parameter t, using De Casteljau's algorithm
    //              the q_n are the points along the hull's segments at parameter t
    //              the r_n are the points along the (q_n, q_n+1) segments at parameter t
    //              s is the split point.

    oc_vec2 q0 = { (1 - t) * p[0].x + t * p[1].x,
                   (1 - t) * p[0].y + t * p[1].y };

    oc_vec2 q1 = { (1 - t) * p[1].x + t * p[2].x,
                   (1 - t) * p[1].y + t * p[2].y };

    oc_vec2 q2 = { (1 - t) * p[2].x + t * p[3].x,
                   (1 - t) * p[2].y + t * p[3].y };

    oc_vec2 r0 = { (1 - t) * q0.x + t * q1.x,
                   (1 - t) * q0.y + t * q1.y };

    oc_vec2 r1 = { (1 - t) * q1.x + t * q2.x,
                   (1 - t) * q1.y + t * q2.y };

    oc_vec2 s = { (1 - t) * r0.x + t * r1.x,
                  (1 - t) * r0.y + t * r1.y };
    ;

    outLeft[0] = p[0];
    outLeft[1] = q0;
    outLeft[2] = r0;
    outLeft[3] = s;

    outRight[0] = s;
    outRight[1] = r1;
    outRight[2] = q2;
    outRight[3] = p[3];
}

void oc_mtl_render_stroke_line(oc_mtl_canvas_backend* backend, oc_vec2* p)
{
    f32 width = backend->primitive->attributes.width;

    oc_vec2 v = { p[1].x - p[0].x, p[1].y - p[0].y };
    oc_vec2 n = { v.y, -v.x };
    f32 norm = sqrt(n.x * n.x + n.y * n.y);
    oc_vec2 offset = oc_vec2_mul(0.5 * width / norm, n);

    oc_vec2 left[2] = { oc_vec2_add(p[0], offset), oc_vec2_add(p[1], offset) };
    oc_vec2 right[2] = { oc_vec2_add(p[1], oc_vec2_mul(-1, offset)), oc_vec2_add(p[0], oc_vec2_mul(-1, offset)) };
    oc_vec2 joint0[2] = { oc_vec2_add(p[0], oc_vec2_mul(-1, offset)), oc_vec2_add(p[0], offset) };
    oc_vec2 joint1[2] = { oc_vec2_add(p[1], offset), oc_vec2_add(p[1], oc_vec2_mul(-1, offset)) };

    oc_mtl_canvas_encode_element(backend, OC_PATH_LINE, right);
    oc_mtl_canvas_encode_element(backend, OC_PATH_LINE, left);
    oc_mtl_canvas_encode_element(backend, OC_PATH_LINE, joint0);
    oc_mtl_canvas_encode_element(backend, OC_PATH_LINE, joint1);
}

void oc_mtl_render_stroke_quadratic(oc_mtl_canvas_backend* backend, oc_vec2* p)
{
    f32 width = backend->primitive->attributes.width;
    f32 tolerance = oc_min(backend->primitive->attributes.tolerance, 0.5 * width);

    //NOTE: check for degenerate line case
    const f32 equalEps = 1e-3;
    if(oc_vec2_close(p[0], p[1], equalEps))
    {
        oc_mtl_render_stroke_line(backend, p + 1);
        return;
    }
    else if(oc_vec2_close(p[1], p[2], equalEps))
    {
        oc_mtl_render_stroke_line(backend, p);
        return;
    }

    oc_vec2 leftHull[3];
    oc_vec2 rightHull[3];

    if(!oc_offset_hull(3, p, leftHull, width / 2)
       || !oc_offset_hull(3, p, rightHull, -width / 2))
    {
        //TODO split and recurse
        //NOTE: offsetting the hull failed, split the curve
        oc_vec2 splitLeft[3];
        oc_vec2 splitRight[3];
        oc_quadratic_split(p, 0.5, splitLeft, splitRight);
        oc_mtl_render_stroke_quadratic(backend, splitLeft);
        oc_mtl_render_stroke_quadratic(backend, splitRight);
    }
    else
    {
        const int CHECK_SAMPLE_COUNT = 5;
        f32 checkSamples[CHECK_SAMPLE_COUNT] = { 1. / 6, 2. / 6, 3. / 6, 4. / 6, 5. / 6 };

        f32 d2LowBound = oc_square(0.5 * width - tolerance);
        f32 d2HighBound = oc_square(0.5 * width + tolerance);

        f32 maxOvershoot = 0;
        f32 maxOvershootParameter = 0;

        for(int i = 0; i < CHECK_SAMPLE_COUNT; i++)
        {
            f32 t = checkSamples[i];

            oc_vec2 c = oc_quadratic_get_point(p, t);
            oc_vec2 cp = oc_quadratic_get_point(leftHull, t);
            oc_vec2 cn = oc_quadratic_get_point(rightHull, t);

            f32 positiveDistSquare = oc_square(c.x - cp.x) + oc_square(c.y - cp.y);
            f32 negativeDistSquare = oc_square(c.x - cn.x) + oc_square(c.y - cn.y);

            f32 positiveOvershoot = oc_max(positiveDistSquare - d2HighBound, d2LowBound - positiveDistSquare);
            f32 negativeOvershoot = oc_max(negativeDistSquare - d2HighBound, d2LowBound - negativeDistSquare);

            f32 overshoot = oc_max(positiveOvershoot, negativeOvershoot);

            if(overshoot > maxOvershoot)
            {
                maxOvershoot = overshoot;
                maxOvershootParameter = t;
            }
        }

        if(maxOvershoot > 0)
        {
            oc_vec2 splitLeft[3];
            oc_vec2 splitRight[3];
            oc_quadratic_split(p, maxOvershootParameter, splitLeft, splitRight);
            oc_mtl_render_stroke_quadratic(backend, splitLeft);
            oc_mtl_render_stroke_quadratic(backend, splitRight);
        }
        else
        {
            oc_vec2 tmp = leftHull[0];
            leftHull[0] = leftHull[2];
            leftHull[2] = tmp;

            oc_mtl_canvas_encode_element(backend, OC_PATH_QUADRATIC, rightHull);
            oc_mtl_canvas_encode_element(backend, OC_PATH_QUADRATIC, leftHull);

            oc_vec2 joint0[2] = { rightHull[2], leftHull[0] };
            oc_vec2 joint1[2] = { leftHull[2], rightHull[0] };
            oc_mtl_canvas_encode_element(backend, OC_PATH_LINE, joint0);
            oc_mtl_canvas_encode_element(backend, OC_PATH_LINE, joint1);
        }
    }
}

void oc_mtl_render_stroke_cubic(oc_mtl_canvas_backend* backend, oc_vec2* p)
{
    f32 width = backend->primitive->attributes.width;
    f32 tolerance = oc_min(backend->primitive->attributes.tolerance, 0.5 * width);

    //NOTE: check degenerate line cases
    f32 equalEps = 1e-3;

    if((oc_vec2_close(p[0], p[1], equalEps) && oc_vec2_close(p[2], p[3], equalEps))
       || (oc_vec2_close(p[0], p[1], equalEps) && oc_vec2_close(p[1], p[2], equalEps))
       || (oc_vec2_close(p[1], p[2], equalEps) && oc_vec2_close(p[2], p[3], equalEps)))
    {
        oc_vec2 line[2] = { p[0], p[3] };
        oc_mtl_render_stroke_line(backend, line);
        return;
    }
    else if(oc_vec2_close(p[0], p[1], equalEps) && oc_vec2_close(p[1], p[3], equalEps))
    {
        oc_vec2 line[2] = { p[0], oc_vec2_add(oc_vec2_mul(5. / 9, p[0]), oc_vec2_mul(4. / 9, p[2])) };
        oc_mtl_render_stroke_line(backend, line);
        return;
    }
    else if(oc_vec2_close(p[0], p[2], equalEps) && oc_vec2_close(p[2], p[3], equalEps))
    {
        oc_vec2 line[2] = { p[0], oc_vec2_add(oc_vec2_mul(5. / 9, p[0]), oc_vec2_mul(4. / 9, p[1])) };
        oc_mtl_render_stroke_line(backend, line);
        return;
    }

    oc_vec2 leftHull[4];
    oc_vec2 rightHull[4];

    if(!oc_offset_hull(4, p, leftHull, width / 2)
       || !oc_offset_hull(4, p, rightHull, -width / 2))
    {
        //TODO split and recurse
        //NOTE: offsetting the hull failed, split the curve
        oc_vec2 splitLeft[4];
        oc_vec2 splitRight[4];
        oc_cubic_split(p, 0.5, splitLeft, splitRight);
        oc_mtl_render_stroke_cubic(backend, splitLeft);
        oc_mtl_render_stroke_cubic(backend, splitRight);
    }
    else
    {
        const int CHECK_SAMPLE_COUNT = 5;
        f32 checkSamples[CHECK_SAMPLE_COUNT] = { 1. / 6, 2. / 6, 3. / 6, 4. / 6, 5. / 6 };

        f32 d2LowBound = oc_square(0.5 * width - tolerance);
        f32 d2HighBound = oc_square(0.5 * width + tolerance);

        f32 maxOvershoot = 0;
        f32 maxOvershootParameter = 0;

        for(int i = 0; i < CHECK_SAMPLE_COUNT; i++)
        {
            f32 t = checkSamples[i];

            oc_vec2 c = oc_cubic_get_point(p, t);
            oc_vec2 cp = oc_cubic_get_point(leftHull, t);
            oc_vec2 cn = oc_cubic_get_point(rightHull, t);

            f32 positiveDistSquare = oc_square(c.x - cp.x) + oc_square(c.y - cp.y);
            f32 negativeDistSquare = oc_square(c.x - cn.x) + oc_square(c.y - cn.y);

            f32 positiveOvershoot = oc_max(positiveDistSquare - d2HighBound, d2LowBound - positiveDistSquare);
            f32 negativeOvershoot = oc_max(negativeDistSquare - d2HighBound, d2LowBound - negativeDistSquare);

            f32 overshoot = oc_max(positiveOvershoot, negativeOvershoot);

            if(overshoot > maxOvershoot)
            {
                maxOvershoot = overshoot;
                maxOvershootParameter = t;
            }
        }

        if(maxOvershoot > 0)
        {
            oc_vec2 splitLeft[4];
            oc_vec2 splitRight[4];
            oc_cubic_split(p, maxOvershootParameter, splitLeft, splitRight);
            oc_mtl_render_stroke_cubic(backend, splitLeft);
            oc_mtl_render_stroke_cubic(backend, splitRight);
        }
        else
        {
            oc_vec2 tmp = leftHull[0];
            leftHull[0] = leftHull[3];
            leftHull[3] = tmp;
            tmp = leftHull[1];
            leftHull[1] = leftHull[2];
            leftHull[2] = tmp;

            oc_mtl_canvas_encode_element(backend, OC_PATH_CUBIC, rightHull);
            oc_mtl_canvas_encode_element(backend, OC_PATH_CUBIC, leftHull);

            oc_vec2 joint0[2] = { rightHull[3], leftHull[0] };
            oc_vec2 joint1[2] = { leftHull[3], rightHull[0] };
            oc_mtl_canvas_encode_element(backend, OC_PATH_LINE, joint0);
            oc_mtl_canvas_encode_element(backend, OC_PATH_LINE, joint1);
        }
    }
}

void oc_mtl_render_stroke_element(oc_mtl_canvas_backend* backend,
                                  oc_path_elt* element,
                                  oc_vec2 currentPoint,
                                  oc_vec2* startTangent,
                                  oc_vec2* endTangent,
                                  oc_vec2* endPoint)
{
    oc_vec2 controlPoints[4] = { currentPoint, element->p[0], element->p[1], element->p[2] };
    int endPointIndex = 0;

    switch(element->type)
    {
        case OC_PATH_LINE:
            oc_mtl_render_stroke_line(backend, controlPoints);
            endPointIndex = 1;
            break;

        case OC_PATH_QUADRATIC:
            oc_mtl_render_stroke_quadratic(backend, controlPoints);
            endPointIndex = 2;
            break;

        case OC_PATH_CUBIC:
            oc_mtl_render_stroke_cubic(backend, controlPoints);
            endPointIndex = 3;
            break;

        case OC_PATH_MOVE:
            OC_ASSERT(0, "should be unreachable");
            break;
    }

    //NOTE: ensure tangents are properly computed even in presence of coincident points
    //TODO: see if we can do this in a less hacky way

    for(int i = 1; i < 4; i++)
    {
        if(controlPoints[i].x != controlPoints[0].x
           || controlPoints[i].y != controlPoints[0].y)
        {
            *startTangent = (oc_vec2){ .x = controlPoints[i].x - controlPoints[0].x,
                                       .y = controlPoints[i].y - controlPoints[0].y };
            break;
        }
    }
    *endPoint = controlPoints[endPointIndex];

    for(int i = endPointIndex - 1; i >= 0; i++)
    {
        if(controlPoints[i].x != endPoint->x
           || controlPoints[i].y != endPoint->y)
        {
            *endTangent = (oc_vec2){ .x = endPoint->x - controlPoints[i].x,
                                     .y = endPoint->y - controlPoints[i].y };
            break;
        }
    }
    OC_DEBUG_ASSERT(startTangent->x != 0 || startTangent->y != 0);
}

void oc_mtl_stroke_cap(oc_mtl_canvas_backend* backend,
                       oc_vec2 p0,
                       oc_vec2 direction)
{
    oc_attributes* attributes = &backend->primitive->attributes;

    //NOTE(martin): compute the tangent and normal vectors (multiplied by half width) at the cap point
    f32 dn = sqrt(oc_square(direction.x) + oc_square(direction.y));
    f32 alpha = 0.5 * attributes->width / dn;

    oc_vec2 n0 = { -alpha * direction.y,
                   alpha * direction.x };

    oc_vec2 m0 = { alpha * direction.x,
                   alpha * direction.y };

    oc_vec2 points[] = { { p0.x + n0.x, p0.y + n0.y },
                         { p0.x + n0.x + m0.x, p0.y + n0.y + m0.y },
                         { p0.x - n0.x + m0.x, p0.y - n0.y + m0.y },
                         { p0.x - n0.x, p0.y - n0.y },
                         { p0.x + n0.x, p0.y + n0.y } };

    oc_mtl_canvas_encode_element(backend, OC_PATH_LINE, points);
    oc_mtl_canvas_encode_element(backend, OC_PATH_LINE, points + 1);
    oc_mtl_canvas_encode_element(backend, OC_PATH_LINE, points + 2);
    oc_mtl_canvas_encode_element(backend, OC_PATH_LINE, points + 3);
}

void oc_mtl_stroke_joint(oc_mtl_canvas_backend* backend,
                         oc_vec2 p0,
                         oc_vec2 t0,
                         oc_vec2 t1)
{
    oc_attributes* attributes = &backend->primitive->attributes;

    //NOTE(martin): compute the normals at the joint point
    f32 norm_t0 = sqrt(oc_square(t0.x) + oc_square(t0.y));
    f32 norm_t1 = sqrt(oc_square(t1.x) + oc_square(t1.y));

    oc_vec2 n0 = { -t0.y, t0.x };
    n0.x /= norm_t0;
    n0.y /= norm_t0;

    oc_vec2 n1 = { -t1.y, t1.x };
    n1.x /= norm_t1;
    n1.y /= norm_t1;

    //NOTE(martin): the sign of the cross product determines if the normals are facing outwards or inwards the angle.
    //              we flip them to face outwards if needed
    f32 crossZ = n0.x * n1.y - n0.y * n1.x;
    if(crossZ > 0)
    {
        n0.x *= -1;
        n0.y *= -1;
        n1.x *= -1;
        n1.y *= -1;
    }

    //NOTE(martin): use the same code as hull offset to find mitter point...
    /*NOTE(martin): let vector u = (n0+n1) and vector v = pIntersect - p1
		then v = u * (2*offset / norm(u)^2)
		(this can be derived from writing the pythagoras theorems in the triangles of the joint)
	*/
    f32 halfW = 0.5 * attributes->width;
    oc_vec2 u = { n0.x + n1.x, n0.y + n1.y };
    f32 uNormSquare = u.x * u.x + u.y * u.y;
    f32 alpha = attributes->width / uNormSquare;
    oc_vec2 v = { u.x * alpha, u.y * alpha };

    f32 excursionSquare = uNormSquare * oc_square(alpha - attributes->width / 4);

    if(attributes->joint == OC_JOINT_MITER
       && excursionSquare <= oc_square(attributes->maxJointExcursion))
    {
        //NOTE(martin): add a mitter joint
        oc_vec2 points[] = { p0,
                             { p0.x + n0.x * halfW, p0.y + n0.y * halfW },
                             { p0.x + v.x, p0.y + v.y },
                             { p0.x + n1.x * halfW, p0.y + n1.y * halfW },
                             p0 };

        oc_mtl_canvas_encode_element(backend, OC_PATH_LINE, points);
        oc_mtl_canvas_encode_element(backend, OC_PATH_LINE, points + 1);
        oc_mtl_canvas_encode_element(backend, OC_PATH_LINE, points + 2);
        oc_mtl_canvas_encode_element(backend, OC_PATH_LINE, points + 3);
    }
    else
    {
        //NOTE(martin): add a bevel joint
        oc_vec2 points[] = { p0,
                             { p0.x + n0.x * halfW, p0.y + n0.y * halfW },
                             { p0.x + n1.x * halfW, p0.y + n1.y * halfW },
                             p0 };

        oc_mtl_canvas_encode_element(backend, OC_PATH_LINE, points);
        oc_mtl_canvas_encode_element(backend, OC_PATH_LINE, points + 1);
        oc_mtl_canvas_encode_element(backend, OC_PATH_LINE, points + 2);
    }
}

u32 oc_mtl_render_stroke_subpath(oc_mtl_canvas_backend* backend,
                                 oc_path_elt* elements,
                                 oc_path_descriptor* path,
                                 u32 startIndex,
                                 oc_vec2 startPoint)
{
    u32 eltCount = path->count;
    OC_DEBUG_ASSERT(startIndex < eltCount);

    oc_vec2 currentPoint = startPoint;
    oc_vec2 endPoint = { 0, 0 };
    oc_vec2 previousEndTangent = { 0, 0 };
    oc_vec2 firstTangent = { 0, 0 };
    oc_vec2 startTangent = { 0, 0 };
    oc_vec2 endTangent = { 0, 0 };

    //NOTE(martin): render first element and compute first tangent
    oc_mtl_render_stroke_element(backend, elements + startIndex, currentPoint, &startTangent, &endTangent, &endPoint);

    firstTangent = startTangent;
    previousEndTangent = endTangent;
    currentPoint = endPoint;

    //NOTE(martin): render subsequent elements along with their joints

    oc_attributes* attributes = &backend->primitive->attributes;

    u32 eltIndex = startIndex + 1;
    for(;
        eltIndex < eltCount && elements[eltIndex].type != OC_PATH_MOVE;
        eltIndex++)
    {
        oc_mtl_render_stroke_element(backend, elements + eltIndex, currentPoint, &startTangent, &endTangent, &endPoint);

        if(attributes->joint != OC_JOINT_NONE)
        {
            oc_mtl_stroke_joint(backend, currentPoint, previousEndTangent, startTangent);
        }
        previousEndTangent = endTangent;
        currentPoint = endPoint;
    }
    u32 subPathEltCount = eltIndex - startIndex;

    //NOTE(martin): draw end cap / joint. We ensure there's at least two segments to draw a closing joint
    if(subPathEltCount > 1
       && startPoint.x == endPoint.x
       && startPoint.y == endPoint.y)
    {
        if(attributes->joint != OC_JOINT_NONE)
        {
            //NOTE(martin): add a closing joint if the path is closed
            oc_mtl_stroke_joint(backend, endPoint, endTangent, firstTangent);
        }
    }
    else if(attributes->cap == OC_CAP_SQUARE)
    {
        //NOTE(martin): add start and end cap
        oc_mtl_stroke_cap(backend, startPoint, (oc_vec2){ -startTangent.x, -startTangent.y });
        oc_mtl_stroke_cap(backend, endPoint, endTangent);
    }
    return (eltIndex);
}

void oc_mtl_render_stroke(oc_mtl_canvas_backend* backend,
                          oc_path_elt* elements,
                          oc_path_descriptor* path)
{
    u32 eltCount = path->count;
    OC_DEBUG_ASSERT(eltCount);

    oc_vec2 startPoint = path->startPoint;
    u32 startIndex = 0;

    while(startIndex < eltCount)
    {
        //NOTE(martin): eliminate leading moves
        while(startIndex < eltCount && elements[startIndex].type == OC_PATH_MOVE)
        {
            startPoint = elements[startIndex].p[0];
            startIndex++;
        }
        if(startIndex < eltCount)
        {
            startIndex = oc_mtl_render_stroke_subpath(backend, elements, path, startIndex, startPoint);
        }
    }
}

void oc_mtl_grow_buffer_if_needed(oc_mtl_canvas_backend* backend, id<MTLBuffer>* buffer, u64 wantedSize)
{
    u64 bufferSize = [(*buffer) length];
    if(bufferSize < wantedSize)
    {
        int newSize = wantedSize * 1.2;

        @autoreleasepool
        {
            //NOTE: MTLBuffers are retained by the command buffer, so we don't risk deallocating while the buffer is in use
            [*buffer release];
            *buffer = nil;

            id<MTLDevice> device = backend->surface->device;
            MTLResourceOptions bufferOptions = MTLResourceStorageModePrivate;

            *buffer = [device newBufferWithLength:newSize options:bufferOptions];
        }
    }
}

void oc_mtl_render_batch(oc_mtl_canvas_backend* backend,
                         oc_mtl_surface* surface,
                         oc_image* images,
                         int tileSize,
                         int nTilesX,
                         int nTilesY,
                         oc_vec2 viewportSize,
                         f32 scale)
{
    int pathBufferOffset = backend->pathBatchStart * sizeof(oc_mtl_path);
    int elementBufferOffset = backend->eltBatchStart * sizeof(oc_mtl_path_elt);
    int pathCount = backend->pathCount - backend->pathBatchStart;
    int eltCount = backend->eltCount - backend->eltBatchStart;

    if(!pathCount || !eltCount)
    {
        return;
    }

    //NOTE: update intermediate buffers sizes if needed

    oc_mtl_grow_buffer_if_needed(backend, &backend->pathQueueBuffer, pathCount * sizeof(oc_mtl_path_queue));
    oc_mtl_grow_buffer_if_needed(backend, &backend->tileQueueBuffer, backend->maxTileQueueCount * sizeof(oc_mtl_tile_queue));
    oc_mtl_grow_buffer_if_needed(backend, &backend->segmentBuffer, backend->maxSegmentCount * sizeof(oc_mtl_segment));
    oc_mtl_grow_buffer_if_needed(backend, &backend->screenTilesBuffer, nTilesX * nTilesY * sizeof(oc_mtl_screen_tile));
    oc_mtl_grow_buffer_if_needed(backend, &backend->tileOpBuffer, backend->maxSegmentCount * 30 * sizeof(oc_mtl_tile_op));

    //NOTE: encode GPU commands
    @autoreleasepool
    {
        //NOTE: clear output texture
        MTLRenderPassDescriptor* clearDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
        clearDescriptor.colorAttachments[0].texture = backend->outTexture;
        clearDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
        clearDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0, 0, 0, 0);
        clearDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

        id<MTLRenderCommandEncoder> clearEncoder = [surface->commandBuffer renderCommandEncoderWithDescriptor:clearDescriptor];
        clearEncoder.label = @"clear out texture pass";
        [clearEncoder endEncoding];

        //NOTE: clear counters
        id<MTLBlitCommandEncoder> blitEncoder = [surface->commandBuffer blitCommandEncoder];
        blitEncoder.label = @"clear counters";
        [blitEncoder fillBuffer:backend->segmentCountBuffer range:NSMakeRange(0, sizeof(int)) value:0];
        [blitEncoder fillBuffer:backend->tileQueueCountBuffer range:NSMakeRange(0, sizeof(int)) value:0];
        [blitEncoder fillBuffer:backend->tileOpCountBuffer range:NSMakeRange(0, sizeof(int)) value:0];
        [blitEncoder fillBuffer:backend->rasterDispatchBuffer range:NSMakeRange(0, sizeof(MTLDispatchThreadgroupsIndirectArguments)) value:0];
        [blitEncoder endEncoding];

        //NOTE: path setup pass
        id<MTLComputeCommandEncoder> pathEncoder = [surface->commandBuffer computeCommandEncoder];
        pathEncoder.label = @"path pass";
        [pathEncoder setComputePipelineState:backend->pathPipeline];

        int tileQueueMax = [backend->tileQueueBuffer length] / sizeof(oc_mtl_tile_queue);

        [pathEncoder setBytes:&pathCount length:sizeof(int) atIndex:0];
        [pathEncoder setBuffer:backend->pathBuffer[backend->bufferIndex] offset:pathBufferOffset atIndex:1];
        [pathEncoder setBuffer:backend->pathQueueBuffer offset:0 atIndex:2];
        [pathEncoder setBuffer:backend->tileQueueBuffer offset:0 atIndex:3];
        [pathEncoder setBuffer:backend->tileQueueCountBuffer offset:0 atIndex:4];
        [pathEncoder setBytes:&tileQueueMax length:sizeof(int) atIndex:5];
        [pathEncoder setBytes:&tileSize length:sizeof(int) atIndex:6];
        [pathEncoder setBytes:&scale length:sizeof(int) atIndex:7];

        MTLSize pathGridSize = MTLSizeMake(pathCount, 1, 1);
        MTLSize pathGroupSize = MTLSizeMake([backend->pathPipeline maxTotalThreadsPerThreadgroup], 1, 1);

        [pathEncoder dispatchThreads:pathGridSize threadsPerThreadgroup:pathGroupSize];
        [pathEncoder endEncoding];

        //NOTE: segment setup pass
        id<MTLComputeCommandEncoder> segmentEncoder = [surface->commandBuffer computeCommandEncoder];
        segmentEncoder.label = @"segment pass";
        [segmentEncoder setComputePipelineState:backend->segmentPipeline];

        int tileOpMax = [backend->tileOpBuffer length] / sizeof(oc_mtl_tile_op);
        int segmentMax = [backend->segmentBuffer length] / sizeof(oc_mtl_segment);

        [segmentEncoder setBytes:&eltCount length:sizeof(int) atIndex:0];
        [segmentEncoder setBuffer:backend->elementBuffer[backend->bufferIndex] offset:elementBufferOffset atIndex:1];
        [segmentEncoder setBuffer:backend->segmentCountBuffer offset:0 atIndex:2];
        [segmentEncoder setBuffer:backend->segmentBuffer offset:0 atIndex:3];
        [segmentEncoder setBuffer:backend->pathQueueBuffer offset:0 atIndex:4];
        [segmentEncoder setBuffer:backend->tileQueueBuffer offset:0 atIndex:5];
        [segmentEncoder setBuffer:backend->tileOpBuffer offset:0 atIndex:6];
        [segmentEncoder setBuffer:backend->tileOpCountBuffer offset:0 atIndex:7];
        [segmentEncoder setBytes:&tileOpMax length:sizeof(int) atIndex:8];
        [segmentEncoder setBytes:&segmentMax length:sizeof(int) atIndex:9];
        [segmentEncoder setBytes:&tileSize length:sizeof(int) atIndex:10];
        [segmentEncoder setBytes:&scale length:sizeof(int) atIndex:11];
        [segmentEncoder setBuffer:backend->logBuffer[backend->bufferIndex] offset:0 atIndex:12];
        [segmentEncoder setBuffer:backend->logOffsetBuffer[backend->bufferIndex] offset:0 atIndex:13];

        MTLSize segmentGridSize = MTLSizeMake(eltCount, 1, 1);
        MTLSize segmentGroupSize = MTLSizeMake([backend->segmentPipeline maxTotalThreadsPerThreadgroup], 1, 1);

        [segmentEncoder dispatchThreads:segmentGridSize threadsPerThreadgroup:segmentGroupSize];
        [segmentEncoder endEncoding];

        //NOTE: backprop pass
        id<MTLComputeCommandEncoder> backpropEncoder = [surface->commandBuffer computeCommandEncoder];
        backpropEncoder.label = @"backprop pass";
        [backpropEncoder setComputePipelineState:backend->backpropPipeline];

        [backpropEncoder setBuffer:backend->pathQueueBuffer offset:0 atIndex:0];
        [backpropEncoder setBuffer:backend->tileQueueBuffer offset:0 atIndex:1];
        [backpropEncoder setBuffer:backend->logBuffer[backend->bufferIndex] offset:0 atIndex:2];
        [backpropEncoder setBuffer:backend->logOffsetBuffer[backend->bufferIndex] offset:0 atIndex:3];

        [backpropEncoder setBuffer:backend->segmentCountBuffer offset:0 atIndex:4];

        MTLSize backpropGroupSize = MTLSizeMake([backend->backpropPipeline maxTotalThreadsPerThreadgroup], 1, 1);
        MTLSize backpropGridSize = MTLSizeMake(pathCount * backpropGroupSize.width, 1, 1);

        [backpropEncoder dispatchThreads:backpropGridSize threadsPerThreadgroup:backpropGroupSize];
        [backpropEncoder endEncoding];

        //NOTE: merge pass
        id<MTLComputeCommandEncoder> mergeEncoder = [surface->commandBuffer computeCommandEncoder];
        mergeEncoder.label = @"merge pass";
        [mergeEncoder setComputePipelineState:backend->mergePipeline];

        [mergeEncoder setBytes:&pathCount length:sizeof(int) atIndex:0];
        [mergeEncoder setBuffer:backend->pathBuffer[backend->bufferIndex] offset:pathBufferOffset atIndex:1];
        [mergeEncoder setBuffer:backend->pathQueueBuffer offset:0 atIndex:2];
        [mergeEncoder setBuffer:backend->tileQueueBuffer offset:0 atIndex:3];
        [mergeEncoder setBuffer:backend->tileOpBuffer offset:0 atIndex:4];
        [mergeEncoder setBuffer:backend->tileOpCountBuffer offset:0 atIndex:5];
        [mergeEncoder setBuffer:backend->rasterDispatchBuffer offset:0 atIndex:6];
        [mergeEncoder setBuffer:backend->screenTilesBuffer offset:0 atIndex:7];
        [mergeEncoder setBytes:&tileOpMax length:sizeof(int) atIndex:8];
        [mergeEncoder setBytes:&tileSize length:sizeof(int) atIndex:9];
        [mergeEncoder setBytes:&scale length:sizeof(float) atIndex:10];
        [mergeEncoder setBuffer:backend->logBuffer[backend->bufferIndex] offset:0 atIndex:11];
        [mergeEncoder setBuffer:backend->logOffsetBuffer[backend->bufferIndex] offset:0 atIndex:12];

        MTLSize mergeGridSize = MTLSizeMake(nTilesX, nTilesY, 1);
        MTLSize mergeGroupSize = MTLSizeMake(OC_MTL_TILE_SIZE, OC_MTL_TILE_SIZE, 1);

        [mergeEncoder dispatchThreads:mergeGridSize threadsPerThreadgroup:mergeGroupSize];
        [mergeEncoder endEncoding];

        //NOTE: raster pass
        id<MTLComputeCommandEncoder> rasterEncoder = [surface->commandBuffer computeCommandEncoder];
        rasterEncoder.label = @"raster pass";
        [rasterEncoder setComputePipelineState:backend->rasterPipeline];

        [rasterEncoder setBuffer:backend->screenTilesBuffer offset:0 atIndex:0];
        [rasterEncoder setBuffer:backend->tileOpBuffer offset:0 atIndex:1];
        [rasterEncoder setBuffer:backend->pathBuffer[backend->bufferIndex] offset:pathBufferOffset atIndex:2];
        [rasterEncoder setBuffer:backend->segmentBuffer offset:0 atIndex:3];
        [rasterEncoder setBytes:&tileSize length:sizeof(int) atIndex:4];
        [rasterEncoder setBytes:&scale length:sizeof(float) atIndex:5];
        [rasterEncoder setBytes:&backend->msaaCount length:sizeof(int) atIndex:6];
        [rasterEncoder setBuffer:backend->logBuffer[backend->bufferIndex] offset:0 atIndex:7];
        [rasterEncoder setBuffer:backend->logOffsetBuffer[backend->bufferIndex] offset:0 atIndex:8];

        [rasterEncoder setTexture:backend->outTexture atIndex:0];

        for(int i = 0; i < OC_MTL_MAX_IMAGES_PER_BATCH; i++)
        {
            if(images[i].h)
            {
                oc_mtl_image_data* image = (oc_mtl_image_data*)oc_image_data_from_handle(images[i]);
                if(image)
                {
                    [rasterEncoder setTexture:image->texture atIndex:1 + i];
                }
            }
        }

        MTLSize rasterGridSize = MTLSizeMake(viewportSize.x, viewportSize.y, 1);
        MTLSize rasterGroupSize = MTLSizeMake(OC_MTL_TILE_SIZE, OC_MTL_TILE_SIZE, 1);

        [rasterEncoder dispatchThreadgroupsWithIndirectBuffer:backend->rasterDispatchBuffer
                                         indirectBufferOffset:0
                                        threadsPerThreadgroup:rasterGroupSize];

        [rasterEncoder endEncoding];

        //NOTE: blit pass
        MTLViewport viewport = { 0, 0, viewportSize.x, viewportSize.y, 0, 1 };

        MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
        renderPassDescriptor.colorAttachments[0].texture = surface->drawable.texture;
        renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
        renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

        id<MTLRenderCommandEncoder> renderEncoder = [surface->commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        renderEncoder.label = @"blit pass";
        [renderEncoder setViewport:viewport];
        [renderEncoder setRenderPipelineState:backend->blitPipeline];
        [renderEncoder setFragmentTexture:backend->outTexture atIndex:0];
        [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle
                          vertexStart:0
                          vertexCount:3];
        [renderEncoder endEncoding];
    }

    backend->pathBatchStart = backend->pathCount;
    backend->eltBatchStart = backend->eltCount;

    backend->maxSegmentCount = 0;
    backend->maxTileQueueCount = 0;
}

void oc_mtl_canvas_resize(oc_mtl_canvas_backend* backend, oc_vec2 size)
{
    @autoreleasepool
    {
        if(backend->screenTilesBuffer)
        {
            [backend->screenTilesBuffer release];
            backend->screenTilesBuffer = nil;
        }
        int tileSize = OC_MTL_TILE_SIZE;
        int nTilesX = (int)(size.x + tileSize - 1) / tileSize;
        int nTilesY = (int)(size.y + tileSize - 1) / tileSize;
        MTLResourceOptions bufferOptions = MTLResourceStorageModePrivate;
        backend->screenTilesBuffer = [backend->surface->device newBufferWithLength:nTilesX * nTilesY * sizeof(oc_mtl_screen_tile)
                                                                           options:bufferOptions];

        if(backend->outTexture)
        {
            [backend->outTexture release];
            backend->outTexture = nil;
        }
        MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
        texDesc.textureType = MTLTextureType2D;
        texDesc.storageMode = MTLStorageModePrivate;
        texDesc.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite | MTLTextureUsageRenderTarget;
        texDesc.pixelFormat = MTLPixelFormatRGBA8Unorm;
        texDesc.width = size.x;
        texDesc.height = size.y;

        backend->outTexture = [backend->surface->device newTextureWithDescriptor:texDesc];

        backend->surface->mtlLayer.drawableSize = (CGSize){ size.x, size.y };

        backend->frameSize = size;
    }
}

void oc_mtl_canvas_render(oc_canvas_backend* interface,
                          oc_color clearColor,
                          u32 primitiveCount,
                          oc_primitive* primitives,
                          u32 eltCount,
                          oc_path_elt* pathElements)
{
    oc_mtl_canvas_backend* backend = (oc_mtl_canvas_backend*)interface;

    //NOTE: update rolling input buffers
    dispatch_semaphore_wait(backend->bufferSemaphore, DISPATCH_TIME_FOREVER);
    backend->bufferIndex = (backend->bufferIndex + 1) % OC_MTL_INPUT_BUFFERS_COUNT;

    //NOTE: ensure screen tiles buffer is correct size
    oc_mtl_surface* surface = backend->surface;

    oc_vec2 frameSize = surface->interface.getSize((oc_surface_data*)surface);

    f32 scale = surface->mtlLayer.contentsScale;
    oc_vec2 viewportSize = { frameSize.x * scale, frameSize.y * scale };
    int tileSize = OC_MTL_TILE_SIZE;
    int nTilesX = (int)(viewportSize.x * scale + tileSize - 1) / tileSize;
    int nTilesY = (int)(viewportSize.y * scale + tileSize - 1) / tileSize;

    if(viewportSize.x != backend->frameSize.x || viewportSize.y != backend->frameSize.y)
    {
        oc_mtl_canvas_resize(backend, viewportSize);
    }

    //NOTE: acquire metal resources for rendering
    oc_mtl_surface_acquire_command_buffer(surface);
    oc_mtl_surface_acquire_drawable(surface);

    @autoreleasepool
    {
        //NOTE: clear log counter
        id<MTLBlitCommandEncoder> blitEncoder = [surface->commandBuffer blitCommandEncoder];
        blitEncoder.label = @"clear log counter";
        [blitEncoder fillBuffer:backend->logOffsetBuffer[backend->bufferIndex] range:NSMakeRange(0, sizeof(int)) value:0];
        [blitEncoder endEncoding];

        //NOTE: clear screen
        MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
        renderPassDescriptor.colorAttachments[0].texture = surface->drawable.texture;
        renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
        renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

        id<MTLRenderCommandEncoder> renderEncoder = [surface->commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        renderEncoder.label = @"clear pass";
        [renderEncoder endEncoding];
    }
    backend->pathCount = 0;
    backend->pathBatchStart = 0;
    backend->eltCount = 0;
    backend->eltBatchStart = 0;
    backend->maxSegmentCount = 0;
    backend->maxTileQueueCount = 0;

    //NOTE: encode and render batches
    oc_vec2 currentPos = { 0 };
    oc_image images[OC_MTL_MAX_IMAGES_PER_BATCH] = { 0 };
    int imageCount = 0;

    for(int primitiveIndex = 0; primitiveIndex < primitiveCount; primitiveIndex++)
    {
        oc_primitive* primitive = &primitives[primitiveIndex];

        if(primitive->attributes.image.h != 0)
        {
            backend->currentImageIndex = -1;
            for(int i = 0; i < imageCount; i++)
            {
                if(images[i].h == primitive->attributes.image.h)
                {
                    backend->currentImageIndex = i;
                }
            }
            if(backend->currentImageIndex <= 0)
            {
                if(imageCount < OC_MTL_MAX_IMAGES_PER_BATCH)
                {
                    images[imageCount] = primitive->attributes.image;
                    backend->currentImageIndex = imageCount;
                    imageCount++;
                }
                else
                {
                    oc_mtl_render_batch(backend,
                                        surface,
                                        images,
                                        tileSize,
                                        nTilesX,
                                        nTilesY,
                                        viewportSize,
                                        scale);

                    images[0] = primitive->attributes.image;
                    backend->currentImageIndex = 0;
                    imageCount = 1;
                }
            }
        }
        else
        {
            backend->currentImageIndex = -1;
        }

        if(primitive->path.count)
        {
            backend->primitive = primitive;
            backend->pathScreenExtents = (oc_vec4){ FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX };
            backend->pathUserExtents = (oc_vec4){ FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX };

            if(primitive->cmd == OC_CMD_STROKE)
            {
                oc_mtl_render_stroke(backend, pathElements + primitive->path.startIndex, &primitive->path);
            }
            else
            {
                for(int eltIndex = 0;
                    (eltIndex < primitive->path.count) && (primitive->path.startIndex + eltIndex < eltCount);
                    eltIndex++)
                {
                    oc_path_elt* elt = &pathElements[primitive->path.startIndex + eltIndex];

                    if(elt->type != OC_PATH_MOVE)
                    {
                        oc_vec2 p[4] = { currentPos, elt->p[0], elt->p[1], elt->p[2] };
                        oc_mtl_canvas_encode_element(backend, elt->type, p);
                    }
                    switch(elt->type)
                    {
                        case OC_PATH_MOVE:
                            currentPos = elt->p[0];
                            break;

                        case OC_PATH_LINE:
                            currentPos = elt->p[0];
                            break;

                        case OC_PATH_QUADRATIC:
                            currentPos = elt->p[1];
                            break;

                        case OC_PATH_CUBIC:
                            currentPos = elt->p[2];
                            break;
                    }
                }
            }
            //NOTE: encode path
            oc_mtl_encode_path(backend, primitive, scale);
        }
    }

    oc_mtl_render_batch(backend,
                        surface,
                        images,
                        tileSize,
                        nTilesX,
                        nTilesY,
                        viewportSize,
                        scale);

    @autoreleasepool
    {
        //NOTE: finalize
        [surface->commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> commandBuffer) {
          oc_mtl_print_log(backend->bufferIndex, backend->logBuffer[backend->bufferIndex], backend->logOffsetBuffer[backend->bufferIndex]);
          dispatch_semaphore_signal(backend->bufferSemaphore);
        }];
    }
}

void oc_mtl_canvas_destroy(oc_canvas_backend* interface)
{
    oc_mtl_canvas_backend* backend = (oc_mtl_canvas_backend*)interface;

    @autoreleasepool
    {
        [backend->pathPipeline release];
        [backend->segmentPipeline release];
        [backend->backpropPipeline release];
        [backend->mergePipeline release];
        [backend->rasterPipeline release];
        [backend->blitPipeline release];

        for(int i = 0; i < OC_MTL_INPUT_BUFFERS_COUNT; i++)
        {
            [backend->pathBuffer[i] release];
            [backend->elementBuffer[i] release];
            [backend->logBuffer[i] release];
            [backend->logOffsetBuffer[i] release];
        }
        [backend->segmentCountBuffer release];
        [backend->segmentBuffer release];
        [backend->tileQueueBuffer release];
        [backend->tileQueueCountBuffer release];
        [backend->tileOpBuffer release];
        [backend->tileOpCountBuffer release];
        [backend->screenTilesBuffer release];
    }

    free(backend);
}

oc_image_data* oc_mtl_canvas_image_create(oc_canvas_backend* interface, oc_vec2 size)
{
    oc_mtl_image_data* image = 0;
    oc_mtl_canvas_backend* backend = (oc_mtl_canvas_backend*)interface;
    oc_mtl_surface* surface = backend->surface;

    @autoreleasepool
    {
        image = oc_malloc_type(oc_mtl_image_data);
        if(image)
        {
            MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
            texDesc.textureType = MTLTextureType2D;
            texDesc.storageMode = MTLStorageModeManaged;
            texDesc.usage = MTLTextureUsageShaderRead;
            texDesc.pixelFormat = MTLPixelFormatRGBA8Unorm;
            texDesc.width = size.x;
            texDesc.height = size.y;

            image->texture = [surface->device newTextureWithDescriptor:texDesc];
            if(image->texture != nil)
            {
                [image->texture retain];
                image->interface.size = size;
            }
            else
            {
                free(image);
                image = 0;
            }
        }
    }
    return ((oc_image_data*)image);
}

void oc_mtl_canvas_image_destroy(oc_canvas_backend* backendInterface, oc_image_data* imageInterface)
{
    oc_mtl_image_data* image = (oc_mtl_image_data*)imageInterface;
    @autoreleasepool
    {
        [image->texture release];
        free(image);
    }
}

void oc_mtl_canvas_image_upload_region(oc_canvas_backend* backendInterface, oc_image_data* imageInterface, oc_rect region, u8* pixels)
{
    @autoreleasepool
    {
        oc_mtl_image_data* image = (oc_mtl_image_data*)imageInterface;
        MTLRegion mtlRegion = MTLRegionMake2D(region.x, region.y, region.w, region.h);
        [image->texture replaceRegion:mtlRegion
                          mipmapLevel:0
                            withBytes:(void*)pixels
                          bytesPerRow:4 * region.w];
    }
}

const u32 OC_MTL_DEFAULT_PATH_BUFFER_LEN = (4 << 10),
          OC_MTL_DEFAULT_ELT_BUFFER_LEN = (4 << 10),

          OC_MTL_DEFAULT_SEGMENT_BUFFER_LEN = (4 << 10),
          OC_MTL_DEFAULT_PATH_QUEUE_BUFFER_LEN = (4 << 10),
          OC_MTL_DEFAULT_TILE_QUEUE_BUFFER_LEN = (4 << 10),
          OC_MTL_DEFAULT_TILE_OP_BUFFER_LEN = (4 << 20);

oc_canvas_backend* oc_mtl_canvas_backend_create(oc_mtl_surface* surface)
{
    oc_mtl_canvas_backend* backend = 0;

    backend = oc_malloc_type(oc_mtl_canvas_backend);
    memset(backend, 0, sizeof(oc_mtl_canvas_backend));

    backend->msaaCount = OC_MTL_MSAA_COUNT;
    backend->surface = surface;

    //NOTE(martin): setup interface functions
    backend->interface.destroy = oc_mtl_canvas_destroy;
    backend->interface.render = oc_mtl_canvas_render;
    backend->interface.imageCreate = oc_mtl_canvas_image_create;
    backend->interface.imageDestroy = oc_mtl_canvas_image_destroy;
    backend->interface.imageUploadRegion = oc_mtl_canvas_image_upload_region;

    @autoreleasepool
    {
        //NOTE: load metal library
        oc_str8 shaderPath = oc_path_executable_relative(oc_scratch(), OC_STR8("mtl_renderer.metallib"));
        NSString* metalFileName = [[NSString alloc] initWithBytes:shaderPath.ptr length:shaderPath.len encoding:NSUTF8StringEncoding];
        NSError* err = 0;
        id<MTLLibrary> library = [surface->device newLibraryWithFile:metalFileName error:&err];
        if(err != nil)
        {
            const char* errStr = [[err localizedDescription] UTF8String];
            oc_log_error("error : %s\n", errStr);
            return (0);
        }
        id<MTLFunction> pathFunction = [library newFunctionWithName:@"mtl_path_setup"];
        id<MTLFunction> segmentFunction = [library newFunctionWithName:@"mtl_segment_setup"];
        id<MTLFunction> backpropFunction = [library newFunctionWithName:@"mtl_backprop"];
        id<MTLFunction> mergeFunction = [library newFunctionWithName:@"mtl_merge"];
        id<MTLFunction> rasterFunction = [library newFunctionWithName:@"mtl_raster"];
        id<MTLFunction> vertexFunction = [library newFunctionWithName:@"mtl_vertex_shader"];
        id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"mtl_fragment_shader"];

        //NOTE: create pipelines
        NSError* error = NULL;

        backend->pathPipeline = [surface->device newComputePipelineStateWithFunction:pathFunction
                                                                               error:&error];

        backend->segmentPipeline = [surface->device newComputePipelineStateWithFunction:segmentFunction
                                                                                  error:&error];

        backend->backpropPipeline = [surface->device newComputePipelineStateWithFunction:backpropFunction
                                                                                   error:&error];

        backend->mergePipeline = [surface->device newComputePipelineStateWithFunction:mergeFunction
                                                                                error:&error];

        backend->rasterPipeline = [surface->device newComputePipelineStateWithFunction:rasterFunction
                                                                                 error:&error];

        MTLRenderPipelineDescriptor* pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineStateDescriptor.label = @"blit pipeline";
        pipelineStateDescriptor.vertexFunction = vertexFunction;
        pipelineStateDescriptor.fragmentFunction = fragmentFunction;
        pipelineStateDescriptor.colorAttachments[0].pixelFormat = surface->mtlLayer.pixelFormat;
        pipelineStateDescriptor.colorAttachments[0].blendingEnabled = YES;
        pipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
        pipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
        pipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        pipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
        pipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
        pipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

        backend->blitPipeline = [surface->device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&err];

        //NOTE: create textures
        oc_vec2 size = surface->interface.getSize((oc_surface_data*)surface);
        f32 scale = surface->mtlLayer.contentsScale;

        backend->frameSize = (oc_vec2){ size.x * scale, size.y * scale };

        MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
        texDesc.textureType = MTLTextureType2D;
        texDesc.storageMode = MTLStorageModePrivate;
        texDesc.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite | MTLTextureUsageRenderTarget;
        texDesc.pixelFormat = MTLPixelFormatRGBA8Unorm;
        texDesc.width = backend->frameSize.x;
        texDesc.height = backend->frameSize.y;

        backend->outTexture = [surface->device newTextureWithDescriptor:texDesc];

        //NOTE: create buffers

        backend->bufferSemaphore = dispatch_semaphore_create(OC_MTL_INPUT_BUFFERS_COUNT);
        backend->bufferIndex = 0;

        MTLResourceOptions bufferOptions = MTLResourceCPUCacheModeWriteCombined
                                         | MTLResourceStorageModeShared;

        for(int i = 0; i < OC_MTL_INPUT_BUFFERS_COUNT; i++)
        {
            backend->pathBuffer[i] = [surface->device newBufferWithLength:OC_MTL_DEFAULT_PATH_BUFFER_LEN * sizeof(oc_mtl_path)
                                                                  options:bufferOptions];

            backend->elementBuffer[i] = [surface->device newBufferWithLength:OC_MTL_DEFAULT_ELT_BUFFER_LEN * sizeof(oc_mtl_path_elt)
                                                                     options:bufferOptions];
        }

        bufferOptions = MTLResourceStorageModePrivate;
        backend->segmentBuffer = [surface->device newBufferWithLength:OC_MTL_DEFAULT_SEGMENT_BUFFER_LEN * sizeof(oc_mtl_segment)
                                                              options:bufferOptions];

        backend->segmentCountBuffer = [surface->device newBufferWithLength:sizeof(int)
                                                                   options:MTLResourceStorageModeShared];

        backend->pathQueueBuffer = [surface->device newBufferWithLength:OC_MTL_DEFAULT_PATH_QUEUE_BUFFER_LEN * sizeof(oc_mtl_path_queue)
                                                                options:bufferOptions];

        backend->tileQueueBuffer = [surface->device newBufferWithLength:OC_MTL_DEFAULT_TILE_QUEUE_BUFFER_LEN * sizeof(oc_mtl_tile_queue)
                                                                options:bufferOptions];

        backend->tileQueueCountBuffer = [surface->device newBufferWithLength:sizeof(int)
                                                                     options:bufferOptions];

        backend->tileOpBuffer = [surface->device newBufferWithLength:OC_MTL_DEFAULT_TILE_OP_BUFFER_LEN * sizeof(oc_mtl_tile_op)
                                                             options:bufferOptions];

        backend->tileOpCountBuffer = [surface->device newBufferWithLength:sizeof(int)
                                                                  options:bufferOptions];

        backend->rasterDispatchBuffer = [surface->device newBufferWithLength:sizeof(MTLDispatchThreadgroupsIndirectArguments)
                                                                     options:bufferOptions];

        int tileSize = OC_MTL_TILE_SIZE;
        int nTilesX = (int)(backend->frameSize.x + tileSize - 1) / tileSize;
        int nTilesY = (int)(backend->frameSize.y + tileSize - 1) / tileSize;
        backend->screenTilesBuffer = [surface->device newBufferWithLength:nTilesX * nTilesY * sizeof(oc_mtl_screen_tile)
                                                                  options:bufferOptions];

        bufferOptions = MTLResourceStorageModeShared;
        for(int i = 0; i < OC_MTL_INPUT_BUFFERS_COUNT; i++)
        {
            backend->logBuffer[i] = [surface->device newBufferWithLength:1 << 20
                                                                 options:bufferOptions];

            backend->logOffsetBuffer[i] = [surface->device newBufferWithLength:sizeof(int)
                                                                       options:bufferOptions];
        }
    }
    return ((oc_canvas_backend*)backend);
}

oc_surface_data* oc_mtl_canvas_surface_create_for_window(oc_window window)
{
    oc_mtl_surface* surface = (oc_mtl_surface*)oc_mtl_surface_create_for_window(window);

    if(surface)
    {
        surface->interface.backend = oc_mtl_canvas_backend_create(surface);
        if(surface->interface.backend)
        {
            surface->interface.api = OC_CANVAS;
        }
        else
        {
            surface->interface.destroy((oc_surface_data*)surface);
            surface = 0;
        }
    }
    return ((oc_surface_data*)surface);
}
