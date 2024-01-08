/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <math.h>
#include <orca.h>

const oc_str8 clockNumberStrings[] = {
    OC_STR8_LIT("12"),
    OC_STR8_LIT("1"),
    OC_STR8_LIT("2"),
    OC_STR8_LIT("3"),
    OC_STR8_LIT("4"),
    OC_STR8_LIT("5"),
    OC_STR8_LIT("6"),
    OC_STR8_LIT("7"),
    OC_STR8_LIT("8"),
    OC_STR8_LIT("9"),
    OC_STR8_LIT("10"),
    OC_STR8_LIT("11"),
};

oc_surface surface = { 0 };
oc_canvas_renderer renderer = { 0 };
oc_canvas_context context = { 0 };
oc_font font = { 0 };
oc_vec2 frameSize = { 100, 100 };
f64 lastSeconds = 0;

oc_mat2x3 mat_transform(f32 x, f32 y, f32 radians)
{
    oc_mat2x3 rotation = oc_mat2x3_rotate(radians);
    oc_mat2x3 translation = oc_mat2x3_translate(x, y);
    return oc_mat2x3_mul_m(translation, rotation);
}

ORCA_EXPORT void oc_on_init(void)
{
    oc_window_set_title(OC_STR8("clock"));
    oc_window_set_size((oc_vec2){ .x = 400, .y = 400 });

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
}

ORCA_EXPORT void oc_on_resize(u32 width, u32 height)
{
    frameSize.x = width;
    frameSize.y = height;
}

ORCA_EXPORT void oc_on_frame_refresh(void)
{
    oc_canvas_context_select(context);
    oc_set_color_rgba(.05, .05, .05, 1);
    oc_clear();

    const f64 timestampSecs = oc_clock_time(OC_CLOCK_DATE);
    const f64 secs = fmod(timestampSecs, 60);
    const f64 minutes = fmod(timestampSecs, 60 * 60) / 60;
    const f64 hours = fmod(timestampSecs, 60 * 60 * 24) / (60 * 60);
    const f64 hoursAs12Format = fmod(hours, 12.0);

    if(lastSeconds != floor(secs))
    {
        lastSeconds = floor(secs);
        oc_log_info("current time: %.0f:%.0f:%.0f", floor(hours), floor(minutes), floor(secs));
    }

    const f32 secondsRotation = (M_PI * 2) * (secs / 60.0) - (M_PI / 2);
    const f32 minutesRotation = (M_PI * 2) * (minutes / 60.0) - (M_PI / 2);
    const f32 hoursRotation = (M_PI * 2) * (hoursAs12Format / 12.0) - (M_PI / 2);

    const f32 centerX = frameSize.x / 2;
    const f32 centerY = frameSize.y / 2;
    const f32 clockRadius = oc_min(frameSize.x, frameSize.y) * 0.5f * 0.85f;

    const f32 DEFAULT_CLOCK_RADIUS = 260;
    const f32 uiScale = clockRadius / DEFAULT_CLOCK_RADIUS;

    const f32 fontSize = 26 * uiScale;
    oc_set_font(font);
    oc_set_font_size(fontSize);

    // clock backing
    oc_set_color_rgba(1, 1, 1, 1);
    oc_circle_fill(centerX, centerY, clockRadius);

    // clock face
    for(int i = 0; i < oc_array_size(clockNumberStrings); ++i)
    {
        oc_rect textRect = oc_font_text_metrics(font, fontSize, clockNumberStrings[i]).ink;

        const f32 angle = i * ((M_PI * 2) / 12.0f) - (M_PI / 2);
        oc_mat2x3 transform = mat_transform(centerX - (textRect.w / 2) - textRect.x,
                                            centerY - (textRect.h / 2) - textRect.y,
                                            angle);

        oc_vec2 pos = oc_mat2x3_mul(transform, (oc_vec2){ clockRadius * 0.8f, 0 });

        oc_set_color_rgba(0.2, 0.2, 0.2, 1);
        oc_text_fill(pos.x, pos.y, clockNumberStrings[i]);
    }

    // hours hand
    oc_matrix_multiply_push(mat_transform(centerX, centerY, hoursRotation));
    {
        oc_set_color_rgba(.2, 0.2, 0.2, 1);
        oc_rounded_rectangle_fill(0, -7.5 * uiScale, clockRadius * 0.5f, 15 * uiScale, 5 * uiScale);
    }
    oc_matrix_pop();

    // minutes hand
    oc_matrix_multiply_push(mat_transform(centerX, centerY, minutesRotation));
    {
        oc_set_color_rgba(.2, 0.2, 0.2, 1);
        oc_rounded_rectangle_fill(0, -5 * uiScale, clockRadius * 0.7f, 10 * uiScale, 5 * uiScale);
    }
    oc_matrix_pop();

    // seconds hand
    oc_matrix_multiply_push(mat_transform(centerX, centerY, secondsRotation));
    {
        oc_set_color_rgba(1, 0.2, 0.2, 1);
        oc_rounded_rectangle_fill(0, -2.5 * uiScale, clockRadius * 0.8f, 5 * uiScale, 5 * uiScale);
    }
    oc_matrix_pop();

    oc_set_color_rgba(.2, 0.2, 0.2, 1);
    oc_circle_fill(centerX, centerY, 10 * uiScale);

    oc_canvas_render(renderer, context, surface);
    oc_canvas_present(renderer, surface);
}
