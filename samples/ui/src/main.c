/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "orca.h"
#include <math.h>

oc_vec2 frameSize = { 1200, 838 };

oc_surface surface;
oc_canvas canvas;
oc_font fontRegular;
oc_font fontBold;
oc_ui_context ui;
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

    surface = oc_surface_canvas();
    canvas = oc_canvas_create();
    oc_ui_init(&ui);

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

    oc_arena_init(&textArena);
    oc_arena_init(&logArena);
    oc_list_init(&logLines.list);
}

ORCA_EXPORT void oc_on_raw_event(oc_event* event)
{
    oc_ui_process_event(event);
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
    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, widthFraction },
                                     .size.height = { OC_UI_SIZE_PARENT, 1 },
                                     .layout.axis = OC_UI_AXIS_Y,
                                     .layout.margin.y = 8,
                                     .layout.spacing = 24,
                                     .bgColor = ui.theme->bg1,
                                     .borderColor = ui.theme->border,
                                     .borderSize = 1,
                                     .roundness = ui.theme->roundnessSmall },
                     OC_UI_STYLE_SIZE
                         | OC_UI_STYLE_LAYOUT_AXIS
                         | OC_UI_STYLE_LAYOUT_MARGIN_Y
                         | OC_UI_STYLE_LAYOUT_SPACING
                         | OC_UI_STYLE_BG_COLOR
                         | OC_UI_STYLE_BORDER_COLOR
                         | OC_UI_STYLE_BORDER_SIZE
                         | OC_UI_STYLE_ROUNDNESS);
    oc_ui_box_begin(header, OC_UI_FLAG_DRAW_BACKGROUND | OC_UI_FLAG_DRAW_BORDER);

    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                     .layout.align.x = OC_UI_ALIGN_CENTER },
                     OC_UI_STYLE_SIZE_WIDTH
                         | OC_UI_STYLE_LAYOUT_ALIGN_X);
    oc_ui_container("header", OC_UI_FLAG_NONE)
    {
        oc_ui_style_next(&(oc_ui_style){ .fontSize = 18 },
                         OC_UI_STYLE_FONT_SIZE);
        oc_ui_label(header);
    }

    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                     .size.height = { OC_UI_SIZE_PARENT, 0.8 },
                                     .layout.align.x = OC_UI_ALIGN_START,
                                     .layout.margin.x = 16,
                                     .layout.spacing = 24 },
                     OC_UI_STYLE_SIZE
                         | OC_UI_STYLE_LAYOUT_ALIGN_X
                         | OC_UI_STYLE_LAYOUT_MARGIN_X
                         | OC_UI_STYLE_LAYOUT_SPACING);
    oc_ui_box_begin("contents", OC_UI_FLAG_NONE);
}

void column_end()
{
    oc_ui_box_end(); // contents
    oc_ui_box_end(); // column
}

#define column(h, w) oc_defer_loop(column_begin(h, w), column_end())

void labeled_slider(const char* label, f32* value)
{
    oc_ui_style_next(&(oc_ui_style){ .layout.axis = OC_UI_AXIS_X,
                                     .layout.spacing = 8 },
                     OC_UI_STYLE_LAYOUT_AXIS
                         | OC_UI_STYLE_LAYOUT_SPACING);
    oc_ui_container(label, OC_UI_FLAG_NONE)
    {
        oc_ui_style_match_after(oc_ui_pattern_owner(),
                                &(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, 100 } },
                                OC_UI_STYLE_SIZE_WIDTH);
        oc_ui_label(label);

        oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, 100 } },
                         OC_UI_STYLE_SIZE_WIDTH);
        oc_ui_slider("slider", value);
    }
}

ORCA_EXPORT void oc_on_frame_refresh(void)
{
    switch(command)
    {
        case CMD_SET_DARK_THEME:
            oc_ui_set_theme(&OC_UI_DARK_THEME);
            break;
        case CMD_SET_LIGHT_THEME:
            oc_ui_set_theme(&OC_UI_LIGHT_THEME);
            break;
        default:
            break;
    }
    command = CMD_NONE;

    oc_arena_scope scratch = oc_scratch_begin();
    oc_ui_style defaultStyle = { .font = fontRegular };
    oc_ui_style_mask defaultMask = OC_UI_STYLE_FONT;
    oc_ui_frame(frameSize, &defaultStyle, defaultMask)
    {
        //--------------------------------------------------------------------------------------------
        // Menu bar
        //--------------------------------------------------------------------------------------------
        oc_ui_menu_bar("menu_bar")
        {
            oc_ui_menu("File")
            {
                if(oc_ui_menu_button("Quit").pressed)
                {
                    oc_request_quit();
                }
            }

            oc_ui_menu("Theme")
            {
                if(oc_ui_menu_button("Dark theme").pressed)
                {
                    command = CMD_SET_DARK_THEME;
                }
                if(oc_ui_menu_button("Light theme").pressed)
                {
                    command = CMD_SET_LIGHT_THEME;
                }
            }
        }

        oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                         .size.height = { OC_UI_SIZE_PARENT_MINUS_PIXELS, 28 },
                                         .layout.axis = OC_UI_AXIS_X,
                                         .layout.margin.x = 16,
                                         .layout.margin.y = 16,
                                         .layout.spacing = 16 },
                         OC_UI_STYLE_SIZE
                             | OC_UI_STYLE_LAYOUT_AXIS
                             | OC_UI_STYLE_LAYOUT_MARGINS
                             | OC_UI_STYLE_LAYOUT_SPACING);
        oc_ui_container("background", OC_UI_FLAG_DRAW_BACKGROUND)
        {
            column("Widgets", 1.0 / 3)
            {

                oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                                 .layout.axis = OC_UI_AXIS_X,
                                                 .layout.spacing = 32 },
                                 OC_UI_STYLE_SIZE_WIDTH
                                     | OC_UI_STYLE_LAYOUT_AXIS
                                     | OC_UI_STYLE_LAYOUT_SPACING);
                oc_ui_container("top", OC_UI_FLAG_NONE)
                {
                    oc_ui_style_next(&(oc_ui_style){ .layout.axis = OC_UI_AXIS_Y,
                                                     .layout.spacing = 24 },
                                     OC_UI_STYLE_LAYOUT_AXIS
                                         | OC_UI_STYLE_LAYOUT_SPACING);
                    oc_ui_container("top_left", OC_UI_FLAG_NONE)
                    {
                        //-----------------------------------------------------------------------------
                        // Label
                        //-----------------------------------------------------------------------------
                        oc_ui_label("Label");

                        //-----------------------------------------------------------------------------
                        // Button
                        //-----------------------------------------------------------------------------
                        if(oc_ui_button("Button").clicked)
                        {
                            log_push("Button clicked");
                        }

                        oc_ui_style_next(&(oc_ui_style){ .layout.axis = OC_UI_AXIS_X,
                                                         .layout.align.y = OC_UI_ALIGN_CENTER,
                                                         .layout.spacing = 8 },
                                         OC_UI_STYLE_LAYOUT_AXIS
                                             | OC_UI_STYLE_LAYOUT_ALIGN_Y
                                             | OC_UI_STYLE_LAYOUT_SPACING);
                        oc_ui_container("checkbox", OC_UI_FLAG_NONE)
                        {
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

                            oc_ui_label("Checkbox");
                        }
                    }

                    //---------------------------------------------------------------------------------
                    // Vertical slider
                    //---------------------------------------------------------------------------------
                    static float vSliderValue = 0;
                    static float vSliderLoggedValue = 0;
                    static f64 vSliderLogTime = 0;
                    oc_ui_style_next(&(oc_ui_style){ .size.height = { OC_UI_SIZE_PIXELS, 130 } },
                                     OC_UI_STYLE_SIZE_HEIGHT);
                    oc_ui_slider("v_slider", &vSliderValue);
                    f64 now = oc_clock_time(OC_CLOCK_MONOTONIC);
                    if((now - vSliderLogTime) >= 0.2 && vSliderValue != vSliderLoggedValue)
                    {
                        log_pushf("Vertical slider moved to %f", vSliderValue);
                        vSliderLoggedValue = vSliderValue;
                        vSliderLogTime = now;
                    }

                    oc_ui_style_next(&(oc_ui_style){ .layout.axis = OC_UI_AXIS_Y,
                                                     .layout.spacing = 24 },
                                     OC_UI_STYLE_LAYOUT_AXIS
                                         | OC_UI_STYLE_LAYOUT_SPACING);
                    oc_ui_container("top_right", OC_UI_FLAG_NONE)
                    {
                        //-----------------------------------------------------------------------------
                        // Tooltip
                        //-----------------------------------------------------------------------------
                        if(oc_ui_label("Tooltip").hovering)
                        {
                            oc_ui_tooltip("Hi");
                        }

                        //-----------------------------------------------------------------------------
                        // Radio group
                        //-----------------------------------------------------------------------------
                        static int radioSelected = 0;
                        oc_str8 options[] = { OC_STR8("Radio 1"),
                                              OC_STR8("Radio 2") };
                        oc_ui_radio_group_info radioGroupInfo = { .selectedIndex = radioSelected,
                                                                  .optionCount = 2,
                                                                  .options = options };
                        oc_ui_radio_group_info result = oc_ui_radio_group("radio_group", &radioGroupInfo);
                        radioSelected = result.selectedIndex;
                        if(result.changed)
                        {
                            log_pushf("Selected Radio %i", result.selectedIndex + 1);
                        }

                        //-----------------------------------------------------------------------------
                        // Horizontal slider
                        //-----------------------------------------------------------------------------
                        static float hSliderValue = 0;
                        static float hSliderLoggedValue = 0;
                        static f64 hSliderLogTime = 0;
                        oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, 130 } },
                                         OC_UI_STYLE_SIZE_WIDTH);
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
                oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, 305 },
                                                 .size.height = { OC_UI_SIZE_TEXT } },
                                 OC_UI_STYLE_SIZE);
                static oc_str8 text = OC_STR8_LIT("Text box");
                oc_ui_text_box_result res = oc_ui_text_box("text", scratch.arena, text);
                if(res.changed)
                {
                    oc_arena_clear(&textArena);
                    text = oc_str8_push_copy(&textArena, res.text);
                }
                if(res.accepted)
                {
                    log_pushf("Entered text \"%s\"", text.ptr);
                }

                //-------------------------------------------------------------------------------------
                // Select
                //-------------------------------------------------------------------------------------
                static int selected = -1;
                oc_str8 options[] = { OC_STR8("Option 1"),
                                      OC_STR8("Option 2") };
                oc_ui_select_popup_info info = { .selectedIndex = selected,
                                                 .optionCount = 2,
                                                 .options = options,
                                                 .placeholder = OC_STR8_LIT("Select") };
                oc_ui_select_popup_info result = oc_ui_select_popup("select", &info);
                if(result.selectedIndex != selected)
                {
                    log_pushf("Selected %s", options[result.selectedIndex].ptr);
                }
                selected = result.selectedIndex;

                //-------------------------------------------------------------------------------------
                // Scrollable panel
                //-------------------------------------------------------------------------------------
                oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                                 .size.height = { OC_UI_SIZE_PIXELS, 430 },
                                                 .layout.margin.x = 16,
                                                 .layout.margin.y = 16,
                                                 .layout.spacing = 8,
                                                 .bgColor = ui.theme->bg2,
                                                 .borderColor = ui.theme->border,
                                                 .borderSize = 1,
                                                 .roundness = ui.theme->roundnessSmall },
                                 OC_UI_STYLE_SIZE
                                     | OC_UI_STYLE_LAYOUT_MARGINS
                                     | OC_UI_STYLE_LAYOUT_SPACING
                                     | OC_UI_STYLE_BG_COLOR
                                     | OC_UI_STYLE_BORDER_COLOR
                                     | OC_UI_STYLE_BORDER_SIZE
                                     | OC_UI_STYLE_ROUNDNESS);
                oc_ui_panel("log", OC_UI_FLAG_DRAW_BACKGROUND | OC_UI_FLAG_DRAW_BORDER)
                {
                    if(oc_list_empty(&logLines.list))
                    {
                        oc_ui_style_next(&(oc_ui_style){ .color = ui.theme->text2 },
                                         OC_UI_STYLE_COLOR);
                        oc_ui_label("Log");
                    }

                    i32 i = 0;
                    oc_list_for(&logLines.list, logLine, oc_str8_elt, listElt)
                    {
                        char id[15];
                        snprintf(id, sizeof(id), "%d", i);
                        oc_ui_container(id, OC_UI_FLAG_NONE)
                        {
                            oc_ui_label_str8(logLine->string);
                        }
                        i++;
                    }
                }
            }

            //-----------------------------------------------------------------------------------------
            // Styling
            //-----------------------------------------------------------------------------------------
            // Initial values here are hardcoded from the dark theme and everything is overridden all
            // the time. In a real program you'd only override what you need and supply the values from
            // ui.theme or ui.theme->palette.
            //
            // Rule-based styling is described at
            // https://www.forkingpaths.dev/posts/23-03-10/rule_based_styling_imgui.html
            column("Styling", 2.0 / 3)
            {
                static f32 unselectedWidth = 16;
                static f32 unselectedHeight = 16;
                static f32 unselectedRoundness = 8;
                static f32 unselectedBgR = 0.086;
                static f32 unselectedBgG = 0.086;
                static f32 unselectedBgB = 0.102;
                static f32 unselectedBgA = 1;
                static f32 unselectedBorderR = 0.976;
                static f32 unselectedBorderG = 0.976;
                static f32 unselectedBorderB = 0.976;
                static f32 unselectedBorderA = 0.35;
                static f32 unselectedBorderSize = 1;
                static oc_ui_status unselectedWhenStatus = OC_UI_NONE;

                static f32 selectedWidth = 16;
                static f32 selectedHeight = 16;
                static f32 selectedRoundness = 8;
                static f32 selectedR = 1;
                static f32 selectedG = 1;
                static f32 selectedB = 1;
                static f32 selectedA = 1;
                static f32 selectedBgR = 0.33;
                static f32 selectedBgG = 0.66;
                static f32 selectedBgB = 1;
                static f32 selectedBgA = 1;
                static oc_ui_status selectedWhenStatus = OC_UI_NONE;

                static oc_color labelFontColor = { 0.976, 0.976, 0.976, 1 };
                static oc_font* labelFont = &fontRegular;
                static f32 labelFontSize = 14;

                oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                                 .size.height = { OC_UI_SIZE_PIXELS, 152 },
                                                 .layout.margin.x = 320,
                                                 .layout.margin.y = 16,
                                                 .bgColor = OC_UI_DARK_THEME.bg0,
                                                 .roundness = OC_UI_DARK_THEME.roundnessSmall },
                                 OC_UI_STYLE_SIZE
                                     | OC_UI_STYLE_LAYOUT_MARGINS
                                     | OC_UI_STYLE_BG_COLOR
                                     | OC_UI_STYLE_ROUNDNESS);
                oc_ui_container("styled_radios", OC_UI_FLAG_DRAW_BACKGROUND | OC_UI_FLAG_DRAW_BORDER)
                {
                    oc_ui_pattern unselectedPattern = { 0 };
                    oc_ui_pattern_push(scratch.arena,
                                       &unselectedPattern,
                                       (oc_ui_selector){ .kind = OC_UI_SEL_TAG,
                                                         .tag = oc_ui_tag_make("radio") });
                    oc_ui_pattern_push(scratch.arena,
                                       &unselectedPattern,
                                       (oc_ui_selector){ .op = OC_UI_SEL_AND,
                                                         .kind = OC_UI_SEL_STATUS,
                                                         .status = unselectedWhenStatus });
                    oc_ui_style_match_after(unselectedPattern,
                                            &(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, unselectedWidth },
                                                            .size.height = { OC_UI_SIZE_PIXELS, unselectedHeight },
                                                            .bgColor = { unselectedBgR, unselectedBgG, unselectedBgB, unselectedBgA },
                                                            .borderColor = { unselectedBorderR, unselectedBorderG, unselectedBorderB, unselectedBorderA },
                                                            .borderSize = unselectedBorderSize,
                                                            .roundness = unselectedRoundness },
                                            OC_UI_STYLE_SIZE
                                                | OC_UI_STYLE_BG_COLOR
                                                | OC_UI_STYLE_BORDER_COLOR
                                                | OC_UI_STYLE_BORDER_SIZE
                                                | OC_UI_STYLE_ROUNDNESS);

                    oc_ui_pattern selectedPattern = { 0 };
                    oc_ui_pattern_push(scratch.arena,
                                       &selectedPattern,
                                       (oc_ui_selector){ .kind = OC_UI_SEL_TAG,
                                                         .tag = oc_ui_tag_make("radio_selected") });
                    oc_ui_pattern_push(scratch.arena,
                                       &selectedPattern,
                                       (oc_ui_selector){ .op = OC_UI_SEL_AND,
                                                         .kind = OC_UI_SEL_STATUS,
                                                         .status = selectedWhenStatus });
                    oc_ui_style_match_after(selectedPattern,
                                            &(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, selectedWidth },
                                                            .size.height = { OC_UI_SIZE_PIXELS, selectedHeight },
                                                            .color = { selectedR, selectedG, selectedB, selectedA },
                                                            .bgColor = { selectedBgR, selectedBgG, selectedBgB, selectedBgA },
                                                            .roundness = selectedRoundness },
                                            OC_UI_STYLE_SIZE
                                                | OC_UI_STYLE_COLOR
                                                | OC_UI_STYLE_BG_COLOR
                                                | OC_UI_STYLE_ROUNDNESS);

                    oc_ui_pattern labelPattern = { 0 };
                    oc_ui_tag labelTag = oc_ui_tag_make("label");
                    oc_ui_pattern_push(scratch.arena, &labelPattern, (oc_ui_selector){ .kind = OC_UI_SEL_TAG, .tag = labelTag });
                    oc_ui_style_match_after(labelPattern,
                                            &(oc_ui_style){ .color = labelFontColor,
                                                            .font = *labelFont,
                                                            .fontSize = labelFontSize },
                                            OC_UI_STYLE_COLOR
                                                | OC_UI_STYLE_FONT
                                                | OC_UI_STYLE_FONT_SIZE);

                    static int selectedIndex = 0;
                    oc_str8 options[] = { OC_STR8("I"),
                                          OC_STR8("Am"),
                                          OC_STR8("Stylish") };
                    oc_ui_radio_group_info radioGroupInfo = { .selectedIndex = selectedIndex,
                                                              .optionCount = oc_array_size(options),
                                                              .options = options };
                    oc_ui_radio_group_info result = oc_ui_radio_group("radio_group", &radioGroupInfo);
                    selectedIndex = result.selectedIndex;
                }

                oc_ui_style_next(&(oc_ui_style){ .layout.axis = OC_UI_AXIS_X,
                                                 .layout.spacing = 32 },
                                 OC_UI_STYLE_LAYOUT_AXIS
                                     | OC_UI_STYLE_LAYOUT_SPACING);
                oc_ui_container("controls", OC_UI_FLAG_NONE)
                {
                    oc_ui_style_next(&(oc_ui_style){ .layout.axis = OC_UI_AXIS_Y,
                                                     .layout.spacing = 16 },
                                     OC_UI_STYLE_LAYOUT_AXIS
                                         | OC_UI_STYLE_LAYOUT_SPACING);
                    oc_ui_container("unselected", OC_UI_FLAG_NONE)
                    {
                        oc_ui_style_next(&(oc_ui_style){ .fontSize = 16 },
                                         OC_UI_STYLE_FONT_SIZE);
                        oc_ui_label("Radio style");

                        oc_ui_style_next(&(oc_ui_style){ .layout.spacing = 4 },
                                         OC_UI_STYLE_LAYOUT_SPACING);
                        oc_ui_container("size", OC_UI_FLAG_NONE)
                        {
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

                        oc_ui_style_next(&(oc_ui_style){ .layout.spacing = 4 },
                                         OC_UI_STYLE_LAYOUT_SPACING);
                        oc_ui_container("background", OC_UI_FLAG_NONE)
                        {
                            labeled_slider("Background R", &unselectedBgR);
                            labeled_slider("Background G", &unselectedBgG);
                            labeled_slider("Background B", &unselectedBgB);
                            labeled_slider("Background A", &unselectedBgA);
                        }

                        oc_ui_style_next(&(oc_ui_style){ .layout.spacing = 4 },
                                         OC_UI_STYLE_LAYOUT_SPACING);
                        oc_ui_container("border", OC_UI_FLAG_NONE)
                        {
                            labeled_slider("Border R", &unselectedBorderR);
                            labeled_slider("Border G", &unselectedBorderG);
                            labeled_slider("Border B", &unselectedBorderB);
                            labeled_slider("Border A", &unselectedBorderA);
                        }

                        f32 borderSizeSlider = unselectedBorderSize / 5;
                        labeled_slider("Border size", &borderSizeSlider);
                        unselectedBorderSize = borderSizeSlider * 5;

                        oc_ui_style_next(&(oc_ui_style){ .layout.spacing = 10 },
                                         OC_UI_STYLE_LAYOUT_SPACING);
                        oc_ui_container("status_override", OC_UI_FLAG_NONE)
                        {
                            oc_ui_label("Override");

                            static int statusIndex = 0;
                            oc_str8 statusOptions[] = { OC_STR8("Always"),
                                                        OC_STR8("When hovering"),
                                                        OC_STR8("When dragging") };
                            oc_ui_radio_group_info statusInfo = { .selectedIndex = statusIndex,
                                                                  .optionCount = oc_array_size(statusOptions),
                                                                  .options = statusOptions };
                            oc_ui_radio_group_info result = oc_ui_radio_group("status", &statusInfo);
                            statusIndex = result.selectedIndex;
                            switch(statusIndex)
                            {
                                case 0:
                                    unselectedWhenStatus = OC_UI_NONE;
                                    break;
                                case 1:
                                    unselectedWhenStatus = OC_UI_HOVER;
                                    break;
                                case 2:
                                    unselectedWhenStatus = OC_UI_DRAGGING;
                                    break;
                                default:
                                    break;
                            }
                        }
                    }

                    oc_ui_style_next(&(oc_ui_style){ .layout.axis = OC_UI_AXIS_Y,
                                                     .layout.spacing = 16 },
                                     OC_UI_STYLE_LAYOUT_AXIS
                                         | OC_UI_STYLE_LAYOUT_SPACING);
                    oc_ui_container("selected", OC_UI_FLAG_NONE)
                    {
                        oc_ui_style_next(&(oc_ui_style){ .fontSize = 16 },
                                         OC_UI_STYLE_FONT_SIZE);
                        oc_ui_label("Radio selected style");

                        oc_ui_style_next(&(oc_ui_style){ .layout.spacing = 4 },
                                         OC_UI_STYLE_LAYOUT_SPACING);
                        oc_ui_container("size", OC_UI_FLAG_NONE)
                        {
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

                        oc_ui_style_next(&(oc_ui_style){ .layout.spacing = 4 },
                                         OC_UI_STYLE_LAYOUT_SPACING);
                        oc_ui_container("color", OC_UI_FLAG_NONE)
                        {
                            labeled_slider("Center R", &selectedR);
                            labeled_slider("Center G", &selectedG);
                            labeled_slider("Center B", &selectedB);
                            labeled_slider("Center A", &selectedA);
                        }

                        oc_ui_style_next(&(oc_ui_style){ .layout.spacing = 4 },
                                         OC_UI_STYLE_LAYOUT_SPACING);
                        oc_ui_container("background", OC_UI_FLAG_NONE)
                        {
                            labeled_slider("Background R", &selectedBgR);
                            labeled_slider("Background G", &selectedBgG);
                            labeled_slider("Background B", &selectedBgB);
                            labeled_slider("Background A", &selectedBgA);
                        }

                        oc_ui_style_next(&(oc_ui_style){ .layout.spacing = 10 },
                                         OC_UI_STYLE_LAYOUT_SPACING);
                        oc_ui_container("status_override", OC_UI_FLAG_NONE)
                        {
                            oc_ui_style_next(&(oc_ui_style){ .size.height = { OC_UI_SIZE_PIXELS, 30 } },
                                             OC_UI_STYLE_SIZE_HEIGHT);
                            oc_ui_box_make("spacer", OC_UI_FLAG_NONE);

                            oc_ui_label("Override");

                            static int statusIndex = 0;
                            oc_str8 statusOptions[] = { OC_STR8("Always"),
                                                        OC_STR8("When hovering"),
                                                        OC_STR8("When dragging") };
                            oc_ui_radio_group_info statusInfo = { .selectedIndex = statusIndex,
                                                                  .optionCount = oc_array_size(statusOptions),
                                                                  .options = statusOptions };
                            oc_ui_radio_group_info result = oc_ui_radio_group("status", &statusInfo);
                            statusIndex = result.selectedIndex;
                            switch(statusIndex)
                            {
                                case 0:
                                    selectedWhenStatus = OC_UI_NONE;
                                    break;
                                case 1:
                                    selectedWhenStatus = OC_UI_HOVER;
                                    break;
                                case 2:
                                    selectedWhenStatus = OC_UI_DRAGGING;
                                    break;
                                default:
                                    break;
                            }
                        }
                    }

                    oc_ui_style_next(&(oc_ui_style){ .layout.axis = OC_UI_AXIS_Y,
                                                     .layout.spacing = 16 },
                                     OC_UI_STYLE_LAYOUT_AXIS
                                         | OC_UI_STYLE_LAYOUT_SPACING);
                    oc_ui_container("label", OC_UI_FLAG_NONE)
                    {
                        oc_ui_style_next(&(oc_ui_style){ .fontSize = 16 },
                                         OC_UI_STYLE_FONT_SIZE);
                        oc_ui_label("Label style");

                        oc_ui_style_next(&(oc_ui_style){ .layout.axis = OC_UI_AXIS_X,
                                                         .layout.spacing = 8 },
                                         OC_UI_STYLE_LAYOUT_AXIS
                                             | OC_UI_STYLE_LAYOUT_SPACING);
                        oc_ui_container("font_color", OC_UI_FLAG_NONE)
                        {
                            oc_ui_style_match_after(oc_ui_pattern_owner(),
                                                    &(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, 100 } },
                                                    OC_UI_STYLE_SIZE_WIDTH);
                            oc_ui_label("Font color");

                            static int colorSelected = 0;
                            oc_str8 colorNames[] = { OC_STR8("Default"),
                                                     OC_STR8("Red"),
                                                     OC_STR8("Orange"),
                                                     OC_STR8("Amber"),
                                                     OC_STR8("Yellow"),
                                                     OC_STR8("Lime"),
                                                     OC_STR8("Light Green"),
                                                     OC_STR8("Green") };
                            oc_color colors[] = { OC_UI_DARK_THEME.text0,
                                                  OC_UI_DARK_THEME.palette->red5,
                                                  OC_UI_DARK_THEME.palette->orange5,
                                                  OC_UI_DARK_THEME.palette->amber5,
                                                  OC_UI_DARK_THEME.palette->yellow5,
                                                  OC_UI_DARK_THEME.palette->lime5,
                                                  OC_UI_DARK_THEME.palette->lightGreen5,
                                                  OC_UI_DARK_THEME.palette->green5 };
                            oc_ui_select_popup_info colorInfo = { .selectedIndex = colorSelected,
                                                                  .optionCount = oc_array_size(colorNames),
                                                                  .options = colorNames };
                            oc_ui_select_popup_info colorResult = oc_ui_select_popup("color", &colorInfo);
                            colorSelected = colorResult.selectedIndex;
                            labelFontColor = colors[colorSelected];
                        }

                        oc_ui_style_next(&(oc_ui_style){ .layout.axis = OC_UI_AXIS_X,
                                                         .layout.spacing = 8 },
                                         OC_UI_STYLE_LAYOUT_AXIS
                                             | OC_UI_STYLE_LAYOUT_SPACING);
                        oc_ui_container("font", OC_UI_FLAG_NONE)
                        {
                            oc_ui_style_match_after(oc_ui_pattern_owner(),
                                                    &(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, 100 } },
                                                    OC_UI_STYLE_SIZE_WIDTH);
                            oc_ui_label("Font");

                            static int fontSelected = 0;
                            oc_str8 fontNames[] = { OC_STR8("Regular"),
                                                    OC_STR8("Bold") };
                            oc_font* fonts[] = { &fontRegular,
                                                 &fontBold };
                            oc_ui_select_popup_info fontInfo = { .selectedIndex = fontSelected,
                                                                 .optionCount = oc_array_size(fontNames),
                                                                 .options = fontNames };
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
    }

    oc_canvas_select(canvas);
    oc_surface_select(surface);
    oc_ui_draw();
    oc_render(canvas);
    oc_surface_present(surface);

    oc_scratch_end(scratch);
}
