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

    oc_wgpu_canvas_debug_set_record_options(renderer,
                                            &(oc_wgpu_canvas_record_options){
                                                .maxRecordCount = 1,
                                                .timingFlags = OC_WGPU_CANVAS_TIMING_ALL,
                                            });

    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 path = oc_path_executable_relative(scratch.arena, OC_STR8("../../resources/triceratops.png"));
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

                case OC_EVENT_KEYBOARD_KEY:
                {
                    if(event->key.action == OC_KEY_PRESS || event->key.action == OC_KEY_REPEAT)
                    {
                        if(event->key.keyCode == OC_KEY_A)
                        {
                            strokeWidth += 1;
                        }
                        if(event->key.keyCode == OC_KEY_Z)
                        {
                            strokeWidth -= 1;
                        }

                        if(event->key.keyCode == OC_KEY_UP)
                        {
                            sampleCountIndex += 1;
                        }
                        if(event->key.keyCode == OC_KEY_DOWN)
                        {
                            sampleCountIndex -= 1;
                        }

                        if(event->key.keyCode == OC_KEY_LEFT)
                        {
                            jointIndex += 1;
                        }
                        if(event->key.keyCode == OC_KEY_RIGHT)
                        {
                            jointIndex -= 1;
                        }

                        oc_wgpu_canvas_debug_display_options options = oc_wgpu_canvas_debug_get_display_options(renderer);

                        if(event->key.keyCode == OC_KEY_G && (event->key.mods & OC_KEYMOD_CMD))
                        {
                            options.showTileBorders = !options.showTileBorders;
                        }

                        if(event->key.keyCode == OC_KEY_C && (event->key.mods & OC_KEYMOD_CMD))
                        {
                            options.showClip = !options.showClip;
                        }

                        if(event->key.keyCode == OC_KEY_P && (event->key.mods & OC_KEYMOD_CMD))
                        {
                            options.showPathArea = !options.showPathArea;
                        }

                        if(event->key.keyCode == OC_KEY_T && (event->key.mods & OC_KEYMOD_CMD))
                        {
                            options.textureOff = !options.textureOff;
                        }

                        if(event->key.keyCode == OC_KEY_K && (event->key.mods & OC_KEYMOD_CMD))
                        {
                            options.debugTileQueues = !options.debugTileQueues;
                        }

                        if(event->key.keyCode == OC_KEY_UP)
                        {
                            if(event->key.mods & OC_KEYMOD_CTRL)
                            {
                                options.pathStart++;
                            }
                            if(event->key.mods & OC_KEYMOD_SHIFT)
                            {
                                options.pathCount++;
                            }
                        }

                        if(event->key.keyCode == OC_KEY_DOWN)
                        {
                            if(event->key.mods & OC_KEYMOD_CTRL)
                            {
                                options.pathStart--;
                            }
                            if(event->key.mods & OC_KEYMOD_SHIFT)
                            {
                                options.pathCount--;
                            }
                        }

                        oc_wgpu_canvas_debug_set_display_options(renderer, &options);
                    }
                }
                break;

                default:
                    break;
            }
        }
        strokeWidth = oc_clamp(strokeWidth, 1, 20);
        jointIndex = jointIndex % 2;

        sampleCountIndex = oc_clamp(sampleCountIndex, 0, 3);
        u32 sampleCount = 1 << sampleCountIndex;

        oc_canvas_context_set_msaa_sample_count(context, sampleCount);

        if(jointIndex == 0)
        {
            oc_set_joint(OC_JOINT_BEVEL);
        }
        else
        {
            oc_set_joint(OC_JOINT_MITER);
            oc_set_max_joint_excursion(50);
        }

        oc_set_image(oc_image_nil());

        // background
        oc_set_color_rgba(0, 1, 1, 1);
        oc_clear();

        oc_move_to(100, 50);
        oc_line_to(250, 100);
        oc_line_to(100, 100);
        oc_close_path();
        oc_set_color_rgba(1, 0.5, 0.1, 1);
        oc_fill();

        oc_move_to(150, 150);
        oc_quadratic_to(160, 200, 350, 250);
        oc_line_to(150, 250);
        oc_close_path();
        oc_set_color_rgba(0.3, 0.1, 1, 1);
        oc_fill();

        oc_move_to(300, 300);
        oc_quadratic_to(200, 350, 300, 400);
        oc_line_to(200, 400);
        oc_line_to(200, 300);
        oc_close_path();
        oc_set_color_rgba(0.9, 0.1, 0.1, 1);
        oc_fill();

        oc_set_gradient(
            (oc_color){ 0, 0, 0, 1 },
            (oc_color){ 1, 1, 1, 1 },
            (oc_color){ 0, 1, 0, 1 },
            (oc_color){ 1, 0, 0, 1 });

        //oc_set_color_rgba(1, 0, 1, 1);
        oc_rectangle_fill(500, 10, 200, 200);

        oc_move_to(350, 450);
        oc_cubic_to(250, 475, 450, 525, 350, 550);
        oc_cubic_to(150, 500, 150, 450, 350, 450);
        oc_set_color_rgba(1, 1, 1, 1);
        oc_set_image(image);
        oc_fill();

        /*
        oc_set_color_rgba(1, 1, 1, 1);
        oc_set_image(image);
        oc_set_image_source_region((oc_rect){ imageSize.x / 2, imageSize.y / 2, imageSize.x / 2, imageSize.y / 2 });
        oc_rectangle_fill(400, 200, 200, 200);
        */

        oc_clip_push(400, 200, 200, 100);
        oc_image_draw(image, (oc_rect){ 400, 200, 200, 200 });
        oc_clip_pop();

        oc_canvas_render(renderer, context, surface);
        oc_canvas_present(renderer, surface);
        oc_list debugRecords = oc_wgpu_canvas_debug_get_records(renderer);

        oc_wgpu_canvas_debug_log_records(debugRecords);

        oc_wgpu_canvas_debug_clear_records(renderer);

        oc_scratch_end(scratch);

        frameTime = oc_clock_time(OC_CLOCK_MONOTONIC) - startTime;
    }

    oc_canvas_context_destroy(context);
    oc_surface_destroy(surface);
    oc_canvas_renderer_destroy(renderer);
    oc_window_destroy(window);

    oc_terminate();

    return (0);
}
