/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <orca.h>
#include "microui.h"
#include "demo.c"

oc_surface surface = { 0 };
oc_canvas_renderer renderer = { 0 };
oc_canvas_context context = { 0 };
oc_font font = { 0 };
f32 fontSize = 12;
oc_vec2 frameSize = { 800, 800 };
mu_Context* ctx;
oc_vec2 mouse = {};

static int text_width(mu_Font mfont, const char *text, int len) {
    oc_str8 textFinal = { (char*) text, len };
    oc_text_metrics metrics = oc_font_text_metrics(font, fontSize, textFinal);
    return metrics.logical.w;
}

static int text_height(mu_Font mfont) {
    oc_font_metrics metrics = oc_font_get_metrics(font, fontSize);
    return metrics.ascent + metrics.descent;
}

ORCA_EXPORT void oc_on_init(void)
{
    oc_window_set_title(OC_STR8("microui"));
    oc_window_set_size(frameSize);

    renderer = oc_canvas_renderer_create();
    surface = oc_canvas_surface_create(renderer);
    context = oc_canvas_context_create();

    oc_unicode_range ranges[5] = {
        OC_UNICODE_BASIC_LATIN,
        OC_UNICODE_C1_CONTROLS_AND_LATIN_1_SUPPLEMENT,
        OC_UNICODE_LATIN_EXTENDED_A,
        OC_UNICODE_LATIN_EXTENDED_B,
        OC_UNICODE_SPECIALS
    };

    font = oc_font_create_from_path(OC_STR8("/segoeui.ttf"), 5, ranges);
    ctx = malloc(sizeof(mu_Context));
    mu_init(ctx);
    ctx->text_width = text_width;
    ctx->text_height = text_height;
}

ORCA_EXPORT void oc_on_resize(u32 width, u32 height)
{
    frameSize.x = width;
    frameSize.y = height;
}

ORCA_EXPORT void oc_on_raw_event(oc_event* event)
{
    if (event->type == OC_EVENT_MOUSE_MOVE) {
        mouse = (oc_vec2) { event->mouse.x, event->mouse.y };
        mu_input_mousemove(ctx, event->mouse.x, event->mouse.y);
    } else if (event->type == OC_EVENT_MOUSE_WHEEL) {
        mu_input_scroll(ctx, event->mouse.deltaX, event->mouse.deltaY);
    } else if (event->type == OC_EVENT_MOUSE_BUTTON) {
        int isDown = event->key.action == OC_KEY_PRESS;

        if (event->key.button == OC_MOUSE_LEFT) {
            if (isDown) {
                mu_input_mousedown(ctx, mouse.x, mouse.y, MU_MOUSE_LEFT);
            } else {
                mu_input_mouseup(ctx, mouse.x, mouse.y, MU_MOUSE_LEFT);
            }
        }
    }
}

void set_color(mu_Color color) {
    oc_set_color_srgba(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
}

ORCA_EXPORT void oc_on_frame_refresh(void)
{
    oc_canvas_context_select(context);

    oc_set_color_srgba(bg[0] / 255, bg[1] / 255, bg[2] / 255, 1);
    oc_clear();

    oc_set_font(font);
    oc_set_font_size(fontSize);

    process_frame(ctx);

    // render
    int doClip = 0;
    mu_Command *cmd = NULL;
    while (mu_next_command(ctx, &cmd)) {
        switch (cmd->type) {
            case MU_COMMAND_TEXT: 
                set_color(cmd->text.color);
                oc_text_fill(cmd->text.pos.x, cmd->text.pos.y + fontSize, OC_STR8(cmd->text.str));
                break;

            case MU_COMMAND_RECT: 
                set_color(cmd->rect.color);
                oc_rectangle_fill(cmd->rect.rect.x, cmd->rect.rect.y, cmd->rect.rect.w, cmd->rect.rect.h);
                break;

            // we just do some custom "icons" here to avoid importing an icon font
            case MU_COMMAND_ICON: 
                set_color(cmd->icon.color);
                oc_set_width(2);
                float w2 = cmd->icon.rect.w/2;
                float h2 = cmd->icon.rect.h/2;
                if (cmd->icon.id == MU_ICON_EXPANDED) {
                    oc_circle_stroke(cmd->icon.rect.x + w2, cmd->icon.rect.y + h2, w2/2);
                } else if (cmd->icon.id == MU_ICON_COLLAPSED || cmd->icon.id == MU_ICON_CHECK) {
                    oc_circle_fill(cmd->icon.rect.x + w2, cmd->icon.rect.y + h2, w2/2);
                } else if (cmd->icon.id == MU_ICON_CLOSE) {
                    float margin = 6;
                    oc_rectangle_stroke(cmd->rect.rect.x + margin, cmd->rect.rect.y + margin, cmd->rect.rect.w - margin*2, cmd->rect.rect.h - margin*2);
                } else {
                    // TODO
                }
                break;

            case MU_COMMAND_CLIP: 
                if (!doClip) {
                    doClip = 1;
                } else {
                    oc_clip_pop();
                }

                oc_clip_push(cmd->rect.rect.x, cmd->rect.rect.y, cmd->rect.rect.w, cmd->rect.rect.h); 
                break;
        }
    }
    if (doClip) {
        oc_clip_pop();
    }

    oc_canvas_render(renderer, context, surface);
    oc_canvas_present(renderer, surface);
}
