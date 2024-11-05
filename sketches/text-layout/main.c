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

oc_font create_font(oc_str8 relPath)
{
    //NOTE(martin): create font
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 fontPath = oc_path_executable_relative(scratch.arena, relPath);
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

    oc_font font = oc_font_create_from_memory(oc_str8_from_buffer(fontDataSize, (char*)fontData));
    free(fontData);
    oc_scratch_end(scratch);
    return (font);
}

int main()
{
    oc_init();

    oc_rect windowRect = { .x = 100, .y = 100, .w = 800, .h = 600 };
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

    oc_font romanFont = create_font(OC_STR8("../../resources/Zapfino.ttf"));
    oc_font japaneseFont = create_font(OC_STR8("../../resources/NotoSansJP-Light.ttf"));
    oc_font arabicFont = create_font(OC_STR8("../../resources/NotoNaskhArabic-Regular.ttf"));

    oc_str8 text = OC_STR8("Hello Harfbuzz! Zapfino Test");
    //    oc_str8 text = OC_STR8("ll");
    f32 fontSize = 32;

    u32 cursor = 0;

    // start app
    oc_window_bring_to_front(window);
    oc_window_focus(window);

    f64 frameTime = 0;

    oc_vec2 mousePoint = { 0 };

    while(!oc_should_quit())
    {
        i32 moveCursor = 0;
        bool mouseClicked = 0;

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
                        if(event->key.keyCode == OC_KEY_LEFT)
                        {
                            moveCursor = -1;
                        }
                        else if(event->key.keyCode == OC_KEY_RIGHT)
                        {
                            moveCursor = 1;
                        }
                    }
                }
                break;

                case OC_EVENT_MOUSE_MOVE:
                {
                    mousePoint = (oc_vec2){ event->mouse.x, event->mouse.y };
                }
                break;

                case OC_EVENT_MOUSE_BUTTON:
                {
                    if(event->key.action == OC_KEY_PRESS)
                    {
                        if(event->key.button == OC_MOUSE_LEFT)
                        {
                            mouseClicked = true;
                        }
                    }
                }
                break;

                default:
                    break;
            }
        }

        // background
        oc_set_color_rgba(0, 1, 1, 1);
        oc_clear();

        oc_font_metrics metrics = oc_font_get_metrics(romanFont, fontSize);

        oc_vec2 origin = { 100, 100 };

        oc_move_to(origin.x, origin.y);
        oc_line_to(origin.x + 600, origin.y);
        oc_set_color_rgba(1, 0, 0, 1);
        oc_set_width(1);
        oc_stroke();

        oc_move_to(origin.x, origin.y - metrics.ascent);
        oc_line_to(origin.x + 600, origin.y - metrics.ascent);
        oc_set_color_rgba(0, 1, 0, 1);
        oc_set_width(1);
        oc_stroke();

        oc_move_to(origin.x, origin.y + metrics.descent);
        oc_line_to(origin.x + 600, origin.y + metrics.descent);
        oc_set_color_rgba(0, 0, 1, 1);
        oc_set_width(1);
        oc_stroke();

        {
            oc_move_to(origin.x, origin.y);
            oc_set_color_rgba(0, 0, 0, 1);

            oc_str32 codepoints = oc_utf8_push_to_codepoints(scratch.arena, text);
            oc_glyph_run* run = oc_text_shape(scratch.arena, romanFont, 0, codepoints, 0, codepoints.len);
            oc_text_draw_run(run, fontSize);

            if(mouseClicked)
            {
                cursor = oc_glyph_run_point_to_cursor(run, fontSize, oc_vec2_sub(mousePoint, origin));
            }

            //TODO: move to next/prev utf8 char.
            //TODO: when we do grapheme segmentation, move to next grapheme
            if(moveCursor == 1 && cursor <= text.len)
            {
                cursor = oc_min(cursor + 1, text.len);
            }
            else if(moveCursor == -1 && cursor > 0)
            {
                cursor--;
            }

            oc_vec2 pos = oc_glyph_run_cursor_to_point(run, fontSize, cursor);
            oc_move_to(origin.x + pos.x, origin.y + pos.y + metrics.descent);
            oc_line_to(origin.x + pos.x, origin.y + pos.y - metrics.ascent);
            oc_set_color_rgba(0, 0, 0, 1);
            oc_set_width(1);
            oc_stroke();

            //oc_text_draw_utf8(text, font, fontSize);
        }

        {
            oc_move_to(200, 300);
            oc_line_to(300, 300);

            oc_set_width(1);
            oc_set_color_rgba(1, 0, 0, 1);
            oc_stroke();

            oc_move_to(200, 300);

            oc_set_color_rgba(0, 0, 0, 1);

            oc_str32 codepoints = oc_utf8_push_to_codepoints(scratch.arena, OC_STR8("以呂波耳本部止"));

            oc_glyph_run* run = oc_text_shape(scratch.arena,
                                              japaneseFont,
                                              &(oc_text_shape_settings){
                                                  .direction = OC_TEXT_DIRECTION_TTB,
                                              },
                                              codepoints,
                                              0,
                                              codepoints.len);
            oc_text_draw_run(run, fontSize);
        }

        {
            //NOTE: same TTB japanese text but withing a y-up coord system
            oc_rect client = oc_window_get_content_rect(window);
            oc_matrix_push((oc_mat2x3){ 1, 0, 0,
                                        0, -1, client.h });

            oc_set_text_flip(true);
            oc_move_to(300, client.h - 300);
            oc_line_to(400, client.h - 300);

            oc_set_width(1);
            oc_set_color_rgba(1, 0, 0, 1);
            oc_stroke();

            oc_move_to(300, client.h - 300);
            oc_set_color_rgba(0, 0, 0, 1);

            oc_str32 codepoints = oc_utf8_push_to_codepoints(scratch.arena, OC_STR8("以呂波耳本部止"));

            oc_glyph_run* run = oc_text_shape(scratch.arena,
                                              japaneseFont,
                                              &(oc_text_shape_settings){
                                                  .direction = OC_TEXT_DIRECTION_TTB,
                                              },
                                              codepoints,
                                              0,
                                              codepoints.len);
            oc_text_draw_run(run, fontSize);

            oc_set_text_flip(false);
            oc_matrix_pop();
        }

        {
            oc_move_to(500, 400);
            oc_line_to(700, 400);

            oc_set_width(1);
            oc_set_color_rgba(1, 0, 0, 1);
            oc_stroke();

            oc_move_to(500, 400);
            oc_set_color_rgba(0, 0, 0, 1);

            oc_str32 codepoints = oc_utf8_push_to_codepoints(scratch.arena, OC_STR8("مرحبا"));

            oc_glyph_run* run = oc_text_shape(scratch.arena,
                                              arabicFont,
                                              &(oc_text_shape_settings){
                                                  .direction = OC_TEXT_DIRECTION_RTL,
                                              },
                                              codepoints,
                                              0,
                                              codepoints.len);
            oc_text_draw_run(run, fontSize);
        }

        {
            oc_move_to(500, 500);

            oc_text_line* line = oc_text_line_from_utf8(scratch.arena,
                                                        OC_STR8("bahrain مصر kuwait"),
                                                        &(oc_text_attributes){
                                                            .font = arabicFont,
                                                            .fontSize = fontSize,
                                                            .color = { 0, 0, 0, 1 },
                                                        });

            oc_text_line_draw(line);
        }

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
