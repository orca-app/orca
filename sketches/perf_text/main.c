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

static const char* TEST_STRING =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nulla quam enim, aliquam in placerat luctus, rutrum in quam. "
    "Cras urna elit, pellentesque ac ipsum at, lobortis scelerisque eros. Aenean et turpis nibh. Maecenas lectus augue, eleifend "
    "nec efficitur eu, faucibus eget turpis. Suspendisse vel nulla mi. Duis imperdiet neque orci, ac ultrices orci molestie a. "
    "Etiam malesuada vulputate hendrerit. Cras ultricies diam in lectus finibus, eu laoreet diam rutrum.\n"
    "\n"
    "Etiam dictum orci arcu, ac fermentum leo dapibus lacinia. Integer vitae elementum ex. Vestibulum tempor nunc eu hendrerit "
    "ornare. Nunc pretium ligula sit amet massa pulvinar, vitae imperdiet justo bibendum. Maecenas consectetur elementum mi, sed "
    "vehicula neque pulvinar sit amet. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nunc tortor erat, accumsan in laoreet "
    "quis, placerat nec enim. Nulla facilisi. Morbi vitae nibh ligula. Suspendisse in molestie magna, eget aliquet mauris. Sed  "
    "aliquam faucibus magna.\n"
    "\n"
    "Sed metus odio, imperdiet et consequat non, faucibus nec risus. Suspendisse facilisis sem neque, id scelerisque dui mattis sit "
    "amet. Nullam tincidunt nisl nec dui dignissim mattis. Proin fermentum ornare ipsum. Proin eleifend, mi vitae porttitor placerat, "
    "neque magna elementum turpis, eu aliquet mi urna et leo. Pellentesque interdum est mauris, sed pellentesque risus blandit in. "
    "Phasellus dignissim consequat eros, at aliquam elit finibus posuere. Proin suscipit tortor leo, id vulputate odio lobortis in. "
    "Vestibulum et orci ligula. Sed scelerisque nunc non nisi aliquam, vel eleifend felis suscipit. Integer posuere sapien elit,  "
    "lacinia ultricies nibh sodales nec.\n"
    "\n"
    "Etiam aliquam purus sit amet purus ultricies tristique. Nunc maximus nunc quis magna ornare, vel interdum urna fermentum. "
    "Vestibulum cursus nisl ut nulla egestas, quis mattis elit venenatis. Praesent malesuada mi non magna aliquam fringilla eget eu "
    "turpis. Integer suscipit elit vel consectetur vulputate. Integer euismod, erat eget elementum tempus, magna metus consectetur "
    "elit, sed feugiat urna sapien sodales sapien. Sed sit amet varius nunc. Curabitur sodales nunc justo, ac scelerisque ipsum semper "
    "eget. Integer ornare, velit ut hendrerit dapibus, erat mauris commodo justo, vel semper urna justo non mauris. Proin blandit, "
    "enim ut posuere placerat, leo nibh tristique eros, ut pulvinar sapien elit eget enim. Pellentesque et mauris lectus. Curabitur "
    "quis lobortis leo, sit amet egestas dui. Nullam ut sapien eu justo lacinia ultrices. Ut tincidunt, sem non luctus tempus, felis "
    "purus imperdiet nisi, non ultricies libero ipsum eu augue. Mauris at luctus enim.\n"
    "\n"
    "Aliquam sed tortor a justo pulvinar dictum consectetur eu felis. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices "
    "posuere cubilia curae; Etiam vehicula porttitor volutpat. Morbi fringilla tortor nec accumsan aliquet. Aliquam in commodo neque. "
    "Sed laoreet tellus in consectetur aliquet. Nullam nibh eros, feugiat sit amet aliquam non, malesuada vel urna. Ut vel egestas nunc. "
    "Pellentesque vitae ante quis ante pharetra pretium. Nam quis eros commodo, mattis enim sed, finibus ante. Quisque lacinia tortor ut "
    "odio laoreet, vel viverra libero porttitor. Vestibulum vitae dapibus ex. Phasellus varius lorem sed justo sollicitudin faucibus. "
    "Etiam aliquam lacinia consectetur. Phasellus nulla ipsum, viverra non nulla in, rhoncus posuere nunc.\n"
    "\n"
    "Phasellus efficitur commodo tellus, eget lobortis erat porta quis. Aenean condimentum tortor ut neque dapibus, vitae vulputate quam "
    "condimentum. Aliquam elementum vitae nulla vitae tristique. Suspendisse feugiat turpis ac magna dapibus, ut blandit diam tincidunt. "
    "Integer id dui id enim ullamcorper dictum. Maecenas malesuada vitae ex pharetra iaculis. Curabitur eu dolor consectetur, tempus augue "
    "sed, finibus est. Nulla facilisi. Vivamus sed lacinia turpis, in gravida dolor. Aenean interdum consectetur enim a malesuada. Sed turpis "
    "nisi, lacinia et fermentum nec, pharetra id dui. Vivamus neque ligula, iaculis sed tempor eget, vehicula blandit quam. Morbi rhoncus quam "
    "semper magna mollis luctus. Donec eu dolor ut ante ullamcorper porta. Mauris et est tristique libero pharetra faucibus.\n"
    "\n"
    "Duis ut elementum sem. Praesent commodo erat nec sem ultricies sollicitudin. Suspendisse a pellentesque sapien. Nunc ac magna a dui "
    "elementum luctus non a mi. Cras elementum nunc sed nunc gravida, sit amet accumsan tortor pulvinar. Etiam elit arcu, pellentesque non ex "
    "id, vestibulum pellentesque velit. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque habitant morbi tristique senectus "
    "et netus et malesuada fames ac turpis egestas. Proin sit amet velit eget tellus vulputate sagittis eget non massa. Cras accumsan tempor  "
    "tortor, quis rutrum neque placerat id. Nullam a egestas eros, eu porta nisi. Aenean rutrum, sapien quis fermentum tempus, dolor orci  "
    "faucibus eros, vel luctus justo leo vitae ante. Curabitur aliquam condimentum ipsum sit amet ultrices. Nullam ac velit semper, dapibus urna "
    "sit amet, malesuada enim. Mauris ultricies nibh orci.";

oc_font create_font(const char* path)
{
    //NOTE(martin): create font
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 fontPath = oc_path_executable_relative(scratch.arena, OC_STR8(path));
    char* fontPathCString = oc_str8_to_cstring(scratch.arena, fontPath);

    FILE* fontFile = fopen(fontPathCString, "r");
    if(!fontFile)
    {
        oc_log_error("Could not load font file '%s'\n", fontPathCString);
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

enum
{
    FONT_COUNT = 3
};

int main()
{
    oc_init();
    oc_clock_init();

    oc_rect rect = { .x = 100, .y = 100, .w = 980, .h = 600 };
    oc_window window = oc_window_create(rect, OC_STR8("test"), 0);

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

    int fontIndex = 0;
    oc_font fonts[FONT_COUNT] = { create_font("../../resources/OpenSansLatinSubset.ttf"),
                                  create_font("../../resources/CMUSerif-Roman.ttf"),
                                  create_font("../../resources/Courier.ttf") };

    oc_font_metrics extents[FONT_COUNT];
    f32 fontScales[FONT_COUNT];
    f32 lineHeights[FONT_COUNT];

    for(int i = 0; i < FONT_COUNT; i++)
    {
        extents[i] = oc_font_get_metrics_unscaled(fonts[i]);
        fontScales[i] = oc_font_get_scale_for_em_pixels(fonts[i], 14);
        lineHeights[i] = fontScales[i] * (extents[i].ascent + extents[i].descent + extents[i].lineGap);
    }

    int codePointCount = oc_utf8_codepoint_count_for_string(OC_STR8((char*)TEST_STRING));
    u32* codePoints = oc_malloc_array(oc_utf32, codePointCount);
    oc_utf8_to_codepoints(codePointCount, codePoints, OC_STR8((char*)TEST_STRING));

    u32 glyphCount = 0;
    for(int i = 0; i < codePointCount; i++)
    {
        if(codePoints[i] != ' ' && codePoints[i] != '\n')
        {
            glyphCount++;
        }
    }

    // start app
    oc_window_bring_to_front(window);
    oc_window_focus(window);

    f64 frameTime = 0;

    bool tracked = false;
    oc_vec2 trackPoint = { 0 };
    f32 zoom = 1;

    f32 startX = 10;
    f32 startY = 10 + lineHeights[fontIndex];

    oc_input_state inputState = { 0 };

    while(!oc_should_quit())
    {
        f64 startFrameTime = oc_clock_time(OC_CLOCK_MONOTONIC);
        oc_arena_scope scratch = oc_scratch_begin();

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
                            trackPoint.x = mousePos.x / zoom - startX;
                            trackPoint.y = mousePos.y / zoom - startY;
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
                    f32 trackX = mousePos.x / zoom - startX;
                    f32 trackY = mousePos.y / zoom - startY;

                    zoom *= 1 + event->mouse.deltaY * 0.01;
                    zoom = oc_clamp(zoom, 0.2, 10);

                    startX = mousePos.x / zoom - trackX;
                    startY = mousePos.y / zoom - trackY;
                }
                break;

                case OC_EVENT_KEYBOARD_KEY:
                {
                    if(event->key.keyCode == OC_KEY_SPACE && event->key.action == OC_KEY_PRESS)
                    {
                        fontIndex = (fontIndex + 1) % FONT_COUNT;
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
            startX = mousePos.x / zoom - trackPoint.x;
            startY = mousePos.y / zoom - trackPoint.y;
        }

        f32 textX = startX;
        f32 textY = startY;

        /*
		oc_set_color_rgba(1, 1, 1, 1);
		oc_clear();
		oc_set_color_rgba(1, 0, 0, 1);
		for(int i=0; i<1000; i++)
		{
			oc_rectangle_fill(0, 0, 100, 100);
		}
*/

        oc_matrix_multiply_push((oc_mat2x3){ zoom, 0, 0,
                                             0, zoom, 0 });

        oc_set_color_rgba(1, 1, 1, 1);
        oc_clear();

        oc_set_font(fonts[fontIndex]);
        oc_set_font_size(14);
        oc_set_color_rgba(0, 0, 0, 1);

        oc_move_to(textX, textY);

        int startIndex = 0;
        while(startIndex < codePointCount)
        {
            bool lineBreak = false;
            int subIndex = 0;
            for(; (startIndex + subIndex) < codePointCount && subIndex < 120; subIndex++)
            {
                if(codePoints[startIndex + subIndex] == '\n')
                {
                    break;
                }
            }

            u32 glyphs[512];
            oc_font_get_glyph_indices(fonts[fontIndex], oc_str32_from_buffer(subIndex, codePoints + startIndex), oc_str32_from_buffer(512, glyphs));

            oc_glyph_outlines(oc_str32_from_buffer(subIndex, glyphs));
            oc_fill();

            textY += lineHeights[fontIndex];
            oc_move_to(textX, textY);
            startIndex++;

            startIndex += subIndex;
        }

        oc_matrix_pop();

        oc_set_color_rgba(0, 0, 1, 1);
        oc_set_font(fonts[fontIndex]);
        oc_set_font_size(14);
        oc_move_to(10, contentRect.h - 10 - lineHeights[fontIndex]);

        oc_str8 text = oc_str8_pushf(scratch.arena,
                                     "Test program: %i glyphs, frame time = %fs, fps = %f",
                                     glyphCount,
                                     frameTime,
                                     1. / frameTime);
        oc_text_outlines(text);
        oc_fill();

        f64 startFlushTime = oc_clock_time(OC_CLOCK_MONOTONIC);

        oc_canvas_render(renderer, context, surface);
        oc_canvas_present(renderer, surface);

        f64 startPresentTime = oc_clock_time(OC_CLOCK_MONOTONIC);

        f64 endFrameTime = oc_clock_time(OC_CLOCK_MONOTONIC);

        frameTime = (endFrameTime - startFrameTime);

        /*
        printf("frame time: %.2fms (%.2fFPS), draw = %f.2ms, flush = %.2fms, present = %.2fms\n",
               frameTime * 1000,
               1. / frameTime,
               (startFlushTime - startFrameTime) * 1000,
               (startPresentTime - startFlushTime) * 1000,
               (endFrameTime - startPresentTime) * 1000);
        */
        oc_input_next_frame(&inputState);
        oc_scratch_end(scratch);
    }

    for(int i = 0; i < FONT_COUNT; i++)
    {
        oc_font_destroy(fonts[i]);
    }
    oc_canvas_context_destroy(context);
    oc_surface_destroy(surface);
    oc_canvas_renderer_destroy(renderer);
    oc_window_destroy(window);
    oc_terminate();

    return (0);
}
