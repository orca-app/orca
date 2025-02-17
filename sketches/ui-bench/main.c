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

    oc_ui_radio_group_info radioInfo = {
        .optionCount = 4,
        .options = (oc_str8[]){
            OC_STR8("Option One"),
            OC_STR8("Option Two"),
            OC_STR8("Option Three"),
            OC_STR8("Option Four"),
        },
    };

    oc_ui_select_popup_info popupInfo = {
        .selectedIndex = -1,
        .optionCount = 3,
        .options = (oc_str8[]){
            OC_STR8("Option One"),
            OC_STR8("Option Two"),
            OC_STR8("Options Three"),
        },
        .placeholder = OC_STR8("None"),
    };

    oc_arena textArena = { 0 };
    oc_arena_init(&textArena);

    oc_ui_text_box_info textBoxInfo = {
        .defaultText = OC_STR8_LIT("type here"),
    };

    oc_arena textArena2 = { 0 };
    oc_arena_init(&textArena2);

    oc_ui_text_box_info textBoxInfo2 = {
        .defaultText = OC_STR8_LIT("type here"),
    };

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
            oc_ui_menu_bar("menu")
            {
                oc_ui_menu("file-menu", "File")
                {
                    oc_ui_menu_button("quit", "Quit");
                }
                oc_ui_menu("theme-menu", "Theme")
                {
                    if(oc_ui_menu_button("dark", "Dark").pressed)
                    {
                        oc_log_info("selected dark theme\n");
                    }
                    if(oc_ui_menu_button("ligth", "Light").pressed)
                    {
                        oc_log_info("selected light theme\n");
                    }
                }
            }

            oc_ui_style_rule("inner lb")
            {
                oc_ui_style_set_color(OC_UI_BG_COLOR, (oc_color){ 0, 1, 0, 1 });
            }

            oc_ui_style_rule("inner .label")
            {
                oc_ui_style_set_color(OC_UI_BG_COLOR, (oc_color){ 1, 0, 0, 1 });
            }

            oc_ui_box("outer-box")
            {
                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 200 });
                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, 200 });
                oc_ui_style_set_color(OC_UI_BG_COLOR, (oc_color){ 1, 0, 0, 1 });

                //                oc_ui_style_set_i32(OC_UI_ALIGN_Y, OC_UI_ALIGN_CENTER);

                oc_ui_box("inner-box")
                {
                    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 100 });
                    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, 100 });

                    oc_ui_style_set_color(OC_UI_BG_COLOR, (oc_color){ 0, 1, 0, 1 });
                    oc_ui_style_set_color(OC_UI_BORDER_COLOR, (oc_color){ 0, 0, 1, 0.5 });
                    oc_ui_style_set_f32(OC_UI_BORDER_SIZE, 50);
                }
            }
            oc_ui_box("container")
            {
                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
                oc_ui_style_set_var(OC_UI_BG_COLOR, "bg-1");

                oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                oc_ui_style_set_f32(OC_UI_MARGIN_X, 10);
                oc_ui_style_set_f32(OC_UI_MARGIN_Y, 10);
                oc_ui_style_set_f32(OC_UI_SPACING, 10);

                oc_ui_style_rule(".label.hover")
                {
                    oc_ui_style_set_color(OC_UI_BG_COLOR, (oc_color){ 1, 0, 1, 1 });
                }

                oc_ui_label("la", "Label A");
                oc_ui_label("lb", "Label B");
                oc_ui_label("lc", "Label C");

                oc_ui_button("ba", "Button A");
                oc_ui_button("bb", "Button B");
                oc_ui_button("bc", "Button C");

                oc_ui_box("inner")
                {
                    oc_ui_style_set_color(OC_UI_BG_COLOR, (oc_color){ 0, 0, 1, 1 });

                    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
                    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
                    oc_ui_style_set_f32(OC_UI_MARGIN_X, 10);
                    oc_ui_style_set_f32(OC_UI_MARGIN_Y, 10);
                    oc_ui_style_set_f32(OC_UI_SPACING, 10);

                    oc_ui_label("la", "label A");
                    oc_ui_label("lb", "label B");
                }
                popupInfo = oc_ui_select_popup("popup", &popupInfo);

                {
                    oc_ui_text_box_result result = oc_ui_text_box("textbox", scratch.arena, &textBoxInfo);
                    if(result.changed)
                    {
                        oc_arena_clear(&textArena);
                        textBoxInfo.text = oc_str8_push_copy(&textArena, result.text);
                    }
                }

                {
                    oc_ui_text_box_result result = oc_ui_text_box("textbox2", scratch.arena, &textBoxInfo2);
                    if(result.changed)
                    {
                        oc_arena_clear(&textArena2);
                        textBoxInfo2.text = oc_str8_push_copy(&textArena2, result.text);
                    }
                }
            }

            oc_ui_box("panel")
            {
                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 300 });
                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, 200 });
                oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_BG_1);

                oc_ui_style_set_i32(OC_UI_OVERFLOW_X, OC_UI_OVERFLOW_CLIP);
                oc_ui_style_set_i32(OC_UI_OVERFLOW_Y, OC_UI_OVERFLOW_ALLOW);

                /*
                oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                oc_ui_style_set_f32(OC_UI_SPACING, 10);
                */
                oc_ui_style_set_f32(OC_UI_MARGIN_X, 20);
                oc_ui_style_set_f32(OC_UI_MARGIN_Y, 20);

                oc_ui_box* box = oc_ui_box("box")
                {
                    //                    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 250 });
                    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, 400 });

                    oc_ui_style_set_f32(OC_UI_MARGIN_X, 10);
                    oc_ui_style_set_f32(OC_UI_MARGIN_Y, 10);
                    oc_ui_style_set_f32(OC_UI_SPACING, 5);

                    oc_ui_style_set_color(OC_UI_BG_COLOR, (oc_color){ 0, 0, 0, 1 });

                    static bool check = true;
                    oc_ui_checkbox("check", &check);

                    static f32 slider = 0;
                    oc_ui_slider("slider", &slider);

                    radioInfo = oc_ui_radio_group("radio", &radioInfo);

                    if(oc_ui_box_get_sig(box).hover)
                    {
                        oc_ui_tooltip("tooltip", "This is a black box");
                    }
                }
                oc_ui_button("mybutton", "clickMe");
            }

            /*
            oc_ui_box("note")
            {
                oc_ui_set_overlay(true);

                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 100 });
                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, 50 });
                oc_ui_style_set_color(OC_UI_BG_COLOR, (oc_color){ 1, 0, 0, 1 });
            }
            */
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
        oc_file file = oc_file_open(fontNames[i], OC_FILE_ACCESS_READ, 0);
        if(oc_file_last_error(file) != OC_IO_OK)
        {
            oc_log_error("Couldn't open file %.*s\n", oc_str8_ip(fontNames[i]));
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
