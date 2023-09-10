
#include <stdio.h>
#include <stdlib.h>

#include "orca.h"

int main()
{
    oc_init();

    oc_rect windowRect = { .x = 100, .y = 100, .w = 810, .h = 610 };
    oc_window window = oc_window_create(windowRect, OC_STR8("test"), 0);

    oc_rect contentRect = oc_window_get_content_rect(window);

    //NOTE: create surface
    oc_surface surface1 = oc_surface_create_for_window(window, OC_CANVAS);
    if(oc_surface_is_nil(surface1))
    {
        printf("Error: couldn't create surface 1\n");
        return (-1);
    }
    oc_surface_swap_interval(surface1, 0);
    //*
    oc_surface surface2 = oc_surface_create_for_window(window, OC_CANVAS);
    if(oc_surface_is_nil(surface2))
    {
        printf("Error: couldn't create surface 2\n");
        return (-1);
    }
    oc_surface_swap_interval(surface2, 0);
    //*/
    oc_canvas canvas1 = oc_canvas_create();
    if(oc_canvas_is_nil(canvas1))
    {
        printf("Error: couldn't create canvas 1\n");
        return (-1);
    }
    //*
    oc_canvas canvas2 = oc_canvas_create();
    if(oc_canvas_is_nil(canvas2))
    {
        printf("Error: couldn't create canvas 2\n");
        return (-1);
    }
    //*/
    // start app
    oc_window_center(window);
    oc_window_bring_to_front(window);
    oc_window_focus(window);

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
                        if(event->key.code == OC_KEY_UP)
                        {
                            oc_surface_bring_to_front(surface2);
                        }
                        else if(event->key.code == OC_KEY_DOWN)
                        {
                            oc_surface_send_to_back(surface2);
                        }
                    }
                }
                break;

                default:
                    break;
            }
        }

        oc_surface_select(surface1);
        oc_canvas_select(canvas1);

        oc_set_color_rgba(0, 0, 0.5, 0.5);
        oc_clear();

        oc_set_color_rgba(1, 0, 0, 1);
        oc_rectangle_fill(100, 100, 300, 150);

        oc_render(canvas1);

        //*
        oc_surface_select(surface2);
        oc_canvas_select(canvas2);

        oc_set_color_rgba(0, 0, 0, 0);
        oc_clear();

        oc_set_color_rgba(0, 0, 1, 1);
        oc_rectangle_fill(200, 200, 300, 200);

        oc_render(canvas2);
        //*/

        oc_surface_present(surface1);
        oc_surface_present(surface2);

        oc_arena_clear(oc_scratch());
    }

    oc_canvas_destroy(canvas1);
    oc_surface_destroy(surface1);
    oc_canvas_destroy(canvas2);
    oc_surface_destroy(surface2);

    oc_window_destroy(window);

    oc_terminate();

    return (0);
}
