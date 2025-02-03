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

    oc_ui_sig sig = oc_ui_box_sig(box);
    return (sig);
}

oc_ui_sig oc_ui_label(const char* key, const char* label)
{
    return (oc_ui_label_str8(OC_STR8((char*)key), OC_STR8((char*)label)));
}

//------------------------------------------------------------------------------
// button
//------------------------------------------------------------------------------

oc_ui_sig oc_ui_button_behavior(oc_ui_box* box)
{
    oc_ui_sig sig = oc_ui_box_sig(box);

    if(sig.hovering)
    {
        oc_ui_box_set_hot(box, true);
        if(sig.dragging)
        {
            oc_ui_box_set_active(box, true);
        }
    }
    else
    {
        oc_ui_box_set_hot(box, false);
    }
    if(!sig.hovering || !sig.dragging)
    {
        oc_ui_box_set_active(box, false);
    }
    return (sig);
}

oc_ui_sig oc_ui_button_str8(oc_str8 key, oc_str8 text)
{
    oc_ui_box* box = oc_ui_box_str8(key)
    {
        oc_ui_set_text(text);
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

    oc_ui_sig sig = oc_ui_button_behavior(box);
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
    oc_rect rect = oc_ui_box_rect(box);

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

    oc_ui_style style = oc_ui_box_style(box);
    oc_set_color(style.color);
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
            oc_ui_style_set_f32(OC_UI_BORDER_SIZE, 1.);

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

        oc_ui_box_set_draw_proc(box, oc_ui_checkbox_draw, 0);
    }

    oc_ui_sig sig = oc_ui_button_behavior(box);
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

        oc_rect frameRect = oc_ui_box_rect(frame);
        oc_ui_axis trackAxis = (frameRect.w > frameRect.h) ? OC_UI_AXIS_X : OC_UI_AXIS_Y;
        oc_ui_axis secondAxis = (trackAxis == OC_UI_AXIS_Y) ? OC_UI_AXIS_X : OC_UI_AXIS_Y;

        f32 trackThickness = 4;
        f32 thumbSize = 24;

        f32 beforeRatio, afterRatio, thumbRatio, trackFillRatio;
        if(trackAxis == OC_UI_AXIS_X)
        {
            thumbRatio = oc_min(thumbSize / frameRect.w, 1.);
            beforeRatio = (*value) * (1. - thumbRatio);
            afterRatio = (1. - *value) * (1. - thumbRatio);
            trackFillRatio = beforeRatio + thumbRatio / 2;
        }
        else
        {
            thumbRatio = oc_min(thumbSize / frameRect.h, 1.);
            beforeRatio = (1. - *value) * (1. - thumbRatio);
            afterRatio = (*value) * (1. - thumbRatio);
            trackFillRatio = thumbRatio / 2 + afterRatio;
        }

        oc_ui_style_set_i32(OC_UI_AXIS, trackAxis);

        oc_ui_box* track = oc_ui_box("track")
        {
            oc_ui_style_set_i32(OC_UI_FLOATING_X, 1);
            oc_ui_style_set_i32(OC_UI_FLOATING_Y, 1);
            oc_ui_style_set_f32(OC_UI_FLOAT_TARGET_X + trackAxis, 0);
            oc_ui_style_set_f32(OC_UI_FLOAT_TARGET_X + secondAxis, 0.5 * (thumbSize - trackThickness));

            oc_ui_style_set_size(OC_UI_WIDTH + trackAxis, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
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
        oc_ui_sig thumbSig = oc_ui_box_sig(thumb);
        oc_ui_sig trackSig = oc_ui_box_sig(track);
        if(thumbSig.dragging)
        {
            oc_rect trackRect = oc_ui_box_rect(track);
            oc_rect thumbRect = oc_ui_box_rect(thumb);

            f32 trackExtents = trackRect.c[2 + trackAxis] - thumbRect.c[2 + trackAxis];
            *value = (trackSig.mouse.c[trackAxis] - thumbSig.lastPressedMouse.c[trackAxis]) / trackExtents;
            *value = oc_clamp(*value, 0, 1);
            if(trackAxis == OC_UI_AXIS_Y)
            {
                *value = 1 - *value;
            }
        }

        if(oc_ui_box_active(frame))
        {
            //NOTE: activated from outside
            oc_ui_box_set_hot(track, true);
            oc_ui_box_set_hot(thumb, true);
            oc_ui_box_set_active(track, true);
            oc_ui_box_set_active(thumb, true);
        }

        if(trackSig.hovering)
        {
            oc_ui_box_set_hot(track, true);
            oc_ui_box_set_hot(thumb, true);
        }
        else if(thumbSig.wheel.c[trackAxis] == 0)
        {
            oc_ui_box_set_hot(track, false);
            oc_ui_box_set_hot(thumb, false);
        }

        if(thumbSig.dragging)
        {
            oc_ui_box_set_active(track, true);
            oc_ui_box_set_active(thumb, true);
        }
        else if(thumbSig.wheel.c[trackAxis] == 0)
        {
            oc_ui_box_set_active(track, false);
            oc_ui_box_set_active(thumb, false);
            oc_ui_box_set_active(frame, false);
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
    oc_rect rect = oc_ui_box_rect(box);
    oc_mat2x3 matrix = {
        rect.w, 0, rect.x,
        0, rect.h, rect.y
    };
    oc_matrix_multiply_push(matrix);

    oc_ui_style style = oc_ui_box_style(box);
    oc_set_color(style.color);
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
                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });

                oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_X);
                oc_ui_style_set_var_str8(OC_UI_SPACING, OC_UI_THEME_SPACING_TIGHT);
                oc_ui_style_set_f32(OC_UI_MARGIN_X, 1);
                oc_ui_style_set_f32(OC_UI_MARGIN_Y, 1);
                oc_ui_style_set_i32(OC_UI_ALIGN_Y, OC_UI_ALIGN_CENTER);

                oc_ui_box* radio = oc_ui_box("radio")
                {
                    oc_ui_box_set_draw_proc(radio, oc_ui_radio_indicator_draw, 0);

                    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 16 });
                    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, 16 });
                    oc_ui_style_set_f32(OC_UI_ROUNDNESS, 8);

                    oc_ui_style_set_i32(OC_UI_CLICK_THROUGH, 1);

                    oc_ui_sig sig = oc_ui_box_sig(row);
                    if(sig.clicked)
                    {
                        result.selectedIndex = i;
                    }
                    oc_ui_box_set_hot(radio, sig.hovering);
                    oc_ui_box_set_active(radio, sig.hovering && sig.dragging);

                    if(result.selectedIndex == i)
                    {
                        oc_ui_style_set_color(OC_UI_COLOR, (oc_color){ 1, 1, 1, 1 });
                        oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_PRIMARY);

                        if(oc_ui_box_hot(radio))
                        {
                            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_PRIMARY_HOVER);
                        }
                        if(oc_ui_box_active(radio))
                        {
                            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_PRIMARY_ACTIVE);
                        }
                    }
                    else
                    {
                        oc_ui_style_set_var_str8(OC_UI_BORDER_COLOR, OC_UI_THEME_TEXT_3);
                        oc_ui_style_set_f32(OC_UI_BORDER_SIZE, 1);

                        if(oc_ui_box_hot(radio))
                        {
                            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_FILL_0);
                            oc_ui_style_set_var_str8(OC_UI_BORDER_COLOR, OC_UI_THEME_PRIMARY);
                        }
                        if(oc_ui_box_active(radio))
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
    oc_rect rect = oc_ui_box_rect(box);
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

    oc_ui_style style = oc_ui_box_style(box);
    //    oc_set_color(style.bgColor);
    oc_set_color((oc_color){ 0.786, 0.792, 0.804, 1, OC_COLOR_SPACE_SRGB });

    oc_fill();

    oc_matrix_pop();
}

void oc_ui_tooltip_str8(oc_str8 key, oc_str8 text)
{
    oc_vec2 p = oc_ui_mouse_position();

    oc_ui_box_str8(key)
    {
        oc_ui_set_overlay(true);

        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
        oc_ui_style_set_i32(OC_UI_FLOATING_X, true);
        oc_ui_style_set_i32(OC_UI_FLOATING_Y, true);
        oc_ui_style_set_f32(OC_UI_FLOAT_TARGET_X, p.x);
        oc_ui_style_set_f32(OC_UI_FLOAT_TARGET_Y, p.y - 10); //TODO: quick fix for aliging single line tooltips arrow to mouse, fix that!

        oc_ui_box* arrow = oc_ui_box("arrow")
        {
            oc_ui_box_set_draw_proc(arrow, oc_ui_tooltip_arrow_draw, 0);

            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 24 });
            oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, 24 });
            //            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_TOOLTIP);
        }

        oc_ui_box("contents")
        {
            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
            oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
            oc_ui_style_set_f32(OC_UI_MARGIN_X, 12);
            oc_ui_style_set_f32(OC_UI_MARGIN_Y, 8);
            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_TOOLTIP);
            oc_ui_style_set_var_str8(OC_UI_ROUNDNESS, OC_UI_THEME_ROUNDNESS_REGULAR);

            oc_ui_label_str8(OC_STR8_LIT("label"), text);
        }
    }
}

void oc_ui_tooltip(const char* key, const char* text)
{
    oc_ui_tooltip_str8(OC_STR8(key), OC_STR8(text));
}

#if 0

//------------------------------------------------------------------------------
// Menus
//------------------------------------------------------------------------------

void oc_ui_menu_bar_begin_str8(oc_str8 name)
{
    oc_ui_style style = {
        .size.width = { OC_UI_SIZE_PARENT, 1, 0 },
        .size.height = { OC_UI_SIZE_CHILDREN },
        .layout.axis = OC_UI_AXIS_X,
    };
    oc_ui_attr_mask mask = OC_UI_MASK_SIZE
                          | OC_UI_MASK_LAYOUT_AXIS;

    oc_ui_style_next(&style, mask);
    oc_ui_box* bar = oc_ui_box_begin_str8(name, OC_UI_FLAG_NONE);

    oc_ui_sig sig = oc_ui_box_sig(bar);
    oc_ui_context* ui = oc_ui_get_context();
    if(!sig.hovering && oc_mouse_released(&ui->input, OC_MOUSE_LEFT))
    {
        oc_ui_box_set_active(bar, false);
    }
}

void oc_ui_menu_bar_begin(const char* name)
{
    oc_ui_menu_bar_begin_str8(OC_STR8(name));
}

void oc_ui_menu_bar_end(void)
{
    oc_ui_box_end(); // menu bar
}

void oc_ui_menu_begin_str8(oc_str8 label)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_theme* theme = ui->theme;
    oc_ui_box* container = oc_ui_box_make_str8(label, 0);
    oc_ui_box_push(container);

    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_CHILDREN },
                                     .size.height = { OC_UI_SIZE_CHILDREN },
                                     .layout.margin.x = 8,
                                     .layout.margin.y = 4,
                                     .bgColor = { 0, 0, 0, 0 } },
                     OC_UI_MASK_SIZE | OC_UI_MASK_LAYOUT_MARGINS | OC_UI_MASK_BG_COLOR);

    oc_ui_pattern hoverPattern = { 0 };
    oc_ui_pattern_push(&ui->frameArena, &hoverPattern, (oc_ui_selector){ .kind = OC_UI_SEL_STATUS, .status = OC_UI_HOVER });
    oc_ui_style hoverStyle = { .bgColor = theme->fill0 };
    oc_ui_style_match_before(hoverPattern, &hoverStyle, OC_UI_MASK_BG_COLOR);

    oc_ui_pattern activePattern = { 0 };
    oc_ui_pattern_push(&ui->frameArena, &activePattern, (oc_ui_selector){ .kind = OC_UI_SEL_STATUS, .status = OC_UI_ACTIVE });
    oc_ui_style activeStyle = { .bgColor = theme->fill2 };
    oc_ui_style_match_before(activePattern, &activeStyle, OC_UI_MASK_BG_COLOR);

    oc_ui_box* button = oc_ui_box_begin("button", OC_UI_FLAG_CLICKABLE);

    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_TEXT },
                                     .size.height = { OC_UI_SIZE_TEXT } },
                     OC_UI_MASK_SIZE);
    oc_ui_box* buttonLabel = oc_ui_box_make_str8(label);

    oc_ui_box_end(); // button

    oc_ui_box* bar = container->parent;

    oc_ui_sig sig = oc_ui_box_sig(button);
    oc_ui_sig barSig = oc_ui_box_sig(bar);

    oc_ui_style style = { .size.width = { OC_UI_SIZE_CHILDREN },
                          .size.height = { OC_UI_SIZE_CHILDREN },
                          .floating.x = true,
                          .floating.y = true,
                          .floatTarget = { button->rect.x,
                                           button->rect.y + button->rect.h },
                          .layout.axis = OC_UI_AXIS_Y,
                          .layout.margin.x = 0,
                          .layout.margin.y = 4,
                          .bgColor = theme->bg1,
                          .borderColor = theme->elevatedBorder,
                          .borderSize = 1 };

    oc_ui_attr_mask mask = OC_UI_MASK_SIZE
                          | OC_UI_MASK_FLOAT
                          | OC_UI_MASK_LAYOUT
                          | OC_UI_MASK_BG_COLOR
                          | OC_UI_MASK_BORDER_COLOR
                          | OC_UI_MASK_BORDER_SIZE;

    oc_ui_flags flags = OC_UI_FLAG_OVERLAY;

    oc_ui_style_next(&style, mask);
    oc_ui_box* menu = oc_ui_box_make("panel", flags);

    if(oc_ui_box_active(bar))
    {
        if(sig.hovering)
        {
            oc_ui_box_set_active(button, true);
        }
        else if(barSig.hovering)
        {
            oc_ui_box_set_active(button, false);
        }

        if(sig.clicked)
        {
            oc_ui_box_set_active(bar, false);
            oc_ui_box_set_active(button, false);
        }
    }
    else
    {
        oc_ui_box_set_active(button, false);
        if(sig.clicked)
        {
            oc_ui_box_set_active(bar, true);
            oc_ui_box_set_active(button, true);
        }
    }

    oc_ui_box_set_closed(menu, !oc_ui_box_active(button));
    oc_ui_box_push(menu);
}

void oc_ui_menu_begin(const char* label)
{
    oc_ui_menu_begin_str8(OC_STR8(label));
}

void oc_ui_menu_end(void)
{
    oc_ui_box_pop(); // menu
    oc_ui_box_pop(); // container
}

oc_ui_sig oc_ui_menu_button_str8(oc_str8 label)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_theme* theme = ui->theme;

    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_TEXT },
                                     .size.height = { OC_UI_SIZE_TEXT },
                                     .layout.margin.x = 8,
                                     .layout.margin.y = 4,
                                     .bgColor = { 0, 0, 0, 0 } },
                     OC_UI_MASK_SIZE
                         | OC_UI_MASK_LAYOUT_MARGINS
                         | OC_UI_MASK_BG_COLOR);

    oc_ui_pattern hoverPattern = { 0 };
    oc_ui_pattern_push(&ui->frameArena, &hoverPattern, (oc_ui_selector){ .kind = OC_UI_SEL_STATUS, .status = OC_UI_HOVER });
    oc_ui_style hoverStyle = { .bgColor = theme->fill0 };
    oc_ui_style_match_before(hoverPattern, &hoverStyle, OC_UI_MASK_BG_COLOR);

    oc_ui_pattern activePattern = { 0 };
    oc_ui_pattern_push(&ui->frameArena, &activePattern, (oc_ui_selector){ .kind = OC_UI_SEL_STATUS, .status = OC_UI_ACTIVE });
    oc_ui_style activeStyle = { .bgColor = theme->fill2 };
    oc_ui_style_match_before(activePattern, &activeStyle, OC_UI_MASK_BG_COLOR);

    oc_ui_flags flags = OC_UI_FLAG_CLICKABLE
                      | OC_UI_FLAG_CLIP;

    oc_ui_box* box = oc_ui_box_make_str8(label, flags);
    oc_ui_sig sig = oc_ui_box_sig(box);
    return (sig);
}

oc_ui_sig oc_ui_menu_button(const char* label)
{
    return oc_ui_menu_button_str8(OC_STR8(label));
}

//------------------------------------------------------------------------------
// Select
//------------------------------------------------------------------------------

void oc_ui_select_popup_draw_arrow(oc_ui_box* box, void* data)
{
    oc_rect rect = oc_ui_box_rect(box);
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
    oc_rect rect = oc_ui_box_rect(box);
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

oc_ui_select_popup_info oc_ui_select_popup_str8(oc_str8 name, oc_ui_select_popup_info* info)
{
    oc_ui_select_popup_info result = *info;

    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_theme* theme = ui->theme;

    oc_ui_container_str8(name, 0)
    {
        oc_ui_pattern hoverPattern = { 0 };
        oc_ui_pattern_push(&ui->frameArena, &hoverPattern, (oc_ui_selector){ .kind = OC_UI_SEL_STATUS, .status = OC_UI_HOVER });
        oc_ui_style hoverStyle = { .bgColor = theme->fill1 };
        oc_ui_style_match_after(hoverPattern, &hoverStyle, OC_UI_MASK_BG_COLOR);

        oc_ui_pattern activePattern = { 0 };
        oc_ui_pattern_push(&ui->frameArena, &activePattern, (oc_ui_selector){ .kind = OC_UI_SEL_STATUS, .status = OC_UI_ACTIVE });
        oc_ui_style activeStyle = { .borderColor = theme->primary,
                                    .borderSize = 1 };
        oc_ui_style_match_after(activePattern, &activeStyle, OC_UI_MASK_BORDER_COLOR | OC_UI_MASK_BORDER_SIZE);

        oc_ui_pattern mouseDownPattern = { 0 };
        oc_ui_pattern_push(&ui->frameArena, &mouseDownPattern, (oc_ui_selector){ .kind = OC_UI_SEL_STATUS, .status = OC_UI_ACTIVE });
        oc_ui_pattern_push(&ui->frameArena, &mouseDownPattern, (oc_ui_selector){ .op = OC_UI_SEL_AND, .kind = OC_UI_SEL_STATUS, .status = OC_UI_HOVER });
        oc_ui_style mouseDownStyle = { .bgColor = theme->fill2,
                                       .borderColor = theme->primary,
                                       .borderSize = 1 };
        oc_ui_style_match_after(mouseDownPattern, &mouseDownStyle, OC_UI_MASK_BG_COLOR | OC_UI_MASK_BORDER_COLOR | OC_UI_MASK_BORDER_SIZE);

        oc_ui_box* button = oc_ui_box_make("button",
                                           OC_UI_FLAG_CLICKABLE
                                               | OC_UI_FLAG_OVERFLOW_ALLOW_X
                                               | OC_UI_FLAG_CLIP);

        f32 maxOptionWidth = 0;
        f32 lineHeight = 0;
        oc_rect bbox = { 0 };
        for(int i = 0; i < info->optionCount; i++)
        {
            bbox = oc_font_text_metrics(button->style.font, button->style.fontSize, info->options[i]).logical;
            maxOptionWidth = oc_max(maxOptionWidth, bbox.w);
        }
        f32 buttonWidth = maxOptionWidth + 2 * button->style.layout.margin.x + button->rect.h;

        oc_ui_style_box_before(button,
                               oc_ui_pattern_owner(),
                               &(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, buttonWidth },
                                               .size.height = { OC_UI_SIZE_CHILDREN },
                                               .layout.margin.x = 12,
                                               .layout.margin.y = 6,
                                               .bgColor = theme->fill0,
                                               .roundness = theme->roundnessSmall },
                               OC_UI_MASK_SIZE
                                   | OC_UI_MASK_LAYOUT_MARGIN_X
                                   | OC_UI_MASK_LAYOUT_MARGIN_Y
                                   | OC_UI_MASK_BG_COLOR
                                   | OC_UI_MASK_ROUNDNESS);

        oc_ui_box_push(button);
        {
            oc_ui_container("selected_option", 0)
            {
                if(info->selectedIndex == -1)
                {
                    oc_ui_style_next(&(oc_ui_style){ .color = theme->text2 },
                                     OC_UI_MASK_COLOR);
                    oc_ui_label_str8(info->placeholder);
                }
                else
                {
                    oc_ui_label_str8(info->options[info->selectedIndex]);
                }
            }

            oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, button->rect.h },
                                             .size.height = { OC_UI_SIZE_PIXELS, button->rect.h },
                                             .floating.x = true,
                                             .floating.y = true,
                                             .floatTarget = { button->rect.w - button->rect.h, 0 },
                                             .color = theme->text2 },
                             OC_UI_MASK_SIZE
                                 | OC_UI_MASK_FLOAT
                                 | OC_UI_MASK_COLOR);

            oc_ui_box* arrow = oc_ui_box_make("arrow", OC_UI_FLAG_NONE);
            oc_ui_box_set_draw_proc(arrow, oc_ui_select_popup_draw_arrow, 0);
        }
        oc_ui_box_pop();

        //panel
        oc_ui_box* panel = oc_ui_box_make("panel",
                                          OC_UI_FLAG_BLOCK_MOUSE
                                              | OC_UI_FLAG_OVERLAY);

        f32 checkmarkSize = 16;
        f32 checkmarkSpacing = 5;

        //TODO: set width to max(button.w, max child...)
        f32 containerWidth = oc_max(maxOptionWidth + checkmarkSize + checkmarkSpacing + 2 * panel->style.layout.margin.x,
                                    button->rect.w);

        oc_ui_style_box_before(panel,
                               oc_ui_pattern_owner(),
                               &(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, containerWidth },
                                               .size.height = { OC_UI_SIZE_CHILDREN },
                                               .floating.x = true,
                                               .floating.y = true,
                                               .floatTarget = { button->rect.x,
                                                                button->rect.y + button->rect.h + 4 },
                                               .layout.axis = OC_UI_AXIS_Y,
                                               .layout.margin.x = 0,
                                               .layout.margin.y = 5,
                                               .bgColor = theme->bg3,
                                               .borderColor = theme->elevatedBorder,
                                               .borderSize = 1,
                                               .roundness = theme->roundnessMedium },
                               OC_UI_MASK_SIZE
                                   | OC_UI_MASK_FLOAT
                                   | OC_UI_MASK_LAYOUT
                                   | OC_UI_MASK_BG_COLOR
                                   | OC_UI_MASK_BORDER_COLOR
                                   | OC_UI_MASK_BORDER_SIZE
                                   | OC_UI_MASK_ROUNDNESS);

        oc_ui_box_push(panel);
        {
            for(int i = 0; i < info->optionCount; i++)
            {
                oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                                 .size.height = { OC_UI_SIZE_TEXT },
                                                 .layout.axis = OC_UI_AXIS_X,
                                                 .layout.align.x = OC_UI_ALIGN_START,
                                                 .layout.align.y = OC_UI_ALIGN_CENTER,
                                                 .layout.spacing = checkmarkSpacing,
                                                 .layout.margin.x = 12,
                                                 .layout.margin.y = 8,
                                                 .bgColor = { 0, 0, 0, 0 } },
                                 OC_UI_MASK_SIZE
                                     | OC_UI_MASK_LAYOUT_AXIS
                                     | OC_UI_MASK_LAYOUT_ALIGN_X
                                     | OC_UI_MASK_LAYOUT_ALIGN_Y
                                     | OC_UI_MASK_LAYOUT_SPACING
                                     | OC_UI_MASK_LAYOUT_MARGIN_X
                                     | OC_UI_MASK_LAYOUT_MARGIN_Y
                                     | OC_UI_MASK_BG_COLOR);

                oc_ui_pattern hoverPattern = { 0 };
                oc_ui_pattern_push(&ui->frameArena, &hoverPattern, (oc_ui_selector){ .kind = OC_UI_SEL_STATUS, .status = OC_UI_HOVER });
                oc_ui_style_match_before(hoverPattern, &(oc_ui_style){ .bgColor = theme->fill0 }, OC_UI_MASK_BG_COLOR);

                oc_ui_pattern activePattern = { 0 };
                oc_ui_pattern_push(&ui->frameArena, &activePattern, (oc_ui_selector){ .kind = OC_UI_SEL_STATUS, .status = OC_UI_ACTIVE });
                oc_ui_style_match_before(activePattern, &(oc_ui_style){ .bgColor = theme->fill2 }, OC_UI_MASK_BG_COLOR);

                oc_ui_box* wrapper = oc_ui_box_begin_str8(info->options[i],
                                                          OC_UI_FLAG_CLICKABLE);
                if(i == info->selectedIndex)
                {
                    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, checkmarkSize },
                                                     .size.height = { OC_UI_SIZE_PIXELS, checkmarkSize },
                                                     .color = theme->text2 },
                                     OC_UI_MASK_SIZE | OC_UI_MASK_COLOR);
                    oc_ui_box* checkmark = oc_ui_box_make("checkmark", OC_UI_FLAG_NONE);
                    oc_ui_box_set_draw_proc(checkmark, oc_ui_select_popup_draw_checkmark, 0);
                }
                else
                {
                    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, checkmarkSize },
                                                     .size.height = { OC_UI_SIZE_PIXELS, checkmarkSize } },
                                     OC_UI_MASK_SIZE);
                    oc_ui_box* spacer = oc_ui_box_make("spacer", 0);
                }

                oc_ui_container("label", 0)
                {
                    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, maxOptionWidth },
                                                     .size.height = { OC_UI_SIZE_TEXT } },
                                     OC_UI_MASK_SIZE);
                    oc_ui_box* label = oc_ui_box_make_str8(info->options[i], OC_UI_FLAG_NONE);
                }

                oc_ui_sig sig = oc_ui_box_sig(wrapper);
                if(sig.clicked)
                {
                    result.selectedIndex = i;
                }

                oc_ui_box_end(); // wrapper
            }
        }
        oc_ui_box_pop();

        oc_ui_context* ui = oc_ui_get_context();
        if(oc_ui_box_active(panel) && oc_mouse_released(&ui->input, OC_MOUSE_LEFT))
        {
            oc_ui_box_set_active(button, false);
            oc_ui_box_set_active(panel, false);
        }
        else
        {
            oc_ui_sig sig = oc_ui_box_sig(button);
            if(sig.pressed)
            {
                oc_ui_box_set_active(button, true);
            }
            if(sig.clicked)
            {
                oc_ui_box_set_active(panel, true);
            }
        }
        oc_ui_box_set_closed(panel, !oc_ui_box_active(panel));
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
oc_str32 oc_ui_edit_replace_selection_with_codepoints(oc_ui_context* ui, oc_str32 codepoints, oc_str32 input)
{
    u32 start = oc_min(ui->editCursor, ui->editMark);
    u32 end = oc_max(ui->editCursor, ui->editMark);

    oc_str32 before = oc_str32_slice(codepoints, 0, start);
    oc_str32 after = oc_str32_slice(codepoints, end, codepoints.len);

    oc_str32_list list = { 0 };
    oc_str32_list_push(&ui->frameArena, &list, before);
    oc_str32_list_push(&ui->frameArena, &list, input);
    oc_str32_list_push(&ui->frameArena, &list, after);

    codepoints = oc_str32_list_join(&ui->frameArena, list);

    ui->editCursor = start + input.len;
    ui->editMark = ui->editCursor;
    return (codepoints);
}

oc_str32 oc_ui_edit_delete_selection(oc_ui_context* ui, oc_str32 codepoints)
{
    return (oc_ui_edit_replace_selection_with_codepoints(ui, codepoints, (oc_str32){ 0 }));
}

void oc_ui_edit_copy_selection_to_clipboard(oc_ui_context* ui, oc_str32 codepoints)
{
    if(ui->editCursor == ui->editMark)
    {
        return;
    }
    u32 start = oc_min(ui->editCursor, ui->editMark);
    u32 end = oc_max(ui->editCursor, ui->editMark);
    oc_str32 selection = oc_str32_slice(codepoints, start, end);
    oc_str8 string = oc_utf8_push_from_codepoints(&ui->frameArena, selection);

    oc_clipboard_set_string(string);
}

oc_str32 oc_ui_edit_replace_selection_with_clipboard(oc_ui_context* ui, oc_str32 codepoints)
{
    #if OC_PLATFORM_ORCA
    oc_str32 result = codepoints;
    #else
    oc_str8 string = oc_clipboard_get_string(&ui->frameArena);
    oc_str32 input = oc_utf8_push_to_codepoints(&ui->frameArena, string);
    oc_str32 result = oc_ui_edit_replace_selection_with_codepoints(ui, codepoints, input);
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

void oc_ui_edit_perform_move(oc_ui_context* ui, oc_ui_edit_move move, int direction, oc_str32 codepoints)
{
    switch(move)
    {
        case OC_UI_EDIT_MOVE_NONE:
            break;

        case OC_UI_EDIT_MOVE_CHAR:
        {
            if(direction < 0 && ui->editCursor > 0)
            {
                ui->editCursor--;
            }
            else if(direction > 0 && ui->editCursor < codepoints.len)
            {
                ui->editCursor++;
            }
        }
        break;

        case OC_UI_EDIT_MOVE_LINE:
        {
            if(direction < 0)
            {
                ui->editCursor = 0;
            }
            else if(direction > 0)
            {
                ui->editCursor = codepoints.len;
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
                while(ui->editCursor > 0 && oc_ui_edit_is_whitespace(codepoints.ptr[ui->editCursor - 1]))
                {
                    ui->editCursor--;
                }

                if(ui->editCursor > 0 && oc_ui_edit_is_word_separator(codepoints.ptr[ui->editCursor - 1]))
                {
                    ui->editCursor--;
                    while(ui->editCursor > 0 && oc_ui_edit_is_word_separator(codepoints.ptr[ui->editCursor - 1]))
                    {
                        ui->editCursor--;
                    }
                }
                else
                {
                    while(ui->editCursor > 0
                          && !oc_ui_edit_is_whitespace(codepoints.ptr[ui->editCursor - 1])
                          && !oc_ui_edit_is_word_separator(codepoints.ptr[ui->editCursor - 1]))
                    {
                        ui->editCursor--;
                    }
                }
            }
            else if(direction > 0)
            {
                if(ui->editCursor < codepoints.len && oc_ui_edit_is_word_separator(codepoints.ptr[ui->editCursor]))
                {
                    ui->editCursor++;
                    while(ui->editCursor < codepoints.len && oc_ui_edit_is_word_separator(codepoints.ptr[ui->editCursor]))
                    {
                        ui->editCursor++;
                    }
                }
                else
                {
                    while(ui->editCursor < codepoints.len
                          && !oc_ui_edit_is_whitespace(codepoints.ptr[ui->editCursor])
                          && !oc_ui_edit_is_word_separator(codepoints.ptr[ui->editCursor]))
                    {
                        ui->editCursor++;
                    }
                }

                while(ui->editCursor < codepoints.len && oc_ui_edit_is_whitespace(codepoints.ptr[ui->editCursor]))
                {
                    ui->editCursor++;
                }
            }
            break;
        }
    }
}

oc_str32 oc_ui_edit_perform_operation(oc_ui_context* ui, oc_ui_edit_op operation, oc_ui_edit_move move, int direction, oc_str32 codepoints)
{
    switch(operation)
    {
        case OC_UI_EDIT_MOVE:
        {
            bool wasSelectionEmpty = ui->editCursor == ui->editMark;

            //NOTE(martin): we place the cursor on the direction-most side of the selection
            //              before performing the move
            u32 cursor = direction < 0 ? oc_min(ui->editCursor, ui->editMark) : oc_max(ui->editCursor, ui->editMark);
            ui->editCursor = cursor;

            if(wasSelectionEmpty || move != OC_UI_EDIT_MOVE_CHAR)
            {
                //NOTE: we special case move-one when there is a selection
                //      (just place the cursor at begining/end of selection)
                oc_ui_edit_perform_move(ui, move, direction, codepoints);
            }
            ui->editMark = ui->editCursor;
        }
        break;

        case OC_UI_EDIT_SELECT:
        {
            oc_ui_edit_perform_move(ui, move, direction, codepoints);
        }
        break;

        case OC_UI_EDIT_SELECT_EXTEND:
        {
            if((direction > 0) != (ui->editCursor > ui->editMark))
            {
                u32 tmp = ui->editCursor;
                ui->editCursor = ui->editMark;
                ui->editMark = tmp;
            }
            oc_ui_edit_perform_move(ui, move, direction, codepoints);
        }
        break;

        case OC_UI_EDIT_DELETE:
        {
            if(ui->editCursor == ui->editMark)
            {
                oc_ui_edit_perform_move(ui, move, direction, codepoints);
            }
            codepoints = oc_ui_edit_delete_selection(ui, codepoints);
            ui->editMark = ui->editCursor;
        }
        break;

        case OC_UI_EDIT_CUT:
        {
            oc_ui_edit_copy_selection_to_clipboard(ui, codepoints);
            codepoints = oc_ui_edit_delete_selection(ui, codepoints);
        }
        break;

        case OC_UI_EDIT_COPY:
        {
            oc_ui_edit_copy_selection_to_clipboard(ui, codepoints);
        }
        break;

        case OC_UI_EDIT_PASTE:
        {
            codepoints = oc_ui_edit_replace_selection_with_clipboard(ui, codepoints);
        }
        break;

        case OC_UI_EDIT_SELECT_ALL:
        {
            ui->editCursor = 0;
            ui->editMark = codepoints.len;
        }
        break;
    }
    ui->editCursorBlinkStart = ui->frameTime;

    return (codepoints);
}

i32 oc_ui_edit_find_word_start(oc_ui_context* ui, oc_str32 codepoints, i32 startChar)
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

i32 oc_ui_edit_find_word_end(oc_ui_context* ui, oc_str32 codepoints, i32 startChar)
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

void oc_ui_text_box_render(oc_ui_box* box, void* data)
{
    oc_str32 codepoints = *(oc_str32*)data;
    oc_ui_context* ui = oc_ui_get_context();

    u32 firstDisplayedChar = 0;
    if(oc_ui_box_active(box))
    {
        firstDisplayedChar = ui->editFirstDisplayedChar;
    }

    oc_ui_style* style = &box->style;
    oc_font_metrics extents = oc_font_get_metrics(style->font, style->fontSize);
    f32 lineHeight = extents.ascent + extents.descent;

    oc_str32 before = oc_str32_slice(codepoints, 0, firstDisplayedChar);
    oc_rect beforeBox = oc_font_text_metrics_utf32(style->font, style->fontSize, before).logical;

    oc_rect rect = oc_ui_box_rect(box);
    f32 textX = rect.x - beforeBox.w;
    f32 textTop = rect.y + 0.5 * (rect.h - lineHeight);
    f32 textY = textTop + extents.ascent;

    if(box->active)
    {
        u32 selectStart = oc_min(ui->editCursor, ui->editMark);
        u32 selectEnd = oc_max(ui->editCursor, ui->editMark);

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

            oc_set_color(ui->theme->palette->blue2);
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
            if(!((u64)(2 * (ui->frameTime - ui->editCursorBlinkStart)) & 1))
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
}

oc_ui_text_box_result oc_ui_text_box_str8(oc_str8 name, oc_arena* arena, oc_str8 text)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_theme* theme = ui->theme;

    oc_ui_text_box_result result = { .text = text };

    oc_ui_style frameStyle = { .layout.margin.x = 12,
                               .layout.margin.y = 6,
                               .bgColor = theme->fill0,
                               .roundness = theme->roundnessSmall };
    oc_ui_attr_mask frameMask = OC_UI_MASK_LAYOUT_MARGIN_X
                               | OC_UI_MASK_LAYOUT_MARGIN_Y
                               | OC_UI_MASK_BG_COLOR
                               | OC_UI_MASK_ROUNDNESS;
    oc_ui_style_next(&frameStyle, frameMask);

    oc_ui_pattern hoverPattern = { 0 };
    oc_ui_pattern_push(&ui->frameArena, &hoverPattern, (oc_ui_selector){ .kind = OC_UI_SEL_STATUS, .status = OC_UI_HOVER });
    oc_ui_style hoverStyle = { .bgColor = theme->fill1 };
    oc_ui_style_match_after(hoverPattern, &hoverStyle, OC_UI_MASK_BG_COLOR);

    oc_ui_pattern activePattern = { 0 };
    oc_ui_pattern_push(&ui->frameArena, &activePattern, (oc_ui_selector){ .kind = OC_UI_SEL_STATUS, .status = OC_UI_ACTIVE });
    oc_ui_style activeStyle = { .borderColor = theme->primary,
                                .borderSize = 1 };
    oc_ui_style_match_after(activePattern, &activeStyle, OC_UI_MASK_BORDER_COLOR | OC_UI_MASK_BORDER_SIZE);

    oc_ui_pattern draggingPattern = { 0 };
    oc_ui_pattern_push(&ui->frameArena, &draggingPattern, (oc_ui_selector){ .kind = OC_UI_SEL_STATUS, .status = OC_UI_DRAGGING });
    oc_ui_style draggingStyle = { .bgColor = theme->fill2 };
    oc_ui_style_match_after(draggingPattern, &draggingStyle, OC_UI_MASK_BG_COLOR);

    oc_ui_flags frameFlags = OC_UI_FLAG_CLICKABLE;
    oc_ui_box* frame = oc_ui_box_begin_str8(name, frameFlags);
    result.frame = frame;
    oc_ui_tag_box(frame, "frame");
    oc_font font = frame->style.font;
    f32 fontSize = frame->style.fontSize;

    oc_ui_style textStyle = { .size.width = (oc_ui_size){ OC_UI_SIZE_PARENT, 1 },
                              .size.height = (oc_ui_size){ OC_UI_SIZE_PARENT, 1 } };
    oc_ui_style_next(&textStyle, OC_UI_MASK_SIZE);

    oc_ui_box* textBox = oc_ui_box_make("text", OC_UI_FLAG_CLIP);
    result.textBox = textBox;
    oc_ui_tag_box(textBox, "text");

    oc_font_metrics extents = oc_font_get_metrics(font, fontSize);

    oc_ui_sig sig = oc_ui_box_sig(frame);

    if(sig.pressed)
    {
        if(!oc_ui_box_active(frame))
        {
            oc_ui_box_set_active(frame, true);
            oc_ui_box_set_active(textBox, true);

            //NOTE: focus
            ui->focus = frame;
            ui->editFirstDisplayedChar = 0;
            ui->editCursor = 0;
            ui->editMark = 0;
        }
        ui->editCursorBlinkStart = ui->frameTime;
    }

    if(sig.pressed || sig.dragging)
    {
        //NOTE: set cursor/extend selection on mouse press or drag
        oc_vec2 pos = oc_ui_mouse_position();
        f32 cursorX = pos.x - textBox->rect.x;

        oc_str32 codepoints = oc_utf8_push_to_codepoints(&ui->frameArena, text);
        i32 newCursor = 0;
        i32 hoveredChar = 0;
        f32 x = 0;
        for(int i = ui->editFirstDisplayedChar; i < codepoints.len; i++)
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
            ui->editCursor = oc_ui_edit_find_word_end(ui, codepoints, hoveredChar);
            ui->editMark = oc_ui_edit_find_word_start(ui, codepoints, hoveredChar);
            ui->editSelectionMode = OC_UI_EDIT_MOVE_WORD;
            ui->editWordSelectionInitialCursor = ui->editCursor;
            ui->editWordSelectionInitialMark = ui->editMark;
        }
        else if(sig.tripleClicked)
        {
            ui->editCursor = codepoints.len;
            ui->editMark = 0;
            ui->editSelectionMode = OC_UI_EDIT_MOVE_LINE;
        }
        else if(sig.pressed
                && (oc_key_mods(&ui->input) & OC_KEYMOD_SHIFT)
                && !(newCursor >= oc_min(ui->editCursor, ui->editMark) && newCursor <= oc_max(ui->editCursor, ui->editMark)))
        {
            //NOTE: put cursor the closest to new cursor (this maximizes the resulting selection,
            //      and seems to be the standard behaviour across a number of text editor)
            if(abs(newCursor - ui->editCursor) > abs(newCursor - ui->editMark))
            {
                ui->editMark = ui->editCursor;
                ui->editCursor = newCursor;
            }
            else
            {
                ui->editCursor = newCursor;
            }
            ui->editSelectionMode = OC_UI_EDIT_MOVE_CHAR;
        }
        else if(sig.pressed)
        {
            ui->editCursor = newCursor;
            ui->editMark = newCursor;
            ui->editSelectionMode = OC_UI_EDIT_MOVE_CHAR;
        }
        else if(ui->editSelectionMode == OC_UI_EDIT_MOVE_LINE)
        {
            oc_rect bbox = oc_font_text_metrics_utf32(font, fontSize, codepoints).logical;
            if(fabsf(bbox.w - cursorX) < fabsf(cursorX))
            {
                ui->editCursor = codepoints.len;
                ui->editMark = 0;
            }
            else
            {
                ui->editCursor = 0;
                ui->editMark = codepoints.len;
            }
        }
        else if(ui->editSelectionMode == OC_UI_EDIT_MOVE_WORD)
        {
            if(oc_min(ui->editCursor, ui->editMark) == oc_min(ui->editWordSelectionInitialCursor, ui->editWordSelectionInitialMark)
               && oc_max(ui->editCursor, ui->editMark) == oc_max(ui->editWordSelectionInitialCursor, ui->editWordSelectionInitialMark))
            {
                oc_rect editCursorPrefixBbox = oc_font_text_metrics_utf32(font, fontSize, oc_str32_slice(codepoints, 0, ui->editCursor)).logical;
                oc_rect editMarkPrefixBbox = oc_font_text_metrics_utf32(font, fontSize, oc_str32_slice(codepoints, 0, ui->editMark)).logical;
                f32 editCursorX = editCursorPrefixBbox.w;
                f32 editMarkX = editMarkPrefixBbox.w;
                if(fabsf(cursorX - editMarkX) < fabsf(cursorX - editCursorX))
                {
                    i32 tmp = ui->editMark;
                    ui->editMark = ui->editCursor;
                    ui->editCursor = tmp;
                }
            }

            if(ui->editCursor >= ui->editMark)
            {
                ui->editCursor = oc_ui_edit_find_word_end(ui, codepoints, hoveredChar);
            }
            else
            {
                ui->editCursor = oc_ui_edit_find_word_start(ui, codepoints, hoveredChar);
            }
        }
        else if(ui->editSelectionMode == OC_UI_EDIT_MOVE_CHAR)
        {
            ui->editCursor = newCursor;
        }
        else
        {
            OC_DEBUG_ASSERT("Unexpected textbox branch");
        }
    }
    else
    {
        ui->editSelectionMode = OC_UI_EDIT_MOVE_CHAR;
    }

    if(sig.hovering)
    {
        oc_ui_box_set_hot(frame, true);
    }
    else
    {
        oc_ui_box_set_hot(frame, false);

        if(oc_mouse_pressed(&ui->input, OC_MOUSE_LEFT) || oc_mouse_pressed(&ui->input, OC_MOUSE_RIGHT) || oc_mouse_pressed(&ui->input, OC_MOUSE_MIDDLE))
        {
            if(oc_ui_box_active(frame))
            {
                oc_ui_box_set_active(frame, false);
                oc_ui_box_set_active(textBox, false);

                //NOTE loose focus
                ui->focus = 0;
            }
        }
    }

    if(oc_ui_box_active(frame))
    {
        oc_str32 oldCodepoints = oc_utf8_push_to_codepoints(&ui->frameArena, text);
        oc_str32 codepoints = oldCodepoints;
        //TODO(martin): check conversion here. Is there a way for editCursor or editMark to be negative at this point?
        ui->editCursor = oc_clamp(ui->editCursor, 0, (i32)codepoints.len);
        ui->editMark = oc_clamp(ui->editMark, 0, (i32)codepoints.len);

        //NOTE replace selection with input codepoints
        oc_str32 input = oc_input_text_utf32(&ui->frameArena, &ui->input);
        if(input.len)
        {
            codepoints = oc_ui_edit_replace_selection_with_codepoints(ui, codepoints, input);
            ui->editCursorBlinkStart = ui->frameTime;
        }

        //NOTE handle shortcuts
        oc_keymod_flags mods = oc_key_mods(&ui->input);
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

            if((oc_key_press_count(&ui->input, command->key) || oc_key_repeat_count(&ui->input, command->key))
               && (mods & ~OC_KEYMOD_MAIN_MODIFIER) == command->mods)
            {
                codepoints = oc_ui_edit_perform_operation(ui, command->operation, command->move, command->direction, codepoints);
                break;
            }
        }

        if(sig.pasted)
        {
            oc_str8 pastedText = oc_clipboard_pasted_text(&ui->input);
            oc_str32 input = oc_utf8_push_to_codepoints(&ui->frameArena, pastedText);
            codepoints = oc_ui_edit_replace_selection_with_codepoints(ui, codepoints, input);
        }

        //NOTE(martin): check changed/accepted
        if(oldCodepoints.ptr != codepoints.ptr)
        {
            result.changed = true;
            result.text = oc_utf8_push_from_codepoints(arena, codepoints);
        }

        if(oc_key_press_count(&ui->input, OC_KEY_ENTER))
        {
            //TODO(martin): extract in gui_edit_complete() (and use below)
            result.accepted = true;
            oc_ui_box_set_active(frame, false);
            oc_ui_box_set_active(textBox, false);
            ui->focus = 0;
        }

        //NOTE slide contents
        {
            if(ui->editCursor < ui->editFirstDisplayedChar)
            {
                ui->editFirstDisplayedChar = ui->editCursor;
            }
            else
            {
                i32 firstDisplayedChar = ui->editFirstDisplayedChar;
                oc_str32 firstToCursor = oc_str32_slice(codepoints, firstDisplayedChar, ui->editCursor);
                oc_rect firstToCursorBox = oc_font_text_metrics_utf32(font, fontSize, firstToCursor).logical;

                while(firstToCursorBox.w > textBox->rect.w)
                {
                    firstDisplayedChar++;
                    firstToCursor = oc_str32_slice(codepoints, firstDisplayedChar, ui->editCursor);
                    firstToCursorBox = oc_font_text_metrics_utf32(font, fontSize, firstToCursor).logical;
                }

                ui->editFirstDisplayedChar = firstDisplayedChar;
            }
        }

        //NOTE: set renderer
        oc_str32* renderCodepoints = oc_arena_push_type(&ui->frameArena, oc_str32);
        *renderCodepoints = oc_str32_push_copy(&ui->frameArena, codepoints);
        oc_ui_box_set_draw_proc(textBox, oc_ui_text_box_render, renderCodepoints);
    }
    else
    {
        //NOTE: set renderer
        oc_str32* renderCodepoints = oc_arena_push_type(&ui->frameArena, oc_str32);
        *renderCodepoints = oc_utf8_push_to_codepoints(&ui->frameArena, text);
        oc_ui_box_set_draw_proc(textBox, oc_ui_text_box_render, renderCodepoints);
    }

    oc_ui_box_end(); // frame

    return (result);
}

oc_ui_text_box_result oc_ui_text_box(const char* name, oc_arena* arena, oc_str8 text)
{
    return oc_ui_text_box_str8(OC_STR8(name), arena, text);
}

#endif // 0
