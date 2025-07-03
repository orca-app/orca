/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "orca.h"

int main()
{
    oc_init();

    oc_rect windowRect = { .x = 100, .y = 100, .w = 800, .h = 600 };
    oc_window window = oc_window_create(windowRect, OC_STR8("test"), 0);

    oc_rect contentRect = oc_window_get_content_rect(window);

    //NOTE: create renderer, surface, and context

    oc_canvas_renderer renderer = oc_canvas_renderer_create();
    if(oc_canvas_renderer_is_nil(renderer))
    {
        oc_log_error("Error: couldn't create renderer\n");
        return (-1);
    }

    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 path = oc_path_executable_relative(scratch.arena, OC_STR8("resources/triceratops.png"));
    oc_image image = oc_image_create_from_path(renderer, path, false);

    if(oc_image_is_nil(image))
    {
        oc_log_error("Couldn't create image\n");
        return (-1);
    }

    oc_vec2 imageSize = oc_image_size(image);

    oc_surface surface = oc_canvas_surface_create_for_window(renderer, window);
    if(oc_surface_is_nil(surface))
    {
        oc_log_error("Error: couldn't create surface\n");
        return (-1);
    }

    oc_canvas_context context = oc_canvas_context_create();
    if(oc_canvas_context_is_nil(context))
    {
        oc_log_error("Error: couldn't create canvas\n");
        return (-1);
    }

    // start app
    oc_window_bring_to_front(window);
    oc_window_focus(window);

    f64 frameTime = 0;

    typedef struct shape_info
    {
        oc_vec2 pos;
        oc_color col;
    } shape_info;

    const f32 triangleSize = 30;
    oc_vec2 triangleVertices[3] = {
        { 0, -triangleSize },
        { triangleSize * cosf(-1 * M_PI / 6), -triangleSize * sinf(-1 * M_PI / 6) },
        { triangleSize * cosf(-5 * M_PI / 6), -triangleSize * sinf(-5 * M_PI / 6) },
    };

    const u32 SHAPE_COUNT = 100000;
    shape_info* shapes = oc_arena_push_array(scratch.arena, shape_info, SHAPE_COUNT);
    for(int i = 0; i < SHAPE_COUNT; i++)
    {
        shapes[i] = (shape_info){
            .pos = {
                (rand() % (u32)windowRect.w),
                (rand() % (u32)windowRect.h),
            },
            .col = {
                (rand() % 255) / 255.,
                (rand() % 255) / 255.,
                (rand() % 255) / 255.,
                1,
            },
        };
    }

    while(!oc_should_quit())
    {
        f64 startTime = oc_clock_time(OC_CLOCK_MONOTONIC);
        oc_arena_scope scratch = oc_scratch_begin();

        oc_pump_events(0);
        oc_event* event = 0;
        while((event = oc_next_event(scratch.arena)) != 0)
        {
            switch(event->type)
            {
                case OC_EVENT_WINDOW_CLOSE:
                {
                    oc_request_quit();
                }
                break;

                default:
                    break;
            }
        }

        oc_set_color_rgba(0, 1, 1, 1);
        oc_clear();

        for(int i = 0; i < SHAPE_COUNT; i++)
        {
            shape_info* shape = &shapes[i];
            oc_move_to(shape->pos.x + triangleVertices[0].x, shape->pos.y + triangleVertices[0].y);
            oc_line_to(shape->pos.x + triangleVertices[1].x, shape->pos.y + triangleVertices[1].y);
            oc_line_to(shape->pos.x + triangleVertices[2].x, shape->pos.y + triangleVertices[2].y);
            oc_close_path();

            oc_set_color(shape->col);
            oc_fill();
        }

        oc_canvas_render(renderer, context, surface);

        oc_scratch_end(scratch);

        frameTime = oc_clock_time(OC_CLOCK_MONOTONIC) - startTime;
        oc_log_info("frameTime = %.2fms (fps = %.2f)\n", frameTime * 1000, 1. / frameTime);
    }

    oc_canvas_context_destroy(context);
    oc_surface_destroy(surface);
    oc_canvas_renderer_destroy(renderer);
    oc_window_destroy(window);

    oc_terminate();

    return (0);
}
