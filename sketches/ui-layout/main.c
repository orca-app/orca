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

typedef struct box_info
{
    oc_ui_size size[OC_UI_AXIS_COUNT];
    oc_ui_layout layout;

} box_info;

box_info BOX_INFOS[4] = {
    {
        .layout.margin.x = 10,
        .layout.margin.y = 10,
        .layout.spacing = 10,
    },
    {
        .size[OC_UI_AXIS_X] = {
            .kind = OC_UI_SIZE_PIXELS,
            .value = 100,
        },
        .size[OC_UI_AXIS_Y] = {
            .kind = OC_UI_SIZE_PIXELS,
            .value = 100,
        },
    },
    {
        .size[OC_UI_AXIS_X] = {
            .kind = OC_UI_SIZE_PIXELS,
            .value = 100,
        },
        .size[OC_UI_AXIS_Y] = {
            .kind = OC_UI_SIZE_PIXELS,
            .value = 100,
        },

    },
    {
        .size[OC_UI_AXIS_X] = {
            .kind = OC_UI_SIZE_PIXELS,
            .value = 100,
        },
        .size[OC_UI_AXIS_Y] = {
            .kind = OC_UI_SIZE_PIXELS,
            .value = 100,
        },
    },
};

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
            oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_X);
            oc_ui_style_set_f32(OC_UI_SPACING, 32);

            oc_ui_box("controls")
            {
                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_CHILDREN, .min = 100, .max = frameSize.x });
                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN, .min = 100, .max = frameSize.y });
                oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                oc_ui_style_set_f32(OC_UI_MARGIN_X, 10);
                oc_ui_style_set_f32(OC_UI_MARGIN_Y, 10);
                oc_ui_style_set_f32(OC_UI_SPACING, 10);

                oc_ui_style_set_i32(OC_UI_WRAP, 1);

                static int selectedBox = 0;

                {
                    oc_ui_select_popup_info info = {
                        .optionCount = 4,
                        .options = (oc_str8[]){
                            OC_STR8_LIT("Outer"),
                            OC_STR8_LIT("A"),
                            OC_STR8_LIT("B"),
                            OC_STR8_LIT("C"),
                        },
                        .selectedIndex = selectedBox,
                    };
                    oc_ui_select_popup_info result = oc_ui_select_popup("select-box", &info);
                    selectedBox = result.selectedIndex;
                }
                box_info* boxInfo = &BOX_INFOS[selectedBox];

                f32 widthSlider = (boxInfo->size[0].value - 100) / 300;
                f32 heightSlider = (boxInfo->size[1].value - 100) / 300;

                oc_ui_box("size-kind-x")
                {
                    oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                    oc_ui_style_set_f32(OC_UI_SPACING, 10);

                    oc_ui_label("size-kind-x-label", "Size kind");

                    oc_ui_select_popup_info info = {
                        .optionCount = 5,
                        .options = (oc_str8[]){
                            OC_STR8_LIT("Children"),
                            OC_STR8_LIT("Text"),
                            OC_STR8_LIT("Pixels"),
                            OC_STR8_LIT("Parent"),
                            OC_STR8_LIT("Parent minus pixels"),
                        },
                        .selectedIndex = boxInfo->size[0].kind,
                    };
                    oc_ui_select_popup_info result = oc_ui_select_popup("select-kind-x", &info);
                    boxInfo->size[0].kind = result.selectedIndex;
                }

                oc_ui_box("size-kind-y")
                {
                    oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                    oc_ui_style_set_f32(OC_UI_SPACING, 10);

                    oc_ui_label("size-kind-y-label", "Size kind");

                    oc_ui_select_popup_info info = {
                        .optionCount = 5,
                        .options = (oc_str8[]){
                            OC_STR8_LIT("Children"),
                            OC_STR8_LIT("Text"),
                            OC_STR8_LIT("Pixels"),
                            OC_STR8_LIT("Parent"),
                            OC_STR8_LIT("Parent minus pixels"),
                        },
                        .selectedIndex = boxInfo->size[1].kind,
                    };
                    oc_ui_select_popup_info result = oc_ui_select_popup("select-kind-y", &info);
                    boxInfo->size[1].kind = result.selectedIndex;
                }

                oc_ui_box("size-x")
                {
                    oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                    oc_ui_style_set_f32(OC_UI_SPACING, 10);

                    oc_ui_label("width-label", "Size value");
                    oc_ui_slider("slider-w", &widthSlider);
                }
                oc_ui_box("size-y")
                {
                    oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                    oc_ui_style_set_f32(OC_UI_SPACING, 10);

                    oc_ui_label("height-label", "Size value");
                    oc_ui_slider("slider-h", &heightSlider);
                }
                boxInfo->size[0].value = widthSlider * 300 + 100;
                boxInfo->size[1].value = heightSlider * 300 + 100;

                oc_ui_box("axis")
                {
                    oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                    oc_ui_style_set_f32(OC_UI_SPACING, 10);

                    oc_ui_radio_group_info axisInfo = {
                        .optionCount = 2,
                        .options = (oc_str8[]){
                            OC_STR8_LIT("X"),
                            OC_STR8_LIT("Y"),
                        },
                        .selectedIndex = (i32)boxInfo->layout.axis,
                    };
                    oc_ui_label("axis-label", "Main Axis");
                    oc_ui_radio_group_info result = oc_ui_radio_group("axis", &axisInfo);
                    boxInfo->layout.axis = result.selectedIndex;
                }

                oc_ui_box("align-x")
                {
                    oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                    oc_ui_style_set_f32(OC_UI_SPACING, 10);

                    oc_ui_radio_group_info alignInfoX = {
                        .optionCount = 4,
                        .options = (oc_str8[]){
                            OC_STR8_LIT("Left"),
                            OC_STR8_LIT("Right"),
                            OC_STR8_LIT("Center"),
                            OC_STR8_LIT("Justify"),
                        },
                        .selectedIndex = (i32)boxInfo->layout.align.x,
                    };
                    oc_ui_label("align-x-label", "Align X");
                    oc_ui_radio_group_info resultX = oc_ui_radio_group("align-x", &alignInfoX);
                    boxInfo->layout.align.x = resultX.selectedIndex;
                }
                oc_ui_box("align-y")
                {
                    oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                    oc_ui_style_set_f32(OC_UI_SPACING, 10);

                    oc_ui_radio_group_info alignInfoY = {
                        .optionCount = 3,
                        .options = (oc_str8[]){
                            OC_STR8_LIT("Top"),
                            OC_STR8_LIT("Bottom"),
                            OC_STR8_LIT("Center"),
                        },
                        .selectedIndex = (i32)boxInfo->layout.align.y,
                    };
                    oc_ui_label("align-y-label", "Align Y");
                    oc_ui_radio_group_info resultY = oc_ui_radio_group("align-y", &alignInfoY);
                    boxInfo->layout.align.y = resultY.selectedIndex;
                }

                oc_ui_box("wrap")
                {
                    oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                    oc_ui_style_set_f32(OC_UI_SPACING, 10);

                    oc_ui_label("wrap-label", "Wrap Content");
                    oc_ui_checkbox("wrap", &boxInfo->layout.wrap);
                }

                oc_ui_box("overflow-x")
                {
                    oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                    oc_ui_style_set_f32(OC_UI_SPACING, 10);

                    oc_ui_radio_group_info overflowInfo = {
                        .optionCount = 3,
                        .options = (oc_str8[]){
                            OC_STR8_LIT("Clip"),
                            OC_STR8_LIT("Allow"),
                            OC_STR8_LIT("Scroll"),
                        },
                        .selectedIndex = (i32)boxInfo->layout.overflow.x,
                    };
                    oc_ui_label("overflow-label-x", "Overflow X");
                    oc_ui_radio_group_info result = oc_ui_radio_group("overflow-x", &overflowInfo);
                    boxInfo->layout.overflow.x = result.selectedIndex;
                }

                oc_ui_box("overflow-y")
                {
                    oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                    oc_ui_style_set_f32(OC_UI_SPACING, 10);

                    oc_ui_radio_group_info overflowInfo = {
                        .optionCount = 3,
                        .options = (oc_str8[]){
                            OC_STR8_LIT("Clip"),
                            OC_STR8_LIT("Allow"),
                            OC_STR8_LIT("Scroll"),
                        },
                        .selectedIndex = (i32)boxInfo->layout.overflow.y,
                    };
                    oc_ui_label("overflow-label-y", "Overflow Y");
                    oc_ui_radio_group_info result = oc_ui_radio_group("overflow-y", &overflowInfo);
                    boxInfo->layout.overflow.y = result.selectedIndex;
                }
            }

            /*
            f32 width = 100 + widthSlider * 400;
            f32 height = 100 + heightSlider * 400;
            */

            oc_ui_box("outer")
            {
                box_info info = BOX_INFOS[0];

                oc_ui_style_set_size(OC_UI_WIDTH, info.size[OC_UI_AXIS_X]);
                oc_ui_style_set_size(OC_UI_HEIGHT, info.size[OC_UI_AXIS_Y]);
                oc_ui_style_set_i32(OC_UI_AXIS, info.layout.axis);
                oc_ui_style_set_i32(OC_UI_ALIGN_X, info.layout.align.x);
                oc_ui_style_set_i32(OC_UI_ALIGN_Y, info.layout.align.y);
                oc_ui_style_set_i32(OC_UI_ALIGN_LINE_X, info.layout.alignLine.x);
                oc_ui_style_set_i32(OC_UI_ALIGN_LINE_Y, info.layout.alignLine.y);
                oc_ui_style_set_f32(OC_UI_MARGIN_X, info.layout.margin.x);
                oc_ui_style_set_f32(OC_UI_MARGIN_Y, info.layout.margin.y);
                oc_ui_style_set_f32(OC_UI_SPACING, info.layout.spacing);
                oc_ui_style_set_i32(OC_UI_OVERFLOW_X, info.layout.overflow.x);
                oc_ui_style_set_i32(OC_UI_OVERFLOW_Y, info.layout.overflow.y);
                oc_ui_style_set_i32(OC_UI_WRAP, info.layout.wrap);

                oc_ui_style_set_color(OC_UI_BG_COLOR, (oc_color){ 0, 1, 1, 1 });

                oc_ui_box("A")
                {
                    box_info info = BOX_INFOS[1];

                    oc_ui_style_set_size(OC_UI_WIDTH, info.size[OC_UI_AXIS_X]);
                    oc_ui_style_set_size(OC_UI_HEIGHT, info.size[OC_UI_AXIS_Y]);
                    oc_ui_style_set_i32(OC_UI_AXIS, info.layout.axis);
                    oc_ui_style_set_i32(OC_UI_ALIGN_X, info.layout.align.x);
                    oc_ui_style_set_i32(OC_UI_ALIGN_Y, info.layout.align.y);
                    oc_ui_style_set_i32(OC_UI_ALIGN_LINE_X, info.layout.alignLine.x);
                    oc_ui_style_set_i32(OC_UI_ALIGN_LINE_Y, info.layout.alignLine.y);
                    oc_ui_style_set_f32(OC_UI_MARGIN_X, info.layout.margin.x);
                    oc_ui_style_set_f32(OC_UI_MARGIN_Y, info.layout.margin.y);
                    oc_ui_style_set_f32(OC_UI_SPACING, info.layout.spacing);
                    oc_ui_style_set_i32(OC_UI_OVERFLOW_X, info.layout.overflow.x);
                    oc_ui_style_set_i32(OC_UI_OVERFLOW_Y, info.layout.overflow.y);
                    oc_ui_style_set_i32(OC_UI_WRAP, info.layout.wrap);

                    oc_ui_style_set_color(OC_UI_BG_COLOR, (oc_color){ 0, 0, 1, 1 });
                }
                oc_ui_box("B")
                {
                    box_info info = BOX_INFOS[2];

                    oc_ui_style_set_size(OC_UI_WIDTH, info.size[OC_UI_AXIS_X]);
                    oc_ui_style_set_size(OC_UI_HEIGHT, info.size[OC_UI_AXIS_Y]);
                    oc_ui_style_set_i32(OC_UI_AXIS, info.layout.axis);
                    oc_ui_style_set_i32(OC_UI_ALIGN_X, info.layout.align.x);
                    oc_ui_style_set_i32(OC_UI_ALIGN_Y, info.layout.align.y);
                    oc_ui_style_set_i32(OC_UI_ALIGN_LINE_X, info.layout.alignLine.x);
                    oc_ui_style_set_i32(OC_UI_ALIGN_LINE_Y, info.layout.alignLine.y);
                    oc_ui_style_set_f32(OC_UI_MARGIN_X, info.layout.margin.x);
                    oc_ui_style_set_f32(OC_UI_MARGIN_Y, info.layout.margin.y);
                    oc_ui_style_set_f32(OC_UI_SPACING, info.layout.spacing);
                    oc_ui_style_set_i32(OC_UI_OVERFLOW_X, info.layout.overflow.x);
                    oc_ui_style_set_i32(OC_UI_OVERFLOW_Y, info.layout.overflow.y);
                    oc_ui_style_set_i32(OC_UI_WRAP, info.layout.wrap);

                    oc_ui_style_set_color(OC_UI_BG_COLOR, (oc_color){ 1, 0, 1, 1 });
                }

                oc_ui_box("C")
                {
                    box_info info = BOX_INFOS[3];

                    oc_ui_style_set_size(OC_UI_WIDTH, info.size[OC_UI_AXIS_X]);
                    oc_ui_style_set_size(OC_UI_HEIGHT, info.size[OC_UI_AXIS_Y]);
                    oc_ui_style_set_i32(OC_UI_AXIS, info.layout.axis);
                    oc_ui_style_set_i32(OC_UI_ALIGN_X, info.layout.align.x);
                    oc_ui_style_set_i32(OC_UI_ALIGN_Y, info.layout.align.y);
                    oc_ui_style_set_i32(OC_UI_ALIGN_LINE_X, info.layout.alignLine.x);
                    oc_ui_style_set_i32(OC_UI_ALIGN_LINE_Y, info.layout.alignLine.y);
                    oc_ui_style_set_f32(OC_UI_MARGIN_X, info.layout.margin.x);
                    oc_ui_style_set_f32(OC_UI_MARGIN_Y, info.layout.margin.y);
                    oc_ui_style_set_f32(OC_UI_SPACING, info.layout.spacing);
                    oc_ui_style_set_i32(OC_UI_OVERFLOW_X, info.layout.overflow.x);
                    oc_ui_style_set_i32(OC_UI_OVERFLOW_Y, info.layout.overflow.y);
                    oc_ui_style_set_i32(OC_UI_WRAP, info.layout.wrap);

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
