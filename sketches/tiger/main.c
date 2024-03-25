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

#include "tiger.c"

oc_font create_font()
{
    //NOTE(martin): create font
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 fontPath = oc_path_executable_relative(scratch.arena, OC_STR8("../../resources/OpenSansLatinSubset.ttf"));
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
    oc_canvas_context_set_msaa_sample_count(context, 8);

    oc_font font = create_font();

    // start app
    oc_window_bring_to_front(window);
    oc_window_focus(window);

    bool tracked = false;
    oc_vec2 trackPoint = { 0 };

    f32 zoom = 1;
    f32 startX = 300, startY = 200;
    bool singlePath = false;
    int singlePathIndex = 0;

    f64 frameTime = 0;

    oc_input_state inputState = { 0 };

    while(!oc_should_quit())
    {
        oc_arena_scope scratch = oc_scratch_begin();
        f64 startTime = oc_clock_time(OC_CLOCK_MONOTONIC);

        oc_pump_events(0);
        oc_event* event = 0;
        while((event = oc_next_event(scratch.arena)) != 0)
        {
            oc_input_process_event(scratch.arena, &inputState, event);

            switch(event->type)
            {
                case OC_EVENT_WINDOW_CLOSE:
                {
                    oc_request_quit();
                }
                break;

                case OC_EVENT_MOUSE_BUTTON:
                {
                    if(event->key.keyCode == OC_MOUSE_LEFT)
                    {
                        if(event->key.action == OC_KEY_PRESS)
                        {
                            tracked = true;
                            oc_vec2 mousePos = oc_mouse_position(&inputState);
                            trackPoint.x = (mousePos.x - startX) / zoom;
                            trackPoint.y = (mousePos.y - startY) / zoom;
                        }
                        else
                        {
                            tracked = false;
                        }
                    }
                }
                break;

                case OC_EVENT_MOUSE_WHEEL:
                {
                    oc_vec2 mousePos = oc_mouse_position(&inputState);
                    f32 pinX = (mousePos.x - startX) / zoom;
                    f32 pinY = (mousePos.y - startY) / zoom;

                    zoom *= 1 + event->mouse.deltaY * 0.01;
                    zoom = oc_clamp(zoom, 0.5, 5);

                    startX = mousePos.x - pinX * zoom;
                    startY = mousePos.y - pinY * zoom;
                }
                break;

                case OC_EVENT_KEYBOARD_KEY:
                {
                    if(event->key.action == OC_KEY_PRESS || event->key.action == OC_KEY_REPEAT)
                    {
                        switch(event->key.keyCode)
                        {
                            case OC_KEY_SPACE:
                                singlePath = !singlePath;
                                break;

                            case OC_KEY_UP:
                            {
                                if(event->key.mods & OC_KEYMOD_SHIFT)
                                {
                                    singlePathIndex++;
                                }
                                else if(event->key.mods & OC_KEYMOD_CTRL)
                                {
                                    startY -= 0.1;
                                }
                                else
                                {
                                    zoom += 0.001;
                                }
                            }
                            break;

                            case OC_KEY_DOWN:
                            {
                                if(event->key.mods & OC_KEYMOD_SHIFT)
                                {
                                    singlePathIndex--;
                                }
                                else if(event->key.mods & OC_KEYMOD_CTRL)
                                {
                                    startY += 0.1;
                                }

                                else
                                {
                                    zoom -= 0.001;
                                }
                            }
                            break;

                            default:
                                break;
                        }
                    }
                }
                break;

                default:
                    break;
            }
        }

        if(tracked)
        {
            oc_vec2 mousePos = oc_mouse_position(&inputState);
            startX = mousePos.x - trackPoint.x * zoom;
            startY = mousePos.y - trackPoint.y * zoom;
        }

        oc_set_color_rgba(1, 0, 1, 1);
        oc_clear();

        oc_matrix_multiply_push((oc_mat2x3){ zoom, 0, startX,
                                             0, zoom, startY });

        draw_tiger(singlePath, singlePathIndex);

        if(singlePath)
        {
            oc_log_info("display single path %i\n", singlePathIndex);
            oc_log_info("viewpos = (%f, %f), zoom = %f\n", startX, startY, zoom);
        }

        oc_matrix_pop();

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

        /*
        oc_log_info("Orca vector graphics test program (frame time = %fs, fps = %f)...\n",
                    frameTime,
                    1. / frameTime);
        */
        oc_canvas_render(renderer, context, surface);
        oc_canvas_present(renderer, surface);
        oc_input_next_frame(&inputState);

        oc_scratch_end(scratch);

        frameTime = oc_clock_time(OC_CLOCK_MONOTONIC) - startTime;
    }

    oc_font_destroy(font);
    oc_canvas_context_destroy(context);
    oc_surface_destroy(surface);
    oc_canvas_renderer_destroy(renderer);
    oc_window_destroy(window);

    oc_terminate();

    return (0);
}
