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

void debug_print_indent(int indent)
{
    for(int i = 0; i < indent; i++)
    {
        oc_log_info("  ");
    }
}

void debug_print_rule(oc_ui_style_rule* rule)
{
    oc_list_for(&rule->pattern.l, selector, oc_ui_selector, listElt)
    {
        switch(selector->kind)
        {
            case OC_UI_SEL_ANY:
                oc_log_info("any: ");
                break;

            case OC_UI_SEL_OWNER:
                oc_log_info("owner: ");
                break;

            case OC_UI_SEL_TEXT:
                oc_log_info("text='%.*s': ", (int)selector->text.len, selector->text.ptr);
                break;

            case OC_UI_SEL_TAG:
                oc_log_info("tag=0x%llx: ", selector->tag.hash);
                break;

            case OC_UI_SEL_STATUS:
            {
                if(selector->status & OC_UI_HOVER)
                {
                    oc_log_info("hover: ");
                }
                if(selector->status & OC_UI_ACTIVE)
                {
                    oc_log_info("active: ");
                }
                if(selector->status & OC_UI_DRAGGING)
                {
                    oc_log_info("dragging: ");
                }
            }
            break;

            case OC_UI_SEL_KEY:
                oc_log_info("key=0x%llx: ", selector->key.hash);
                break;

            default:
                oc_log_info("unknown: ");
                break;
        }
    }
    oc_log_info("=> font size = %f\n", rule->style->fontSize);
}

void debug_print_size(oc_ui_box* box, oc_ui_axis axis, int indent)
{
    debug_print_indent(indent);
    oc_log_info("size %s: ", axis == OC_UI_AXIS_X ? "x" : "y");
    f32 value = box->targetStyle->size.c[axis].value;
    switch(box->targetStyle->size.c[axis].kind)
    {
        case OC_UI_SIZE_TEXT:
            oc_log_info("text\n");
            break;

        case OC_UI_SIZE_CHILDREN:
            oc_log_info("children\n");
            break;

        case OC_UI_SIZE_PARENT:
            oc_log_info("parent: %f\n", value);
            break;

        case OC_UI_SIZE_PARENT_MINUS_PIXELS:
            oc_log_info("parent minus pixels: %f\n", value);
            break;

        case OC_UI_SIZE_PIXELS:
            oc_log_info("pixels: %f\n", value);
            break;
    }
}

void debug_print_styles(oc_ui_box* box, int indent)
{
    debug_print_indent(indent);
    oc_log_info("### box '%.*s'\n", (int)box->string.len, box->string.ptr);
    indent++;

    debug_print_indent(indent);
    oc_log_info("font size: %f\n", box->targetStyle->fontSize);

    debug_print_size(box, OC_UI_AXIS_X, indent);
    debug_print_size(box, OC_UI_AXIS_Y, indent);

    if(!oc_list_empty(&box->beforeRules))
    {
        debug_print_indent(indent);
        oc_log_info("before rules:\n");
        oc_list_for(&box->beforeRules, rule, oc_ui_style_rule, boxElt)
        {
            debug_print_indent(indent + 1);
            debug_print_rule(rule);
        }
    }

    if(!oc_list_empty(&box->afterRules))
    {
        debug_print_indent(indent);
        oc_log_info("after rules:\n");
        oc_list_for(&box->afterRules, rule, oc_ui_style_rule, boxElt)
        {
            debug_print_indent(indent + 1);
            debug_print_rule(rule);
        }
    }

    if(!oc_list_empty(&box->children))
    {
        debug_print_indent(indent);
        oc_log_info("children:\n");
        indent++;
        oc_list_for(&box->children, child, oc_ui_box, listElt)
        {
            debug_print_styles(child, indent);
        }
    }
}

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

int main()
{
    oc_init();
    oc_clock_init(); //TODO put that in oc_init()?

    oc_ui_context context;
    oc_ui_init(&context);
    oc_ui_set_context(&context);

    oc_rect windowRect = { .x = 100, .y = 100, .w = 810, .h = 610 };
    oc_window window = oc_window_create(windowRect, OC_STR8("test"), 0);

    oc_rect contentRect = oc_window_get_content_rect(window);

    //NOTE: create surface
    oc_surface surface = oc_surface_create_for_window(window, OC_CANVAS);
    oc_surface_swap_interval(surface, 0);

    //TODO: create canvas
    oc_canvas canvas = oc_canvas_create();

    if(oc_canvas_is_nil(canvas))
    {
        oc_log_error("Error: couldn't create canvas\n");
        return (-1);
    }

    oc_font font = create_font();

    oc_arena textArena = { 0 };
    oc_arena_init(&textArena);

    // start app
    oc_window_bring_to_front(window);
    oc_window_focus(window);

    while(!oc_should_quit())
    {
        oc_arena_scope scratch = oc_scratch_begin();

        bool printDebugStyle = false;

        f64 startTime = oc_clock_time(OC_CLOCK_MONOTONIC);

        oc_pump_events(0);
        oc_event* event = 0;
        while((event = oc_next_event(scratch)) != 0)
        {
            oc_ui_process_event(event);

            switch(event->type)
            {
                case OC_EVENT_WINDOW_CLOSE:
                {
                    oc_request_quit();
                }
                break;

                case OC_EVENT_KEYBOARD_KEY:
                {
                    if(event->key.action == OC_KEY_PRESS && event->key.code == OC_KEY_P)
                    {
                        printDebugStyle = true;
                    }
                }
                break;

                default:
                    break;
            }
        }

        //TEST UI
        oc_ui_style defaultStyle = { .bgColor = { 0 },
                                     .color = { 1, 1, 1, 1 },
                                     .font = font,
                                     .fontSize = 16,
                                     .borderColor = { 1, 0, 0, 1 },
                                     .borderSize = 2 };

        oc_ui_style_mask defaultMask = OC_UI_STYLE_BG_COLOR
                                     | OC_UI_STYLE_COLOR
                                     | OC_UI_STYLE_BORDER_COLOR
                                     | OC_UI_STYLE_BORDER_SIZE
                                     | OC_UI_STYLE_FONT
                                     | OC_UI_STYLE_FONT_SIZE;

        oc_ui_flags debugFlags = OC_UI_FLAG_DRAW_BORDER;

        oc_ui_box* root = 0;

        oc_vec2 frameSize = oc_surface_get_size(surface);

        oc_ui_frame(frameSize, &defaultStyle, defaultMask)
        {
            root = oc_ui_box_top();
            oc_ui_style_match_before(oc_ui_pattern_all(), &defaultStyle, defaultMask);

            oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                             .size.height = { OC_UI_SIZE_PARENT, 1 },
                                             .layout.axis = OC_UI_AXIS_Y,
                                             .layout.align.x = OC_UI_ALIGN_CENTER,
                                             .layout.align.y = OC_UI_ALIGN_START,
                                             .layout.spacing = 10,
                                             .layout.margin.x = 10,
                                             .layout.margin.y = 10,
                                             .bgColor = { 0.11, 0.11, 0.11, 1 } },
                             OC_UI_STYLE_SIZE
                                 | OC_UI_STYLE_LAYOUT
                                 | OC_UI_STYLE_BG_COLOR);

            oc_ui_container("background", OC_UI_FLAG_DRAW_BACKGROUND)
            {
                oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                                 .size.height = { OC_UI_SIZE_CHILDREN },
                                                 .layout.align.x = OC_UI_ALIGN_CENTER },
                                 OC_UI_STYLE_SIZE
                                     | OC_UI_STYLE_LAYOUT_ALIGN_X);
                oc_ui_container("title", debugFlags)
                {
                    oc_ui_style_next(&(oc_ui_style){ .fontSize = 26 }, OC_UI_STYLE_FONT_SIZE);
                    oc_ui_label("Orca UI Demo");

                    if(oc_ui_box_sig(oc_ui_box_top()).hovering)
                    {
                        oc_ui_tooltip("tooltip")
                        {
                            oc_ui_style_next(&(oc_ui_style){ .bgColor = { 1, 0.99, 0.82, 1 } },
                                             OC_UI_STYLE_BG_COLOR);

                            oc_ui_container("background", OC_UI_FLAG_DRAW_BACKGROUND)
                            {
                                oc_ui_style_next(&(oc_ui_style){ .color = { 0, 0, 0, 1 } },
                                                 OC_UI_STYLE_COLOR);

                                oc_ui_label("That is a tooltip!");
                            }
                        }
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
                                                 .size.height = { OC_UI_SIZE_PARENT, 1, 1 } },
                                 OC_UI_STYLE_SIZE);

                oc_ui_style_next(&(oc_ui_style){ .layout.axis = OC_UI_AXIS_X }, OC_UI_STYLE_LAYOUT_AXIS);
                oc_ui_container("contents", debugFlags)
                {
                    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 0.5 },
                                                     .size.height = { OC_UI_SIZE_PARENT, 1 },
                                                     .borderColor = { 0, 0, 1, 1 } },
                                     OC_UI_STYLE_SIZE
                                         | OC_UI_STYLE_BORDER_COLOR);

                    oc_ui_container("left", debugFlags)
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

                        oc_ui_container("up", debugFlags)
                        {
                            oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 0.5 },
                                                             .size.height = { OC_UI_SIZE_PARENT, 1 } },
                                             OC_UI_STYLE_SIZE);
                            widget_view("Buttons")
                            {
                                if(oc_ui_button("Test Dialog").clicked)
                                {
                                    static oc_str8 options_strings[] = {
                                        OC_STR8_LIT("Accept"),
                                        OC_STR8_LIT("Reject"),
                                    };

                                    oc_str8_list options = { 0 };
                                    oc_str8_list_push(scratch, &options, options_strings[0]);
                                    oc_str8_list_push(scratch, &options, options_strings[1]);

                                    int res = oc_alert_popup(OC_STR8("test dialog"), OC_STR8("dialog message"), options);
                                    if(res >= 0)
                                    {
                                        oc_log_info("selected options %i: %s\n", res, options_strings[res].ptr);
                                    }
                                    else
                                    {
                                        oc_log_info("no options selected\n");
                                    }
                                }

                                if(oc_ui_button("Open").clicked)
                                {
                                    oc_str8_list filters = { 0 };
                                    oc_str8_list_push(scratch, &filters, OC_STR8("md"));

                                    oc_str8 file = oc_open_dialog(scratch, OC_STR8("Open File"), OC_STR8("C:\\Users"), filters, false);
                                    oc_log_info("selected file %.*s\n", (int)file.len, file.ptr);
                                }

                                if(oc_ui_button("Save").clicked)
                                {
                                    oc_str8_list filters = { 0 };

                                    oc_str8 file = oc_save_dialog(scratch, OC_STR8("Save File"), OC_STR8("C:\\Users"), filters);
                                    oc_log_info("selected file %.*s\n", (int)file.len, file.ptr);
                                }
                            }

                            oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 0.5 },
                                                             .size.height = { OC_UI_SIZE_PARENT, 1 } },
                                             OC_UI_STYLE_SIZE);

                            oc_ui_pattern pattern = { 0 };
                            oc_ui_pattern_push(scratch, &pattern, (oc_ui_selector){ .kind = OC_UI_SEL_TAG, .tag = oc_ui_tag_make("checkbox") });
                            oc_ui_style_match_after(pattern,
                                                    &(oc_ui_style){ .bgColor = { 0, 1, 0, 1 },
                                                                    .color = { 1, 1, 1, 1 } },
                                                    OC_UI_STYLE_COLOR | OC_UI_STYLE_BG_COLOR);

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

                        oc_ui_container("down", debugFlags)
                        {
                            widget_view("Vertical Sliders")
                            {
                                oc_ui_style_next(&(oc_ui_style){ .layout.axis = OC_UI_AXIS_X,
                                                                 .layout.spacing = 10 },
                                                 OC_UI_STYLE_LAYOUT_AXIS
                                                     | OC_UI_STYLE_LAYOUT_SPACING);
                                oc_ui_container("contents", 0)
                                {
                                    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, 20 },
                                                                     .size.height = { OC_UI_SIZE_PIXELS, 200 } },
                                                     OC_UI_STYLE_SIZE);
                                    static f32 slider1 = 0;
                                    oc_ui_slider("slider1", 0.2, &slider1);

                                    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, 20 },
                                                                     .size.height = { OC_UI_SIZE_PIXELS, 200 } },
                                                     OC_UI_STYLE_SIZE);
                                    static f32 slider2 = 0;
                                    oc_ui_slider("slider2", 0.2, &slider2);

                                    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, 20 },
                                                                     .size.height = { OC_UI_SIZE_PIXELS, 200 } },
                                                     OC_UI_STYLE_SIZE);
                                    static f32 slider3 = 0;
                                    oc_ui_slider("slider3", 0.2, &slider3);
                                }
                            }

                            widget_view("Horizontal Sliders")
                            {
                                oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, 200 },
                                                                 .size.height = { OC_UI_SIZE_PIXELS, 20 } },
                                                 OC_UI_STYLE_SIZE);
                                static f32 slider1 = 0;
                                oc_ui_slider("slider1", 0.2, &slider1);

                                oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, 200 },
                                                                 .size.height = { OC_UI_SIZE_PIXELS, 20 } },
                                                 OC_UI_STYLE_SIZE);
                                static f32 slider2 = 0;
                                oc_ui_slider("slider2", 0.2, &slider2);

                                oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, 200 },
                                                                 .size.height = { OC_UI_SIZE_PIXELS, 20 } },
                                                 OC_UI_STYLE_SIZE);
                                static f32 slider3 = 0;
                                oc_ui_slider("slider3", 0.2, &slider3);
                            }
                        }
                    }

                    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 0.5 },
                                                     .size.height = { OC_UI_SIZE_PARENT, 1 } },
                                     OC_UI_STYLE_SIZE);

                    oc_ui_container("right", debugFlags)
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
                            oc_ui_text_box_result res = oc_ui_text_box("textbox", scratch.arena, text);
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
                            oc_ui_pattern pattern = { 0 };
                            oc_ui_pattern_push(scratch.arena, &pattern, (oc_ui_selector){ .kind = OC_UI_SEL_TEXT, .text = OC_STR8("panel") });
                            oc_ui_style_match_after(pattern, &(oc_ui_style){ .bgColor = { 0.3, 0.3, 1, 1 } }, OC_UI_STYLE_BG_COLOR);

                            static int selected = 0;
                            oc_str8 options[] = { OC_STR8("option 1"),
                                                  OC_STR8("option 2"),
                                                  OC_STR8("long option 3"),
                                                  OC_STR8("option 4"),
                                                  OC_STR8("option 5") };
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
        if(printDebugStyle)
        {
            debug_print_styles(root, 0);
        }

        oc_surface_select(surface);

        oc_ui_draw();

        oc_render(canvas);
        oc_surface_present(surface);

        oc_scratch_end(scratch);
    }

    oc_surface_destroy(surface);
    oc_terminate();

    return (0);
}
