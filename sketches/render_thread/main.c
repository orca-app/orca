/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "orca.h"

oc_surface surface = { 0 };
oc_canvas canvas = { 0 };

i32 render_thread(void* user)
{
    while(!oc_should_quit())
    {
        oc_arena_scope scratch = oc_scratch_begin();

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

        oc_surface_select(surface);
        oc_canvas_select(canvas);

        oc_set_color_rgba(0, 0, 0.5, 0.5);
        oc_clear();

        oc_set_color_rgba(1, 0, 0, 1);
        oc_rectangle_fill(100, 100, 300, 150);

        oc_render(canvas);

        oc_surface_present(surface);

        oc_scratch_end(scratch);
    }
    return (0);
}

int main()
{
    oc_init();

    oc_rect windowRect = { .x = 100, .y = 100, .w = 810, .h = 610 };
    oc_window window = oc_window_create(windowRect, OC_STR8("test"), 0);

    oc_rect contentRect = oc_window_get_content_rect(window);

    //NOTE: create surface
    surface = oc_surface_create_for_window(window, OC_CANVAS);
    if(oc_surface_is_nil(surface))
    {
        printf("Error: couldn't create surface 1\n");
        return (-1);
    }
    oc_surface_swap_interval(surface, 0);

    canvas = oc_canvas_create();
    if(oc_canvas_is_nil(canvas))
    {
        printf("Error: couldn't create canvas 1\n");
        return (-1);
    }

    oc_surface dummy = oc_surface_create_for_window(window, OC_CANVAS);

    // start app
    oc_window_center(window);
    oc_window_bring_to_front(window);
    oc_window_focus(window);

    oc_thread* renderThread = oc_thread_create(render_thread, NULL);

    while(!oc_should_quit())
    {
        oc_pump_events(0);
    }

    oc_thread_join(renderThread, NULL);

    oc_canvas_destroy(canvas);
    oc_surface_destroy(surface);

    oc_window_destroy(window);

    oc_terminate();

    return (0);
}
