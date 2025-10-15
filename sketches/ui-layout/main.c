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

oc_vec2 frameSize = { 1200, 838 };

oc_surface surface;
oc_canvas_renderer renderer;
oc_canvas_context context;
oc_font fontRegular;
oc_font fontBold;

i32 ui_runloop(void* user)
{
    context = oc_canvas_context_create();

    oc_ui_context* ui = oc_ui_context_create(fontRegular);

    while(!oc_should_quit())
    {
        oc_arena_scope scratch = oc_scratch_begin();

        oc_event* event = 0;
        while((event = oc_next_event(scratch.arena)) != 0)
        {
            oc_ui_process_event(event);

            switch(event->type)
            {
                case OC_EVENT_WINDOW_CLOSE:
                {
                    oc_request_quit();
                }
                break;

                case OC_EVENT_WINDOW_RESIZE:
                {
                    frameSize = (oc_vec2){ event->move.content.w, event->move.content.h };
                }
                break;

                default:
                    break;
            }
        }

        oc_ui_frame(frameSize)
        {
            static f32 slider = 1;
            static oc_ui_align alignX = OC_UI_ALIGN_START;
            static oc_ui_align alignY = OC_UI_ALIGN_START;
            static bool wrap = true;
            static oc_ui_overflow overflow = OC_UI_OVERFLOW_SCROLL;

            oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_X);
            oc_ui_style_set_f32(OC_UI_MARGIN_X, 10);
            oc_ui_style_set_f32(OC_UI_MARGIN_Y, 10);
            oc_ui_style_set_f32(OC_UI_SPACING, 10);

            oc_ui_box("controls")
            {
                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
                oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                oc_ui_style_set_f32(OC_UI_MARGIN_X, 10);
                oc_ui_style_set_f32(OC_UI_MARGIN_Y, 10);
                oc_ui_style_set_f32(OC_UI_SPACING, 10);

                oc_ui_label("width-label", "Container Width");
                oc_ui_slider("slider", &slider);

                oc_ui_label("wrap-label", "Wrap Content");
                oc_ui_checkbox("wrap", &wrap);

                {
                    oc_ui_radio_group_info overflowInfo = {
                        .optionCount = 3,
                        .options = (oc_str8[]){
                            OC_STR8_LIT("Clip"),
                            OC_STR8_LIT("Allow"),
                            OC_STR8_LIT("Scroll"),
                        },
                        .selectedIndex = (i32)overflow,
                    };
                    oc_ui_label("overflow-label", "Overflow");
                    oc_ui_radio_group_info result = oc_ui_radio_group("overflow-x", &overflowInfo);
                    overflow = result.selectedIndex;
                }

                {
                    oc_ui_radio_group_info alignInfoX = {
                        .optionCount = 3,
                        .options = (oc_str8[]){
                            OC_STR8_LIT("Left"),
                            OC_STR8_LIT("Right"),
                            OC_STR8_LIT("Center"),
                        },
                        .selectedIndex = (i32)alignX,
                    };
                    oc_ui_label("align-x-label", "Align X");
                    oc_ui_radio_group_info resultX = oc_ui_radio_group("align-x", &alignInfoX);
                    alignX = resultX.selectedIndex;
                }

                {
                    oc_ui_radio_group_info alignInfoY = {
                        .optionCount = 3,
                        .options = (oc_str8[]){
                            OC_STR8_LIT("Top"),
                            OC_STR8_LIT("Bottom"),
                            OC_STR8_LIT("Center"),
                        },
                        .selectedIndex = (i32)alignY,
                    };
                    oc_ui_label("align-y-label", "Align Y");
                    oc_ui_radio_group_info resultY = oc_ui_radio_group("align-y", &alignInfoY);
                    alignY = resultY.selectedIndex;
                }
            }

            f32 width = 100 + slider * 400;

            oc_ui_box("a")
            {
                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, width });
                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
                oc_ui_style_set_f32(OC_UI_MARGIN_X, 10);
                oc_ui_style_set_f32(OC_UI_MARGIN_Y, 10);
                oc_ui_style_set_f32(OC_UI_SPACING, 10);
                oc_ui_style_set_color(OC_UI_BG_COLOR, (oc_color){ 0, 1, 1, 1 });

                oc_ui_style_set_i32(OC_UI_WRAP, wrap);
                oc_ui_style_set_i32(OC_UI_OVERFLOW_X, overflow);

                oc_ui_style_set_i32(OC_UI_ALIGN_X, alignX);
                oc_ui_style_set_i32(OC_UI_ALIGN_Y, alignY);

                oc_ui_box("b")
                {
                    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 150, .relax = 0, .min = 70, .max = 200 });
                    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, 150, .relax = 0 });
                    oc_ui_style_set_color(OC_UI_BG_COLOR, (oc_color){ 0, 0, 1, 1 });
                }
                oc_ui_box("c")
                {
                    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 150, .relax = 0, .min = 70, .max = 200 });
                    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, 100 });
                    oc_ui_style_set_color(OC_UI_BG_COLOR, (oc_color){ 1, 0, 1, 1 });
                }

                oc_ui_box("d")
                {
                    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 50, .relax = 0, .min = 70, .max = 200 });
                    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, 100 });
                    oc_ui_style_set_color(OC_UI_BG_COLOR, (oc_color){ 1, 1, 0, 1 });
                }
            }
        }

        oc_ui_draw();
        oc_canvas_render(renderer, context, surface);
        oc_canvas_present(renderer, surface);

        oc_scratch_end(scratch);
    }
    return (0);
}

int main()
{
    oc_init();
    oc_clock_init(); //TODO put that in oc_init()?

    oc_rect windowRect = { .x = 100, .y = 100, .w = frameSize.x, .h = frameSize.y };
    oc_window window = oc_window_create(windowRect, OC_STR8("test"), 0);

    oc_rect contentRect = oc_window_get_content_rect(window);

    oc_window_set_title(window, OC_STR8("UI Test"));

    renderer = oc_canvas_renderer_create();
    surface = oc_canvas_surface_create_for_window(renderer, window);

    oc_arena_scope scratch = oc_scratch_begin();

    oc_font* fonts[2] = { &fontRegular, &fontBold };
    oc_str8 fontNames[2] = {
        oc_path_executable_relative(scratch.arena, OC_STR8("../OpenSans-Regular.ttf")),
        oc_path_executable_relative(scratch.arena, OC_STR8("../OpenSans-Bold.ttf"))
    };

    for(int i = 0; i < 2; i++)
    {
        oc_file file = oc_catch(oc_file_open(fontNames[i], OC_FILE_ACCESS_READ, 0))
        {
            oc_log_error("Couldn't open file %.*s\n", oc_str8_ip(fontNames[i]));
            return -1;
        }
        u64 size = oc_file_size(file);
        char* buffer = (char*)oc_arena_push(scratch.arena, size);
        oc_file_read(file, size, buffer);
        oc_file_close(file);
        oc_unicode_range ranges[5] = { OC_UNICODE_BASIC_LATIN,
                                       OC_UNICODE_C1_CONTROLS_AND_LATIN_1_SUPPLEMENT,
                                       OC_UNICODE_LATIN_EXTENDED_A,
                                       OC_UNICODE_LATIN_EXTENDED_B,
                                       OC_UNICODE_SPECIALS };

        *fonts[i] = oc_font_create_from_memory(oc_str8_from_buffer(size, buffer), 5, ranges);
    }
    oc_scratch_end(scratch);

    // start app
    oc_window_bring_to_front(window);
    oc_window_focus(window);

    oc_thread* runloopThread = oc_thread_create(ui_runloop, 0);

    while(!oc_should_quit())
    {
        oc_pump_events(-1);
        //TODO: what to do with mem scratch here?
    }

    i64 exitCode = 0;
    oc_thread_join(runloopThread, &exitCode);

    oc_surface_destroy(surface);
    oc_terminate();

    return (0);
}
