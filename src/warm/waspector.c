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

typedef struct wa_box
{
    oc_list_elt listElt;
    oc_list children;

    wa_ast* ast;
    oc_str8 string;
    oc_str8 keyString;
    oc_str8 addrString;
    oc_str8 bytesString;

    oc_vec2 textOffset;
    oc_rect rect;
    oc_rect childrenRect;
    oc_rect addrRect;

} wa_box;

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
    wa_box* rootBox;

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

wa_box* build_ast_boxes(oc_arena* arena, app_data* app, wa_ast* ast, oc_vec2 pos)
{
    wa_box* box = oc_arena_push_type(arena, wa_box);
    memset(box, 0, sizeof(wa_box));

    box->ast = ast;
    box->keyString = oc_str8_pushf(arena, "%p", ast);

    box->addrString = oc_str8_pushf(arena, "0x%08x", ast->loc.start);
    {
        oc_text_metrics metrics = oc_font_text_metrics(app->font, app->fontSize, box->addrString);
        box->addrRect = (oc_rect){
            BOX_MARGIN_W,
            pos.y,
            metrics.logical.w + 2 * BOX_MARGIN_W,
            metrics.logical.h + 2 * BOX_MARGIN_H,
        };
    }

    oc_arena_scope scratch = oc_scratch_begin_next(arena);
    oc_str8_list strList = { 0 };

    if(ast->kind == WA_AST_FUNC)
    {
        wa_func* func = ast->func;
        u32 funcIndex = func - app->module->functions;
        oc_str8 name = find_function_export_name(app, funcIndex);
        if(name.len)
        {
            oc_str8_list_pushf(scratch.arena, &strList, "function %.*s ", oc_str8_ip(name));
        }
        push_func_type_str8_list(scratch.arena, &strList, func->type);
    }
    else if(ast->kind == WA_AST_FUNC_INDEX)
    {
        oc_str8 name = find_function_export_name(app, ast->valU32);
        if(name.len)
        {
            oc_str8_list_pushf(scratch.arena, &strList, "\"%.*s\"", oc_str8_ip(name));
        }
    }
    else if(ast->kind == WA_AST_TYPE)
    {
        wa_func_type* type = ast->type;
        push_func_type_str8_list(scratch.arena, &strList, type);
    }
    else if(ast->kind == WA_AST_INSTR)
    {
        oc_str8_list_pushf(scratch.arena, &strList, "%s", wa_instr_strings[ast->instr->op]);
    }

    if(oc_list_empty(strList.list))
    {
        if(ast->label.len)
        {
            oc_str8_list_pushf(scratch.arena,
                               &strList,
                               "%.*s",
                               oc_str8_ip(ast->label));
        }
        else
        {
            oc_str8_list_pushf(scratch.arena,
                               &strList,
                               "[%s]",
                               wa_ast_kind_strings[ast->kind]);
        }
    }
    switch(ast->kind)
    {
        case WA_AST_U8:
            oc_str8_list_pushf(scratch.arena, &strList, ": 0x%.2hhx", ast->valU8);
            break;
        case WA_AST_U32:
            oc_str8_list_pushf(scratch.arena, &strList, ": %u", ast->valU32);
            break;
        case WA_AST_I32:
            oc_str8_list_pushf(scratch.arena, &strList, ": %i", ast->valI32);
            break;
        case WA_AST_U64:
            oc_str8_list_pushf(scratch.arena, &strList, ": %llu", ast->valU64);
            break;
        case WA_AST_I64:
            oc_str8_list_pushf(scratch.arena, &strList, ": %lli", ast->valI64);
            break;
        case WA_AST_F32:
            oc_str8_list_pushf(scratch.arena, &strList, ": %f", ast->valF32);
            break;
        case WA_AST_F64:
            oc_str8_list_pushf(scratch.arena, &strList, ": %f", ast->valF64);
            break;
        case WA_AST_NAME:
            oc_str8_list_pushf(scratch.arena, &strList, ": %.*s", oc_str8_ip(ast->str8));
            break;

        case WA_AST_VALUE_TYPE:
            oc_str8_list_pushf(scratch.arena, &strList, ": %s", wa_value_type_string(ast->valU32));
            break;

        case WA_AST_FUNC_ENTRY:
        case WA_AST_TYPE_INDEX:
            oc_str8_list_pushf(scratch.arena, &strList, ": %i", (i32)ast->valU32);
            break;

        case WA_AST_MAGIC:
        {
            oc_str8_list_pushf(scratch.arena, &strList, ": \\0asm");
        }
        break;

        default:
            break;
    }

    box->string = oc_str8_list_join(arena, strList);

    if(oc_list_empty(ast->children))
    {
        oc_str8_list bytesList = { 0 };
        for(u64 i = 0; i < ast->loc.len; i++)
        {
            oc_str8_list_pushf(scratch.arena, &bytesList, "0x%02hhx", app->contents.ptr[ast->loc.start + i]);
            if(i < ast->loc.len - 1)
            {
                oc_str8_list_pushf(scratch.arena, &bytesList, " ");
            }
        }

        box->bytesString = oc_str8_list_join(arena, bytesList);
    }
    oc_scratch_end(scratch);

    oc_text_metrics metrics = oc_font_text_metrics(app->font, app->fontSize, box->string);
    box->rect = (oc_rect){
        pos.x,
        pos.y,
        metrics.logical.w + 2 * BOX_MARGIN_W,
        metrics.logical.h + 2 * BOX_MARGIN_H,
    };
    box->childrenRect = box->rect;

    //TODO: shouldn't we have that available in oc_text_metrics?
    oc_font_metrics fontMetrics = oc_font_get_metrics(app->font, app->fontSize);

    box->textOffset = (oc_vec2){
        BOX_MARGIN_W - metrics.logical.x,
        BOX_MARGIN_H + fontMetrics.ascent,
    };

    oc_vec2 nextPos = {
        pos.x + app->indentW,
        pos.y += box->rect.h + BOX_LINE_GAP
    };

    oc_list_for(ast->children, child, wa_ast, parentElt)
    {
        wa_box* childBox = build_ast_boxes(arena, app, child, nextPos);
        nextPos.y += childBox->childrenRect.h + BOX_LINE_GAP;

        oc_vec2 xy1 = {
            oc_min(box->childrenRect.x, childBox->childrenRect.x),
            oc_min(box->childrenRect.y, childBox->childrenRect.y),
        };

        oc_vec2 xy2 = {
            oc_max(box->childrenRect.x + box->childrenRect.w, childBox->childrenRect.x + childBox->childrenRect.w),
            oc_max(box->childrenRect.y + box->childrenRect.h, childBox->childrenRect.y + childBox->childrenRect.h),
        };

        box->childrenRect = (oc_rect){
            xy1.x,
            xy1.y,
            xy2.x - xy1.x,
            xy2.y - xy1.y,
        };

        oc_list_push_back(&box->children, &childBox->listElt);
    }
    return (box);
}

/*
void draw_ast_boxes(app_data* app, oc_list boxes)
{
    oc_set_color_rgba(1, 1, 1, 1);
    oc_clear();

    oc_list_for(boxes, box, wa_box, listElt)
    {
        oc_set_color_rgba(1, 0, 0, 1);
        oc_set_width(2);
        oc_rectangle_stroke(box->rect.x, box->rect.y, box->rect.w, box->rect.h);

        oc_set_color_rgba(0, 0, 0, 1);
        oc_set_font(app->font);
        oc_set_font_size(app->fontSize);
        oc_text_fill(box->rect.x + box->textOffset.x, box->rect.y + box->textOffset.y, box->string);
    }

    oc_canvas_render(app->renderer, app->canvas, app->surface);
    oc_canvas_present(app->renderer, app->surface);
}
*/

void build_box_ui(app_data* app, wa_box* box)
{
    oc_ui_style_mask styleMask = OC_UI_STYLE_COLOR
                               | OC_UI_STYLE_SIZE
                               | OC_UI_STYLE_BORDER_SIZE
                               | OC_UI_STYLE_FLOAT
                               | OC_UI_STYLE_LAYOUT_MARGIN_X
                               | OC_UI_STYLE_LAYOUT_MARGIN_Y;

    oc_ui_style_next(&(oc_ui_style){
                         .borderSize = 2,
                         .color = { 0, 0, 0, 1 },
                         .floating = { true, true },
                         .floatTarget = (oc_vec2){
                             0,
                             box->rect.y },
                         .layout.margin = { BOX_MARGIN_W, BOX_MARGIN_H },
                     },
                     styleMask);

    oc_ui_container_str8(box->keyString, 0)
    {
        oc_ui_style_next(&(oc_ui_style){
                             .borderSize = 2,
                             .color = { 0, 0, 0, 1 },
                             .floating = { true, true },
                             .floatTarget = (oc_vec2){
                                 box->addrRect.x,
                                 0 },
                             .layout.margin = { BOX_MARGIN_W, BOX_MARGIN_H },
                         },
                         styleMask);

        oc_ui_box_make_str8(box->addrString, OC_UI_FLAG_DRAW_TEXT);

        oc_ui_style_next(&(oc_ui_style){
                             .borderSize = 2,
                             .color = { 0, 0, 0, 1 },
                             .floating = { true, true },
                             .floatTarget = (oc_vec2){
                                 box->addrRect.x + box->addrRect.w + box->rect.x,
                                 0 },
                             .layout.margin = { BOX_MARGIN_W, BOX_MARGIN_H },
                         },
                         styleMask);

        oc_ui_box* uiBox = oc_ui_box_make_str8(box->string, OC_UI_FLAG_DRAW_TEXT);

        if(box->ast && oc_list_empty(box->ast->children))
        {
            oc_ui_style_next(&(oc_ui_style){
                                 .borderSize = 2,
                                 .color = { 0, 0, 0, 1 },
                                 .floating = { true, true },
                                 .floatTarget = (oc_vec2){
                                     box->addrRect.x + box->addrRect.w + app->rootBox->childrenRect.w + 8 * BOX_MARGIN_W,
                                     0 },
                                 .layout.margin = { BOX_MARGIN_W, BOX_MARGIN_H },
                             },
                             styleMask);

            oc_ui_box_make_str8(box->bytesString, OC_UI_FLAG_DRAW_TEXT);
        }

        if(oc_ui_box_sig(uiBox).hovering)
        {
            if(box->ast && (box->ast->kind == WA_AST_TYPE_INDEX || box->ast->kind == WA_AST_FUNC_ENTRY))
            {
                wa_func_type* type = &app->module->types[box->ast->valU32];

                oc_arena_scope scratch = oc_scratch_begin();
                oc_str8_list list = { 0 };

                if(box->ast->kind == WA_AST_FUNC_ENTRY)
                {
                    //TODO: fix this hack
                    u32 funcIndex = 0;
                    oc_list_for(box->ast->parent->children, child, wa_ast, parentElt)
                    {
                        if(child == box->ast)
                        {
                            break;
                        }
                        funcIndex++;
                    }
                    funcIndex--;

                    oc_str8 name = find_function_export_name(app, funcIndex);
                    if(name.len)
                    {
                        oc_str8_list_push(scratch.arena, &list, name);
                        oc_str8_list_push(scratch.arena, &list, OC_STR8(" "));
                    }
                }

                push_func_type_str8_list(scratch.arena, &list, type);
                oc_str8 str = oc_str8_list_join(scratch.arena, list);

                oc_ui_tooltip_str8(str);

                oc_scratch_end(scratch);
            }
        }
    }

    if(!oc_list_empty(box->children))
    {
        oc_list_for(box->children, child, wa_box, listElt)
        {
            build_box_ui(app, child);
        }
    }
}

void build_boxes_ui(app_data* app)
{
    oc_ui_style defaultStyle = { .bgColor = { 0 },
                                 .color = { 1, 1, 1, 1 },
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
        if(app->rootBox)
        {
            oc_ui_panel("boxtree", 0)
            {

                oc_ui_style_next(&(oc_ui_style){
                                     .size = {
                                         .width = { OC_UI_SIZE_PIXELS, app->rootBox->childrenRect.w + 2 * BOX_MARGIN_W },
                                         .height = { OC_UI_SIZE_PIXELS, app->rootBox->childrenRect.h + 2 * BOX_MARGIN_H },
                                     },
                                 },
                                 OC_UI_STYLE_SIZE);

                oc_ui_container("contents", 0)
                {
                    build_box_ui(app, app->rootBox);
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

void update_ui(app_data* app)
{
    oc_arena_scope scratch = oc_scratch_begin();

    build_boxes_ui(app);

    oc_scratch_end(scratch);
}

void load_module(app_data* app, oc_str8 modulePath)
{
    //NOTE: unload previous module
    app->rootBox = 0;
    app->module = 0;
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
    }
    else
    {
        wa_ast_print(app->module->root, app->contents);
        wa_print_code(app->module);
    }

    if(app->module->root)
    {
        app->rootBox = build_ast_boxes(app->moduleArena, app, app->module->root, (oc_vec2){ BOX_MARGIN_W, BOX_MARGIN_H });
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
    app.window = oc_window_create(windowRect, OC_STR8("waspector"), 0);

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
