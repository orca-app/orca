#include <stdio.h>
#include <errno.h>

#include "orca.h"
#include "warm.c"

//-------------------------------------------------------------------------
// main
//-------------------------------------------------------------------------

oc_font font_create(const char* resourcePath)
{
    //NOTE(martin): create default fonts
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 fontPath = oc_path_executable_relative(scratch.arena, OC_STR8(resourcePath));

    oc_font font = oc_font_nil();

    FILE* fontFile = fopen(fontPath.ptr, "r");
    if(!fontFile)
    {
        oc_log_error("Could not load font file '%s': %s\n", fontPath.ptr, strerror(errno));
    }
    else
    {
        char* fontData = 0;
        fseek(fontFile, 0, SEEK_END);
        u32 fontDataSize = ftell(fontFile);
        rewind(fontFile);
        fontData = malloc(fontDataSize);
        fread(fontData, 1, fontDataSize, fontFile);
        fclose(fontFile);

        oc_unicode_range ranges[5] = { OC_UNICODE_BASIC_LATIN,
                                       OC_UNICODE_C1_CONTROLS_AND_LATIN_1_SUPPLEMENT,
                                       OC_UNICODE_LATIN_EXTENDED_A,
                                       OC_UNICODE_LATIN_EXTENDED_B,
                                       OC_UNICODE_SPECIALS };

        font = oc_font_create_from_memory(oc_str8_from_buffer(fontDataSize, fontData), 5, ranges);

        free(fontData);
    }
    oc_scratch_end(scratch);
    return (font);
}

typedef struct app_data
{
    oc_window window;
    oc_canvas_renderer renderer;
    oc_surface surface;
    oc_canvas_context canvas;
    oc_font font;
    oc_ui_context ui;

    f32 fontSize;
    f32 indentW;

    oc_str8 contents;
    oc_arena* moduleArena;
    wa_module* module;
    wa_instance* instance;
} app_data;

static const f32 BOX_MARGIN_W = 2,
                 BOX_MARGIN_H = 2,
                 BOX_LINE_GAP = 4;

oc_str8 find_function_export_name(app_data* app, u32 funcIndex)
{
    oc_str8 res = { 0 };
    for(int exportIndex = 0; exportIndex < app->module->exportCount; exportIndex++)
    {
        wa_export* export = &app->module->exports[exportIndex];
        if(export->kind == WA_EXPORT_FUNCTION && export->index == funcIndex)
        {
            res = export->name;
            break;
        }
    }
    return (res);
}

void push_func_type_str8_list(oc_arena* arena, oc_str8_list* list, wa_func_type* type)
{
    oc_str8_list_push(arena, list, OC_STR8("("));
    for(u32 paramIndex = 0; paramIndex < type->paramCount; paramIndex++)
    {
        oc_str8_list_push(arena, list, OC_STR8(wa_value_type_string(type->params[paramIndex])));
        if(paramIndex < type->paramCount - 1)
        {
            oc_str8_list_push(arena, list, OC_STR8(" "));
        }
    }
    oc_str8_list_push(arena, list, OC_STR8(") -> ("));
    for(u32 returnIndex = 0; returnIndex < type->returnCount; returnIndex++)
    {
        oc_str8_list_push(arena, list, OC_STR8(wa_value_type_string(type->returns[returnIndex])));
        if(returnIndex < type->returnCount - 1)
        {
            oc_str8_list_push(arena, list, OC_STR8(" "));
        }
    }
    oc_str8_list_push(arena, list, OC_STR8(")"));
}

oc_str8 push_func_type_str8(oc_arena* arena, wa_func_type* type)
{
    oc_arena_scope scratch = oc_scratch_begin_next(arena);

    oc_str8_list list = { 0 };
    push_func_type_str8_list(scratch.arena, &list, type);
    oc_str8 str = oc_str8_list_join(arena, list);
    oc_scratch_end(scratch);

    return (str);
}

void build_bytecode_ui(app_data* app)
{
    oc_arena_scope scratch = oc_scratch_begin();

    wa_func* func = wa_instance_find_function(app->instance, OC_STR8("start"));

    for(u64 codeIndex = 0; codeIndex < func->codeLen; codeIndex++)
    {
        u64 startIndex = codeIndex;

        wa_code* c = &func->code[codeIndex];
        const wa_instr_info* info = &wa_instr_infos[c->opcode];

        oc_str8 key = oc_str8_pushf(scratch.arena, "0x%08llx", codeIndex);

        oc_ui_style_next(&(oc_ui_style){
                             .layout = {
                                 .axis = OC_UI_AXIS_X,
                                 .spacing = BOX_MARGIN_W * 5,
                                 .margin.x = BOX_MARGIN_W,
                                 .margin.y = BOX_MARGIN_H,
                                 .align = OC_UI_ALIGN_START,
                             },
                         },
                         OC_UI_STYLE_LAYOUT);

        oc_ui_container_str8(key, 0)
        {
            // address
            oc_ui_label_str8(key);

            // spacer
            oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, 10 * BOX_MARGIN_W } },
                             OC_UI_STYLE_SIZE_WIDTH);
            oc_ui_box_make("spacer", 0);

            // opcode
            oc_ui_label(wa_instr_strings[c->opcode]);

            // operands
            for(u32 opdIndex = 0; opdIndex < info->opdCount; opdIndex++)
            {
                wa_code* opd = &func->code[codeIndex + opdIndex + 1];
                oc_str8 opdKey = oc_str8_pushf(scratch.arena, "opd%u", opdIndex);

                oc_ui_container_str8(opdKey, 0)
                {
                    oc_str8 s = { 0 };

                    switch(info->opd[opdIndex])
                    {
                        case WA_OPD_CONST_I32:
                            s = oc_str8_pushf(scratch.arena, "%i", opd->valI32);
                            break;
                        case WA_OPD_CONST_I64:
                            s = oc_str8_pushf(scratch.arena, "%lli", opd->valI64);
                            break;
                        case WA_OPD_CONST_F32:
                            s = oc_str8_pushf(scratch.arena, "%f", opd->valF32);
                            break;
                        case WA_OPD_CONST_F64:
                            s = oc_str8_pushf(scratch.arena, "%f", opd->valF64);
                            break;

                        case WA_OPD_LOCAL_INDEX:
                            s = oc_str8_pushf(scratch.arena, "r%u", opd->valU32);
                            break;
                        case WA_OPD_JUMP_TARGET:
                            s = oc_str8_pushf(scratch.arena, "%+lli", opd->valI64);
                            break;
                        default:
                            s = oc_str8_pushf(scratch.arena, "0x%08llx", opd->valU64);
                            break;
                    }
                    oc_ui_label_str8(s);
                }
            }
        }

        codeIndex += info->opdCount;
        /*
            printf("0x%08llx ", codeIndex);
        printf("%-16s0x%02x ", wa_instr_strings[c->opcode], c->opcode);

        const wa_instr_info* info = &wa_instr_infos[c->opcode];

        for(u64 i = 0; i < info->opdCount; i++)
        {
            codeIndex++;
            if(codeIndex >= len)
            {
                break;
            }
            printf("0x%02llx ", bytecode[codeIndex].valI64);
        }

        if(c->opcode == WA_INSTR_jump_table)
        {
            printf("\n\t");
            u64 brCount = bytecode[startIndex + 1].valI32;
            for(u64 i = 0; i < brCount; i++)
            {
                codeIndex++;
                printf("0x%02llx ", bytecode[codeIndex].valI64);
            }
        }
        printf("\n");
        */
    }

    oc_scratch_end(scratch);
}

void update_ui(app_data* app)
{
    oc_ui_style defaultStyle = { .bgColor = { 0 },
                                 .color = { 0, 0, 0, 1 },
                                 .font = app->font,
                                 .fontSize = app->fontSize,
                                 .borderColor = { 1, 0, 0, 1 },
                                 .borderSize = 2 };

    oc_ui_style_mask defaultMask = OC_UI_STYLE_BG_COLOR
                                 | OC_UI_STYLE_COLOR
                                 | OC_UI_STYLE_BORDER_COLOR
                                 | OC_UI_STYLE_BORDER_SIZE
                                 | OC_UI_STYLE_FONT
                                 | OC_UI_STYLE_FONT_SIZE;

    oc_vec2 frameSize = oc_surface_get_size(app->surface);

    oc_ui_set_theme(&OC_UI_LIGHT_THEME);
    oc_ui_frame(frameSize, &defaultStyle, defaultMask)
    {
        if(app->instance)
        {
            oc_ui_panel("boxtree", 0)
            {

                oc_ui_style_next(&(oc_ui_style){
                                     .size = {
                                         .width = { OC_UI_SIZE_PARENT, 1 },
                                         .height = { OC_UI_SIZE_PARENT, 1 },
                                     },
                                 },
                                 OC_UI_STYLE_SIZE);

                oc_ui_container("contents", 0)
                {
                    build_bytecode_ui(app);
                }
            }
        }
        else
        {
            oc_ui_style_next(&(oc_ui_style){
                                 .size = {
                                     .width = { OC_UI_SIZE_PARENT, 1 },
                                     .height = { OC_UI_SIZE_PARENT, 1 },
                                 },
                                 .layout.align = { OC_UI_ALIGN_CENTER, OC_UI_ALIGN_CENTER },
                             },
                             OC_UI_STYLE_SIZE | OC_UI_STYLE_LAYOUT_ALIGN_X | OC_UI_STYLE_LAYOUT_ALIGN_Y);

            oc_ui_container("droppanel", 0)
            {
                oc_ui_style_next(&(oc_ui_style){
                                     .color = { 0, 0, 0, 1 },
                                 },
                                 OC_UI_STYLE_COLOR);

                oc_ui_label("Drop a Wasm Module Here");
            }
        }
    }

    oc_ui_draw();

    oc_canvas_render(app->renderer, app->canvas, app->surface);
    oc_canvas_present(app->renderer, app->surface);
}

void load_module(app_data* app, oc_str8 modulePath)
{
    //NOTE: unload previous module
    app->module = 0;
    app->instance = 0;
    app->contents = (oc_str8){ 0 };

    oc_arena_clear(app->moduleArena);

    //NOTE: load module
    oc_file file = oc_file_open(modulePath, OC_FILE_ACCESS_READ, OC_FILE_OPEN_NONE);

    app->contents.len = oc_file_size(file);
    app->contents.ptr = oc_arena_push(app->moduleArena, app->contents.len);

    oc_file_read(file, app->contents.len, app->contents.ptr);
    oc_file_close(file);

    app->module = wa_module_create(app->moduleArena, app->contents);

    if(!oc_list_empty(app->module->errors))
    {
        wa_module_print_errors(app->module);
        //TODO: display / handle errors

        app->module = 0;
        oc_arena_clear(app->moduleArena);
    }
    else
    {
        wa_ast_print(app->module->root, app->contents);
        wa_print_code(app->module);

        app->instance = wa_instance_create(app->moduleArena, app->module, &(wa_instance_options){ 0 });
        if(wa_instance_status(app->instance) != WA_OK)
        {
            printf("Couldn't instantiate module: %s.\n", wa_status_string(wa_instance_status(app->instance)));
            app->instance = 0;
            oc_arena_clear(app->moduleArena);
        }
    }
}

int main(int argc, char** argv)
{
    oc_arena arena = { 0 };
    oc_arena_init(&arena);

    //Create window
    oc_init();

    app_data app = {
        .moduleArena = &arena,
    };

    oc_rect windowRect = { .x = 100, .y = 100, .w = 810, .h = 610 };
    app.window = oc_window_create(windowRect, OC_STR8("wastep"), 0);

    app.renderer = oc_canvas_renderer_create();
    app.surface = oc_canvas_surface_create_for_window(app.renderer, app.window);
    app.canvas = oc_canvas_context_create();

    app.font = font_create("../resources/Menlo.ttf");
    app.fontSize = 16;
    oc_text_metrics metrics = oc_font_text_metrics(app.font, app.fontSize, OC_STR8("x"));
    app.indentW = 2 * metrics.advance.x;

    oc_ui_init(&app.ui);

    if(argc >= 2)
    {
        load_module(&app, OC_STR8(argv[1]));
    }

    oc_window_bring_to_front(app.window);
    oc_window_focus(app.window);
    oc_window_center(app.window);

    while(!oc_should_quit())
    {

        oc_pump_events(0);
        //TODO: what to do with mem scratch here?

        oc_arena_scope scratch = oc_scratch_begin();
        oc_event* event = 0;
        while((event = oc_next_event(scratch.arena)) != 0)
        {
            oc_ui_process_event(event);

            switch(event->type)
            {
                case OC_EVENT_WINDOW_CLOSE:
                case OC_EVENT_QUIT:
                {
                    oc_request_quit();
                }
                break;

                case OC_EVENT_PATHDROP:
                {
                    oc_str8 path = oc_str8_list_first(event->paths);
                    load_module(&app, path);
                }
                break;

                default:
                    break;
            }
        }

        update_ui(&app);

        oc_scratch_end(scratch);
    }

    return (0);
}
