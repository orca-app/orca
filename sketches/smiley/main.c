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

oc_font create_font()
{
    //NOTE(martin): create font
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 fontPath = oc_path_executable_relative(scratch.arena, OC_STR8("resources/OpenSansLatinSubset.ttf"));
    char* fontPathCString = oc_str8_to_cstring(scratch.arena, fontPath);

    FILE* fontFile = fopen(fontPathCString, "r");
    if(!fontFile)
    {
        oc_log_error("Could not load font file '%s': %s\n", fontPathCString, strerror(errno));
        oc_scratch_end(scratch);
        return (oc_font_nil());
    }
    unsigned char* fontData = 0;
    fseek(fontFile, 0, SEEK_END);
    u32 fontDataSize = ftell(fontFile);
    rewind(fontFile);
    fontData = (unsigned char*)malloc(fontDataSize);
    fread(fontData, 1, fontDataSize, fontFile);
    fclose(fontFile);

    oc_unicode_range ranges[5] = { OC_UNICODE_BASIC_LATIN,
                                   OC_UNICODE_C1_CONTROLS_AND_LATIN_1_SUPPLEMENT,
                                   OC_UNICODE_LATIN_EXTENDED_A,
                                   OC_UNICODE_LATIN_EXTENDED_B,
                                   OC_UNICODE_SPECIALS };

    oc_font font = oc_font_create_from_memory(oc_str8_from_buffer(fontDataSize, (char*)fontData), 5, ranges);
    free(fontData);
    oc_scratch_end(scratch);
    return (font);
}

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

    oc_font font = create_font();

    // start app
    oc_window_bring_to_front(window);
    oc_window_focus(window);

    f32 x = 400, y = 300;
    f32 speed = 0;
    f32 dx = speed, dy = speed;
    f64 frameTime = 0;

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
                        f32 factor = (event->key.mods & OC_KEYMOD_SHIFT) ? 10 : 1;

                        if(event->key.keyCode == OC_KEY_LEFT)
                        {
                            x -= 0.3 * factor;
                        }
                        else if(event->key.keyCode == OC_KEY_RIGHT)
                        {
                            x += 0.3 * factor;
                        }
                        else if(event->key.keyCode == OC_KEY_UP)
                        {
                            y -= 0.3 * factor;
                        }
                        else if(event->key.keyCode == OC_KEY_DOWN)
                        {
                            y += 0.3 * factor;
                        }
                    }
                }
                break;

                default:
                    break;
            }
        }

        if(x - 200 < 0)
        {
            x = 200;
            dx = speed;
        }
        if(x + 200 > contentRect.w)
        {
            x = contentRect.w - 200;
            dx = -speed;
        }
        if(y - 200 < 0)
        {
            y = 200;
            dy = speed;
        }
        if(y + 200 > contentRect.h)
        {
            y = contentRect.h - 200;
            dy = -speed;
        }
        x += dx;
        y += dy;

        // background
        oc_set_color_rgba(0, 1, 1, 1);
        oc_clear();

        oc_set_color_rgba(1, 0, 1, 1);
        oc_rectangle_fill(0, 0, 100, 100);

        // head
        oc_set_color_rgba(1, 1, 0, 1);

        oc_circle_fill(x, y, 200);

        // smile
        f32 frown = frameTime > 0.033 ? -100 : 0;

        oc_set_color_rgba(0, 0, 0, 1);
        oc_set_width(20);
        oc_move_to(x - 100, y + 100);
        oc_cubic_to(x - 50, y + 150 + frown, x + 50, y + 150 + frown, x + 100, y + 100);
        oc_stroke();

        // eyes
        oc_ellipse_fill(x - 70, y - 50, 30, 50);
        oc_ellipse_fill(x + 70, y - 50, 30, 50);

        // text
        oc_set_color_rgba(0, 0, 1, 1);
        oc_set_font(font);
        oc_set_font_size(12);
        oc_move_to(50, 600 - 50);

        oc_str8 text = oc_str8_pushf(scratch.arena,
                                     "Orca vector graphics test program (frame time = %fs, fps = %f)...",
                                     frameTime,
                                     1. / frameTime);
        oc_text_outlines(text);
        oc_fill();

        oc_log_info("Orca vector graphics test program (frame time = %fs, fps = %f)...\n",
                    frameTime,
                    1. / frameTime);

        oc_canvas_render(renderer, context, surface);
        oc_canvas_present(renderer, surface);

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
