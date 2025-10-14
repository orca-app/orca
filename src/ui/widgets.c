/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "widgets.h"

//-----------------------------------------------------------------------------
// label
//-----------------------------------------------------------------------------

oc_ui_sig oc_ui_label_str8(oc_str8 key, oc_str8 label)
{
    oc_ui_box* box = oc_ui_box_str8(key)
    {
        oc_ui_tag("label");
        oc_ui_set_text(label);

        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_TEXT, 0, 0 });
        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_TEXT, 0, 0 });
        oc_ui_style_set_var_str8(OC_UI_COLOR, OC_UI_THEME_TEXT_0);
        oc_ui_style_set_var_str8(OC_UI_FONT, OC_UI_THEME_FONT_REGULAR);
        oc_ui_style_set_var_str8(OC_UI_TEXT_SIZE, OC_UI_THEME_TEXT_SIZE_REGULAR);
        oc_ui_style_set_i32(OC_UI_CLICK_THROUGH, 1);
    }

    oc_ui_sig sig = oc_ui_box_get_sig(box);
    return (sig);
}

oc_ui_sig oc_ui_label(const char* key, const char* label)
{
    return (oc_ui_label_str8(OC_STR8((char*)key), OC_STR8((char*)label)));
}

//------------------------------------------------------------------------------
// button
//------------------------------------------------------------------------------

oc_ui_sig oc_ui_button_str8(oc_str8 key, oc_str8 text)
{
    oc_ui_box* box = oc_ui_box_str8(key)
    {
        oc_ui_box_set_text(box, text);
        oc_ui_tag("button");

        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_TEXT });
        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_TEXT });
        oc_ui_style_set_i32(OC_UI_ALIGN_X, OC_UI_ALIGN_CENTER);
        oc_ui_style_set_i32(OC_UI_ALIGN_Y, OC_UI_ALIGN_CENTER);
        oc_ui_style_set_var_str8(OC_UI_MARGIN_X, OC_UI_THEME_SPACING_TIGHT);
        oc_ui_style_set_var_str8(OC_UI_MARGIN_Y, OC_UI_THEME_SPACING_EXTRA_TIGHT);

        oc_ui_style_set_var_str8(OC_UI_COLOR, OC_UI_THEME_PRIMARY);
        oc_ui_style_set_var_str8(OC_UI_FONT, OC_UI_THEME_FONT_REGULAR);
        oc_ui_style_set_var_str8(OC_UI_TEXT_SIZE, OC_UI_THEME_TEXT_SIZE_REGULAR);

        oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_FILL_0);
        oc_ui_style_set_var_str8(OC_UI_ROUNDNESS, OC_UI_THEME_ROUNDNESS_SMALL);

        oc_ui_style_rule(".hover")
        {
            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_FILL_1);
        }
        oc_ui_style_rule(".hover.active")
        {
            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_FILL_2);
        }
    }

    oc_ui_sig sig = oc_ui_box_get_sig(box);
    return (sig);
}

oc_ui_sig oc_ui_button(const char* key, const char* text)
{
    return (oc_ui_button_str8(OC_STR8((char*)key), OC_STR8((char*)text)));
}

//-----------------------------------------------------------
// checkbox
//-----------------------------------------------------------
void oc_ui_checkbox_draw(oc_ui_box* box, void* data)
{
    oc_rect rect = box->rect;

    oc_mat2x3 matrix = {
        rect.w, 0, rect.x,
        0, rect.h, rect.y
    };
    oc_matrix_multiply_push(matrix);

    oc_move_to(0.7255, 0.3045);
    oc_cubic_to(0.7529, 0.3255, 0.7581, 0.3647, 0.7371, 0.3921);
    oc_line_to(0.4663, 0.7463);
    oc_cubic_to(0.4545, 0.7617, 0.4363, 0.7708, 0.4169, 0.7708);
    oc_cubic_to(0.3975, 0.7709, 0.3792, 0.762, 0.3673, 0.7467);
    oc_line_to(0.2215, 0.5592);
    oc_cubic_to(0.2003, 0.532, 0.2052, 0.4927, 0.2325, 0.4715);
    oc_cubic_to(0.2597, 0.4503, 0.299, 0.4552, 0.3202, 0.4825);
    oc_line_to(0.4162, 0.606);
    oc_line_to(0.6379, 0.3162);
    oc_cubic_to(0.6588, 0.2888, 0.698, 0.2836, 0.7255, 0.3045);

    oc_set_color(box->style.color);
    oc_fill();

    oc_matrix_pop();
}

oc_ui_sig oc_ui_checkbox_str8(oc_str8 name, bool* checked)
{
    oc_ui_box* box = oc_ui_box_str8(name)
    {
        oc_ui_tag("checkbox");

        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 16 });
        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, 16 });
        oc_ui_style_set_var_str8(OC_UI_ROUNDNESS, OC_UI_THEME_ROUNDNESS_SMALL);
        oc_ui_style_set_f32(OC_UI_BORDER_SIZE, 1.);

        if(*checked)
        {
            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_PRIMARY);
            oc_ui_style_set_color(OC_UI_COLOR, (oc_color){ 1, 1, 1, 1 });

            oc_ui_style_rule(".hover")
            {
                oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_PRIMARY_HOVER);
            }
            oc_ui_style_rule(".hover.active")
            {
                oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_PRIMARY_ACTIVE);
            }
        }
        else
        {
            oc_ui_style_set_var_str8(OC_UI_BORDER_COLOR, OC_UI_THEME_TEXT_3);

            oc_ui_style_rule(".hover")
            {
                oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_FILL_0);
                oc_ui_style_set_var_str8(OC_UI_BORDER_COLOR, OC_UI_THEME_PRIMARY);
            }
            oc_ui_style_rule(".hover.active")
            {
                oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_FILL_1);
                oc_ui_style_set_var_str8(OC_UI_BORDER_COLOR, OC_UI_THEME_PRIMARY);
            }
        }

        oc_ui_set_draw_proc(oc_ui_checkbox_draw, 0);
    }

    oc_ui_sig sig = oc_ui_box_get_sig(box);
    if(sig.clicked)
    {
        *checked = !*checked;
    }

    return (sig);
}

oc_ui_sig oc_ui_checkbox(const char* name, bool* checked)
{
    return oc_ui_checkbox_str8(OC_STR8(name), checked);
}

//------------------------------------------------------------------------------
// slider
//------------------------------------------------------------------------------
oc_ui_box* oc_ui_slider_str8(oc_str8 name, f32* value)
{
    oc_ui_box* frame = oc_ui_box_str8(name)
    {
        oc_ui_tag("slider");

        //NOTE: default size:
        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 100 });
        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, 24 });
        oc_ui_style_set_f32(OC_UI_MARGIN_X, 1);
        oc_ui_style_set_f32(OC_UI_MARGIN_Y, 1);

        oc_rect frameRect = frame->rect;
        oc_ui_axis trackAxis = (frameRect.w > frameRect.h) ? OC_UI_AXIS_X : OC_UI_AXIS_Y;
        oc_ui_axis secondAxis = (trackAxis == OC_UI_AXIS_Y) ? OC_UI_AXIS_X : OC_UI_AXIS_Y;

        oc_ui_style_set_i32(OC_UI_ALIGN_X + secondAxis, OC_UI_ALIGN_CENTER);

        f32 trackThickness = 4;
        f32 thumbSize = oc_min(frame->rect.w, frame->rect.h) - 2;

        f32 beforeRatio, afterRatio, thumbRatio, trackFillRatio;
        if(trackAxis == OC_UI_AXIS_X)
        {
            thumbRatio = oc_min((thumbSize + 2) / (frameRect.w - 2), 1.);
            beforeRatio = (*value) * (1. - thumbRatio);
            afterRatio = (1. - *value) * (1. - thumbRatio);
            trackFillRatio = beforeRatio + thumbRatio / 2;
        }
        else
        {
            thumbRatio = oc_min((thumbSize + 2) / (frameRect.h - 2), 1.);
            beforeRatio = (1. - *value) * (1. - thumbRatio);
            afterRatio = (*value) * (1. - thumbRatio);
            trackFillRatio = thumbRatio / 2 + afterRatio;
        }

        oc_ui_style_set_i32(OC_UI_AXIS, trackAxis);

        oc_ui_box* track = oc_ui_box("track")
        {
            oc_ui_style_set_i32(OC_UI_POSITION, OC_UI_POSITION_PARENT);
            oc_ui_style_set_i32(OC_UI_FOOTPRINT, OC_UI_FOOTPRINT_UNSIZED);
            oc_ui_style_set_f32(OC_UI_OFFSET_X + trackAxis, 2);

            f32 frameThickness = frameRect.c[2 + secondAxis];
            if(!frameThickness)
            {
                frameThickness = thumbSize;
            }
            oc_ui_style_set_f32(OC_UI_OFFSET_X + secondAxis, 0.5 * (frameThickness - trackThickness));

            oc_ui_style_set_size(OC_UI_WIDTH + trackAxis, (oc_ui_size){ OC_UI_SIZE_PARENT_MINUS_PIXELS, 2 });
            oc_ui_style_set_size(OC_UI_WIDTH + secondAxis, (oc_ui_size){ OC_UI_SIZE_PIXELS, trackThickness });

            oc_ui_style_set_f32(OC_UI_ROUNDNESS, trackThickness / 2);

            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_FILL_0);

            oc_ui_style_set_i32(OC_UI_AXIS, trackAxis);
            if(trackAxis == OC_UI_AXIS_Y)
            {
                oc_ui_style_set_i32(OC_UI_ALIGN_Y, OC_UI_ALIGN_END);
            }

            oc_ui_box("fill")
            {
                oc_ui_style_set_size(OC_UI_WIDTH + trackAxis, (oc_ui_size){ OC_UI_SIZE_PARENT, trackFillRatio });
                oc_ui_style_set_size(OC_UI_WIDTH + secondAxis, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });

                oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_PRIMARY);
                oc_ui_style_set_f32(OC_UI_ROUNDNESS, trackThickness / 2);
            }
        }

        oc_ui_box("before-spacer")
        {
            oc_ui_style_set_size(OC_UI_WIDTH + trackAxis, (oc_ui_size){ OC_UI_SIZE_PARENT, beforeRatio });
        }

        oc_ui_box* thumb = oc_ui_box("thumb")
        {
            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, thumbSize });
            oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, thumbSize });
            oc_ui_style_set_f32(OC_UI_ROUNDNESS, thumbSize / 2);
            oc_ui_style_set_color(OC_UI_BG_COLOR, (oc_color){ 1, 1, 1, 1 });
            oc_ui_style_set_var_str8(OC_UI_BORDER_COLOR, OC_UI_THEME_BORDER);
            oc_ui_style_set_f32(OC_UI_BORDER_SIZE, 1);

            oc_ui_style_rule(".active")
            {
                oc_ui_style_set_var_str8(OC_UI_BORDER_COLOR, OC_UI_THEME_PRIMARY);
            }
        }

        oc_ui_box("after-spacer")
        {
            oc_ui_style_set_size(OC_UI_WIDTH + trackAxis, (oc_ui_size){ OC_UI_SIZE_PARENT, afterRatio });
        }

        //NOTE: interaction
        oc_ui_sig thumbSig = oc_ui_box_get_sig(thumb);
        oc_ui_sig trackSig = oc_ui_box_get_sig(track);

        if(thumbSig.active)
        {
            oc_rect trackRect = track->rect;
            oc_rect thumbRect = thumb->rect;

            f32 trackExtents = trackRect.c[2 + trackAxis] - thumbRect.c[2 + trackAxis];
            *value = (trackSig.mouse.c[trackAxis] - thumbSig.lastPressedMouse.c[trackAxis]) / trackExtents;
            *value = oc_clamp(*value, 0, 1);
            if(trackAxis == OC_UI_AXIS_Y)
            {
                *value = 1 - *value;
            }
        }
    }
    return (frame);
}

oc_ui_box* oc_ui_slider(const char* name, f32* value)
{
    return oc_ui_slider_str8(OC_STR8(name), value);
}

//------------------------------------------------------------------------------
// Radio group
//------------------------------------------------------------------------------

void oc_ui_radio_indicator_draw(oc_ui_box* box, void* data)
{
    oc_rect rect = box->rect;
    oc_mat2x3 matrix = {
        rect.w, 0, rect.x,
        0, rect.h, rect.y
    };
    oc_matrix_multiply_push(matrix);

    oc_set_color(box->style.color);
    oc_circle_fill(0.5, 0.5, 35.0 / 192);

    oc_matrix_pop();
}

oc_ui_radio_group_info oc_ui_radio_group_str8(oc_str8 name, oc_ui_radio_group_info* info)
{
    oc_ui_radio_group_info result = *info;

    oc_ui_box_str8(name)
    {
        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });

        oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
        oc_ui_style_set_var_str8(OC_UI_SPACING, OC_UI_THEME_SPACING_REGULAR_TIGHT);

        for(int i = 0; i < info->optionCount; i++)
        {
            oc_ui_box* row = oc_ui_box_str8(info->options[i])
            {
                oc_ui_tag("radio-row");

                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });

                oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_X);
                oc_ui_style_set_var_str8(OC_UI_SPACING, OC_UI_THEME_SPACING_TIGHT);
                oc_ui_style_set_i32(OC_UI_ALIGN_Y, OC_UI_ALIGN_CENTER);

                oc_ui_box* radio = oc_ui_box("radio")
                {
                    oc_ui_set_draw_proc(oc_ui_radio_indicator_draw, 0);

                    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 16 });
                    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, 16 });
                    oc_ui_style_set_f32(OC_UI_ROUNDNESS, 8);
                    oc_ui_style_set_f32(OC_UI_BORDER_SIZE, 1);

                    oc_ui_style_set_i32(OC_UI_CLICK_THROUGH, 1);

                    oc_ui_sig sig = oc_ui_box_get_sig(row);
                    if(sig.clicked)
                    {
                        result.selectedIndex = i;
                    }

                    if(result.selectedIndex == i)
                    {
                        oc_ui_style_set_color(OC_UI_COLOR, (oc_color){ 1, 1, 1, 1 });
                        oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_PRIMARY);

                        if(sig.hover)
                        {
                            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_PRIMARY_HOVER);
                        }
                        if(sig.hover && sig.active)
                        {
                            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_PRIMARY_ACTIVE);
                        }
                    }
                    else
                    {
                        oc_ui_style_set_var_str8(OC_UI_BORDER_COLOR, OC_UI_THEME_TEXT_3);

                        if(sig.hover)
                        {
                            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_FILL_0);
                            oc_ui_style_set_var_str8(OC_UI_BORDER_COLOR, OC_UI_THEME_PRIMARY);
                        }
                        if(sig.hover && sig.active)
                        {
                            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_FILL_1);
                            oc_ui_style_set_var_str8(OC_UI_BORDER_COLOR, OC_UI_THEME_PRIMARY);
                        }
                    }

                    //TODO: change to "radio" and "selected"?
                    const char* defaultTagStr = "radio";
                    const char* selectedTagStr = "radio_selected";
                    const char* radioTagStr = result.selectedIndex == i ? selectedTagStr : defaultTagStr;
                    oc_ui_tag(radioTagStr);
                }

                oc_ui_label_str8(OC_STR8("label"), info->options[i]);
            }
        }
    }
    result.changed = result.selectedIndex != info->selectedIndex;
    return (result);
}

oc_ui_radio_group_info oc_ui_radio_group(const char* name, oc_ui_radio_group_info* info)
{
    return oc_ui_radio_group_str8(OC_STR8(name), info);
}

//------------------------------------------------------------------------------
// tooltips
//------------------------------------------------------------------------------

void oc_ui_tooltip_arrow_draw(oc_ui_box* box, void* data)
{
    oc_rect rect = box->rect;
    oc_mat2x3 matrix = {
        -rect.w, 0, rect.x + rect.w + 2,
        0, rect.h, rect.y + 5
    };
    oc_matrix_multiply_push(matrix);

    oc_move_to(0, 0);
    oc_line_to(0.0417, 0);
    oc_cubic_to(0.0417, 0.1667, 0.0833, 0.2292, 0.1667, 0.3125);
    oc_quadratic_to(0.2917, 0.4167, 0.2917, 0.5);
    oc_quadratic_to(0.25, 0.6042, 0.1667, 0.6875);
    oc_quadratic_to(0.0417, 0.8333, 0.0417, 1);
    oc_line_to(0, 1);
    oc_line_to(0, 0);

    oc_set_color(box->style.bgColor);
    oc_fill();

    oc_matrix_pop();
}

void oc_ui_tooltip_str8(oc_str8 key, oc_str8 text)
{
    oc_vec2 p = oc_mouse_position(oc_ui_input());

    oc_ui_box_str8(key)
    {
        oc_ui_set_overlay(true);

        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
        oc_ui_style_set_i32(OC_UI_POSITION, OC_UI_POSITION_FLOW);
        oc_ui_style_set_i32(OC_UI_FOOTPRINT, OC_UI_FOOTPRINT_UNSIZED);
        oc_ui_style_set_f32(OC_UI_OFFSET_X, p.x);
        oc_ui_style_set_f32(OC_UI_OFFSET_Y, p.y - 10); //TODO: quick fix for aliging single line tooltips arrow to mouse, fix that!

        oc_ui_box* arrow = oc_ui_box("arrow")
        {
            oc_ui_set_draw_proc(oc_ui_tooltip_arrow_draw, 0);

            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 24 });
            oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, 24 });
            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_TOOLTIP);
            oc_ui_style_set_i32(OC_UI_DRAW_MASK, OC_UI_DRAW_MASK_BACKGROUND);
        }

        oc_ui_box("contents")
        {
            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
            oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
            oc_ui_style_set_f32(OC_UI_MARGIN_X, 12);
            oc_ui_style_set_f32(OC_UI_MARGIN_Y, 8);
            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_TOOLTIP);
            oc_ui_style_set_var_str8(OC_UI_ROUNDNESS, OC_UI_THEME_ROUNDNESS_REGULAR);

            oc_ui_label_str8(OC_STR8("label"), text);
        }
    }
}

void oc_ui_tooltip(const char* key, const char* text)
{
    oc_ui_tooltip_str8(OC_STR8(key), OC_STR8(text));
}

//------------------------------------------------------------------------------
// Menus
//------------------------------------------------------------------------------

typedef struct oc_ui_menu_info
{
    bool active;

} oc_ui_menu_info;

void oc_ui_menu_bar_begin_str8(oc_str8 key)
{
    oc_ui_box* container = oc_ui_box_begin_str8(key);
    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN, 1, 0 });

    //NOTE: menu bar must allow overflow because it "contains" the panels
    /////////////////////////////////////////////////////////////////////////
    //TODO: overlay roots should probably not be clipped by their parents
    /////////////////////////////////////////////////////////////////////////
    oc_ui_style_set_i32(OC_UI_OVERFLOW_X, OC_UI_OVERFLOW_ALLOW);
    oc_ui_style_set_i32(OC_UI_OVERFLOW_Y, OC_UI_OVERFLOW_ALLOW);

    oc_ui_box* bar = oc_ui_box_begin("menus");
    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_CHILDREN, 1 });
    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN, 1, 0 });
    oc_ui_style_set_i32(OC_UI_OVERFLOW_X, OC_UI_OVERFLOW_ALLOW);
    oc_ui_style_set_i32(OC_UI_OVERFLOW_Y, OC_UI_OVERFLOW_ALLOW);

    oc_ui_menu_info* oldMenuInfo = (oc_ui_menu_info*)oc_ui_box_user_data_get(bar);
    oc_ui_menu_info* menuInfo = (oc_ui_menu_info*)oc_ui_box_user_data_push(bar, sizeof(oc_ui_menu_info));
    if(!oldMenuInfo)
    {
        memset(menuInfo, 0, sizeof(oc_ui_menu_info));
    }
    else
    {
        memcpy(menuInfo, oldMenuInfo, sizeof(oc_ui_menu_info));
    }

    oc_ui_sig sig = oc_ui_box_get_sig(bar);
    if(!sig.hover && oc_mouse_released(oc_ui_input(), OC_MOUSE_LEFT))
    {
        menuInfo->active = false;
    }
}

void oc_ui_menu_bar_begin(const char* key)
{
    oc_ui_menu_bar_begin_str8(OC_STR8(key));
}

void oc_ui_menu_bar_end(void)
{
    oc_ui_box_end(); // bar
    oc_ui_box_end(); // container
}

void oc_ui_menu_begin_str8(oc_str8 key, oc_str8 text)
{
    oc_ui_box* container = oc_ui_box_begin_str8(key);

    //NOTE: container is just so we scope button/panel under a unique key,
    //      so we make it click through and don't clip its content
    oc_ui_style_set_i32(OC_UI_OVERFLOW_X, OC_UI_OVERFLOW_ALLOW);
    oc_ui_style_set_i32(OC_UI_OVERFLOW_Y, OC_UI_OVERFLOW_ALLOW);
    oc_ui_style_set_i32(OC_UI_CLICK_THROUGH, 1);

    oc_ui_box* button = oc_ui_box("button")
    {
        oc_ui_style_set_f32(OC_UI_MARGIN_X, 8);
        oc_ui_style_set_f32(OC_UI_MARGIN_X, 4);

        oc_ui_style_set_i32(OC_UI_CLICK_THROUGH, 1);

        oc_ui_style_rule("button.hover")
        {
            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_FILL_0);
        }
        oc_ui_style_rule("button.active")
        {
            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_FILL_2);
        }

        //TODO: could put text in button here?
        oc_ui_box("label")
        {
            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_TEXT });
            oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_TEXT });

            oc_ui_style_set_var_str8(OC_UI_COLOR, OC_UI_THEME_TEXT_0);
            oc_ui_style_set_var_str8(OC_UI_TEXT_SIZE, OC_UI_THEME_TEXT_SIZE_REGULAR);
            oc_ui_style_set_var_str8(OC_UI_FONT, OC_UI_THEME_FONT_REGULAR);

            oc_ui_style_set_i32(OC_UI_CLICK_THROUGH, 1);

            oc_ui_set_text(text);
        }
    }

    oc_ui_box* bar = container->parent;
    oc_ui_menu_info* menuInfo = (oc_ui_menu_info*)oc_ui_box_user_data_get(bar);
    OC_DEBUG_ASSERT(menuInfo);

    oc_ui_box* menu = oc_ui_box_begin("panel");

    oc_ui_set_overlay(true);

    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
    oc_ui_style_set_i32(OC_UI_POSITION, OC_UI_POSITION_PARENT);
    oc_ui_style_set_i32(OC_UI_FOOTPRINT, OC_UI_FOOTPRINT_UNSIZED);

    oc_rect buttonRect = button->rect;
    oc_ui_style_set_f32(OC_UI_OFFSET_X, buttonRect.x);
    oc_ui_style_set_f32(OC_UI_OFFSET_Y, buttonRect.y + buttonRect.h);

    oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
    oc_ui_style_set_f32(OC_UI_MARGIN_Y, 4);
    oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_BG_1);
    oc_ui_style_set_var_str8(OC_UI_BORDER_COLOR, OC_UI_THEME_BORDER);
    oc_ui_style_set_f32(OC_UI_BORDER_SIZE, 1);

    oc_ui_sig sig = oc_ui_box_get_sig(button);
    oc_ui_sig barSig = oc_ui_box_get_sig(bar);

    if(menuInfo->active)
    {
        if(sig.hover)
        {
            oc_ui_box_set_closed(menu, false);
        }
        else if(barSig.hover)
        {
            oc_ui_box_set_closed(menu, true);
        }

        if(sig.clicked)
        {
            menuInfo->active = false;
            oc_ui_box_set_closed(menu, true);
        }
    }
    else
    {
        oc_ui_box_set_closed(menu, true);

        if(sig.clicked)
        {
            menuInfo->active = true;
            oc_ui_box_set_closed(menu, false);
        }
    }
}

void oc_ui_menu_begin(const char* key, const char* text)
{
    oc_ui_menu_begin_str8(OC_STR8(key), OC_STR8(text));
}

void oc_ui_menu_end(void)
{
    oc_ui_box_end(); // menu
    oc_ui_box_end(); // container
}

oc_ui_sig oc_ui_menu_button_str8(oc_str8 key, oc_str8 text)
{
    oc_ui_box* box = oc_ui_box_str8(key)
    {
        oc_ui_set_text(text);

        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_TEXT });
        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_TEXT });
        oc_ui_style_set_f32(OC_UI_MARGIN_X, 8);
        oc_ui_style_set_f32(OC_UI_MARGIN_Y, 4);

        oc_ui_style_set_var_str8(OC_UI_COLOR, OC_UI_THEME_TEXT_0);
        oc_ui_style_set_var_str8(OC_UI_TEXT_SIZE, OC_UI_THEME_TEXT_SIZE_REGULAR);
        oc_ui_style_set_var_str8(OC_UI_FONT, OC_UI_THEME_FONT_REGULAR);

        oc_ui_style_rule(".hover")
        {
            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_FILL_0);
        }
        oc_ui_style_rule(".active")
        {
            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_FILL_2);
        }
    }

    oc_ui_sig sig = oc_ui_box_get_sig(box);
    return (sig);
}

oc_ui_sig oc_ui_menu_button(const char* key, const char* text)
{
    return oc_ui_menu_button_str8(OC_STR8(key), OC_STR8(text));
}

//------------------------------------------------------------------------------
// Select
//------------------------------------------------------------------------------

void oc_ui_select_popup_draw_arrow(oc_ui_box* box, void* data)
{
    oc_rect rect = box->rect;
    oc_mat2x3 matrix = {
        rect.w / 2, 0, rect.x + rect.w / 4,
        0, rect.h / 2, rect.y + rect.h / 4
    };
    oc_matrix_multiply_push(matrix);

    oc_move_to(0.17, 0.3166);
    oc_cubic_to(0.1944, 0.2922, 0.234, 0.2922, 0.2584, 0.3166);
    oc_line_to(0.4941, 0.5523);
    oc_line_to(0.7298, 0.3166);
    oc_cubic_to(0.7542, 0.2922, 0.7938, 0.2922, 0.8182, 0.3166);
    oc_cubic_to(0.8426, 0.341, 0.8426, 0.3806, 0.8182, 0.405);
    oc_line_to(0.5383, 0.6849);
    oc_cubic_to(0.5139, 0.7093, 0.4743, 0.7093, 0.4499, 0.6849);
    oc_line_to(0.17, 0.405);
    oc_cubic_to(0.1456, 0.3806, 0.1456, 0.341, 0.17, 0.3166);
    oc_close_path();

    oc_set_color(box->style.color);
    oc_fill();

    oc_matrix_pop();
}

void oc_ui_select_popup_draw_checkmark(oc_ui_box* box, void* data)
{
    oc_rect rect = box->rect;
    oc_mat2x3 matrix = {
        rect.w, 0, rect.x,
        0, rect.h, rect.y
    };
    oc_matrix_multiply_push(matrix);

    oc_move_to(0.8897, 0.1777);
    oc_cubic_to(0.9181, 0.1973, 0.9252, 0.2362, 0.9056, 0.2647);
    oc_line_to(0.489, 0.8688);
    oc_cubic_to(0.4782, 0.8844, 0.4609, 0.8943, 0.442, 0.8957);
    oc_cubic_to(0.4231, 0.897, 0.4046, 0.8898, 0.3917, 0.8759);
    oc_line_to(0.1209, 0.5842);
    oc_cubic_to(0.0974, 0.5589, 0.0988, 0.5194, 0.1241, 0.4959);
    oc_cubic_to(0.1494, 0.4724, 0.189, 0.4738, 0.2125, 0.4991);
    oc_line_to(0.4303, 0.7337);
    oc_line_to(0.8027, 0.1937);
    oc_cubic_to(0.8223, 0.1653, 0.8612, 0.1581, 0.8897, 0.1777);
    oc_close_path();

    oc_set_color(box->style.color);
    oc_fill();

    oc_matrix_pop();
}

oc_ui_select_popup_info oc_ui_select_popup_str8(oc_str8 key, oc_ui_select_popup_info* info)
{
    oc_ui_select_popup_info result = *info;

    oc_ui_box* container = oc_ui_box_str8(key)
    {
        oc_ui_style_set_i32(OC_UI_OVERFLOW_X, OC_UI_OVERFLOW_ALLOW);
        oc_ui_style_set_i32(OC_UI_OVERFLOW_Y, OC_UI_OVERFLOW_ALLOW);
        oc_ui_style_set_i32(OC_UI_CLICK_THROUGH, 1);

        //TODO: we need to set that in order to compute maxOptionWidth, but
        //      at this point we can't ensure font/size will be the same for container, button and option
        oc_ui_style_set_var_str8(OC_UI_TEXT_SIZE, OC_UI_THEME_TEXT_SIZE_REGULAR);
        oc_ui_style_set_var_str8(OC_UI_FONT, OC_UI_THEME_FONT_REGULAR);

        f32 maxOptionWidth = 0;
        f32 lineHeight = 0;
        oc_rect bbox = { 0 };
        for(int i = 0; i < info->optionCount; i++)
        {
            bbox = oc_font_text_metrics(container->style.font, container->style.fontSize, info->options[i]).logical;
            maxOptionWidth = oc_max(maxOptionWidth, bbox.w);
        }

        oc_ui_box* button = oc_ui_box("button")
        {
            f32 buttonWidth = maxOptionWidth + 2 * button->style.layout.margin.x + button->rect.h;

            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, buttonWidth });
            oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
            oc_ui_style_set_f32(OC_UI_MARGIN_X, 12);
            oc_ui_style_set_f32(OC_UI_MARGIN_Y, 6);
            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_FILL_0);
            oc_ui_style_set_var_str8(OC_UI_ROUNDNESS, OC_UI_THEME_ROUNDNESS_SMALL);
            oc_ui_style_set_f32(OC_UI_BORDER_SIZE, 1);

            oc_ui_style_rule("button.hover")
            {
                oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_FILL_1);
            }
            oc_ui_style_rule("button.active")
            {
                oc_ui_style_set_var_str8(OC_UI_BORDER_COLOR, OC_UI_THEME_PRIMARY);
            }
            oc_ui_style_rule("button.hover.active")
            {
                oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_FILL_2);
            }

            oc_ui_box("label")
            {
                oc_ui_style_set_i32(OC_UI_CLICK_THROUGH, 1);

                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_TEXT });
                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_TEXT });

                oc_ui_style_set_var_str8(OC_UI_COLOR, OC_UI_THEME_TEXT_0);
                oc_ui_style_set_var_str8(OC_UI_TEXT_SIZE, OC_UI_THEME_TEXT_SIZE_REGULAR);
                oc_ui_style_set_var_str8(OC_UI_FONT, OC_UI_THEME_FONT_REGULAR);

                if(info->selectedIndex == -1)
                {
                    oc_ui_style_set_var_str8(OC_UI_COLOR, OC_UI_THEME_TEXT_2);
                    oc_ui_set_text(info->placeholder);
                }
                else
                {
                    oc_ui_set_text(info->options[info->selectedIndex]);
                }
            }
            oc_ui_box* arrow = oc_ui_box("arrow")
            {
                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, button->rect.h });
                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, button->rect.h });
                oc_ui_style_set_i32(OC_UI_POSITION, OC_UI_POSITION_PARENT);
                oc_ui_style_set_i32(OC_UI_FOOTPRINT, OC_UI_FOOTPRINT_UNSIZED);
                oc_ui_style_set_f32(OC_UI_OFFSET_X, button->rect.w - button->rect.h);
                oc_ui_style_set_var_str8(OC_UI_COLOR, OC_UI_THEME_TEXT_2);
                oc_ui_style_set_i32(OC_UI_CLICK_THROUGH, 1);

                oc_ui_set_draw_proc(oc_ui_select_popup_draw_arrow, 0);
            }
        }

        oc_ui_box* panel = oc_ui_box("panel")
        {
            oc_ui_set_overlay(true);

            f32 checkmarkSize = 16;
            f32 checkmarkSpacing = 5;

            //TODO: set width to max(button.w, max child...)
            f32 containerWidth = oc_max(maxOptionWidth + checkmarkSize + checkmarkSpacing + 2 * panel->style.layout.margin.x,
                                        button->rect.w);

            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, containerWidth });
            oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });

            oc_ui_style_set_i32(OC_UI_POSITION, OC_UI_POSITION_PARENT);
            oc_ui_style_set_i32(OC_UI_FOOTPRINT, OC_UI_FOOTPRINT_UNSIZED);

            oc_ui_style_set_f32(OC_UI_OFFSET_X, button->rect.x);
            oc_ui_style_set_f32(OC_UI_OFFSET_Y, button->rect.y + button->rect.h + 4);
            oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
            oc_ui_style_set_f32(OC_UI_MARGIN_Y, 5);
            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_BG_3);
            oc_ui_style_set_var_str8(OC_UI_BORDER_COLOR, OC_UI_THEME_BORDER);
            oc_ui_style_set_f32(OC_UI_BORDER_SIZE, 1);
            oc_ui_style_set_var_str8(OC_UI_ROUNDNESS, OC_UI_THEME_ROUNDNESS_REGULAR);

            for(int i = 0; i < info->optionCount; i++)
            {
                oc_ui_box* wrapper = oc_ui_box_str8(info->options[i])
                {
                    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
                    oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_X);
                    oc_ui_style_set_i32(OC_UI_ALIGN_X, OC_UI_ALIGN_START);
                    oc_ui_style_set_i32(OC_UI_ALIGN_Y, OC_UI_ALIGN_CENTER);
                    oc_ui_style_set_f32(OC_UI_SPACING, checkmarkSpacing);
                    oc_ui_style_set_f32(OC_UI_MARGIN_X, 12);
                    oc_ui_style_set_f32(OC_UI_MARGIN_Y, 8);

                    oc_ui_tag("^");
                    oc_ui_style_rule(".^.hover")
                    {
                        oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_FILL_0);
                    }
                    oc_ui_style_rule(".^.active")
                    {
                        oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_FILL_2);
                    }

                    if(i == info->selectedIndex)
                    {
                        oc_ui_box("checkmark")
                        {
                            oc_ui_style_set_i32(OC_UI_CLICK_THROUGH, 1);

                            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, checkmarkSize });
                            oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, checkmarkSize });
                            oc_ui_style_set_var_str8(OC_UI_COLOR, OC_UI_THEME_TEXT_2);

                            oc_ui_set_draw_proc(oc_ui_select_popup_draw_checkmark, 0);
                        }
                    }
                    else
                    {
                        oc_ui_box("spacer")
                        {
                            oc_ui_style_set_i32(OC_UI_CLICK_THROUGH, 1);

                            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, checkmarkSize });
                            oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, checkmarkSize });
                        }
                    }

                    oc_ui_box("label")
                    {
                        oc_ui_style_set_i32(OC_UI_CLICK_THROUGH, 1);

                        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, maxOptionWidth });
                        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_TEXT });

                        oc_ui_style_set_var_str8(OC_UI_COLOR, OC_UI_THEME_TEXT_0);
                        oc_ui_style_set_var_str8(OC_UI_TEXT_SIZE, OC_UI_THEME_TEXT_SIZE_REGULAR);
                        oc_ui_style_set_var_str8(OC_UI_FONT, OC_UI_THEME_FONT_REGULAR);

                        oc_ui_set_text(info->options[i]);
                    }

                    oc_ui_sig sig = oc_ui_box_get_sig(wrapper);
                    if(sig.clicked)
                    {
                        result.selectedIndex = i;
                    }
                }
            }
        }

        oc_ui_sig panelSig = oc_ui_box_get_sig(panel);
        if(panel->fresh)
        {
            oc_ui_box_set_closed(panel, true);
        }
        if(!panelSig.closed && oc_mouse_released(oc_ui_input(), OC_MOUSE_LEFT))
        {
            oc_ui_box_set_closed(panel, true);
        }
        else
        {
            oc_ui_sig sig = oc_ui_box_get_sig(button);

            if(sig.clicked)
            {
                oc_ui_box_set_closed(panel, false);
            }
        }
    }
    result.changed = result.selectedIndex != info->selectedIndex;
    return (result);
}

oc_ui_select_popup_info oc_ui_select_popup(const char* name, oc_ui_select_popup_info* info)
{
    return oc_ui_select_popup_str8(OC_STR8(name), info);
}

//------------------------------------------------------------------------------
// text box
//------------------------------------------------------------------------------

oc_str32 oc_ui_edit_replace_selection_with_codepoints(oc_ui_text_box_info* info, oc_str32 codepoints, oc_str32 input)
{
    u32 start = oc_min(info->cursor, info->mark);
    u32 end = oc_max(info->cursor, info->mark);

    oc_str32 before = oc_str32_slice(codepoints, 0, start);
    oc_str32 after = oc_str32_slice(codepoints, end, codepoints.len);

    oc_arena_scope scratch = oc_scratch_begin();

    oc_str32_list list = { 0 };
    oc_str32_list_push(scratch.arena, &list, before);
    oc_str32_list_push(scratch.arena, &list, input);
    oc_str32_list_push(scratch.arena, &list, after);

    codepoints = oc_str32_list_join(oc_ui_frame_arena(), list);

    oc_scratch_end(scratch);

    info->cursor = start + input.len;
    info->mark = info->cursor;
    return (codepoints);
}

oc_str32 oc_ui_edit_delete_selection(oc_ui_text_box_info* info, oc_str32 codepoints)
{
    return (oc_ui_edit_replace_selection_with_codepoints(info, codepoints, (oc_str32){ 0 }));
}

void oc_ui_edit_copy_selection_to_clipboard(oc_ui_text_box_info* info, oc_str32 codepoints)
{
    if(info->cursor == info->mark)
    {
        return;
    }
    u32 start = oc_min(info->cursor, info->mark);
    u32 end = oc_max(info->cursor, info->mark);
    oc_str32 selection = oc_str32_slice(codepoints, start, end);
    oc_str8 string = oc_utf8_push_from_codepoints(oc_ui_frame_arena(), selection);

    oc_clipboard_set_string(string);
}

oc_str32 oc_ui_edit_replace_selection_with_clipboard(oc_ui_text_box_info* info, oc_str32 codepoints)
{
#if OC_PLATFORM_ORCA
    oc_str32 result = codepoints;
#else
    oc_arena* frameArena = oc_ui_frame_arena();
    oc_str8 string = oc_clipboard_get_string(frameArena);
    oc_str32 input = oc_utf8_push_to_codepoints(frameArena, string);
    oc_str32 result = oc_ui_edit_replace_selection_with_codepoints(info, codepoints, input);
#endif
    return (result);
}

typedef enum
{
    OC_UI_EDIT_MOVE,
    OC_UI_EDIT_SELECT,
    OC_UI_EDIT_SELECT_EXTEND,
    OC_UI_EDIT_DELETE,
    OC_UI_EDIT_CUT,
    OC_UI_EDIT_COPY,
    OC_UI_EDIT_PASTE,
    OC_UI_EDIT_SELECT_ALL
} oc_ui_edit_op;

typedef struct oc_ui_edit_command
{
    oc_key_code key;
    oc_keymod_flags mods;

    oc_ui_edit_op operation;
    oc_ui_edit_move move;
    int direction;

} oc_ui_edit_command;

const oc_ui_edit_command OC_UI_EDIT_COMMANDS_MACOS[] = {
    //NOTE(martin): move one left
    {
        .key = OC_KEY_LEFT,
        .operation = OC_UI_EDIT_MOVE,
        .move = OC_UI_EDIT_MOVE_CHAR,
        .direction = -1 },
    //NOTE(martin): move one right
    {
        .key = OC_KEY_RIGHT,
        .operation = OC_UI_EDIT_MOVE,
        .move = OC_UI_EDIT_MOVE_CHAR,
        .direction = 1 },
    //NOTE(martin): move one word left
    {
        .key = OC_KEY_LEFT,
        .mods = OC_KEYMOD_ALT,
        .operation = OC_UI_EDIT_MOVE,
        .move = OC_UI_EDIT_MOVE_WORD,
        .direction = -1 },
    //NOTE(martin): move one word right
    {
        .key = OC_KEY_RIGHT,
        .mods = OC_KEYMOD_ALT,
        .operation = OC_UI_EDIT_MOVE,
        .move = OC_UI_EDIT_MOVE_WORD,
        .direction = 1 },
    //NOTE(martin): move start
    {
        .key = OC_KEY_A,
        .mods = OC_KEYMOD_CTRL,
        .operation = OC_UI_EDIT_MOVE,
        .move = OC_UI_EDIT_MOVE_LINE,
        .direction = -1 },

    { .key = OC_KEY_UP,
      .operation = OC_UI_EDIT_MOVE,
      .move = OC_UI_EDIT_MOVE_LINE,
      .direction = -1 },

    { .key = OC_KEY_UP,
      .mods = OC_KEYMOD_CMD,
      .operation = OC_UI_EDIT_MOVE,
      .move = OC_UI_EDIT_MOVE_LINE,
      .direction = -1 },

    { .key = OC_KEY_LEFT,
      .mods = OC_KEYMOD_CMD,
      .operation = OC_UI_EDIT_MOVE,
      .move = OC_UI_EDIT_MOVE_LINE,
      .direction = -1 },

    //NOTE(martin): move end
    {
        .key = OC_KEY_E,
        .mods = OC_KEYMOD_CTRL,
        .operation = OC_UI_EDIT_MOVE,
        .move = OC_UI_EDIT_MOVE_LINE,
        .direction = 1 },

    { .key = OC_KEY_DOWN,
      .operation = OC_UI_EDIT_MOVE,
      .move = OC_UI_EDIT_MOVE_LINE,
      .direction = 1 },

    { .key = OC_KEY_DOWN,
      .mods = OC_KEYMOD_CMD,
      .operation = OC_UI_EDIT_MOVE,
      .move = OC_UI_EDIT_MOVE_LINE,
      .direction = 1 },

    { .key = OC_KEY_RIGHT,
      .mods = OC_KEYMOD_CMD,
      .operation = OC_UI_EDIT_MOVE,
      .move = OC_UI_EDIT_MOVE_LINE,
      .direction = 1 },

    //NOTE(martin): select one left
    {
        .key = OC_KEY_LEFT,
        .mods = OC_KEYMOD_SHIFT,
        .operation = OC_UI_EDIT_SELECT,
        .move = OC_UI_EDIT_MOVE_CHAR,
        .direction = -1 },
    //NOTE(martin): select one right
    {
        .key = OC_KEY_RIGHT,
        .mods = OC_KEYMOD_SHIFT,
        .operation = OC_UI_EDIT_SELECT,
        .move = OC_UI_EDIT_MOVE_CHAR,
        .direction = 1 },
    //NOTE(martin): select one word left
    {
        .key = OC_KEY_LEFT,
        .mods = OC_KEYMOD_ALT | OC_KEYMOD_SHIFT,
        .operation = OC_UI_EDIT_SELECT,
        .move = OC_UI_EDIT_MOVE_WORD,
        .direction = -1 },
    //NOTE(martin): select one word right
    {
        .key = OC_KEY_RIGHT,
        .mods = OC_KEYMOD_ALT | OC_KEYMOD_SHIFT,
        .operation = OC_UI_EDIT_SELECT,
        .move = OC_UI_EDIT_MOVE_WORD,
        .direction = 1 },
    //NOTE(martin): extend select to start
    {
        .key = OC_KEY_A,
        .mods = OC_KEYMOD_CTRL | OC_KEYMOD_SHIFT,
        .operation = OC_UI_EDIT_SELECT_EXTEND,
        .move = OC_UI_EDIT_MOVE_LINE,
        .direction = -1 },

    { .key = OC_KEY_UP,
      .mods = OC_KEYMOD_SHIFT,
      .operation = OC_UI_EDIT_SELECT_EXTEND,
      .move = OC_UI_EDIT_MOVE_LINE,
      .direction = -1 },

    { .key = OC_KEY_UP,
      .mods = OC_KEYMOD_CMD | OC_KEYMOD_SHIFT,
      .operation = OC_UI_EDIT_SELECT_EXTEND,
      .move = OC_UI_EDIT_MOVE_LINE,
      .direction = -1 },

    { .key = OC_KEY_LEFT,
      .mods = OC_KEYMOD_CMD | OC_KEYMOD_SHIFT,
      .operation = OC_UI_EDIT_SELECT_EXTEND,
      .move = OC_UI_EDIT_MOVE_LINE,
      .direction = -1 },
    //NOTE(martin): extend select to end
    {
        .key = OC_KEY_E,
        .mods = OC_KEYMOD_CTRL | OC_KEYMOD_SHIFT,
        .operation = OC_UI_EDIT_SELECT_EXTEND,
        .move = OC_UI_EDIT_MOVE_LINE,
        .direction = 1 },

    { .key = OC_KEY_DOWN,
      .mods = OC_KEYMOD_SHIFT,
      .operation = OC_UI_EDIT_SELECT_EXTEND,
      .move = OC_UI_EDIT_MOVE_LINE,
      .direction = 1 },

    { .key = OC_KEY_DOWN,
      .mods = OC_KEYMOD_CMD | OC_KEYMOD_SHIFT,
      .operation = OC_UI_EDIT_SELECT_EXTEND,
      .move = OC_UI_EDIT_MOVE_LINE,
      .direction = 1 },

    { .key = OC_KEY_RIGHT,
      .mods = OC_KEYMOD_CMD | OC_KEYMOD_SHIFT,
      .operation = OC_UI_EDIT_SELECT_EXTEND,
      .move = OC_UI_EDIT_MOVE_LINE,
      .direction = 1 },

    //NOTE(martin): select all
    {
        .key = OC_KEY_A,
        .mods = OC_KEYMOD_CMD,
        .operation = OC_UI_EDIT_SELECT_ALL,
        .move = OC_UI_EDIT_MOVE_NONE },
    //NOTE(martin): delete
    {
        .key = OC_KEY_DELETE,
        .operation = OC_UI_EDIT_DELETE,
        .move = OC_UI_EDIT_MOVE_CHAR,
        .direction = 1 },
    //NOTE(martin): delete word
    {
        .key = OC_KEY_DELETE,
        .mods = OC_KEYMOD_ALT,
        .operation = OC_UI_EDIT_DELETE,
        .move = OC_UI_EDIT_MOVE_WORD,
        .direction = 1 },
    //NOTE(martin): backspace
    {
        .key = OC_KEY_BACKSPACE,
        .operation = OC_UI_EDIT_DELETE,
        .move = OC_UI_EDIT_MOVE_CHAR,
        .direction = -1 },
    //NOTE(martin): backspace word
    {
        .key = OC_KEY_BACKSPACE,
        .mods = OC_KEYMOD_ALT,
        .operation = OC_UI_EDIT_DELETE,
        .move = OC_UI_EDIT_MOVE_WORD,
        .direction = -1 },
    //NOTE(martin): cut
    {
        .key = OC_KEY_X,
        .mods = OC_KEYMOD_CMD,
        .operation = OC_UI_EDIT_CUT,
        .move = OC_UI_EDIT_MOVE_NONE },
    //NOTE(martin): copy
    {
        .key = OC_KEY_C,
        .mods = OC_KEYMOD_CMD,
        .operation = OC_UI_EDIT_COPY,
        .move = OC_UI_EDIT_MOVE_NONE },
    //NOTE(martin): paste
    {
        .key = OC_KEY_V,
        .mods = OC_KEYMOD_CMD,
        .operation = OC_UI_EDIT_PASTE,
        .move = OC_UI_EDIT_MOVE_NONE }
};

const oc_ui_edit_command OC_UI_EDIT_COMMANDS_WINDOWS[] = {
    //NOTE(martin): move one left
    {
        .key = OC_KEY_LEFT,
        .operation = OC_UI_EDIT_MOVE,
        .move = OC_UI_EDIT_MOVE_CHAR,
        .direction = -1 },
    //NOTE(martin): move one right
    {
        .key = OC_KEY_RIGHT,
        .operation = OC_UI_EDIT_MOVE,
        .move = OC_UI_EDIT_MOVE_CHAR,
        .direction = 1 },
    //NOTE(martin): move one word left
    {
        .key = OC_KEY_LEFT,
        .mods = OC_KEYMOD_CTRL,
        .operation = OC_UI_EDIT_MOVE,
        .move = OC_UI_EDIT_MOVE_WORD,
        .direction = -1 },
    //NOTE(martin): move one word right
    {
        .key = OC_KEY_RIGHT,
        .mods = OC_KEYMOD_CTRL,
        .operation = OC_UI_EDIT_MOVE,
        .move = OC_UI_EDIT_MOVE_WORD,
        .direction = 1 },
    //NOTE(martin): move start
    {
        .key = OC_KEY_HOME,
        .operation = OC_UI_EDIT_MOVE,
        .move = OC_UI_EDIT_MOVE_LINE,
        .direction = -1 },
    { .key = OC_KEY_UP,
      .operation = OC_UI_EDIT_MOVE,
      .move = OC_UI_EDIT_MOVE_LINE,
      .direction = -1 },
    //NOTE(martin): move end
    {
        .key = OC_KEY_END,
        .operation = OC_UI_EDIT_MOVE,
        .move = OC_UI_EDIT_MOVE_LINE,
        .direction = 1 },

    { .key = OC_KEY_DOWN,
      .operation = OC_UI_EDIT_MOVE,
      .move = OC_UI_EDIT_MOVE_LINE,
      .direction = 1 },
    //NOTE(martin): select one left
    {
        .key = OC_KEY_LEFT,
        .mods = OC_KEYMOD_SHIFT,
        .operation = OC_UI_EDIT_SELECT,
        .move = OC_UI_EDIT_MOVE_CHAR,
        .direction = -1 },
    //NOTE(martin): select one right
    {
        .key = OC_KEY_RIGHT,
        .mods = OC_KEYMOD_SHIFT,
        .operation = OC_UI_EDIT_SELECT,
        .move = OC_UI_EDIT_MOVE_CHAR,
        .direction = 1 },
    //NOTE(martin): select one word left
    {
        .key = OC_KEY_LEFT,
        .mods = OC_KEYMOD_CTRL | OC_KEYMOD_SHIFT,
        .operation = OC_UI_EDIT_SELECT,
        .move = OC_UI_EDIT_MOVE_WORD,
        .direction = -1 },
    //NOTE(martin): select one word right
    {
        .key = OC_KEY_RIGHT,
        .mods = OC_KEYMOD_CTRL | OC_KEYMOD_SHIFT,
        .operation = OC_UI_EDIT_SELECT,
        .move = OC_UI_EDIT_MOVE_WORD,
        .direction = 1 },
    //NOTE(martin): extend select to start
    {
        .key = OC_KEY_HOME,
        .mods = OC_KEYMOD_SHIFT,
        .operation = OC_UI_EDIT_SELECT_EXTEND,
        .move = OC_UI_EDIT_MOVE_LINE,
        .direction = -1 },
    { .key = OC_KEY_UP,
      .mods = OC_KEYMOD_SHIFT,
      .operation = OC_UI_EDIT_SELECT_EXTEND,
      .move = OC_UI_EDIT_MOVE_LINE,
      .direction = -1 },
    //NOTE(martin): extend select to end
    {
        .key = OC_KEY_END,
        .mods = OC_KEYMOD_SHIFT,
        .operation = OC_UI_EDIT_SELECT_EXTEND,
        .move = OC_UI_EDIT_MOVE_LINE,
        .direction = 1 },
    { .key = OC_KEY_DOWN,
      .mods = OC_KEYMOD_SHIFT,
      .operation = OC_UI_EDIT_SELECT_EXTEND,
      .move = OC_UI_EDIT_MOVE_LINE,
      .direction = 1 },
    //NOTE(martin): select all
    {
        .key = OC_KEY_A,
        .mods = OC_KEYMOD_CTRL,
        .operation = OC_UI_EDIT_SELECT_ALL,
        .move = OC_UI_EDIT_MOVE_NONE },
    //NOTE(martin): delete
    {
        .key = OC_KEY_DELETE,
        .operation = OC_UI_EDIT_DELETE,
        .move = OC_UI_EDIT_MOVE_CHAR,
        .direction = 1 },
    //NOTE(martin): delete word
    {
        .key = OC_KEY_DELETE,
        .mods = OC_KEYMOD_CTRL,
        .operation = OC_UI_EDIT_DELETE,
        .move = OC_UI_EDIT_MOVE_WORD,
        .direction = 1 },
    //NOTE(martin): backspace
    {
        .key = OC_KEY_BACKSPACE,
        .operation = OC_UI_EDIT_DELETE,
        .move = OC_UI_EDIT_MOVE_CHAR,
        .direction = -1 },
    //NOTE(martin): backspace word
    {
        .key = OC_KEY_BACKSPACE,
        .mods = OC_KEYMOD_CTRL,
        .operation = OC_UI_EDIT_DELETE,
        .move = OC_UI_EDIT_MOVE_WORD,
        .direction = -1 },
    //NOTE(martin): cut
    {
        .key = OC_KEY_X,
        .mods = OC_KEYMOD_CTRL,
        .operation = OC_UI_EDIT_CUT,
        .move = OC_UI_EDIT_MOVE_NONE },
    { .key = OC_KEY_DELETE,
      .mods = OC_KEYMOD_SHIFT,
      .operation = OC_UI_EDIT_CUT,
      .move = OC_UI_EDIT_MOVE_NONE },
    //NOTE(martin): copy
    {
        .key = OC_KEY_C,
        .mods = OC_KEYMOD_CTRL,
        .operation = OC_UI_EDIT_COPY,
        .move = OC_UI_EDIT_MOVE_NONE },
    { .key = OC_KEY_INSERT,
      .mods = OC_KEYMOD_CTRL,
      .operation = OC_UI_EDIT_COPY,
      .move = OC_UI_EDIT_MOVE_NONE },
    //NOTE(martin): paste
    {
        .key = OC_KEY_V,
        .mods = OC_KEYMOD_CTRL,
        .operation = OC_UI_EDIT_PASTE,
        .move = OC_UI_EDIT_MOVE_NONE },
    { .key = OC_KEY_INSERT,
      .mods = OC_KEYMOD_SHIFT,
      .operation = OC_UI_EDIT_PASTE,
      .move = OC_UI_EDIT_MOVE_NONE }
};

const u32 OC_UI_EDIT_COMMAND_MACOS_COUNT = sizeof(OC_UI_EDIT_COMMANDS_MACOS) / sizeof(oc_ui_edit_command);
const u32 OC_UI_EDIT_COMMAND_WINDOWS_COUNT = sizeof(OC_UI_EDIT_COMMANDS_WINDOWS) / sizeof(oc_ui_edit_command);

bool oc_ui_edit_is_word_separator(u32 codepoint)
{
    //NOTE(ilia): Printable ascii character, except for alphanumeric and _
    return ('!' <= codepoint && codepoint <= '~'
            && !('0' <= codepoint && codepoint <= '9')
            && !('A' <= codepoint && codepoint <= 'Z')
            && !('a' <= codepoint && codepoint <= 'z')
            && codepoint != '_');
}

bool oc_ui_edit_is_whitespace(u32 codepoint)
{
    return (codepoint == ' '
            || (0x09 <= codepoint && codepoint <= 0x0d) // HT, LF, VT, FF, CR
            || codepoint == 0x85                        // NEXT LINE (NEL)
            || codepoint == 0xa0                        // ↓ Unicode Separator, Space (Zs)
            || (0x2000 <= codepoint && codepoint <= 0x200a)
            || codepoint == 0x202f
            || codepoint == 0x205f
            || codepoint == 0x3000);
}

void oc_ui_edit_perform_move(oc_ui_text_box_info* info, oc_ui_edit_move move, int direction, oc_str32 codepoints)
{
    switch(move)
    {
        case OC_UI_EDIT_MOVE_NONE:
            break;

        case OC_UI_EDIT_MOVE_CHAR:
        {
            if(direction < 0 && info->cursor > 0)
            {
                info->cursor--;
            }
            else if(direction > 0 && info->cursor < codepoints.len)
            {
                info->cursor++;
            }
        }
        break;

        case OC_UI_EDIT_MOVE_LINE:
        {
            if(direction < 0)
            {
                info->cursor = 0;
            }
            else if(direction > 0)
            {
                info->cursor = codepoints.len;
            }
        }
        break;

        case OC_UI_EDIT_MOVE_WORD:
        {
            //NOTE(ilia): a simple word break algorithm borrowed from Qt
            //            https://github.com/qt/qtbase/blob/cbea2f5705c39e31600cb7fff552db92198afd34/src/gui/text/qtextlayout.cpp#L643-L714
            //            proper implementation would involve bringing in ICU or querying unicode ranges and parsing ICU rules
            if(direction < 0)
            {
                while(info->cursor > 0 && oc_ui_edit_is_whitespace(codepoints.ptr[info->cursor - 1]))
                {
                    info->cursor--;
                }

                if(info->cursor > 0 && oc_ui_edit_is_word_separator(codepoints.ptr[info->cursor - 1]))
                {
                    info->cursor--;
                    while(info->cursor > 0 && oc_ui_edit_is_word_separator(codepoints.ptr[info->cursor - 1]))
                    {
                        info->cursor--;
                    }
                }
                else
                {
                    while(info->cursor > 0
                          && !oc_ui_edit_is_whitespace(codepoints.ptr[info->cursor - 1])
                          && !oc_ui_edit_is_word_separator(codepoints.ptr[info->cursor - 1]))
                    {
                        info->cursor--;
                    }
                }
            }
            else if(direction > 0)
            {
                if(info->cursor < codepoints.len && oc_ui_edit_is_word_separator(codepoints.ptr[info->cursor]))
                {
                    info->cursor++;
                    while(info->cursor < codepoints.len && oc_ui_edit_is_word_separator(codepoints.ptr[info->cursor]))
                    {
                        info->cursor++;
                    }
                }
                else
                {
                    while(info->cursor < codepoints.len
                          && !oc_ui_edit_is_whitespace(codepoints.ptr[info->cursor])
                          && !oc_ui_edit_is_word_separator(codepoints.ptr[info->cursor]))
                    {
                        info->cursor++;
                    }
                }

                while(info->cursor < codepoints.len && oc_ui_edit_is_whitespace(codepoints.ptr[info->cursor]))
                {
                    info->cursor++;
                }
            }
            break;
        }
    }
}

oc_str32 oc_ui_edit_perform_operation(oc_ui_text_box_info* info, oc_ui_edit_op operation, oc_ui_edit_move move, int direction, oc_str32 codepoints)
{
    switch(operation)
    {
        case OC_UI_EDIT_MOVE:
        {
            bool wasSelectionEmpty = info->cursor == info->mark;

            //NOTE(martin): we place the cursor on the direction-most side of the selection
            //              before performing the move
            u32 cursor = direction < 0 ? oc_min(info->cursor, info->mark) : oc_max(info->cursor, info->mark);
            info->cursor = cursor;

            if(wasSelectionEmpty || move != OC_UI_EDIT_MOVE_CHAR)
            {
                //NOTE: we special case move-one when there is a selection
                //      (just place the cursor at begining/end of selection)
                oc_ui_edit_perform_move(info, move, direction, codepoints);
            }
            info->mark = info->cursor;
        }
        break;

        case OC_UI_EDIT_SELECT:
        {
            oc_ui_edit_perform_move(info, move, direction, codepoints);
        }
        break;

        case OC_UI_EDIT_SELECT_EXTEND:
        {
            if((direction > 0) != (info->cursor > info->mark))
            {
                u32 tmp = info->cursor;
                info->cursor = info->mark;
                info->mark = tmp;
            }
            oc_ui_edit_perform_move(info, move, direction, codepoints);
        }
        break;

        case OC_UI_EDIT_DELETE:
        {
            if(info->cursor == info->mark)
            {
                oc_ui_edit_perform_move(info, move, direction, codepoints);
            }
            codepoints = oc_ui_edit_delete_selection(info, codepoints);
            info->mark = info->cursor;
        }
        break;

        case OC_UI_EDIT_CUT:
        {
            oc_ui_edit_copy_selection_to_clipboard(info, codepoints);
            codepoints = oc_ui_edit_delete_selection(info, codepoints);
        }
        break;

        case OC_UI_EDIT_COPY:
        {
            oc_ui_edit_copy_selection_to_clipboard(info, codepoints);
        }
        break;

        case OC_UI_EDIT_PASTE:
        {
            codepoints = oc_ui_edit_replace_selection_with_clipboard(info, codepoints);
        }
        break;

        case OC_UI_EDIT_SELECT_ALL:
        {
            info->cursor = 0;
            info->mark = codepoints.len;
        }
        break;
    }
    info->cursorBlinkStart = oc_ui_frame_time();

    return (codepoints);
}

i32 oc_ui_edit_find_word_start(oc_ui_text_box_info* info, oc_str32 codepoints, i32 startChar)
{
    i32 c = startChar;
    if(oc_ui_edit_is_whitespace(codepoints.ptr[startChar]))
    {
        while(c > 0 && oc_ui_edit_is_whitespace(codepoints.ptr[c - 1]))
        {
            c--;
        }
    }
    else if(!oc_ui_edit_is_word_separator(codepoints.ptr[startChar]))
    {
        while(c > 0 && !oc_ui_edit_is_word_separator(codepoints.ptr[c - 1]) && !oc_ui_edit_is_whitespace(codepoints.ptr[c - 1]))
        {
            c--;
        }
    }
    return c;
}

i32 oc_ui_edit_find_word_end(oc_ui_text_box_info* info, oc_str32 codepoints, i32 startChar)
{
    i32 c = oc_min(startChar + 1, codepoints.len);
    if(startChar < codepoints.len && oc_ui_edit_is_whitespace(codepoints.ptr[startChar]))
    {
        while(c < codepoints.len && oc_ui_edit_is_whitespace(codepoints.ptr[c]))
        {
            c++;
        }
    }
    else if(startChar < codepoints.len && !oc_ui_edit_is_word_separator(codepoints.ptr[startChar]))
    {
        while(c < codepoints.len && !oc_ui_edit_is_word_separator(codepoints.ptr[c]) && !oc_ui_edit_is_whitespace(codepoints.ptr[c]))
        {
            c++;
        }
    }
    return c;
}

typedef struct oc_ui_text_box_render_data
{
    oc_str32 codepoints;
    u32 firstDisplayedChar;
    i32 cursor;
    i32 mark;
    f64 frameTime;
    f64 cursorBlinkStart;

} oc_ui_text_box_render_data;

void oc_ui_text_box_render(oc_ui_box* box, void* data)
{
    oc_ui_text_box_render_data* renderData = (oc_ui_text_box_render_data*)data;
    oc_str32 codepoints = renderData->codepoints;

    u32 firstDisplayedChar = 0;

    oc_ui_sig sig = oc_ui_box_get_sig(box);
    if(sig.focus)
    {
        firstDisplayedChar = renderData->firstDisplayedChar;
    }

    oc_ui_style* style = &box->style;
    oc_font_metrics extents = oc_font_get_metrics(style->font, style->fontSize);
    f32 lineHeight = extents.ascent + extents.descent;

    oc_str32 before = oc_str32_slice(codepoints, 0, firstDisplayedChar);
    oc_rect beforeBox = oc_font_text_metrics_utf32(style->font, style->fontSize, before).logical;

    oc_rect rect = {
        box->rect.x + box->style.layout.margin.x,
        box->rect.y + box->style.layout.margin.y,
        box->rect.w - 2 * box->style.layout.margin.x,
        box->rect.h - 2 * box->style.layout.margin.y,
    };
    f32 textX = rect.x - beforeBox.w;
    f32 textTop = rect.y + 0.5 * (rect.h - lineHeight);
    f32 textY = textTop + extents.ascent;

    oc_clip_push(rect.x, rect.y, rect.w, rect.h);

    if(sig.focus)
    {
        u32 selectStart = oc_min(renderData->cursor, renderData->mark);
        u32 selectEnd = oc_max(renderData->cursor, renderData->mark);

        oc_str32 beforeSelect = oc_str32_slice(codepoints, 0, selectStart);
        oc_rect beforeSelectBox = oc_font_text_metrics_utf32(style->font, style->fontSize, beforeSelect).logical;
        beforeSelectBox.x += textX;
        beforeSelectBox.y += textY;

        if(selectStart != selectEnd)
        {
            oc_str32 select = oc_str32_slice(codepoints, selectStart, selectEnd);
            oc_str32 afterSelect = oc_str32_slice(codepoints, selectEnd, codepoints.len);
            oc_rect selectBox = oc_font_text_metrics_utf32(style->font, style->fontSize, select).logical;
            oc_rect afterSelectBox = oc_font_text_metrics_utf32(style->font, style->fontSize, afterSelect).logical;

            selectBox.x += beforeSelectBox.x + beforeSelectBox.w;
            selectBox.y += textY;

            /////////////////////////////////////////////////////////////////
            //TODO: use style oc_set_color(ui->theme->palette->blue2);
            /////////////////////////////////////////////////////////////////
            oc_set_color_rgba(0.074, 0.361, 0.722, 1);
            oc_rectangle_fill(selectBox.x, selectBox.y, selectBox.w, lineHeight);

            oc_set_font(style->font);
            oc_set_font_size(style->fontSize);
            oc_set_color(style->color);

            oc_move_to(textX, textY);
            oc_codepoints_outlines(beforeSelect);
            oc_fill();

            oc_set_color(box->style.color);
            oc_codepoints_outlines(select);
            oc_fill();

            oc_set_color(style->color);
            oc_codepoints_outlines(afterSelect);
            oc_fill();
        }
        else
        {
            if(!((u64)(2 * (renderData->frameTime - renderData->cursorBlinkStart)) & 1))
            {
                f32 caretX = rect.x - beforeBox.w + beforeSelectBox.w;
                f32 caretY = textTop;
                oc_set_color(style->color);
                oc_rectangle_fill(caretX, caretY, 1, lineHeight);
            }
            oc_set_font(style->font);
            oc_set_font_size(style->fontSize);
            oc_set_color(style->color);

            oc_move_to(textX, textY);
            oc_codepoints_outlines(codepoints);
            oc_fill();
        }
    }
    else
    {
        oc_set_font(style->font);
        oc_set_font_size(style->fontSize);
        oc_set_color(style->color);

        oc_move_to(textX, textY);
        oc_codepoints_outlines(codepoints);
        oc_fill();
    }

    oc_clip_pop();
}

oc_ui_text_box_result oc_ui_text_box_str8(oc_str8 key, oc_arena* arena, oc_ui_text_box_info* info)
{
    oc_ui_text_box_result result = { .text = info->text };

    oc_arena* frameArena = oc_ui_frame_arena();
    oc_input_state* input = oc_ui_input();

    oc_ui_box* box = oc_ui_box_str8(key)
    {
        result.box = box;

        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 200 });
        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, 32 });

        oc_ui_style_set_f32(OC_UI_MARGIN_X, 12);
        oc_ui_style_set_f32(OC_UI_MARGIN_Y, 6);
        oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_FILL_0);
        oc_ui_style_set_var_str8(OC_UI_ROUNDNESS, OC_UI_THEME_ROUNDNESS_SMALL);
        oc_ui_style_set_f32(OC_UI_BORDER_SIZE, 1);

        oc_ui_style_set_var_str8(OC_UI_COLOR, OC_UI_THEME_TEXT_0);
        oc_ui_style_set_var_str8(OC_UI_TEXT_SIZE, OC_UI_THEME_TEXT_SIZE_REGULAR);
        oc_ui_style_set_var_str8(OC_UI_FONT, OC_UI_THEME_FONT_REGULAR);

        oc_ui_tag("text-box");
        oc_ui_style_rule(".text-box.hover")
        {
            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_FILL_1);
        }
        oc_ui_style_rule(".text-box.focus")
        {
            oc_ui_style_set_var_str8(OC_UI_BORDER_COLOR, OC_UI_THEME_PRIMARY);
        }
        oc_ui_style_rule(".text-box.active")
        {
            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_FILL_2);
        }

        oc_ui_sig sig = oc_ui_box_get_sig(box);

        if(sig.pressed)
        {
            if(!sig.focus)
            {
                oc_ui_box_request_focus(box);

                //NOTE: gain focus
                info->firstDisplayedChar = 0;
                info->cursor = 0;
                info->mark = 0;
            }
            info->cursorBlinkStart = oc_ui_frame_time();
        }

        oc_font font = box->style.font;
        f32 fontSize = box->style.fontSize;
        oc_font_metrics extents = oc_font_get_metrics(font, fontSize);

        oc_rect textRect = {
            box->rect.x + box->style.layout.margin.x,
            box->rect.y + box->style.layout.margin.y,
            box->rect.w - 2 * box->style.layout.margin.x,
            box->rect.h - 2 * box->style.layout.margin.y,
        };

        if(sig.pressed || sig.active)
        {
            //NOTE: set cursor/extend selection on mouse press or drag
            oc_vec2 pos = oc_mouse_position(oc_ui_input());
            f32 cursorX = pos.x - textRect.x;

            oc_str32 codepoints = oc_utf8_push_to_codepoints(frameArena, info->text); //TODO: need to get frame arena
            i32 newCursor = 0;
            i32 hoveredChar = 0;
            f32 x = 0;
            for(int i = info->firstDisplayedChar; i < codepoints.len; i++)
            {
                oc_rect bbox = oc_font_text_metrics_utf32(font, fontSize, oc_str32_slice(codepoints, i, i + 1)).logical;
                if(x < cursorX)
                {
                    hoveredChar = i;
                }
                if(x + 0.5 * bbox.w > cursorX)
                {
                    newCursor = i;
                    break;
                }
                if(i == codepoints.len - 1)
                {
                    newCursor = codepoints.len;
                }
                x += bbox.w;
            }

            if(sig.doubleClicked)
            {
                info->cursor = oc_ui_edit_find_word_end(info, codepoints, hoveredChar);
                info->mark = oc_ui_edit_find_word_start(info, codepoints, hoveredChar);
                info->selectionMode = OC_UI_EDIT_MOVE_WORD;
                info->wordSelectionInitialCursor = info->cursor;
                info->wordSelectionInitialMark = info->mark;
            }
            else if(sig.tripleClicked)
            {
                info->cursor = codepoints.len;
                info->mark = 0;
                info->selectionMode = OC_UI_EDIT_MOVE_LINE;
            }
            else if(sig.pressed
                    && (oc_key_mods(oc_ui_input()) & OC_KEYMOD_SHIFT)
                    && !(newCursor >= oc_min(info->cursor, info->mark) && newCursor <= oc_max(info->cursor, info->mark)))
            {
                //NOTE: put cursor the closest to new cursor (this maximizes the resulting selection,
                //      and seems to be the standard behaviour across a number of text editor)
                if(abs(newCursor - info->cursor) > abs(newCursor - info->mark))
                {
                    info->mark = info->cursor;
                    info->cursor = newCursor;
                }
                else
                {
                    info->cursor = newCursor;
                }
                info->selectionMode = OC_UI_EDIT_MOVE_CHAR;
            }
            else if(sig.pressed)
            {
                info->cursor = newCursor;
                info->mark = newCursor;
                info->selectionMode = OC_UI_EDIT_MOVE_CHAR;
            }
            else if(info->selectionMode == OC_UI_EDIT_MOVE_LINE)
            {
                oc_rect bbox = oc_font_text_metrics_utf32(font, fontSize, codepoints).logical;
                if(fabsf(bbox.w - cursorX) < fabsf(cursorX))
                {
                    info->cursor = codepoints.len;
                    info->mark = 0;
                }
                else
                {
                    info->cursor = 0;
                    info->mark = codepoints.len;
                }
            }
            else if(info->selectionMode == OC_UI_EDIT_MOVE_WORD)
            {
                if(oc_min(info->cursor, info->mark) == oc_min(info->wordSelectionInitialCursor, info->wordSelectionInitialMark)
                   && oc_max(info->cursor, info->mark) == oc_max(info->wordSelectionInitialCursor, info->wordSelectionInitialMark))
                {
                    oc_rect cursorPrefixBbox = oc_font_text_metrics_utf32(font, fontSize, oc_str32_slice(codepoints, 0, info->cursor)).logical;
                    oc_rect markPrefixBbox = oc_font_text_metrics_utf32(font, fontSize, oc_str32_slice(codepoints, 0, info->mark)).logical;
                    f32 cursorX = cursorPrefixBbox.w;
                    f32 markX = markPrefixBbox.w;
                    if(fabsf(cursorX - markX) < fabsf(cursorX - cursorX))
                    {
                        i32 tmp = info->mark;
                        info->mark = info->cursor;
                        info->cursor = tmp;
                    }
                }

                if(info->cursor >= info->mark)
                {
                    info->cursor = oc_ui_edit_find_word_end(info, codepoints, hoveredChar);
                }
                else
                {
                    info->cursor = oc_ui_edit_find_word_start(info, codepoints, hoveredChar);
                }
            }
            else if(info->selectionMode == OC_UI_EDIT_MOVE_CHAR)
            {
                info->cursor = newCursor;
            }
            else
            {
                OC_DEBUG_ASSERT("Unexpected textbox branch");
            }
        }
        else
        {
            info->selectionMode = OC_UI_EDIT_MOVE_CHAR;
        }

        if(!sig.hover)
        {
            if(oc_mouse_pressed(input, OC_MOUSE_LEFT) || oc_mouse_pressed(input, OC_MOUSE_RIGHT) || oc_mouse_pressed(input, OC_MOUSE_MIDDLE))
            {
                if(sig.focus)
                {
                    //NOTE loose focus
                    oc_ui_box_release_focus(box);
                }
            }
        }

        if(sig.focus)
        {
            oc_str32 oldCodepoints = oc_utf8_push_to_codepoints(frameArena, info->text);
            oc_str32 codepoints = oldCodepoints;
            //TODO(martin): check conversion here. Is there a way for cursor or mark to be negative at this point?
            info->cursor = oc_clamp(info->cursor, 0, (i32)codepoints.len);
            info->mark = oc_clamp(info->mark, 0, (i32)codepoints.len);

            //NOTE replace selection with input codepoints
            oc_str32 inputCodepoints = oc_input_text_utf32(frameArena, input);
            if(inputCodepoints.len)
            {
                codepoints = oc_ui_edit_replace_selection_with_codepoints(info, codepoints, inputCodepoints);
                info->cursorBlinkStart = oc_ui_frame_time();
            }

            //NOTE handle shortcuts
            oc_keymod_flags mods = oc_key_mods(input);
            const oc_ui_edit_command* editCommands;
            u32 editCommandCount;
            oc_host_platform hostPlatform = oc_get_host_platform();
            switch(hostPlatform)
            {
                case OC_HOST_PLATFORM_MACOS:
                    editCommands = OC_UI_EDIT_COMMANDS_MACOS;
                    editCommandCount = OC_UI_EDIT_COMMAND_MACOS_COUNT;
                    break;
                case OC_HOST_PLATFORM_WINDOWS:
                    editCommands = OC_UI_EDIT_COMMANDS_WINDOWS;
                    editCommandCount = OC_UI_EDIT_COMMAND_WINDOWS_COUNT;
                    break;
                default:
                    OC_ASSERT(0, "unknown host platform: %i", hostPlatform);
            }

            for(int i = 0; i < editCommandCount; i++)
            {
                const oc_ui_edit_command* command = &(editCommands[i]);

                if((oc_key_press_count(input, command->key) || oc_key_repeat_count(input, command->key))
                   && (mods & ~OC_KEYMOD_MAIN_MODIFIER) == command->mods)
                {
                    codepoints = oc_ui_edit_perform_operation(info, command->operation, command->move, command->direction, codepoints);
                    break;
                }
            }

            if(sig.pasted)
            {
                oc_str8 pastedText = oc_clipboard_pasted_text(input);
                oc_str32 input = oc_utf8_push_to_codepoints(frameArena, pastedText);
                codepoints = oc_ui_edit_replace_selection_with_codepoints(info, codepoints, input);
            }

            //NOTE(martin): check changed/accepted
            if(oldCodepoints.ptr != codepoints.ptr)
            {
                result.changed = true;
                result.text = oc_utf8_push_from_codepoints(arena, codepoints);
            }

            if(oc_key_press_count(input, OC_KEY_ENTER))
            {
                //TODO(martin): extract in gui_edit_complete() (and use below)
                result.accepted = true;
                oc_ui_box_release_focus(box);
            }

            //NOTE slide contents
            {
                if(info->cursor < info->firstDisplayedChar)
                {
                    info->firstDisplayedChar = info->cursor;
                }
                else
                {
                    i32 firstDisplayedChar = info->firstDisplayedChar;
                    oc_str32 firstToCursor = oc_str32_slice(codepoints, firstDisplayedChar, info->cursor);
                    oc_rect firstToCursorBox = oc_font_text_metrics_utf32(font, fontSize, firstToCursor).logical;

                    while(firstToCursorBox.w > textRect.w)
                    {
                        firstDisplayedChar++;
                        firstToCursor = oc_str32_slice(codepoints, firstDisplayedChar, info->cursor);
                        firstToCursorBox = oc_font_text_metrics_utf32(font, fontSize, firstToCursor).logical;
                    }

                    info->firstDisplayedChar = firstDisplayedChar;
                }
            }

            //NOTE: set renderer
            oc_ui_text_box_render_data* renderData = oc_arena_push_type(frameArena, oc_ui_text_box_render_data);

            *renderData = (oc_ui_text_box_render_data){
                .codepoints = oc_str32_push_copy(frameArena, codepoints),
                .firstDisplayedChar = info->firstDisplayedChar,
                .cursor = info->cursor,
                .mark = info->mark,
                .frameTime = oc_ui_frame_time(),
                .cursorBlinkStart = info->cursorBlinkStart,
            };

            oc_ui_set_draw_proc(oc_ui_text_box_render, renderData);
        }
        else
        {
            //NOTE: set renderer
            oc_ui_text_box_render_data* renderData = oc_arena_push_type(frameArena, oc_ui_text_box_render_data);

            *renderData = (oc_ui_text_box_render_data){
                .firstDisplayedChar = info->firstDisplayedChar,
                .cursor = info->cursor,
                .mark = info->mark,
                .frameTime = oc_ui_frame_time(),
                .cursorBlinkStart = info->cursorBlinkStart,
            };

            if(info->text.len)
            {
                renderData->codepoints = oc_utf8_push_to_codepoints(frameArena, info->text);
            }
            else
            {
                oc_ui_style_set_var_str8(OC_UI_COLOR, OC_UI_THEME_TEXT_2);
                renderData->codepoints = oc_utf8_push_to_codepoints(frameArena, info->defaultText);
            }

            oc_ui_set_draw_proc(oc_ui_text_box_render, renderData);
        }
    }

    return (result);
}

oc_ui_text_box_result oc_ui_text_box(const char* name, oc_arena* arena, oc_ui_text_box_info* info)
{
    return oc_ui_text_box_str8(OC_STR8(name), arena, info);
}
