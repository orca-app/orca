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
#include <math.h>

#include "orca.h"
#include "graphics/wgpu_renderer_debug.h"
//------------------------------------------------------------------------------------------
// Test struct
//------------------------------------------------------------------------------------------
typedef struct perf_test perf_test;

typedef perf_test (*test_init_proc)(oc_arena* arena, oc_rect contentRect, oc_canvas_renderer renderer, int argc, char** argv);
typedef void (*test_draw_proc)(void* user);

struct perf_test
{
    test_draw_proc draw;
    void* data;
};

//------------------------------------------------------------------------------------------
// Triangle Test
//------------------------------------------------------------------------------------------
typedef struct shape_info
{
    oc_vec2 pos;
    oc_color col;
} shape_info;

typedef struct shape_test_data
{
    u32 count;
    f32 size;
    bool stroke;
    bool textured;
    bool translucent;
    oc_image image;
    shape_info* shapes;

} shape_test_data;

void triangles_draw(void* user);
void circles_draw(void* user);

perf_test shapes_init(oc_arena* arena,
                      oc_rect contentRect,
                      oc_canvas_renderer renderer,
                      int argc,
                      char** argv)
{
    shape_test_data* data = oc_arena_push_type(arena, shape_test_data);
    memset(data, 0, sizeof(shape_test_data));

    //defaults
    test_draw_proc draw = triangles_draw;
    data->count = 10000;
    data->size = 30;

    bool error = false;
    for(int argIndex = 0; argIndex < argc; argIndex++)
    {
        if(!strcmp(argv[argIndex], "-c") || !strcmp(argv[argIndex], "--count"))
        {
            if(argIndex + 1 >= argc || argv[argIndex + 1][0] == '-')
            {
                oc_log_error("option %s needs an argument\n", argv[argIndex]);
                error = true;
                break;
            }
            argIndex++;
            char* end = 0;
            data->count = strtoul(argv[argIndex], &end, 10);
            if(end == argv[argIndex] || end[0] != '\0')
            {
                oc_log_error("option %s should be an integer\n", argv[argIndex - 1]);
                error = true;
                break;
            }
        }
        else if(!strcmp(argv[argIndex], "--shape"))
        {
            if(argIndex + 1 >= argc || argv[argIndex + 1][0] == '-')
            {
                oc_log_error("option %s needs an argument\n", argv[argIndex]);
                error = true;
                break;
            }
            argIndex++;
            if(!strcmp(argv[argIndex], "triangle"))
            {
                draw = triangles_draw;
            }
            else if(!strcmp(argv[argIndex], "circle"))
            {
                draw = circles_draw;
            }
            else
            {
                oc_log_error("unrecognized argument for option %s\n", argv[argIndex - 1]);
                error = true;
                break;
            }
        }
        else if(!strcmp(argv[argIndex], "-s") || !strcmp(argv[argIndex], "--size"))
        {
            if(argIndex + 1 >= argc || argv[argIndex + 1][0] == '-')
            {
                oc_log_error("option %s needs an argument\n", argv[argIndex]);
                error = true;
                break;
            }
            argIndex++;
            char* end = 0;
            data->size = strtoul(argv[argIndex], &end, 10);
            if(end == argv[argIndex] || end[0] != '\0')
            {
                oc_log_error("option %s should be an integer\n", argv[argIndex - 1]);
                error = true;
                break;
            }
        }
        else if(!strcmp(argv[argIndex], "--stroke"))
        {
            data->stroke = true;
        }
        else if(!strcmp(argv[argIndex], "--fill"))
        {
            data->stroke = false;
        }
        else if(!strcmp(argv[argIndex], "--textured"))
        {
            data->textured = true;
        }
        else if(!strcmp(argv[argIndex], "--translucent"))
        {
            data->translucent = true;
        }
    }
    if(error)
    {
        return ((perf_test){ 0 });
    }

    data->shapes = oc_arena_push_array(arena, shape_info, data->count);
    for(int i = 0; i < data->count; i++)
    {
        data->shapes[i] = (shape_info){
            .pos = {
                (rand() % (u32)contentRect.w),
                (rand() % (u32)contentRect.h),
            },
            .col = {
                (rand() % 255) / 255.,
                (rand() % 255) / 255.,
                (rand() % 255) / 255.,
                1,
            },
        };
    }

    if(data->textured)
    {
        data->image = oc_image_create_from_path(renderer, OC_STR8("./resources/checkerboard.png"), false);
    }

    perf_test test = { .draw = draw, .data = data };
    return (test);
}

void triangles_draw(void* user)
{
    shape_test_data* data = (shape_test_data*)user;

    const oc_vec2 triangleVertices[3] = {
        { 0, -data->size },
        { data->size * cosf(-1 * M_PI / 6), -data->size * sinf(-1 * M_PI / 6) },
        { data->size * cosf(-5 * M_PI / 6), -data->size * sinf(-5 * M_PI / 6) },
    };

    oc_set_color_rgba(0, 1, 1, 1);
    oc_clear();

    for(int i = 0; i < data->count; i++)
    {
        shape_info* shape = &data->shapes[i];
        oc_move_to(shape->pos.x + triangleVertices[0].x, shape->pos.y + triangleVertices[0].y);
        oc_line_to(shape->pos.x + triangleVertices[1].x, shape->pos.y + triangleVertices[1].y);
        oc_line_to(shape->pos.x + triangleVertices[2].x, shape->pos.y + triangleVertices[2].y);
        oc_close_path();

        oc_color col = shape->col;
        if(data->translucent)
        {
            col.a = 0.5;
        }
        oc_set_color(col);

        if(data->stroke)
        {
            oc_stroke();
        }
        else
        {
            if(data->textured)
            {
                oc_set_image(data->image);
            }
            oc_fill();
        }
    }
}

void circles_draw(void* user)
{
    shape_test_data* data = (shape_test_data*)user;

    oc_set_color_rgba(0, 1, 1, 1);
    oc_clear();

    for(int i = 0; i < data->count; i++)
    {
        shape_info* shape = &data->shapes[i];

        oc_color col = shape->col;
        if(data->translucent)
        {
            col.a = 0.5;
        }
        oc_set_color(col);

        if(data->stroke)
        {
            oc_circle_stroke(shape->pos.x, shape->pos.y, data->size);
        }
        else
        {
            if(data->textured)
            {
                oc_set_image(data->image);
            }
            oc_circle_fill(shape->pos.x, shape->pos.y, data->size);
        }
    }
}

//------------------------------------------------------------------------------------------
// Full screen quads Test
//------------------------------------------------------------------------------------------

typedef struct full_screen_quads_data
{
    oc_vec2 size;
    u32 count;
    bool textured;
    bool translucent;
    oc_image image;

} full_screen_quads_data;

void full_screen_quads_draw(void* user);

perf_test full_screen_quads_init(oc_arena* arena,
                                 oc_rect contentRect,
                                 oc_canvas_renderer renderer,
                                 int argc,
                                 char** argv)
{
    full_screen_quads_data* data = oc_arena_push_type(arena, full_screen_quads_data);
    memset(data, 0, sizeof(full_screen_quads_data));

    //defaults
    data->size = (oc_vec2){ contentRect.w, contentRect.h };
    data->count = 10000;

    bool error = false;
    for(int argIndex = 0; argIndex < argc; argIndex++)
    {
        if(!strcmp(argv[argIndex], "-c") || !strcmp(argv[argIndex], "--count"))
        {
            if(argIndex + 1 >= argc || argv[argIndex + 1][0] == '-')
            {
                oc_log_error("option %s needs an argument\n", argv[argIndex]);
                error = true;
                break;
            }
            argIndex++;
            char* end = 0;
            data->count = strtoul(argv[argIndex], &end, 10);
            if(end == argv[argIndex] || end[0] != '\0')
            {
                oc_log_error("option %s should be an integer\n", argv[argIndex - 1]);
                error = true;
                break;
            }
        }
        else if(!strcmp(argv[argIndex], "--textured"))
        {
            data->textured = true;
        }
        else if(!strcmp(argv[argIndex], "--translucent"))
        {
            data->translucent = true;
        }
    }
    if(error)
    {
        return ((perf_test){ 0 });
    }

    if(data->textured)
    {
        data->image = oc_image_create_from_path(renderer, OC_STR8("./resources/checkerboard.png"), false);
    }

    perf_test test = { .draw = full_screen_quads_draw, .data = data };
    return (test);
}

void full_screen_quads_draw(void* user)
{
    full_screen_quads_data* data = (full_screen_quads_data*)user;

    oc_set_color_rgba(0, 1, 1, 1);
    oc_clear();

    for(int i = 0; i < data->count; i++)
    {
        oc_color col = { 1, 0, 0, 1 };
        if(data->translucent)
        {
            col.a = 0.5;
        }
        oc_set_color(col);

        if(data->textured)
        {
            oc_set_image(data->image);
        }
        oc_rectangle_fill(0, 0, data->size.x, data->size.y);
    }
}

//------------------------------------------------------------------------------------------
// Image batches Test
//------------------------------------------------------------------------------------------

typedef struct image_batches_data
{
    oc_vec2 size;
    u32 count;
    oc_vec2* pos;
    u32 imageCount;
    oc_image* images;
    oc_color* tints;

} image_batches_data;

void image_batches_draw(void* user);

perf_test image_batches_init(oc_arena* arena,
                             oc_rect contentRect,
                             oc_canvas_renderer renderer,
                             int argc,
                             char** argv)
{
    image_batches_data* data = oc_arena_push_type(arena, image_batches_data);
    memset(data, 0, sizeof(image_batches_data));

    //defaults
    data->size = (oc_vec2){ 100, 100 };
    data->count = 1000;
    data->imageCount = 20;

    bool error = false;
    for(int argIndex = 0; argIndex < argc; argIndex++)
    {
        if(!strcmp(argv[argIndex], "-c") || !strcmp(argv[argIndex], "--count"))
        {
            if(argIndex + 1 >= argc || argv[argIndex + 1][0] == '-')
            {
                oc_log_error("option %s needs an argument\n", argv[argIndex]);
                error = true;
                break;
            }
            argIndex++;
            char* end = 0;
            data->count = strtoul(argv[argIndex], &end, 10);
            if(end == argv[argIndex] || end[0] != '\0')
            {
                oc_log_error("option %s should be an integer\n", argv[argIndex - 1]);
                error = true;
                break;
            }
        }
        if(!strcmp(argv[argIndex], "-i") || !strcmp(argv[argIndex], "--image-count"))
        {
            if(argIndex + 1 >= argc || argv[argIndex + 1][0] == '-')
            {
                oc_log_error("option %s needs an argument\n", argv[argIndex]);
                error = true;
                break;
            }
            argIndex++;
            char* end = 0;
            data->imageCount = strtoul(argv[argIndex], &end, 10);
            if(end == argv[argIndex] || end[0] != '\0')
            {
                oc_log_error("option %s should be an integer\n", argv[argIndex - 1]);
                error = true;
                break;
            }
        }
    }
    if(error)
    {
        return ((perf_test){ 0 });
    }

    data->pos = oc_arena_push_array(arena, oc_vec2, data->count);
    for(int i = 0; i < data->count; i++)
    {
        data->pos[i] = (oc_vec2){
            rand() % ((u32)contentRect.w),
            rand() % ((u32)contentRect.h),
        };
    }

    data->images = oc_arena_push_array(arena, oc_image, data->imageCount);
    data->tints = oc_arena_push_array(arena, oc_color, data->imageCount);
    for(int i = 0; i < data->imageCount; i++)
    {
        data->images[i] = oc_image_create_from_path(renderer, OC_STR8("./resources/checkerboard.png"), false);
        data->tints[i] = (oc_color){
            (rand() % 255) / 255.,
            (rand() % 255) / 255.,
            (rand() % 255) / 255.,
            1,
        };
    }

    perf_test test = { .draw = image_batches_draw, .data = data };
    return (test);
}

void image_batches_draw(void* user)
{
    image_batches_data* data = (image_batches_data*)user;

    oc_set_color_rgba(0, 1, 1, 1);
    oc_clear();

    for(int i = 0; i < data->count; i++)
    {
        OC_ASSERT(!oc_image_is_nil(data->images[i % data->imageCount]));

        oc_set_color(data->tints[i % data->imageCount]);
        oc_set_image(data->images[i % data->imageCount]);
        oc_rectangle_fill(data->pos[i].x, data->pos[i].y, data->size.x, data->size.y);
    }
}

//------------------------------------------------------------------------------------------
// Wall of Text Test
//------------------------------------------------------------------------------------------

static const char* TEST_STRING =
    "Nadie lo vio desembarcar en la unánime noche, nadie vio la canoa de bambú sumiéndose en el fango sagrado,"
    "pero a los pocos días nadie ignoraba que el hombre taciturno venía del Sur y que su patria era una de las infinitas"
    "aldeas que están aguas arriba, en el flanco violento de la montaña, donde el idioma zend no está contaminado de"
    "griego y donde es infrecuente la lepra.Lo cierto es que el hombre gris besó el fango, repechó la ribera sin apartar"
    "(probablemente, sin sentir) las cortaderas que le dilaceraban las carnes y se arrastró, mareado y ensangrentado,"
    "hasta el recinto circular que corona un tigre o caballo de piedra, que tuvo alguna vez el color del fuego y ahora"
    "el de la ceniza.Ese redondel es un templo que devoraron los incendios antiguos, que la selva palúdica ha profanado"
    "y cuyo dios no recibe honor de los hombres.El forastero se tendió bajo el pedestal.Lo despertó el sol alto.Comprobó"
    "sin asombro que las heridas habían cicatrizado; cerró los ojos pálidos y durmió, no por flaqueza de la carne sino"
    "por determinación de la voluntad. Sabía que ese templo era el lugar que requería su invencible propósito; sabía que los"
    "árboles incesantes no habían logrado estrangular, río abajo, las ruinas de otro templo propicio, también de dioses"
    "incendiados y muertos; sabía que su inmediata obligación era el sueño.Hacia la medianoche lo despertó el grito"
    "inconsolable de un pájaro.Rastros de pies descalzos, unos higos y un cántaro le advirtieron que los hombres de la región"
    "habían espiado con respeto su sueño y solicitaban su amparo o temían su magia.Sintió el frío del miedo y buscó en la"
    "muralla dilapidada un nicho sepulcral y se tapó con hojas desconocidas.El propósito que lo guiaba no era imposible,"
    "aunque sí sobrenatural.Quería soñar un hombre : quería soñarlo con integridad minuciosa e imponerlo a la realidad."
    "Ese proyecto mágico había agotado el espacio entero de su alma; si alguien le hubiera preguntado su propio nombre o"
    "cualquier rasgo de su vida anterior, no habría acertado a responder.Le convenía el templo inhabitado y despedazado,"
    "porque era un mínimo de mundo visible; la cercanía de los leñadores también, porque éstos se encargaban de subvenir a sus"
    "necesidades frugales.El arroz y las frutas de su tributo eran pábulo suficiente para su cuerpo, consagrado a la única"
    "tarea de dormir y soñar.1 Al principio, los sueños eran caóticos; poco después, fueron de naturaleza dialéctica.El"
    "forastero se soñaba en el centro de un anfiteatro circular que era de algún modo el templo incendiado : nubes de alumnos"
    "taciturnos fatigaban las gradas; las caras de los últimos pendían a muchos siglos de distancia y a una altura estelar,"
    "pero eran del todo precisas.El hombre les dictaba lecciones de anatomía, de cosmografía, de magia : los rostros"
    "escuchaban con ansiedad y procuraban responder con entendimiento, como si adivinaran la importancia de aquel examen,"
    "que redimiría a uno de ellos de su condición de vana apariencia y lo interpolaría en el mundo real.El hombre, en el sueño"
    "y en la vigilia, consideraba las respuestas de sus fantasmas, no se dejaba embaucar por los impostores, adivinaba en"
    "ciertas perplejidades una inteligencia creciente.Buscaba un alma que mereciera participar en el universo. A las nueve"
    "o diez noches comprendió con alguna amargura que nada podía esperar de aquellos alumnos que aceptaban con pasividad su"
    "doctrina y sí de aquellos que arriesgaban, a veces, una contradicción razonable. Los primeros, aunque dignos de amor y"
    "de buen afecto, no podían ascender a individuos; los últimos preexistían un poco más. Una tarde (ahora también las tardes"
    "eran tributarias del sueño, ahora no velaba sino un par de horas en el amanecer) licenció para siempre el vasto colegio"
    "ilusorio y se quedó con un solo alumno.";

typedef struct wall_text_data
{
    oc_font font;
    f32 fontSize;
    f32 lineHeight;

    oc_str32 codePoints;
    u32 glyphCount;

} wall_text_data;

void wall_text_draw(void* user);

perf_test wall_text_init(oc_arena* arena,
                         oc_rect contentRect,
                         oc_canvas_renderer renderer,
                         int argc,
                         char** argv)
{
    wall_text_data* data = oc_arena_push_type(arena, wall_text_data);
    memset(data, 0, sizeof(wall_text_data));

    //defaults
    oc_unicode_range ranges[5] = { OC_UNICODE_BASIC_LATIN,
                                   OC_UNICODE_C1_CONTROLS_AND_LATIN_1_SUPPLEMENT,
                                   OC_UNICODE_LATIN_EXTENDED_A,
                                   OC_UNICODE_LATIN_EXTENDED_B,
                                   OC_UNICODE_SPECIALS };

    data->font = oc_font_create_from_path(OC_STR8("./resources/CMUSerif-Roman.ttf"), 5, ranges);

    if(oc_font_is_nil(data->font))
    {
        return ((perf_test){ 0 });
    }
    data->fontSize = 14;

    oc_font_metrics extents = oc_font_get_metrics_unscaled(data->font);
    f32 fontScale = oc_font_get_scale_for_em_pixels(data->font, data->fontSize);
    data->lineHeight = fontScale * (extents.ascent + extents.descent + extents.lineGap);

    data->codePoints.len = oc_utf8_codepoint_count_for_string(OC_STR8((char*)TEST_STRING));
    data->codePoints.ptr = oc_arena_push_array(arena, oc_utf32, data->codePoints.len);
    oc_utf8_to_codepoints(data->codePoints.len, data->codePoints.ptr, OC_STR8((char*)TEST_STRING));

    for(int i = 0; i < data->codePoints.len; i++)
    {
        if(data->codePoints.ptr[i] != ' ' && data->codePoints.ptr[i] != '\n')
        {
            data->glyphCount++;
        }
    }

    perf_test test = { .draw = wall_text_draw, .data = data };
    return (test);
}

void wall_text_draw(void* user)
{
    wall_text_data* data = (wall_text_data*)user;

    oc_set_color_rgba(1, 1, 1, 1);
    oc_clear();

    oc_set_font(data->font);
    oc_set_font_size(data->fontSize);
    oc_set_color_rgba(0, 0, 0, 1);

    u32 textX = 10;
    u32 textY = 10;

    oc_move_to(textX, textY);

    int startIndex = 0;
    while(startIndex < data->codePoints.len)
    {
        bool lineBreak = false;
        int subIndex = 0;
        for(; (startIndex + subIndex) < data->codePoints.len && subIndex < 120; subIndex++)
        {
            if(data->codePoints.ptr[startIndex + subIndex] == '\n')
            {
                subIndex++;
                break;
            }
        }

        u32 glyphs[512];
        oc_font_get_glyph_indices(data->font, oc_str32_from_buffer(subIndex, data->codePoints.ptr + startIndex), oc_str32_from_buffer(512, glyphs));

        oc_glyph_outlines(oc_str32_from_buffer(subIndex, glyphs));
        oc_fill();

        textY += data->lineHeight;
        oc_move_to(textX, textY);

        startIndex += subIndex;
    }
}

//------------------------------------------------------------------------------------------
// Svg Test
//------------------------------------------------------------------------------------------

#include "tiger_cmd.c"

void svg_draw_tiger(void* user);

perf_test svg_init(oc_arena* arena,
                   oc_rect contentRect,
                   oc_canvas_renderer renderer,
                   int argc,
                   char** argv)
{
    //TODO: later add other test svg
    test_draw_proc draw = svg_draw_tiger;
    perf_test test = { .draw = draw, .data = 0 };
    return (test);
}

void svg_draw_tiger(void* user)
{
    oc_set_color_rgba(0, 1, 1, 1);
    oc_clear();

    f32 startX = 300, startY = 200;
    oc_matrix_multiply_push((oc_mat2x3){ 1, 0, startX,
                                         0, 1, startY });

    draw_tiger();

    oc_matrix_pop();
}

//------------------------------------------------------------------------------------------
// Tests table
//------------------------------------------------------------------------------------------

typedef struct test_entry
{
    const char* name;
    test_init_proc init;

} test_entry;

#define TEST(t)                               \
    {                                         \
        .name = #t, .init = OC_CAT2(t, _init) \
    }

test_entry TESTS[] = {
    TEST(shapes),
    TEST(full_screen_quads),
    TEST(image_batches),
    TEST(wall_text),
    TEST(svg),
};
const u32 TEST_COUNT = sizeof(TESTS) / sizeof(perf_test);

//------------------------------------------------------------------------------------------
// Driver
//------------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    oc_log_set_level(OC_LOG_LEVEL_WARNING);
    bool autoMode = false;
    bool csvOutput = false;
    int autoCount = 100;
    int argIndex = 1;
    for(; argIndex < argc; argIndex++)
    {
        if(!strcmp(argv[argIndex], "--auto"))
        {
            autoMode = true;
            argIndex++;
            if(argIndex >= argc)
            {
                oc_log_error("option %s should have an integer argument\n", argv[argIndex - 1]);
            }
            char* end = 0;
            autoCount = strtoul(argv[argIndex], &end, 10);
            if(end == argv[argIndex] || end[0] != '\0')
            {
                oc_log_error("option %s should have an integer argument\n", argv[argIndex - 1]);
                return (-1);
            }
        }
        else if(!strcmp(argv[argIndex], "--csv"))
        {
            csvOutput = true;
        }
        else
        {
            break;
        }
    }

    if(argIndex >= argc)
    {
        oc_log_error("test name required\n");
        return (-1);
    }
    const char* testName = argv[argIndex];
    argIndex++;

    test_entry* testEntry = 0;
    for(int i = 0; i < TEST_COUNT; i++)
    {
        if(!strcmp(TESTS[i].name, testName))
        {
            testEntry = &TESTS[i];
            break;
        }
    }

    if(!testEntry)
    {
        oc_log_error("test %s not found\n", testName);
        return (-1);
    }

    oc_arena_scope programScratch = oc_scratch_begin();

    //NOTE: create window renderer, surface, and context
    oc_init();

    oc_rect windowRect = { .x = 100, .y = 100, .w = 800, .h = 600 };
    oc_window window = oc_window_create(windowRect, OC_STR8("renderer perf tests"), 0);
    oc_rect contentRect = oc_window_get_content_rect(window);

    oc_canvas_renderer renderer = oc_canvas_renderer_create();
    if(oc_canvas_renderer_is_nil(renderer))
    {
        oc_log_error("Error: couldn't create renderer\n");
        return (-1);
    }

    oc_wgpu_canvas_debug_set_record_options(
        renderer,
        &(oc_wgpu_canvas_record_options){
            .maxRecordCount = autoCount,
            .timingFlags = OC_WGPU_CANVAS_TIMING_FRAME,
        });

    oc_surface surface = oc_canvas_surface_create_for_window(renderer, window);
    if(oc_surface_is_nil(surface))
    {
        oc_log_error("Error: couldn't create surface\n");
        return (-1);
    }

    oc_canvas_context context = oc_canvas_context_create();
    if(oc_canvas_context_is_nil(context))
    {
        oc_log_error("Error: couldn't create canvas\n");
        return (-1);
    }

    // start app
    oc_window_bring_to_front(window);
    oc_window_focus(window);

    f64 frameTime = 0;

    const u32 SHAPE_COUNT = 100000;

    perf_test test = testEntry->init(programScratch.arena, contentRect, renderer, argc - argIndex, argv + argIndex);
    if(!test.draw)
    {
        return (-1);
    }

    u32 frameCount = 0;
    u32 warmupCount = 180;

    while(!oc_should_quit())
    {
        oc_arena_scope frameScratch = oc_scratch_begin();
        f64 startTime = oc_clock_time(OC_CLOCK_MONOTONIC);

        oc_pump_events(0);
        oc_event* event = 0;
        while((event = oc_next_event(frameScratch.arena)) != 0)
        {
            switch(event->type)
            {
                case OC_EVENT_WINDOW_CLOSE:
                {
                    oc_request_quit();
                }
                break;

                default:
                    break;
            }
        }

        test.draw(test.data);

        oc_canvas_render(renderer, context, surface);
        oc_canvas_present(renderer, surface);

        oc_scratch_end(frameScratch);

        frameCount++;

        oc_wgpu_canvas_frame_stats stats = oc_wgpu_canvas_get_frame_stats(renderer, autoCount);

        if(autoMode && frameCount > autoCount + warmupCount)
        {
            if(stats.cpuEncodeTime.sampleCount >= autoCount)
            {
                break;
            }
        }
        else if(!autoMode)
        {
            printf("GPU Frame Time\n"
                   "  smp: %llu\n"
                   "  min: %.2fms\n"
                   "  max: %.2fms\n"
                   "  avg: %.2fms\n"
                   "  std: %.2f\n"
                   "CPU Encode Time\n"
                   "  smp: %llu\n"
                   "  min: %.2fms\n"
                   "  max: %.2fms\n"
                   "  avg: %.2fms\n"
                   "  std: %.2f\n"
                   "CPU Frame Time\n"
                   "  smp: %llu\n"
                   "  min: %.2fms (%.0ffps)\n"
                   "  max: %.2fms (%.0ffps)\n"
                   "  avg: %.2fms (%.0ffps)\n"
                   "  std: %.2f\n",
                   stats.gpuTime.sampleCount,
                   stats.gpuTime.minSample,
                   stats.gpuTime.maxSample,
                   stats.gpuTime.avg,
                   stats.gpuTime.std,
                   stats.cpuEncodeTime.sampleCount,
                   stats.cpuEncodeTime.minSample,
                   stats.cpuEncodeTime.maxSample,
                   stats.cpuEncodeTime.avg,
                   stats.cpuEncodeTime.std,
                   stats.cpuFrameTime.sampleCount,
                   stats.cpuFrameTime.minSample,
                   1000 / (stats.cpuFrameTime.minSample),
                   stats.cpuFrameTime.maxSample,
                   1000 / (stats.cpuFrameTime.maxSample),
                   stats.cpuFrameTime.avg,
                   1000 / (stats.cpuFrameTime.avg),
                   stats.cpuFrameTime.std);
        }
    }

    if(autoMode)
    {
        oc_wgpu_canvas_frame_stats stats = oc_wgpu_canvas_get_frame_stats(renderer, autoCount);

        if(csvOutput)
        {
            printf("%llu ; %.2f ; %.2f ; %.2f ; %.2f ; %llu ; %.2f ; %.2f ; %.2f ; %.2f ; %llu ; %.2f ; %.2f ; %.2f ; %.2f \n",
                   stats.gpuTime.sampleCount,
                   stats.gpuTime.minSample,
                   stats.gpuTime.maxSample,
                   stats.gpuTime.avg,
                   stats.gpuTime.std,
                   stats.cpuEncodeTime.sampleCount,
                   stats.cpuEncodeTime.minSample,
                   stats.cpuEncodeTime.maxSample,
                   stats.cpuEncodeTime.avg,
                   stats.cpuEncodeTime.std,
                   stats.cpuFrameTime.sampleCount,
                   stats.cpuFrameTime.minSample,
                   stats.cpuFrameTime.maxSample,
                   stats.cpuFrameTime.avg,
                   stats.cpuFrameTime.std);
        }
        else
        {
            printf("GPU Frame Time\n"
                   "  smp: %llu\n"
                   "  min: %.2fms\n"
                   "  max: %.2fms\n"
                   "  avg: %.2fms\n"
                   "  std: %.2f\n"
                   "CPU Encode Time\n"
                   "  smp: %llu\n"
                   "  min: %.2fms\n"
                   "  max: %.2fms\n"
                   "  avg: %.2fms\n"
                   "  std: %.2f\n"
                   "CPU Frame Time\n"
                   "  smp: %llu\n"
                   "  min: %.2fms (%.0ffps)\n"
                   "  max: %.2fms (%.0ffps)\n"
                   "  avg: %.2fms (%.0ffps)\n"
                   "  std: %.2f\n",
                   stats.gpuTime.sampleCount,
                   stats.gpuTime.minSample,
                   stats.gpuTime.maxSample,
                   stats.gpuTime.avg,
                   stats.gpuTime.std,
                   stats.cpuEncodeTime.sampleCount,
                   stats.cpuEncodeTime.minSample,
                   stats.cpuEncodeTime.maxSample,
                   stats.cpuEncodeTime.avg,
                   stats.cpuEncodeTime.std,
                   stats.cpuFrameTime.sampleCount,
                   stats.cpuFrameTime.minSample,
                   1000 / stats.cpuFrameTime.minSample,
                   stats.cpuFrameTime.maxSample,
                   1000 / stats.cpuFrameTime.maxSample,
                   stats.cpuFrameTime.avg,
                   1000 / stats.cpuFrameTime.avg,
                   stats.cpuFrameTime.std);
        }
    }

    oc_canvas_context_destroy(context);
    oc_surface_destroy(surface);
    oc_canvas_renderer_destroy(renderer);
    //    oc_window_destroy(window);

    oc_terminate();

    return (0);
}
