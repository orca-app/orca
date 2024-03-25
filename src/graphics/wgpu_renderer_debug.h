/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#ifndef __WGPU_RENDERER_DEBUG_H_
#define __WGPU_RENDERER_DEBUG_H_

#include "graphics.h"

//TODO: remove?
typedef enum
{
    OC_WGPU_CANVAS_TIMING_NONE = 0,
    OC_WGPU_CANVAS_TIMING_FRAME = 1 << 0,
    OC_WGPU_CANVAS_TIMING_ALL = OC_WGPU_CANVAS_TIMING_FRAME,
    //...
} oc_wgpu_canvas_timing_flags;

typedef struct oc_wgpu_canvas_record_options
{
    u32 maxRecordCount;
    u32 timingFlags;

} oc_wgpu_canvas_record_options;

typedef struct oc_wgpu_canvas_batch_counters
{
    oc_list_elt listElt;

    u32 encodedPathCount;
    u32 encodedElementCount;

} oc_wgpu_canvas_batch_counters;

enum
{
    OC_WGPU_CANVAS_FRAME_STATS_FIELD_COUNT = 3
};

typedef struct oc_wgpu_canvas_frame_counters
{
    oc_list_elt listElt;

    u32 frameIndex;

    u32 inputPathCount;
    u32 inputElementCount;

    union
    {
        struct
        {
            f64 gpuTime;
            f64 cpuEncodeTime;
            f64 cpuFrameTime;
        };

        f64 frameTimings[OC_WGPU_CANVAS_FRAME_STATS_FIELD_COUNT];
    };

    u64 batchCount;
    u64 recordedBatchCount;
    oc_list batches;

} oc_wgpu_canvas_frame_counters;

typedef struct oc_wgpu_canvas_stats
{
    u64 sampleCount;
    f64 minSample;
    f64 maxSample;
    f64 avg;
    f64 std;
} oc_wgpu_canvas_stats;

enum
{
    OC_WGPU_CANVAS_STATS_BUFFER_SIZE = 4096,
    OC_WGPU_CANVAS_STATS_MIN_SAMPLE_COUNT = 60,
};

typedef struct oc_wgpu_canvas_stats_buffer
{
    int sampleCount;
    int nextSample;
    f64 samples[OC_WGPU_CANVAS_STATS_BUFFER_SIZE];
} oc_wgpu_canvas_stats_buffer;

typedef struct oc_wgpu_canvas_frame_stats
{
    union
    {
        struct
        {
            oc_wgpu_canvas_stats gpuTime;
            oc_wgpu_canvas_stats cpuEncodeTime;
            oc_wgpu_canvas_stats cpuFrameTime;
        };

        oc_wgpu_canvas_stats frameStats[OC_WGPU_CANVAS_FRAME_STATS_FIELD_COUNT];
    };
} oc_wgpu_canvas_frame_stats;

typedef struct oc_wgpu_canvas_debug_display_options
{
    bool showTileBorders;
    bool showPathArea;
    bool showClip;
    bool textureOff;
    bool debugTileQueues;
    i32 pathStart;
    i32 pathCount;

} oc_wgpu_canvas_debug_display_options;

ORCA_API void oc_wgpu_canvas_debug_set_record_options(oc_canvas_renderer handle, oc_wgpu_canvas_record_options* options);
ORCA_API oc_wgpu_canvas_record_options oc_wgpu_canvas_debug_get_record_options(oc_canvas_renderer handle);

ORCA_API void oc_wgpu_canvas_debug_clear_records(oc_canvas_renderer handle);
ORCA_API oc_list oc_wgpu_canvas_debug_get_records(oc_canvas_renderer handle);
ORCA_API void oc_wgpu_canvas_debug_log_records(oc_list records);

ORCA_API oc_wgpu_canvas_frame_stats oc_wgpu_canvas_get_frame_stats(oc_canvas_renderer handle, int desiredSampleCount);
ORCA_API int oc_wgpu_canvas_stats_sample_count_for_95_confidence(oc_wgpu_canvas_stats* stats, f32 marginRelativeToMean);

ORCA_API void oc_wgpu_canvas_debug_set_display_options(oc_canvas_renderer handle, oc_wgpu_canvas_debug_display_options* options);
ORCA_API oc_wgpu_canvas_debug_display_options oc_wgpu_canvas_debug_get_display_options(oc_canvas_renderer handle);

#endif //__WGPU_RENDERER_DEBUG_H_
