/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "graphics.h"
#include "surface.h"

typedef struct oc_image_base
{
    oc_list_elt listElt;
    u32 generation;
    oc_canvas_renderer renderer;
    oc_vec2 size;

} oc_image_base;

typedef struct oc_canvas_renderer_base oc_canvas_renderer_base;

typedef void (*oc_canvas_renderer_destroy_proc)(oc_canvas_renderer_base* renderer);

typedef oc_surface (*oc_canvas_renderer_create_surface_for_window_proc)(oc_canvas_renderer_base* renderer, oc_window window);

typedef oc_image_base* (*oc_canvas_renderer_image_create_proc)(oc_canvas_renderer_base* renderer, oc_vec2 size);
typedef void (*oc_canvas_renderer_image_destroy_proc)(oc_canvas_renderer_base* renderer, oc_image_base* image);
typedef void (*oc_canvas_renderer_image_upload_region_proc)(oc_canvas_renderer_base* renderer,
                                                            oc_image_base* image,
                                                            oc_rect region,
                                                            u8* pixels);

typedef void (*oc_canvas_renderer_submit_proc)(oc_canvas_renderer_base* renderer,
                                               oc_surface surface,
                                               u32 sampleCount,
                                               bool clear,
                                               oc_color clearColor,
                                               u32 primitiveCount,
                                               oc_primitive* primitives,
                                               u32 eltCount,
                                               oc_path_elt* pathElements);

typedef void (*oc_canvas_renderer_present_proc)(oc_canvas_renderer_base* renderer, oc_surface surface);

typedef struct oc_canvas_renderer_base
{
    oc_canvas_renderer_destroy_proc destroy;
    oc_canvas_renderer_create_surface_for_window_proc createSurfaceForWindow;
    oc_canvas_renderer_image_create_proc imageCreate;
    oc_canvas_renderer_image_destroy_proc imageDestroy;
    oc_canvas_renderer_image_upload_region_proc imageUploadRegion;
    oc_canvas_renderer_submit_proc submit;
    oc_canvas_renderer_present_proc present;

} oc_canvas_renderer_base;
