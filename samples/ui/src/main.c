/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "orca.h"
#include <math.h>
#include <stdio.h>

oc_vec2 frameSize = { 1200, 838 };

oc_surface surface;
oc_canvas_renderer renderer;
oc_canvas_context canvas;
oc_font fontRegular;
oc_font fontBold;
oc_ui_context* ui;
oc_arena textArena = { 0 };
oc_arena logArena = { 0 };
oc_str8_list logLines;

typedef enum cmd
{
    CMD_NONE,
    CMD_SET_DARK_THEME,
    CMD_SET_LIGHT_THEME
} cmd;

cmd command = CMD_NONE;

ORCA_EXPORT void oc_on_init(void)
{
    oc_window_set_title(OC_STR8("Orca UI Demo"));
    oc_window_set_size(frameSize);

    renderer = oc_canvas_renderer_create();
    surface = oc_canvas_surface_create(renderer);
    canvas = oc_canvas_context_create();

    oc_font* fonts[2] = { &fontRegular, &fontBold };
    const char* fontNames[2] = { "/OpenSans-Regular.ttf", "/OpenSans-Bold.ttf" };
    for(int i = 0; i < 2; i++)
    {
        oc_arena_scope scratch = oc_scratch_begin();

        oc_file file = oc_file_open(OC_STR8(fontNames[i]), OC_FILE_ACCESS_READ, 0);
        if(oc_file_last_error(file) != OC_IO_OK)
        {
            oc_log_error("Couldn't open file %s\n", fontNames[i]);
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

        oc_scratch_end(scratch);
    }

    ui = oc_ui_context_create(fontRegular);

    oc_arena_init(&textArena);
    oc_arena_init(&logArena);
    oc_list_init(&logLines.list);
}

ORCA_EXPORT void oc_on_raw_event(oc_event* event)
{
    oc_ui_process_event(event);
}

ORCA_EXPORT void oc_on_resize(u32 width, u32 height)
{
    frameSize.x = width;
    frameSize.y = height;
}

void log_push(const char* line)
{
    oc_str8_list_push(&logArena, &logLines, (oc_str8)OC_STR8(line));
}

void log_pushf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    oc_str8 str = oc_str8_pushfv(&logArena, format, args);
    va_end(args);
    oc_str8_list_push(&logArena, &logLines, str);
}

void column_begin(const char* header, f32 widthFraction)
{
    oc_ui_box_begin(header);

    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, widthFraction, 1 });
    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
    oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
    oc_ui_style_set_f32(OC_UI_MARGIN_Y, 8);
    oc_ui_style_set_f32(OC_UI_SPACING, 24);
    oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_BG_1);
    oc_ui_style_set_var_str8(OC_UI_BORDER_COLOR, OC_UI_THEME_BORDER);
    oc_ui_style_set_f32(OC_UI_BORDER_SIZE, 1);
    oc_ui_style_set_var_str8(OC_UI_ROUNDNESS, OC_UI_THEME_ROUNDNESS_SMALL);
    oc_ui_style_set_i32(OC_UI_CONSTRAIN_Y, 1);

    oc_ui_box("header")
    {
        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
        oc_ui_style_set_i32(OC_UI_ALIGN_X, OC_UI_ALIGN_CENTER);

        oc_ui_style_rule(".label")
        {
            oc_ui_style_set_f32(OC_UI_TEXT_SIZE, 18);
        }
        oc_ui_label("label", header);
    }

    oc_ui_box_begin("contents");

    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1, 1 });
    oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
    oc_ui_style_set_i32(OC_UI_ALIGN_X, OC_UI_ALIGN_START);
    oc_ui_style_set_f32(OC_UI_MARGIN_X, 16);
    oc_ui_style_set_f32(OC_UI_SPACING, 24);
    oc_ui_style_set_i32(OC_UI_CONSTRAIN_Y, 1);
}

void column_end()
{
    oc_ui_box_end(); // contents
    oc_ui_box_end(); // column
}

#define column(h, w) oc_defer_loop(column_begin(h, w), column_end())

void labeled_slider(const char* label, f32* value)
{
    oc_ui_box(label)
    {
        oc_ui_style_set_f32(OC_UI_SPACING, 8);

        oc_ui_style_rule("label")
        {
            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 100 });
        }
        oc_ui_label("label", label);

        oc_ui_style_rule("slider")
        {
            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 100 });
        }
        oc_ui_slider("slider", value);
    }
}

ORCA_EXPORT void oc_on_frame_refresh(void)
{
    oc_arena_scope scratch = oc_scratch_begin();

    static bool darkTheme = true;

    switch(command)
    {
        case CMD_SET_DARK_THEME:
            darkTheme = true;
            break;
        case CMD_SET_LIGHT_THEME:
            darkTheme = false;
            break;
        default:
            break;
    }
    command = CMD_NONE;

    oc_ui_frame(frameSize)
    {
        if(darkTheme)
        {
            oc_ui_theme_dark();
        }
        else
        {
            oc_ui_theme_light();
        }

        oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_BG_0);
        oc_ui_style_set_i32(OC_UI_CONSTRAIN_Y, 1);

        //--------------------------------------------------------------------------------------------
        // Menu bar
        //--------------------------------------------------------------------------------------------
        oc_ui_menu_bar("menu_bar")
        {
            oc_ui_menu("file-menu", "File")
            {
                if(oc_ui_menu_button("quit", "Quit").pressed)
                {
                    oc_request_quit();
                }
            }

            oc_ui_menu("theme-menu", "Theme")
            {
                if(oc_ui_menu_button("dark", "Dark theme").pressed)
                {
                    command = CMD_SET_DARK_THEME;
                }
                if(oc_ui_menu_button("light", "Light theme").pressed)
                {
                    command = CMD_SET_LIGHT_THEME;
                }
            }
        }

        oc_ui_box("main panel")
        {
            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
            oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1, 1 });

            oc_ui_box("background")
            {
                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1, 1 });
                oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_X);
                oc_ui_style_set_f32(OC_UI_MARGIN_X, 16);
                oc_ui_style_set_f32(OC_UI_MARGIN_Y, 16);
                oc_ui_style_set_f32(OC_UI_SPACING, 16);

                column("Widgets", 1.0 / 3)
                {
                    oc_ui_box("top")
                    {
                        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                        oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_X);
                        oc_ui_style_set_f32(OC_UI_SPACING, 32);

                        oc_ui_box("top_left")
                        {
                            oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                            oc_ui_style_set_f32(OC_UI_SPACING, 24);

                            //-----------------------------------------------------------------------------
                            // Label
                            //-----------------------------------------------------------------------------
                            oc_ui_label("label", "Label");

                            //-----------------------------------------------------------------------------
                            // Button
                            //-----------------------------------------------------------------------------
                            if(oc_ui_button("button", "Button").clicked)
                            {
                                log_push("Button clicked");
                            }

                            oc_ui_box("checkbox")
                            {
                                oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_X);
                                oc_ui_style_set_i32(OC_UI_ALIGN_Y, OC_UI_ALIGN_CENTER);
                                oc_ui_style_set_f32(OC_UI_SPACING, 8);

                                //-------------------------------------------------------------------------
                                // Checkbox
                                //-------------------------------------------------------------------------
                                static bool checked = false;
                                if(oc_ui_checkbox("checkbox", &checked).clicked)
                                {
                                    if(checked)
                                    {
                                        log_push("Checkbox checked");
                                    }
                                    else
                                    {
                                        log_push("Checkbox unchecked");
                                    }
                                }

                                oc_ui_label("label", "Checkbox");
                            }
                        }

                        //---------------------------------------------------------------------------------
                        // Vertical slider
                        //---------------------------------------------------------------------------------
                        static float vSliderValue = 0;
                        static float vSliderLoggedValue = 0;
                        static f64 vSliderLogTime = 0;

                        oc_ui_style_rule("v_slider")
                        {
                            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 32 });
                            oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, 130 });
                        }

                        oc_ui_slider("v_slider", &vSliderValue);

                        f64 now = oc_clock_time(OC_CLOCK_MONOTONIC);
                        if((now - vSliderLogTime) >= 0.2 && vSliderValue != vSliderLoggedValue)
                        {
                            log_pushf("Vertical slider moved to %f", vSliderValue);
                            vSliderLoggedValue = vSliderValue;
                            vSliderLogTime = now;
                        }

                        oc_ui_box("top_right")
                        {
                            oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                            oc_ui_style_set_f32(OC_UI_SPACING, 24);

                            //-----------------------------------------------------------------------------
                            // Tooltip
                            //-----------------------------------------------------------------------------
                            if(oc_ui_label("label", "Tooltip").hovering)
                            {
                                oc_ui_tooltip("tooltip", "Hi");
                            }

                            //-----------------------------------------------------------------------------
                            // Radio group
                            //-----------------------------------------------------------------------------
                            static int radioSelected = 0;
                            oc_str8 options[] = {
                                OC_STR8("Radio 1"),
                                OC_STR8("Radio 2"),
                            };
                            oc_ui_radio_group_info radioGroupInfo = {
                                .selectedIndex = radioSelected,
                                .optionCount = 2,
                                .options = options,
                            };
                            oc_ui_radio_group_info result = oc_ui_radio_group("radio_group", &radioGroupInfo);
                            radioSelected = result.selectedIndex;
                            if(result.changed)
                            {
                                log_pushf("Selected %s", options[result.selectedIndex].ptr);
                            }

                            //-----------------------------------------------------------------------------
                            // Horizontal slider
                            //-----------------------------------------------------------------------------
                            static float hSliderValue = 0;
                            static float hSliderLoggedValue = 0;
                            static f64 hSliderLogTime = 0;

                            oc_ui_style_rule("h_slider")
                            {
                                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 130 });
                                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, 32 });
                            }

                            oc_ui_slider("h_slider", &hSliderValue);

                            f64 now = oc_clock_time(OC_CLOCK_MONOTONIC);
                            if((now - hSliderLogTime) >= 0.2 && hSliderValue != hSliderLoggedValue)
                            {
                                log_pushf("Slider moved to %f", hSliderValue);
                                hSliderLoggedValue = hSliderValue;
                                hSliderLogTime = now;
                            }
                        }
                    }

                    //-------------------------------------------------------------------------------------
                    // Text box
                    //-------------------------------------------------------------------------------------
                    {
                        oc_ui_style_rule("text")
                        {
                            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 305 });
                        }

                        static oc_ui_text_box_info textInfo = {
                            .defaultText = OC_STR8_LIT("Type here"),
                        };
                        oc_ui_text_box_result result = oc_ui_text_box("text", scratch.arena, &textInfo);
                        if(result.changed)
                        {
                            oc_arena_clear(&textArena);
                            textInfo.text = oc_str8_push_copy(&textArena, result.text);
                        }
                        if(result.accepted)
                        {
                            log_pushf("Entered text \"%s\"", textInfo.text.ptr);
                        }
                    }

                    //-------------------------------------------------------------------------------------
                    // Select
                    //-------------------------------------------------------------------------------------
                    {
                        static int selected = -1;
                        oc_str8 options[] = { OC_STR8("Option 1"),
                                              OC_STR8("Option 2") };
                        oc_ui_select_popup_info info = {
                            .selectedIndex = selected,
                            .optionCount = 2,
                            .options = options,
                            .placeholder = OC_STR8_LIT("Select"),
                        };
                        oc_ui_select_popup_info result = oc_ui_select_popup("select", &info);
                        if(result.selectedIndex != selected)
                        {
                            log_pushf("Selected %s", options[result.selectedIndex].ptr);
                        }
                        selected = result.selectedIndex;
                    }

                    //-------------------------------------------------------------------------------------
                    // Scrollable panel
                    //-------------------------------------------------------------------------------------
                    oc_ui_box("log")
                    {
                        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1, 1, .minSize = 200 });
                        oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_BG_2);
                        oc_ui_style_set_var_str8(OC_UI_BORDER_COLOR, OC_UI_THEME_BORDER);
                        oc_ui_style_set_f32(OC_UI_BORDER_SIZE, 1);
                        oc_ui_style_set_var_str8(OC_UI_ROUNDNESS, OC_UI_THEME_ROUNDNESS_SMALL);

                        oc_ui_style_set_i32(OC_UI_OVERFLOW_Y, OC_UI_OVERFLOW_SCROLL);

                        oc_ui_box("contents")
                        {
                            oc_ui_style_set_f32(OC_UI_MARGIN_X, 16);
                            oc_ui_style_set_f32(OC_UI_MARGIN_Y, 16);
                            oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);

                            if(oc_list_empty(logLines.list))
                            {
                                oc_ui_style_rule("label")
                                {
                                    oc_ui_style_set_var_str8(OC_UI_COLOR, OC_UI_THEME_TEXT_2);
                                }
                                oc_ui_label("label", "Log");
                            }

                            i32 i = 0;
                            oc_list_for(logLines.list, logLine, oc_str8_elt, listElt)
                            {
                                char id[15];
                                snprintf(id, sizeof(id), "%d", i);
                                oc_ui_label_str8(OC_STR8(id), logLine->string);
                                i++;
                            }
                        }
                    }
                }

                //-----------------------------------------------------------------------------------------
                // Styling
                //-----------------------------------------------------------------------------------------
                column("Styling", 2.0 / 3)
                {
                    static f32 unselectedWidth = 16;
                    static f32 unselectedHeight = 16;
                    static f32 unselectedRoundness = 8;
                    static oc_color unselectedBgColor = { 0 };
                    static oc_color unselectedBorderColor = { 0.976, 0.976, 0.976, 0.35 };
                    static f32 unselectedBorderSize = 1;
                    static oc_str8 unselectedWhenStatus = OC_STR8("");

                    static f32 selectedWidth = 16;
                    static f32 selectedHeight = 16;
                    static f32 selectedRoundness = 8;
                    static oc_color selectedCenterColor = { 1, 1, 1, 1 };
                    static oc_color selectedBgColor = { 0.33, 0.66, 1, 1 };
                    static oc_str8 selectedWhenStatus = OC_STR8("");

                    static oc_color labelFontColor = { 0.976, 0.976, 0.976, 1 };
                    static oc_font* labelFont = &fontRegular;
                    static f32 labelFontSize = 14;

                    oc_ui_box("styled_radios")
                    {
                        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, 152 });
                        oc_ui_style_set_color(OC_UI_BG_COLOR, (oc_color){ 0.086, 0.086, 0.102, 1 });
                        oc_ui_style_set_var_str8(OC_UI_ROUNDNESS, OC_UI_THEME_ROUNDNESS_SMALL);

                        oc_ui_style_set_i32(OC_UI_ALIGN_X, OC_UI_ALIGN_CENTER);
                        oc_ui_style_set_i32(OC_UI_ALIGN_Y, OC_UI_ALIGN_CENTER);

                        {
                            oc_str8_list list = { 0 };
                            oc_str8_list_push(scratch.arena, &list, OC_STR8("radio_group .radio-row"));
                            oc_str8_list_push(scratch.arena, &list, unselectedWhenStatus);
                            oc_str8_list_push(scratch.arena, &list, OC_STR8(" .radio"));
                            oc_str8 unselectedPattern = oc_str8_list_join(scratch.arena, list);

                            oc_ui_style_rule_str8(unselectedPattern)
                            {
                                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, unselectedWidth });
                                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, unselectedHeight });
                                oc_ui_style_set_color(OC_UI_BG_COLOR, unselectedBgColor);
                                oc_ui_style_set_color(OC_UI_BORDER_COLOR, unselectedBorderColor);
                                oc_ui_style_set_f32(OC_UI_BORDER_SIZE, unselectedBorderSize);
                                oc_ui_style_set_f32(OC_UI_ROUNDNESS, unselectedRoundness);
                            }
                        }

                        {
                            oc_str8_list list = { 0 };
                            oc_str8_list_push(scratch.arena, &list, OC_STR8("radio_group .radio-row"));
                            oc_str8_list_push(scratch.arena, &list, selectedWhenStatus);
                            oc_str8_list_push(scratch.arena, &list, OC_STR8(" .radio_selected"));
                            oc_str8 selectedPattern = oc_str8_list_join(scratch.arena, list);

                            oc_ui_style_rule_str8(selectedPattern)
                            {
                                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, selectedWidth });
                                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, selectedHeight });
                                oc_ui_style_set_color(OC_UI_BG_COLOR, selectedBgColor);
                                oc_ui_style_set_color(OC_UI_COLOR, selectedCenterColor);
                                oc_ui_style_set_f32(OC_UI_ROUNDNESS, selectedRoundness);
                            }
                        }

                        oc_ui_style_rule("radio_group label")
                        {
                            oc_ui_style_set_color(OC_UI_COLOR, labelFontColor);
                            oc_ui_style_set_font(OC_UI_FONT, *labelFont);
                            oc_ui_style_set_f32(OC_UI_TEXT_SIZE, labelFontSize);
                        }

                        static int selectedIndex = 0;
                        oc_str8 options[] = {
                            OC_STR8("I"),
                            OC_STR8("Am"),
                            OC_STR8("Stylish"),
                        };
                        oc_ui_radio_group_info radioGroupInfo = {
                            .selectedIndex = selectedIndex,
                            .optionCount = oc_array_size(options),
                            .options = options,
                        };
                        oc_ui_radio_group_info result = oc_ui_radio_group("radio_group", &radioGroupInfo);
                        selectedIndex = result.selectedIndex;
                    }

                    oc_ui_box("controls")
                    {
                        oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_X);
                        oc_ui_style_set_f32(OC_UI_SPACING, 32);

                        oc_ui_box("unselected")
                        {
                            oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                            oc_ui_style_set_f32(OC_UI_SPACING, 16);

                            oc_ui_style_rule("radio-label")
                            {
                                oc_ui_style_set_f32(OC_UI_TEXT_SIZE, 16);
                            }
                            oc_ui_label("radio-label", "Radio style");

                            oc_ui_box("size")
                            {
                                oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                                oc_ui_style_set_f32(OC_UI_SPACING, 4);

                                f32 widthSlider = (unselectedWidth - 8) / 16;
                                labeled_slider("Width", &widthSlider);
                                unselectedWidth = 8 + widthSlider * 16;

                                f32 heightSlider = (unselectedHeight - 8) / 16;
                                labeled_slider("Height", &heightSlider);
                                unselectedHeight = 8 + heightSlider * 16;

                                f32 roundnessSlider = (unselectedRoundness - 4) / 8;
                                labeled_slider("Roundness", &roundnessSlider);
                                unselectedRoundness = 4 + roundnessSlider * 8;
                            }

                            oc_ui_box("background")
                            {
                                oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                                oc_ui_style_set_f32(OC_UI_SPACING, 4);
                                labeled_slider("Background R", &unselectedBgColor.r);
                                labeled_slider("Background G", &unselectedBgColor.g);
                                labeled_slider("Background B", &unselectedBgColor.b);
                                labeled_slider("Background A", &unselectedBgColor.a);
                            }

                            oc_ui_box("border")
                            {
                                oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                                oc_ui_style_set_f32(OC_UI_SPACING, 4);
                                labeled_slider("Border R", &unselectedBorderColor.r);
                                labeled_slider("Border G", &unselectedBorderColor.g);
                                labeled_slider("Border B", &unselectedBorderColor.b);
                                labeled_slider("Border A", &unselectedBorderColor.a);
                            }

                            f32 borderSizeSlider = unselectedBorderSize / 5;
                            labeled_slider("Border size", &borderSizeSlider);
                            unselectedBorderSize = borderSizeSlider * 5;

                            oc_ui_box("status_override")
                            {
                                oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                                oc_ui_style_set_f32(OC_UI_SPACING, 10);
                                oc_ui_label("label", "Override");

                                static int statusIndex = 0;
                                oc_str8 statusOptions[] = {
                                    OC_STR8("Always"),
                                    OC_STR8("When hovering"),
                                    OC_STR8("When active"),
                                };
                                oc_ui_radio_group_info statusInfo = {
                                    .selectedIndex = statusIndex,
                                    .optionCount = oc_array_size(statusOptions),
                                    .options = statusOptions,
                                };
                                oc_ui_radio_group_info result = oc_ui_radio_group("status", &statusInfo);
                                statusIndex = result.selectedIndex;
                                switch(statusIndex)
                                {
                                    case 0:
                                        unselectedWhenStatus = OC_STR8("");
                                        break;
                                    case 1:
                                        unselectedWhenStatus = OC_STR8(".hover");
                                        break;
                                    case 2:
                                        unselectedWhenStatus = OC_STR8(".active");
                                        break;
                                    default:
                                        break;
                                }
                            }
                        }

                        oc_ui_box("selected")
                        {
                            oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                            oc_ui_style_set_f32(OC_UI_SPACING, 16);

                            oc_ui_style_rule("radio-label")
                            {
                                oc_ui_style_set_f32(OC_UI_TEXT_SIZE, 16);
                            }
                            oc_ui_label("radio-label", "Radio style");

                            oc_ui_box("size")
                            {
                                oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                                oc_ui_style_set_f32(OC_UI_SPACING, 4);

                                f32 widthSlider = (selectedWidth - 8) / 16;
                                labeled_slider("Width", &widthSlider);
                                selectedWidth = 8 + widthSlider * 16;

                                f32 heightSlider = (selectedHeight - 8) / 16;
                                labeled_slider("Height", &heightSlider);
                                selectedHeight = 8 + heightSlider * 16;

                                f32 roundnessSlider = (selectedRoundness - 4) / 8;
                                labeled_slider("Roundness", &roundnessSlider);
                                selectedRoundness = 4 + roundnessSlider * 8;
                            }

                            oc_ui_box("background")
                            {
                                oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                                oc_ui_style_set_f32(OC_UI_SPACING, 4);
                                labeled_slider("Background R", &selectedBgColor.r);
                                labeled_slider("Background G", &selectedBgColor.g);
                                labeled_slider("Background B", &selectedBgColor.b);
                                labeled_slider("Background A", &selectedBgColor.a);
                            }

                            oc_ui_box("center")
                            {
                                oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                                oc_ui_style_set_f32(OC_UI_SPACING, 4);
                                labeled_slider("Center R", &selectedCenterColor.r);
                                labeled_slider("Center G", &selectedCenterColor.g);
                                labeled_slider("Center B", &selectedCenterColor.b);
                                labeled_slider("Center A", &selectedCenterColor.a);
                            }

                            oc_ui_box("spacer")
                            {
                                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, 24 });
                            }

                            oc_ui_box("status_override")
                            {
                                oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                                oc_ui_style_set_f32(OC_UI_SPACING, 10);
                                oc_ui_label("label", "Override");

                                static int statusIndex = 0;
                                oc_str8 statusOptions[] = {
                                    OC_STR8("Always"),
                                    OC_STR8("When hovering"),
                                    OC_STR8("When active"),
                                };
                                oc_ui_radio_group_info statusInfo = {
                                    .selectedIndex = statusIndex,
                                    .optionCount = oc_array_size(statusOptions),
                                    .options = statusOptions,
                                };
                                oc_ui_radio_group_info result = oc_ui_radio_group("status", &statusInfo);
                                statusIndex = result.selectedIndex;
                                switch(statusIndex)
                                {
                                    case 0:
                                        selectedWhenStatus = OC_STR8("");
                                        break;
                                    case 1:
                                        selectedWhenStatus = OC_STR8(".hover");
                                        break;
                                    case 2:
                                        selectedWhenStatus = OC_STR8(".active");
                                        break;
                                    default:
                                        break;
                                }
                            }
                        }

                        oc_ui_box("label")
                        {
                            oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                            oc_ui_style_set_f32(OC_UI_SPACING, 16);

                            oc_ui_style_rule("label-style")
                            {
                                oc_ui_style_set_f32(OC_UI_TEXT_SIZE, 16);
                            }
                            oc_ui_label("label-style", "Label style");

                            oc_ui_box("font_color")
                            {
                                oc_ui_style_set_f32(OC_UI_SPACING, 8);

                                oc_ui_style_rule("font-color")
                                {
                                    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 100 });
                                }
                                oc_ui_label("font-color", "Font color");

                                static int colorSelected = 0;
                                oc_str8 colorNames[] = {
                                    OC_STR8("Default"),
                                    OC_STR8("Red"),
                                    OC_STR8("Orange"),
                                    OC_STR8("Amber"),
                                    OC_STR8("Yellow"),
                                    OC_STR8("Lime"),
                                    OC_STR8("Light Green"),
                                    OC_STR8("Green"),
                                };
                                oc_color colors[] = {
                                    { 1, 1, 1, 1, OC_COLOR_SPACE_SRGB },
                                    { 0.988, 0.447, 0.353, 1, OC_COLOR_SPACE_SRGB },
                                    { 1.000, 0.682, 0.263, 1, OC_COLOR_SPACE_SRGB },
                                    { 0.961, 0.792, 0.314, 1, OC_COLOR_SPACE_SRGB },
                                    { 0.992, 0.871, 0.263, 1, OC_COLOR_SPACE_SRGB },
                                    { 0.682, 0.863, 0.227, 1, OC_COLOR_SPACE_SRGB },
                                    { 0.592, 0.776, 0.373, 1, OC_COLOR_SPACE_SRGB },
                                    { 0.365, 0.761, 0.392, 1, OC_COLOR_SPACE_SRGB },
                                };
                                oc_ui_select_popup_info colorInfo = {
                                    .selectedIndex = colorSelected,
                                    .optionCount = oc_array_size(colorNames),
                                    .options = colorNames,
                                };
                                oc_ui_select_popup_info colorResult = oc_ui_select_popup("color", &colorInfo);
                                colorSelected = colorResult.selectedIndex;
                                labelFontColor = colors[colorSelected];
                            }

                            oc_ui_box("font")
                            {
                                oc_ui_style_set_f32(OC_UI_SPACING, 8);

                                oc_ui_style_rule("font-label")
                                {
                                    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 100 });
                                }
                                oc_ui_label("font-label", "Font");

                                static int fontSelected = 0;
                                oc_str8 fontNames[] = {
                                    OC_STR8("Regular"),
                                    OC_STR8("Bold"),
                                };
                                oc_font* fonts[] = {
                                    &fontRegular,
                                    &fontBold,
                                };
                                oc_ui_select_popup_info fontInfo = {
                                    .selectedIndex = fontSelected,
                                    .optionCount = oc_array_size(fontNames),
                                    .options = fontNames,
                                };
                                oc_ui_select_popup_info fontResult = oc_ui_select_popup("font_style", &fontInfo);
                                fontSelected = fontResult.selectedIndex;
                                labelFont = fonts[fontSelected];
                            }

                            f32 fontSizeSlider = (labelFontSize - 8) / 16;
                            labeled_slider("Font size", &fontSizeSlider);
                            labelFontSize = 8 + fontSizeSlider * 16;
                        }
                    }
                }
            }
        } // main panel
    }     // frame

    oc_canvas_context_select(canvas);

    oc_ui_draw();
    oc_canvas_render(renderer, canvas, surface);
    oc_canvas_present(renderer, surface);

    oc_scratch_end(scratch);
}
