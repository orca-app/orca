#include <math.h>

#include <orca.h>

#define ARRAYSIZE(array) (sizeof(array) / sizeof(array[0]))

oc_vec2 frameSize = { 100, 100 };

oc_surface surface;
oc_canvas canvas;
oc_font font;

f64 lastSeconds = 0;

oc_mat2x3 mat_rotation(f32 radians);
oc_mat2x3 mat_translation(f32 x, f32 y);
oc_mat2x3 mat_transform(f32 x, f32 y, f32 radians);
f32 minf(f32 a, f32 b);

ORCA_EXPORT void oc_on_init(void)
{
    oc_runtime_window_set_title(OC_STR8("clock"));
    oc_runtime_window_set_size(400, 400);

    surface = oc_surface_canvas();
    canvas = oc_canvas_create();

    {
        oc_str8 filename = OC_STR8("/segoeui.ttf");
        oc_file file = oc_file_open(filename, OC_FILE_ACCESS_READ, 0);
        if(oc_file_last_error(file) != OC_IO_OK)
        {
            oc_log_error("Couldn't open file %s\n", oc_str8_to_cstring(oc_scratch(), filename));
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
}

ORCA_EXPORT void oc_on_resize(u32 width, u32 height)
{
    frameSize.x = width;
    frameSize.y = height;
}

ORCA_EXPORT void oc_on_frame_refresh(void)
{
    const oc_str8 clock_number_strings[] = {
        OC_STR8("12"),
        OC_STR8("1"),
        OC_STR8("2"),
        OC_STR8("3"),
        OC_STR8("4"),
        OC_STR8("5"),
        OC_STR8("6"),
        OC_STR8("7"),
        OC_STR8("8"),
        OC_STR8("9"),
        OC_STR8("10"),
        OC_STR8("11"),
    };

    oc_canvas_set_current(canvas);
    oc_surface_select(surface);
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
    const f32 clockRadius = minf(frameSize.x, frameSize.y) * 0.5f * 0.85f;

    const f32 DEFAULT_CLOCK_RADIUS = 260;
    const f32 uiScale = clockRadius / DEFAULT_CLOCK_RADIUS;

    const f32 fontSize = 26 * uiScale;
    oc_set_font(font);
    oc_set_font_size(fontSize);

    // clock backing
    oc_set_color_rgba(1, 1, 1, 1);
    oc_circle_fill(centerX, centerY, clockRadius);

    // clock face
    for(int i = 0; i < ARRAYSIZE(clock_number_strings); ++i)
    {
        const f32 rot = -i * ((M_PI * 2) / 12.0f) + (M_PI / 2);
        const f32 sinRot = sinf(rot);
        const f32 cosRot = cosf(rot);

        oc_rect textRect = oc_text_bounding_box(font, fontSize, clock_number_strings[i]);
        textRect.h -= 10 * uiScale; // oc_text_bounding_box height doesn't seem to be a tight fit around the glyph

        const f32 x = cosRot * clockRadius * 0.8f - (textRect.w / 2) + centerX;
        const f32 y = -sinRot * clockRadius * 0.8f + (textRect.h / 2) + centerY;

        oc_set_color_rgba(0.2, 0.2, 0.2, 1);
        oc_move_to(x, y);
        oc_text_outlines(clock_number_strings[i]);
        oc_fill();
    }

    oc_matrix_push(mat_transform(centerX, centerY, hoursRotation));
    {
        oc_set_color_rgba(.2, 0.2, 0.2, 1);
        oc_rounded_rectangle_fill(0, -7.5 * uiScale, clockRadius * 0.5f, 15 * uiScale, 5 * uiScale);

        oc_matrix_pop();
    }

    oc_matrix_push(mat_transform(centerX, centerY, minutesRotation));
    {
        oc_set_color_rgba(.2, 0.2, 0.2, 1);
        oc_rounded_rectangle_fill(0, -5 * uiScale, clockRadius * 0.7f, 10 * uiScale, 5 * uiScale);

        oc_matrix_pop();
    }

    oc_matrix_push(mat_transform(centerX, centerY, secondsRotation));
    {
        oc_set_color_rgba(1, 0.2, 0.2, 1);
        oc_rounded_rectangle_fill(0, -2.5 * uiScale, clockRadius * 0.8f, 5 * uiScale, 5 * uiScale);

        oc_matrix_pop();
    }

    oc_set_color_rgba(.2, 0.2, 0.2, 1);
    oc_circle_fill(centerX, centerY, 10 * uiScale);

    oc_render(surface, canvas);
    oc_surface_present(surface);
}

oc_mat2x3 mat_rotation(f32 radians)
{
    const f32 sinRot = sinf(radians);
    const f32 cosRot = cosf(radians);
    oc_mat2x3 rot = {
        cosRot, -sinRot, 0,
        sinRot, cosRot, 0
    };
    return rot;
}

oc_mat2x3 mat_translation(f32 x, f32 y)
{
    oc_mat2x3 translation = {
        1, 0, x,
        0, 1, y
    };
    return translation;
}

oc_mat2x3 mat_transform(f32 x, f32 y, f32 radians)
{
    oc_mat2x3 rotation = mat_rotation(radians);
    oc_mat2x3 translation = mat_translation(x, y);
    return oc_mat2x3_mul_m(translation, rotation);
}

f32 minf(f32 a, f32 b)
{
    return (a < b) ? a : b;
}
