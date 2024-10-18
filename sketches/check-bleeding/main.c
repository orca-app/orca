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

#define _USE_MATH_DEFINES //NOTE: necessary for MSVC
#include <math.h>

#include "orca.h"
#include "graphics/wgpu_renderer_debug.h"

int main()
{
    oc_init();

    oc_rect windowRect = { .x = 100, .y = 100, .w = 800, .h = 700 };
    oc_window window = oc_window_create(windowRect, OC_STR8("test"), 0);

    oc_rect contentRect = oc_window_get_content_rect(window);

    oc_canvas_renderer renderer = oc_canvas_renderer_create();
    if(oc_canvas_renderer_is_nil(renderer))
    {
        oc_log_error("Error: couldn't create renderer\n");
        return (-1);
    }

    oc_wgpu_canvas_debug_display_options displayOptions = {
        .showTileBorders = true,
    };

    oc_wgpu_canvas_debug_set_display_options(renderer, &displayOptions);

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

    //NOTE: create image
    oc_arena_scope scratch = oc_scratch_begin();

    oc_str8 imagePath = oc_path_executable_relative(scratch.arena, OC_STR8("../../resources/square_small.png"));
    oc_image image = oc_image_create_from_path(renderer, imagePath, false);
    oc_vec2 imageSize = oc_image_size(image);

    oc_scratch_end(scratch);
    // start app
    oc_window_bring_to_front(window);
    oc_window_focus(window);

    f32 x = 100;
    f32 y = 100;
    f32 width = 128;
    f32 height = 128;

    while(!oc_should_quit())
    {
        scratch = oc_scratch_begin();

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
                        if(event->key.keyCode == OC_KEY_LEFT)
                        {
                            if(event->key.mods & OC_KEYMOD_SHIFT)
                            {
                                x--;
                            }
                            else
                            {
                                width--;
                            }
                        }
                        else if(event->key.keyCode == OC_KEY_RIGHT)
                        {
                            if(event->key.mods & OC_KEYMOD_SHIFT)
                            {
                                x++;
                            }
                            else
                            {
                                width++;
                            }
                        }
                        else if(event->key.keyCode == OC_KEY_UP)
                        {
                            if(event->key.mods & OC_KEYMOD_SHIFT)
                            {
                                y--;
                            }
                            else
                            {
                                height--;
                            }
                        }
                        else if(event->key.keyCode == OC_KEY_DOWN)
                        {
                            if(event->key.mods & OC_KEYMOD_SHIFT)
                            {
                                y++;
                            }
                            else
                            {
                                height++;
                            }
                        }
                    }
                }
                break;

                default:
                    break;
            }
        }

        oc_set_color_rgba(0, 0, 0, 1);
        oc_clear();

        oc_set_color_rgba(0, 1, 0, 1);
        oc_rectangle_fill(x, y, width, height);

        oc_set_color_rgba(1, 1, 1, 1);
        oc_image_draw(image, (oc_rect){ x, y, width, height });

        oc_canvas_render(renderer, context, surface);
        oc_canvas_present(renderer, surface);

        oc_scratch_end(scratch);
    }

    oc_image_destroy(image);
    //    oc_canvas_renderer_destroy(renderer);
    oc_surface_destroy(surface);
    oc_window_destroy(window);

    oc_terminate();

    return (0);
}
