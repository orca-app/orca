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

int main()
{
    oc_init();

    oc_rect windowRect = { .x = 100, .y = 100, .w = 810, .h = 610 };
    oc_window window = oc_window_create(windowRect, OC_STR8("test"), 0);

    oc_rect contentRect = oc_window_get_content_rect(window);

    oc_canvas_renderer renderer = oc_canvas_renderer_create();
    if(oc_canvas_renderer_is_nil(renderer))
    {
        oc_log_error("Error: couldn't create renderer\n");
        return (-1);
    }

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

    oc_str8 imagePath = oc_path_executable_relative(scratch.arena, OC_STR8("../../resources/triceratops.png"));
    oc_image image = oc_image_create_from_path(renderer, imagePath, false);
    oc_vec2 imageSize = oc_image_size(image);

    oc_str8 imagePath2 = oc_path_executable_relative(scratch.arena, OC_STR8("../../resources/Top512.png"));
    oc_image image2 = oc_image_create_from_path(renderer, imagePath2, false);
    oc_vec2 imageSize2 = oc_image_size(image2);

    oc_scratch_end(scratch);
    // start app
    oc_window_bring_to_front(window);
    oc_window_focus(window);

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

                default:
                    break;
            }
        }

        oc_set_color_rgba(0, 1, 1, 1);
        oc_clear();

        oc_set_color_rgba(1, 1, 1, 1);
        /*
			oc_matrix_multiply_push((oc_mat2x3){0.707, -0.707, 200,
			                           0.707, 0.707, 100});
			oc_set_image(image);
			oc_set_image_source_region((oc_rect){500, 500, 2000, 1400});

			oc_move_to(0, 0);
			oc_line_to(200, 0);
			oc_line_to(300, 100);
			oc_line_to(200, 200);
			oc_line_to(0, 200);
			oc_line_to(100, 100);
			oc_close_path();
			oc_fill();

			oc_matrix_pop();

			oc_image_draw(image2, (oc_rect){300, 200, 300, 300});
*/
        oc_image_draw(image, (oc_rect){ 100, 100, 362, 256 });
        oc_image_draw(image2, (oc_rect){ 300, 200, 300, 300 });

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
