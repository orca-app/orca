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
#include "graphics/wgpu_renderer_debug.h"

#include "ext/stb/stb_image.h"

int main()
{
    oc_init();

    oc_rect windowRect = { .x = 100, .y = 100, .w = 810, .h = 610 };
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

    oc_str8 dalaiPath = oc_path_executable_relative(scratch.arena, OC_STR8("../../resources/gamma_dalai_lama_gray.png"));
    oc_image dalai = oc_image_create_from_path(renderer, dalaiPath, false);

    oc_str8 offensivePath = oc_path_executable_relative(scratch.arena, OC_STR8("../../resources/gamma-1.0-or-2.2.png"));
    oc_image offensive = oc_image_create_from_path(renderer, offensivePath, false);

    oc_wgpu_canvas_debug_set_record_options(renderer,
                                            &(oc_wgpu_canvas_record_options){
                                                .maxRecordCount = 1,
                                                .timingFlags = OC_WGPU_CANVAS_TIMING_ALL,
                                            });

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
    i32 sampleCountIndex = 3;
    i32 jointIndex = 0;
    i32 strokeWidth = 10;

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

        // black to white, linear, cpu
        int y = 0;
        for(int i = 0; i < 64; i++)
        {
            oc_set_color_rgba(4 * i / 256., 4 * i / 256., 4 * i / 256., 1);
            oc_rectangle_fill(10 + i * 8, y, 8, 20);
        }
        y += 25;

        // black to white, linear, gpu
        oc_set_gradient(OC_GRADIENT_BLEND_LINEAR,
                        (oc_color){ 0, 0, 0, 1 },
                        (oc_color){ 1, 1, 1, 1 },
                        (oc_color){ 1, 1, 1, 1 },
                        (oc_color){ 0, 0, 0, 1 });

        oc_rectangle_fill(10, y, 8 * 64, 20);
        y += 25;

        // black to white, srgb, cpu
        for(int i = 0; i < 64; i++)
        {
            oc_set_color_srgba(4 * i / 256., 4 * i / 256., 4 * i / 256., 1);
            oc_rectangle_fill(10 + i * 8, y, 8, 20);
        }
        y += 25;

        // black to white, srgb, gpu
        oc_set_gradient(OC_GRADIENT_BLEND_SRGB,
                        (oc_color){ 0, 0, 0, 1 },
                        (oc_color){ 1, 1, 1, 1 },
                        (oc_color){ 1, 1, 1, 1 },
                        (oc_color){ 0, 0, 0, 1 });

        oc_rectangle_fill(10, y, 8 * 64, 20);
        y += 25;

        // red to green, linear, cpu
        for(int i = 0; i < 64; i++)
        {
            oc_set_color_rgba((255 - 4 * i) / 256., 4 * i / 256., 0, 1);
            oc_rectangle_fill(10 + i * 8, y, 8, 20);
        }
        y += 25;

        // red to green, linear, gpu
        oc_set_gradient(OC_GRADIENT_BLEND_LINEAR,
                        (oc_color){ 1, 0, 0, 1 },
                        (oc_color){ 0, 1, 0, 1 },
                        (oc_color){ 0, 1, 0, 1 },
                        (oc_color){ 1, 0, 0, 1 });

        oc_rectangle_fill(10, y, 8 * 64, 20);
        y += 25;

        // red to green, srgb, cpu
        for(int i = 0; i < 64; i++)
        {
            oc_set_color_srgba((255 - 4 * i) / 256., 4 * i / 256., 0, 1);
            oc_rectangle_fill(10 + i * 8, y, 8, 20);
        }
        y += 25;

        // red to green, srgb, gpu
        oc_set_gradient(OC_GRADIENT_BLEND_SRGB,
                        (oc_color){ 1, 0, 0, 1 },
                        (oc_color){ 0, 1, 0, 1 },
                        (oc_color){ 0, 1, 0, 1 },
                        (oc_color){ 1, 0, 0, 1 });

        oc_rectangle_fill(10, y, 8 * 64, 20);
        y += 25;

        // dalai lama image rescaled
        oc_image_draw(dalai, (oc_rect){ 10, y, 129, 111 });
        y += 120;

        // offensive image rescaled
        oc_image_draw(offensive, (oc_rect){ 10, y, 256, 128 });

        for(int i = 0; i < 16; i++)
        {
            f32 l = i / 256.;

            oc_set_color_rgba(l, l, l, 1);
            oc_rectangle_fill(300 + i * 30, y, 30, 30);

            f32 s = l * 12.92;
            if(l > 0.0031308)
            {
                s = 1.055 * powf(l, 1.0 / 2.4) - 0.055;
            }

            oc_set_color_srgba(s, s, s, 1);
            oc_rectangle_fill(300 + i * 30, y + 30, 30, 30);
        }

        oc_canvas_render(renderer, context, surface);
        oc_canvas_present(renderer, surface);

        oc_scratch_end(scratch);
    }

    oc_terminate();

    return (0);
}
