/************************************************************/ /**
*
*	@file: main.cpp
*	@author: Martin Fouilleul
*	@date: 30/07/2022
*	@revision:
*
*****************************************************************/
#include <errno.h>
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

    //NOTE: create surface
    oc_surface surface = oc_surface_create_for_window(window, OC_CANVAS);
    if(oc_surface_is_nil(surface))
    {
        oc_log_error("couldn't create surface\n");
        return (-1);
    }
    oc_surface_swap_interval(surface, 0);

    //NOTE: create canvas
    oc_canvas canvas = oc_canvas_create();
    if(oc_canvas_is_nil(canvas))
    {
        oc_log_error("Error: couldn't create canvas\n");
        return (-1);
    }

    //NOTE: create atlas
    oc_arena permanentArena = { 0 };
    oc_arena_init(&permanentArena);

    oc_rect_atlas* atlas = oc_rect_atlas_create(&permanentArena, 16000, 16000);
    oc_image atlasImage = oc_image_create(surface, 16000, 16000);

    oc_str8 path1 = oc_path_executable_relative(oc_scratch(), OC_STR8("../../../sketches/resources/triceratops.png"));
    oc_str8 path2 = oc_path_executable_relative(oc_scratch(), OC_STR8("../../../sketches/resources/Top512.png"));

    oc_image_region image1 = oc_image_atlas_alloc_from_file(atlas, atlasImage, path1, false);
    oc_image_region image2 = oc_image_atlas_alloc_from_file(atlas, atlasImage, path2, false);

    // start app
    oc_window_bring_to_front(window);
    oc_window_focus(window);

    while(!oc_should_quit())
    {
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

                default:
                    break;
            }
        }

        oc_surface_select(surface);

        oc_set_color_rgba(0, 1, 1, 1);
        oc_clear();

        oc_set_color_rgba(1, 1, 1, 1);

        oc_image_draw_region(image1.image, image1.rect, (oc_rect){ 100, 100, 300, 300 });
        oc_image_draw_region(image2.image, image2.rect, (oc_rect){ 300, 200, 300, 300 });

        oc_render(surface, canvas);
        oc_surface_present(surface);

        oc_arena_clear(oc_scratch());
    }

    oc_image_atlas_recycle(atlas, image1);
    oc_image_atlas_recycle(atlas, image2);

    oc_canvas_destroy(canvas);
    oc_surface_destroy(surface);
    oc_window_destroy(window);

    oc_terminate();

    return (0);
}
