/************************************************************/ /**
*
*	@file: main.cpp
*	@author: Martin Fouilleul
*	@date: 30/07/2022
*	@revision:
*
*****************************************************************/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "orca.h"

int main()
{
    oc_init();

    oc_rect windowRect = { .x = 100, .y = 100, .w = 810, .h = 610 };
    oc_window window = oc_window_create(windowRect, OC_STR8("test"), 0);

    oc_rect contentRect = oc_window_get_content_rect(window);

    //NOTE: create surface
    oc_surface surface = oc_surface_create_for_window(window, OC_CANVAS);
    oc_surface_swap_interval(surface, 1);

    if(oc_surface_is_nil(surface))
    {
        oc_log_error("Error: couldn't create surface\n");
        return (-1);
    }

    //TODO: create canvas
    oc_canvas canvas = oc_canvas_create();

    if(oc_canvas_is_nil(canvas))
    {
        oc_log_error("Error: couldn't create canvas\n");
        return (-1);
    }

    // start app
    oc_window_bring_to_front(window);
    oc_window_focus(window);

    f64 frameTime = 0;
    f32 x = 0, y = 0;

    while(!oc_should_quit())
    {
        f64 startTime = oc_clock_time(OC_CLOCK_MONOTONIC);

        oc_pump_events(0);
        oc_event* event = 0;
        while((event = oc_next_event(oc_scratch())) != 0)
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
                    if(event->key.action == OC_KEY_PRESS)
                    {
                        if(event->key.code == OC_KEY_LEFT)
                        {
                            x -= 1;
                        }
                        if(event->key.code == OC_KEY_RIGHT)
                        {
                            x += 1;
                        }
                        if(event->key.code == OC_KEY_UP)
                        {
                            y -= 1;
                        }
                        if(event->key.code == OC_KEY_DOWN)
                        {
                            y += 1;
                        }
                    }
                }
                break;

                default:
                    break;
            }
        }

        oc_surface_select(surface);

        // background
        oc_set_color_rgba(0, 1, 1, 1);
        oc_clear();

        oc_move_to(100, 100);
        oc_line_to(150, 150);
        oc_line_to(100, 200);
        oc_line_to(50, 150);
        oc_close_path();
        oc_set_color_rgba(1, 0, 0, 1);
        oc_fill();

        oc_move_to(200, 100);
        oc_line_to(410, 100);
        oc_line_to(410, 200);
        oc_line_to(200, 200);
        oc_close_path();
        oc_set_color_rgba(0, 1, 0, 1);
        oc_fill();

        oc_set_color_rgba(0, 0.5, 1, 0.5);
        oc_rectangle_fill(120, 120, 200, 200);

        oc_set_color_rgba(1, 0, 0.5, 1);
        oc_rectangle_fill(700, 500, 200, 200);

        oc_move_to(300, 300);
        oc_quadratic_to(400, 500, 500, 300);
        oc_close_path();
        oc_set_color_rgba(0, 0, 1, 1);
        oc_fill();

        oc_move_to(200, 450);
        oc_cubic_to(200, 250, 400, 550, 400, 450);
        oc_close_path();
        oc_set_color_rgba(1, 0.5, 0, 1);
        oc_fill();

        /*
			oc_set_joint(OC_JOINT_NONE);
			oc_set_max_joint_excursion(20);

			oc_set_cap(OC_CAP_SQUARE);

			oc_move_to(x+200, y+200);
			oc_line_to(x+300, y+300);
			oc_line_to(x+200, y+400);
			oc_line_to(x+100, y+300);
			oc_close_path();
			oc_set_color_rgba(1, 0, 0, 1);
		//	oc_set_width(2);
			oc_stroke();

			oc_move_to(400, 400);
			oc_quadratic_to(600, 601, 800, 400);
			oc_set_color_rgba(0, 0, 1, 1);
			oc_stroke();

			oc_move_to(x+400, y+300);
			oc_cubic_to(x+400, y+100, x+600, y+400, x+600, y+300);
			oc_close_path();
			oc_set_color_rgba(0, 0, 1, 1);
			oc_stroke();

			oc_set_color_rgba(1, 0, 0, 1);
			oc_rounded_rectangle_fill(100, 100, 200, 300, 20);

			oc_move_to(x+8, y+8);
			oc_line_to(x+33, y+8);
			oc_line_to(x+33, y+19);
			oc_line_to(x+8, y+19);
			oc_close_path();
			oc_set_color_rgba(0, 0, 1, 1);
			oc_fill();
*/

        oc_set_width(1);
        oc_set_color_rgba(1, 0, 0, 1);
        oc_rounded_rectangle_stroke(400, 400, 160, 160, 80);

        oc_log_info("Orca vector graphics test program (frame time = %fs, fps = %f)...\n",
                    frameTime,
                    1. / frameTime);

        oc_render(surface, canvas);
        oc_surface_present(surface);

        oc_arena_clear(oc_scratch());
        frameTime = oc_clock_time(OC_CLOCK_MONOTONIC) - startTime;
    }

    oc_canvas_destroy(canvas);
    oc_surface_destroy(surface);
    oc_window_destroy(window);

    oc_terminate();

    return (0);
}
