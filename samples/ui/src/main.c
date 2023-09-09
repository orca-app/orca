#include "orca.h"

oc_vec2 frameSize = { 100, 100 };

oc_surface surface;
oc_canvas canvas;
oc_font font;
oc_ui_context ui;
oc_arena textArena = { 0 };

ORCA_EXPORT void oc_on_init(void)
{
    oc_window_set_title(OC_STR8("ui"));

    surface = oc_surface_canvas();
    canvas = oc_canvas_create();
    oc_ui_init(&ui);

    //NOTE: load font
    {
        oc_file file = oc_file_open(OC_STR8("/OpenSansLatinSubset.ttf"), OC_FILE_ACCESS_READ, 0);
        if(oc_file_last_error(file) != OC_IO_OK)
        {
            oc_log_error("Couldn't open file OpenSansLatinSubset.ttf\n");
        }
        u64 size = oc_file_size(file);
        char* buffer = oc_arena_push(oc_scratch(), size);
        oc_file_read(file, size, buffer);
        oc_file_close(file);
        oc_unicode_range ranges[5] = { OC_UNICODE_BASIC_LATIN,
                                       OC_UNICODE_C1_CONTROLS_AND_LATIN_1_SUPPLEMENT,
                                       OC_UNICODE_LATIN_EXTENDED_A,
                                       OC_UNICODE_LATIN_EXTENDED_B,
                                       OC_UNICODE_SPECIALS };

        font = oc_font_create_from_memory(oc_str8_from_buffer(size, buffer), 5, ranges);
    }

    oc_arena_clear(oc_scratch());
    oc_arena_init(&textArena);
}

ORCA_EXPORT void oc_on_resize(u32 width, u32 height)
{
    oc_log_info("frame resize %u, %u", width, height);
    frameSize.x = width;
    frameSize.y = height;
}

ORCA_EXPORT void oc_on_raw_event(oc_event* event)
{
    oc_ui_process_event(event);
}

void widget_begin_view(char* str)
{
    oc_ui_style_next(&(oc_ui_style){ .layout.axis = OC_UI_AXIS_Y,
                                     .layout.spacing = 10,
                                     .layout.margin.x = 10,
                                     .layout.margin.y = 10,
                                     .layout.align.x = OC_UI_ALIGN_CENTER,
                                     .layout.align.y = OC_UI_ALIGN_START },
                     OC_UI_STYLE_LAYOUT);

    oc_ui_box_begin(str, OC_UI_FLAG_DRAW_BORDER);
    oc_ui_label(str);
}

void widget_end_view(void)
{
    oc_ui_box_end();
}

#define widget_view(s) oc_defer_loop(widget_begin_view(s), widget_end_view())

ORCA_EXPORT void oc_on_frame_refresh(void)
{
    oc_ui_style defaultStyle = { .font = font };

    oc_ui_style_mask defaultMask = OC_UI_STYLE_FONT;

    oc_ui_frame(frameSize, &defaultStyle, defaultMask)
    {
        oc_ui_style_match_before(oc_ui_pattern_all(), &defaultStyle, defaultMask);

        oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                         .size.height = { OC_UI_SIZE_PARENT, 1 },
                                         .layout.axis = OC_UI_AXIS_Y,
                                         .layout.align.x = OC_UI_ALIGN_CENTER,
                                         .layout.align.y = OC_UI_ALIGN_START,
                                         .layout.spacing = 10},
                         OC_UI_STYLE_SIZE
                             | OC_UI_STYLE_LAYOUT);

        oc_ui_container("background", OC_UI_FLAG_DRAW_BACKGROUND)
        {
            oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                             .size.height = { OC_UI_SIZE_CHILDREN },
                                             .layout.align.x = OC_UI_ALIGN_CENTER,
                                             .layout.margin.x = 10,
                                             .layout.margin.y = 10 },
                             OC_UI_STYLE_SIZE
                                 | OC_UI_STYLE_LAYOUT_ALIGN_X
                                 | OC_UI_STYLE_LAYOUT_MARGINS);
            oc_ui_container("title", 0)
            {
                oc_ui_style_next(&(oc_ui_style){ .fontSize = 26 }, OC_UI_STYLE_FONT_SIZE);
                oc_ui_label("Orca UI Demo");

                if(oc_ui_box_sig(oc_ui_box_top()).hovering)
                {
                    oc_ui_tooltip("That is a tooltip!");
                }
            }

            oc_ui_menu_bar("Menu bar")
            {
                oc_ui_menu("Menu 1")
                {
                    if(oc_ui_menu_button("Option 1.1").pressed)
                    {
                        oc_log_info("Pressed option 1.1\n");
                    }
                    oc_ui_menu_button("Option 1.2");
                    oc_ui_menu_button("Option 1.3");
                    oc_ui_menu_button("Option 1.4");
                }

                oc_ui_menu("Menu 2")
                {
                    oc_ui_menu_button("Option 2.1");
                    oc_ui_menu_button("Option 2.2");
                    oc_ui_menu_button("Option 2.3");
                    oc_ui_menu_button("Option 2.4");
                }

                oc_ui_menu("Menu 3")
                {
                    oc_ui_menu_button("Option 3.1");
                    oc_ui_menu_button("Option 3.2");
                    oc_ui_menu_button("Option 3.3");
                    oc_ui_menu_button("Option 3.4");
                }
            }

            oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                             .size.height = { OC_UI_SIZE_PARENT, 1, 1 },
                                             .layout.margin.x = 10,
                                             .layout.margin.y = 10 },
                             OC_UI_STYLE_SIZE | OC_UI_STYLE_LAYOUT_MARGINS);

            oc_ui_style_next(&(oc_ui_style){ .layout.axis = OC_UI_AXIS_X }, OC_UI_STYLE_LAYOUT_AXIS);
            oc_ui_container("contents", 0)
            {
                oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 0.5 },
                                                 .size.height = { OC_UI_SIZE_PARENT, 1 } },
                                 OC_UI_STYLE_SIZE);

                oc_ui_container("left", 0)
                {
                    oc_ui_style_next(&(oc_ui_style){ .layout.axis = OC_UI_AXIS_X,
                                                     .layout.spacing = 10,
                                                     .layout.margin.x = 10,
                                                     .layout.margin.y = 10,
                                                     .size.width = { OC_UI_SIZE_PARENT, 1 },
                                                     .size.height = { OC_UI_SIZE_PARENT, 0.5 } },
                                     OC_UI_STYLE_LAYOUT_AXIS
                                         | OC_UI_STYLE_LAYOUT_SPACING
                                         | OC_UI_STYLE_LAYOUT_MARGIN_X
                                         | OC_UI_STYLE_LAYOUT_MARGIN_Y
                                         | OC_UI_STYLE_SIZE);

                    oc_ui_container("up", 0)
                    {
                        oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 0.5 },
                                                         .size.height = { OC_UI_SIZE_PARENT, 1 } },
                                         OC_UI_STYLE_SIZE);
                        widget_view("Buttons")
                        {
                            if(oc_ui_button("Button A").clicked)
                            {
                                oc_log_info("A clicked");
                            }

                            if(oc_ui_button("Button B").clicked)
                            {
                                oc_log_info("B clicked");
                            }

                            if(oc_ui_button("Button C").clicked)
                            {
                                oc_log_info("C clicked");
                            }
                        }

                        oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 0.5 },
                                                         .size.height = { OC_UI_SIZE_PARENT, 1 } },
                                         OC_UI_STYLE_SIZE);

                        widget_view("checkboxes")
                        {
                            static bool check1 = true;
                            static bool check2 = false;
                            static bool check3 = false;

                            oc_ui_checkbox("check1", &check1);
                            oc_ui_checkbox("check2", &check2);
                            oc_ui_checkbox("check3", &check3);
                        }
                    }

                    oc_ui_style_next(&(oc_ui_style){ .layout.axis = OC_UI_AXIS_X,
                                                     .size.width = { OC_UI_SIZE_PARENT, 1 },
                                                     .size.height = { OC_UI_SIZE_PARENT, 0.5 } },
                                     OC_UI_STYLE_LAYOUT_AXIS
                                         | OC_UI_STYLE_SIZE);

                    oc_ui_container("down", 0)
                    {
                        widget_view("Vertical Sliders")
                        {
                            oc_ui_style_next(&(oc_ui_style){ .layout.axis = OC_UI_AXIS_X,
                                                             .layout.spacing = 10 },
                                             OC_UI_STYLE_LAYOUT_AXIS
                                                 | OC_UI_STYLE_LAYOUT_SPACING);
                            oc_ui_container("contents", 0)
                            {
                                oc_ui_style_next(&(oc_ui_style){ .size.height = { OC_UI_SIZE_PIXELS, 200 } },
                                                 OC_UI_STYLE_SIZE_HEIGHT);
                                static f32 slider1 = 0;
                                oc_ui_slider("slider1", &slider1);

                                oc_ui_style_next(&(oc_ui_style){ .size.height = { OC_UI_SIZE_PIXELS, 200 } },
                                                 OC_UI_STYLE_SIZE_HEIGHT);
                                static f32 slider2 = 0;
                                oc_ui_slider("slider2", &slider2);

                                oc_ui_style_next(&(oc_ui_style){ .size.height = { OC_UI_SIZE_PIXELS, 200 } },
                                                 OC_UI_STYLE_SIZE_HEIGHT);
                                static f32 slider3 = 0;
                                oc_ui_slider("slider3", &slider3);
                            }
                        }

                        widget_view("Horizontal Sliders")
                        {
                            oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, 200 } },
                                             OC_UI_STYLE_SIZE_WIDTH);
                            static f32 slider1 = 0;
                            oc_ui_slider("slider1", &slider1);

                            oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, 200 } },
                                             OC_UI_STYLE_SIZE_WIDTH);
                            static f32 slider2 = 0;
                            oc_ui_slider("slider2", &slider2);

                            oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, 200 } },
                                             OC_UI_STYLE_SIZE_WIDTH);
                            static f32 slider3 = 0;
                            oc_ui_slider("slider3", &slider3);
                        }
                    }
                }

                oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 0.5 },
                                                 .size.height = { OC_UI_SIZE_PARENT, 1 } },
                                 OC_UI_STYLE_SIZE);

                oc_ui_container("right", 0)
                {

                    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                                     .size.height = { OC_UI_SIZE_PARENT, 0.33 } },
                                     OC_UI_STYLE_SIZE);
                    widget_view("Text box")
                    {
                        oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, 300 },
                                                         .size.height = { OC_UI_SIZE_TEXT } },
                                         OC_UI_STYLE_SIZE);
                        static oc_str8 text = { 0 };
                        oc_ui_text_box_result res = oc_ui_text_box("textbox", oc_scratch(), text);
                        if(res.changed)
                        {
                            oc_arena_clear(&textArena);
                            text = oc_str8_push_copy(&textArena, res.text);
                        }
                    }

                    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                                     .size.height = { OC_UI_SIZE_PARENT, 0.33 } },
                                     OC_UI_STYLE_SIZE);
                    widget_view("Test")
                    {
                        static int selected = 0;
                        oc_str8 options[] = { OC_STR8("Option 1"),
                                              OC_STR8("Option 2"),
                                              OC_STR8("Long option 3"),
                                              OC_STR8("Option 4"),
                                              OC_STR8("Option 5") };
                        oc_ui_select_popup_info info = { .selectedIndex = selected,
                                                         .optionCount = 5,
                                                         .options = options };

                        oc_ui_select_popup_info result = oc_ui_select_popup("popup", &info);
                        selected = result.selectedIndex;
                    }

                    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                                     .size.height = { OC_UI_SIZE_PARENT, 0.33 } },
                                     OC_UI_STYLE_SIZE);
                    widget_view("Color")
                    {
                        oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                                         .size.height = { OC_UI_SIZE_PARENT, 0.7 },
                                                         .layout.axis = OC_UI_AXIS_X },
                                         OC_UI_STYLE_SIZE
                                             | OC_UI_STYLE_LAYOUT_AXIS);

                        oc_ui_panel("Panel", OC_UI_FLAG_DRAW_BORDER)
                        {
                            oc_ui_style_next(&(oc_ui_style){ .layout.axis = OC_UI_AXIS_X },
                                             OC_UI_STYLE_LAYOUT_AXIS);
                            oc_ui_container("contents", 0)
                            {
                                oc_ui_style_next(&(oc_ui_style){ .layout.spacing = 20 },
                                                 OC_UI_STYLE_LAYOUT_SPACING);
                                oc_ui_container("buttons", 0)
                                {
                                    oc_ui_button("Button A");
                                    oc_ui_button("Button B");
                                    oc_ui_button("Button C");
                                    oc_ui_button("Button D");
                                }

                                oc_ui_style_next(&(oc_ui_style){ .layout.axis = OC_UI_AXIS_X,
                                                                 .layout.spacing = 20 },
                                                 OC_UI_STYLE_LAYOUT_SPACING
                                                     | OC_UI_STYLE_LAYOUT_AXIS);

                                oc_ui_container("buttons2", 0)
                                {
                                    oc_ui_button("Button A");
                                    oc_ui_button("Button B");
                                    oc_ui_button("Button C");
                                    oc_ui_button("Button D");
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    oc_canvas_set_current(canvas);
    oc_surface_select(surface);
    oc_ui_draw();
    oc_render(surface, canvas);
    oc_surface_present(surface);

    oc_arena_clear(oc_scratch());
}
