/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <math.h>
#include <orca.h>

oc_surface surface = { 0 };
oc_canvas_renderer renderer = { 0 };
oc_canvas_context context = { 0 };
oc_font font = { 0 };
oc_vec2 frameSize = { 100, 100 };
f64 lastSeconds = 0;

ORCA_EXPORT void oc_on_init(void)
{
    oc_window_set_title(OC_STR8("Test"));
    oc_window_set_size((oc_vec2){ .x = 400, .y = 400 });

    renderer = oc_canvas_renderer_create();
    surface = oc_canvas_surface_create(renderer);
    context = oc_canvas_context_create();
}

ORCA_EXPORT void oc_on_resize(u32 width, u32 height)
{
    frameSize.x = width;
    frameSize.y = height;
}

ORCA_EXPORT void oc_on_frame_refresh(void)
{
    oc_canvas_context_select(context);
    oc_set_color_rgba(.9, 0, .9, 1);
    oc_clear();

    const f32 centerX = frameSize.x / 2;
    const f32 centerY = frameSize.y / 2;
    const f32 radius = oc_min(frameSize.x, frameSize.y) * 0.5f * 0.85f;

    oc_set_color_rgba(0.2, 0.2, 0.2, 1);
    oc_circle_fill(centerX, centerY, radius);

    oc_canvas_render(renderer, context, surface);
    oc_canvas_present(renderer, surface);
}
