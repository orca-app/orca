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

typedef struct wa_bytecode_loc
{
    wa_instance* instance;
    wa_func* func;
    u32 index;
} wa_bytecode_loc;

typedef struct wa_breakpoint
{
    oc_list_elt listElt;
    wa_bytecode_loc loc;
    wa_code savedOpcode;

} wa_breakpoint;

typedef struct wa_debugger
{
    wa_interpreter* interpreter;

    wa_func* lastFunc;
    wa_value cachedRegs[WA_MAX_SLOT_COUNT];

    oc_arena arena;

    oc_list breakpoints;
    oc_list breakpointFreeList;

    wa_bytecode_loc nextLoc;
} wa_debugger;

typedef enum view_kind
{
    VIEW_BYTECODE,
    VIEW_WASM,
    VIEW_SOURCE,
} view_kind;

typedef struct app_data
{
    oc_window window;
    oc_canvas_renderer renderer;
    oc_surface surface;
    oc_canvas_context canvas;
    oc_font font;
    oc_ui_context ui;

    bool autoScroll;
    f32 lastScroll;

    f32 fontSize;
    f32 indentW;

    oc_str8 contents;
    oc_arena* moduleArena;
    wa_module* module;
    wa_instance* instance;
    wa_interpreter interpreter;

    wa_debugger debugger;

    view_kind viewKind;

    oc_str8 source;

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

//------------------------------------------------------------------------------------
// Debugger Control
//------------------------------------------------------------------------------------

void wa_debugger_init(wa_debugger* debugger, wa_interpreter* interpreter)
{
    debugger->interpreter = interpreter;
    oc_arena_init(&debugger->arena);
    debugger->breakpoints = (oc_list){ 0 };
    debugger->breakpointFreeList = (oc_list){ 0 };

    //TODO: maybe extract and pull same code from single step etc.
    debugger->lastFunc = interpreter->controlStack[interpreter->controlStackTop].func;
    u32 regCount = debugger->lastFunc->maxRegCount;
    memcpy(&debugger->cachedRegs, interpreter->locals, regCount * sizeof(wa_value));

    u32 funcIndex = debugger->lastFunc - interpreter->instance->functions;
    debugger->nextLoc = (wa_bytecode_loc){
        .instance = interpreter->instance,
        .func = debugger->lastFunc,
        .index = 0,
    };
}

wa_breakpoint* wa_debugger_find_breakpoint(wa_debugger* debugger, wa_bytecode_loc* loc)
{
    wa_breakpoint* result = 0;

    oc_list_for(debugger->breakpoints, bp, wa_breakpoint, listElt)
    {
        if(bp->loc.instance == loc->instance
           && bp->loc.func == loc->func
           && bp->loc.index == loc->index)
        {
            result = bp;
            break;
        }
    }

    return result;
}

wa_breakpoint* wa_debugger_add_breakpoint(wa_debugger* debugger, wa_bytecode_loc* loc)
{
    wa_breakpoint* bp = wa_debugger_find_breakpoint(debugger, loc);
    if(bp == 0)
    {
        bp = oc_list_pop_front_entry(&debugger->breakpointFreeList, wa_breakpoint, listElt);
        if(!bp)
        {
            bp = oc_arena_push_type(&debugger->arena, wa_breakpoint);
        }
        bp->loc = *loc;
        oc_list_push_back(&debugger->breakpoints, &bp->listElt);

        bp->savedOpcode = bp->loc.func->code[bp->loc.index];
        bp->loc.func->code[bp->loc.index].opcode = WA_INSTR_breakpoint;
    }
    return bp;
}

void wa_debugger_remove_breakpoint(wa_debugger* debugger, wa_breakpoint* bp)
{
    bp->loc.func->code[bp->loc.index] = bp->savedOpcode;

    oc_list_remove(&debugger->breakpoints, &bp->listElt);
    oc_list_push_back(&debugger->breakpointFreeList, &bp->listElt);
}

wa_status wa_debugger_single_step(wa_debugger* debugger)
{
    wa_interpreter* interpreter = debugger->interpreter;
    wa_func* func = interpreter->controlStack[interpreter->controlStackTop].func;
    u32 codeIndex = interpreter->pc - func->code;

    debugger->lastFunc = func;
    memcpy(debugger->cachedRegs, interpreter->locals, func->maxRegCount * sizeof(wa_value));

    //NOTE: if we start on a breakpoint, temporarily deactivate it.
    wa_breakpoint* bp = wa_debugger_find_breakpoint(debugger, &debugger->nextLoc);
    if(bp)
    {
        bp->loc.func->code[bp->loc.index] = bp->savedOpcode;
    }

    // single step
    wa_status status = wa_interpreter_run(interpreter, true);

    //TODO check if program terminated?

    //NOTE: re-enable breakpoint
    if(bp)
    {
        bp->loc.func->code[bp->loc.index].opcode = WA_INSTR_breakpoint;
    }

    //NOTE: store next location
    wa_func* nextFunc = interpreter->controlStack[interpreter->controlStackTop].func;
    u32 nextCodeIndex = interpreter->pc - nextFunc->code;

    debugger->nextLoc = (wa_bytecode_loc){
        .instance = interpreter->controlStack[interpreter->controlStackTop].instance,
        .func = nextFunc,
        .index = nextCodeIndex,
    };

    return status;
}

bool wa_instr_is_silent(wa_instr* instr)
{
    return (instr->op == WA_INSTR_nop || instr->op == WA_INSTR_local_get);
}

wa_status wa_debugger_single_step_wasm(wa_debugger* debugger)
{
    /*
        Some wasm instr generate multiple bytecode instr, so we need to step through the bytecode
        until we hit a _new_ instruction.

        Some wasm instructions are "silent", ie they don't generate bytecode at all, and bytecode-stepping
        skips those. This is the case of `nop` and `local.get`.

        We could "fake" stepping through these instructions, but this could be confusing (e.g. register contents
        wouldn't change), and complicates stepping code a fair bit. So for now, we only allow stepping or setting
        breakpoints at real bytecode locations.
    */
    wa_bytecode_loc startLoc = debugger->nextLoc;
    wa_instr* startInstr = wa_bytecode_to_instr(startLoc.instance->module,
                                                startLoc.func - startLoc.instance->functions,
                                                startLoc.index);

    wa_status status = WA_OK;
    while((status = wa_debugger_single_step(debugger)) == WA_TRAP_STEP)
    {
        wa_bytecode_loc nextLoc = debugger->nextLoc;
        wa_instr* instr = wa_bytecode_to_instr(nextLoc.instance->module,
                                               nextLoc.func - nextLoc.instance->functions,
                                               nextLoc.index);
        if(instr != startInstr)
        {
            break;
        }
    }
    return status;
}

wa_status wa_debugger_continue(wa_debugger* debugger)
{
    wa_interpreter* interpreter = debugger->interpreter;
    wa_func* func = interpreter->controlStack[interpreter->controlStackTop].func;
    u32 codeIndex = interpreter->pc - func->code;

    debugger->lastFunc = func;
    memcpy(debugger->cachedRegs, interpreter->locals, func->maxRegCount * sizeof(wa_value));

    //NOTE: if we start on a "real" breakpoint, step over it (step takes care of disabling/reenabling breakpoints)
    wa_breakpoint* breakpoint = wa_debugger_find_breakpoint(debugger, &debugger->nextLoc);
    if(breakpoint)
    {
        wa_debugger_single_step(debugger);
    }

    wa_status status = wa_interpreter_run(interpreter, false);

    wa_func* nextFunc = interpreter->controlStack[interpreter->controlStackTop].func;
    u32 nextCodeIndex = interpreter->pc - nextFunc->code;

    debugger->nextLoc = (wa_bytecode_loc){
        .instance = interpreter->controlStack[interpreter->controlStackTop].instance,
        .func = nextFunc,
        .index = nextCodeIndex,
    };
    return (status);
}

//------------------------------------------------------------------------------------
// Debugger UI
//------------------------------------------------------------------------------------

void draw_breakpoint_cursor_path(oc_rect rect)
{
    oc_vec2 center = { rect.x + rect.w / 2, rect.y + rect.h / 2 };
    f32 dx = 12;
    f32 dy = 7;
    f32 r = 3;

    f32 h = sqrt(dy * dy + dx * dx / 4);
    f32 px = dx * (1 - (h - r) / (2 * h));
    f32 py = dy * (h - r) / h;

    // top left corner
    oc_move_to(center.x - dx, center.y - dy + r);
    oc_quadratic_to(center.x - dx, center.y - dy, center.x - dx + r, center.y - dy);

    //top
    oc_line_to(center.x + dx / 2 - r, center.y - dy);

    // tip top corner
    oc_quadratic_to(center.x + dx / 2, center.y - dy, center.x + px, center.y - py);

    // arrow tip
    f32 tx = dx * (1 - r / (2 * h));
    f32 ty = dy * r / h;

    oc_line_to(center.x + tx, center.y - ty);
    oc_quadratic_to(center.x + dx, center.y, center.x + tx, center.y + ty);
    oc_line_to(center.x + px, center.y + py);

    // tip bottom corner
    oc_quadratic_to(center.x + dx / 2, center.y + dy, center.x + dx / 2 - r, center.y + dy);

    // bottom
    oc_line_to(center.x - dx + r, center.y + dy);

    // bottom left corner
    oc_quadratic_to(center.x - dx, center.y + dy, center.x - dx, center.y + dy - r);

    oc_close_path();
}

void draw_breakpoint_cursor_proc(oc_ui_box* box, void* data)
{
    /*
    oc_set_color_rgba(1, 0, 0, 1);
    oc_set_width(2);
    oc_rectangle_stroke(box->rect.x, box->rect.y, box->rect.w, box->rect.h);
    */

    draw_breakpoint_cursor_path(box->rect);
    oc_set_color_rgba(1, 0.2, 0.2, 1);
    oc_fill();

    draw_breakpoint_cursor_path(box->rect);
    oc_set_color_rgba(1, 0, 0, 1);
    oc_set_width(2);
    oc_stroke();
}

void build_wasm_ui(app_data* app, oc_ui_box* scrollPanel)
{
    //WARN: scroll is adjusted during layout, which can result in a slightly different
    //      computation from lastScroll, so compare with a 1 pixel threshold
    if(fabs(app->lastScroll - scrollPanel->scroll.y) > 1)
    {
        //NOTE: if user has adjusted scroll manually, deactivate auto-scroll
        app->autoScroll = false;
    }

    oc_arena_scope scratch = oc_scratch_begin();

    oc_ui_style_next(&(oc_ui_style){
                         .size.width = { OC_UI_SIZE_PARENT, 1 },
                     },
                     OC_UI_STYLE_SIZE_WIDTH);

    oc_ui_container("wasm-view", 0)
    {
        for(u32 funcIndex = 0; funcIndex < app->instance->module->functionCount; funcIndex++)
        {
            wa_func* func = &app->instance->functions[funcIndex];
            if(func->code)
            {
                oc_str8_list list = { 0 };
                oc_str8_list_push(scratch.arena, &list, OC_STR8("func "));

                u32 funcIndex = func - app->instance->functions;
                oc_str8 funcName = find_function_export_name(app, funcIndex);
                if(funcName.len)
                {
                    oc_str8_list_push(scratch.arena, &list, funcName);
                }
                else
                {
                    oc_str8_list_pushf(scratch.arena, &list, "%i", funcIndex);
                }
                oc_str8_list_push(scratch.arena, &list, OC_STR8(" "));
                push_func_type_str8_list(scratch.arena, &list, func->type);

                oc_str8 funcText = oc_str8_list_join(scratch.arena, list);

                oc_ui_style_next(&(oc_ui_style){
                                     .size.width = { OC_UI_SIZE_PARENT, 1 },
                                     .layout = {
                                         .axis = OC_UI_AXIS_Y,
                                         .spacing = BOX_MARGIN_H,
                                         .margin.x = BOX_MARGIN_W,
                                         .margin.y = BOX_MARGIN_H,
                                         .align = OC_UI_ALIGN_START,
                                     },
                                 },
                                 OC_UI_STYLE_SIZE_WIDTH | OC_UI_STYLE_LAYOUT);

                oc_ui_container_str8(funcText, 0)
                {
                    oc_ui_label_str8(funcText);

                    oc_list_for(func->instructions, instr, wa_instr, listElt)
                    {
                        const wa_instr_info* info = &wa_instr_infos[instr->op];

                        oc_str8 key = oc_str8_pushf(scratch.arena, "0x%08llx", instr->ast->loc.start);

                        oc_ui_style_next(&(oc_ui_style){
                                             .size.width = { OC_UI_SIZE_PARENT, 1 },
                                             .layout = {
                                                 .axis = OC_UI_AXIS_X,
                                                 .spacing = BOX_MARGIN_W * 5,
                                                 .margin.y = BOX_MARGIN_H,
                                                 .align = OC_UI_ALIGN_START,
                                             },
                                         },
                                         OC_UI_STYLE_SIZE_WIDTH | OC_UI_STYLE_LAYOUT);

                        bool makeExecCursor = false;
                        if(app->instance)
                        {
                            wa_bytecode_loc nextLoc = app->debugger.nextLoc;
                            wa_instr* nextInstr = wa_bytecode_to_instr(nextLoc.instance->module,
                                                                       nextLoc.func - nextLoc.instance->functions,
                                                                       nextLoc.index);
                            if(instr == nextInstr)
                            {
                                makeExecCursor = true;
                            }
                        }

                        if(makeExecCursor)
                        {
                            oc_ui_style_next(&(oc_ui_style){
                                                 .bgColor = { 0.4, 1, 0.4, 1 },
                                             },
                                             OC_UI_STYLE_BG_COLOR);
                        }

                        ///////////////////////////////////////////////////////////////////////////////////
                        //TODO: display breakpoints at the same instr but different bytecode offset, if any
                        ///////////////////////////////////////////////////////////////////////////////////

                        wa_breakpoint* breakpoint = 0;

                        if(!wa_instr_is_silent(instr))
                        {
                            breakpoint = wa_debugger_find_breakpoint(&app->debugger,
                                                                     &(wa_bytecode_loc){
                                                                         .instance = app->debugger.interpreter->instance,
                                                                         .func = func,
                                                                         .index = instr->codeIndex,
                                                                     });
                        }

                        oc_ui_container_str8(key, OC_UI_FLAG_DRAW_BACKGROUND)
                        {
                            // address
                            oc_ui_box* label = oc_ui_label_str8(key).box;

                            if(makeExecCursor)
                            {
                                //NOTE: we compute auto-scroll on label box instead of cursor box, because the cursor box is not permanent,
                                //      so its rect might not be set every frame, resulting in brief jumps.
                                //      Maybe the cursor box shouldnt be parented to the function UI namespace and be floating to begin with...

                                if(app->autoScroll)
                                {
                                    f32 targetScroll = scrollPanel->scroll.y;

                                    f32 scrollMargin = 60;

                                    if(label->rect.y < scrollPanel->rect.y + scrollMargin)
                                    {
                                        targetScroll = scrollPanel->scroll.y
                                                     - scrollPanel->rect.y
                                                     + label->rect.y
                                                     - scrollMargin;
                                    }
                                    else if(label->rect.y + label->rect.h + scrollMargin > scrollPanel->rect.y + scrollPanel->rect.h)
                                    {
                                        targetScroll = scrollPanel->scroll.y
                                                     + label->rect.y
                                                     + label->rect.h
                                                     + scrollMargin
                                                     - scrollPanel->rect.y
                                                     - scrollPanel->rect.h;
                                    }
                                    targetScroll = oc_clamp(targetScroll, 0, scrollPanel->childrenSum[1] - scrollPanel->rect.h);

                                    scrollPanel->scroll.y += 0.1 * (targetScroll - scrollPanel->scroll.y);
                                }
                            }

                            // spacer or exec cursor
                            oc_ui_style_next(&(oc_ui_style){
                                                 .size.width = { OC_UI_SIZE_PIXELS, 10 * BOX_MARGIN_W },
                                                 .size.height = { OC_UI_SIZE_PARENT, 1 },
                                             },
                                             OC_UI_STYLE_SIZE);
                            if(breakpoint)
                            {
                                oc_ui_box* box = oc_ui_box_make("bp", OC_UI_FLAG_DRAW_PROC | OC_UI_FLAG_CLICKABLE);
                                oc_ui_box_set_draw_proc(box, draw_breakpoint_cursor_proc, 0);

                                if(oc_ui_box_sig(box).clicked)
                                {
                                    wa_debugger_remove_breakpoint(&app->debugger, breakpoint);
                                }
                            }
                            else
                            {
                                oc_ui_box* box = oc_ui_box_make("spacer", OC_UI_FLAG_CLICKABLE);

                                if(oc_ui_box_sig(box).clicked)
                                {
                                    //TODO: find next instruction that is not silent
                                    wa_instr* nextInstr = instr;
                                    for(;
                                        nextInstr != 0;
                                        nextInstr = oc_list_next_entry(nextInstr, wa_instr, listElt))
                                    {
                                        if(!wa_instr_is_silent(nextInstr))
                                        {
                                            break;
                                        }
                                    }
                                    wa_debugger_add_breakpoint(&app->debugger,
                                                               &(wa_bytecode_loc){
                                                                   .instance = app->debugger.interpreter->instance,
                                                                   .func = func,
                                                                   .index = nextInstr->codeIndex,
                                                               });
                                }
                            }

                            // opcode
                            oc_ui_label(wa_instr_strings[instr->op]);

                            // operands
                            for(u32 immIndex = 0; immIndex < instr->immCount; immIndex++)
                            {

                                wa_code* imm = &instr->imm[immIndex];
                                oc_str8 immKey = oc_str8_pushf(scratch.arena, "imm%u", immIndex);

                                oc_ui_container_str8(immKey, 0)
                                {
                                    oc_str8 s = { 0 };

                                    switch(info->imm[immIndex])
                                    {
                                        case WA_IMM_ZERO:
                                            break;
                                        case WA_IMM_I32:
                                            s = oc_str8_pushf(scratch.arena, "%i", imm->valI32);

                                            break;
                                        case WA_IMM_I64:
                                            s = oc_str8_pushf(scratch.arena, "%lli", imm->valI64);

                                            break;
                                        case WA_IMM_F32:
                                            s = oc_str8_pushf(scratch.arena, "%f", imm->valF32);
                                            break;
                                        case WA_IMM_F64:
                                            s = oc_str8_pushf(scratch.arena, "%f", imm->valF64);
                                            break;
                                        case WA_IMM_VALUE_TYPE:
                                            break;
                                        case WA_IMM_REF_TYPE:
                                            break;
                                        case WA_IMM_FUNC_INDEX:
                                            s = find_function_export_name(app, imm->valU32);
                                            if(s.len == 0)
                                            {
                                                s = oc_str8_pushf(scratch.arena, "%u", imm->valU32);
                                            }
                                            break;
                                        case WA_IMM_LOCAL_INDEX:
                                        case WA_IMM_GLOBAL_INDEX:
                                        case WA_IMM_TYPE_INDEX:
                                        case WA_IMM_TABLE_INDEX:
                                        case WA_IMM_ELEM_INDEX:
                                        case WA_IMM_DATA_INDEX:
                                        case WA_IMM_LABEL:
                                            s = oc_str8_pushf(scratch.arena, "%u", imm->valU32);
                                            break;

                                        case WA_IMM_MEM_ARG:
                                            s = oc_str8_pushf(scratch.arena, "a%u:+%u", imm->memArg.align, imm->memArg.offset);
                                            break;

                                        default:
                                            s = oc_str8_pushf(scratch.arena, "0x%08llx", imm->valU64);
                                            break;
                                    }
                                    oc_ui_label_str8(s);
                                }
                            }
                        }
                    }
                    oc_ui_style_next(&(oc_ui_style){ .size.height = { OC_UI_SIZE_PIXELS, 10 * BOX_MARGIN_H } },
                                     OC_UI_STYLE_SIZE_HEIGHT);
                    oc_ui_box_make("vspacer", 0);
                }
            }
        }
    }
    app->lastScroll = scrollPanel->scroll.y;
    oc_scratch_end(scratch);
}

void build_bytecode_ui(app_data* app, oc_ui_box* scrollPanel)
{
    //WARN: scroll is adjusted during layout, which can result in a slightly different
    //      computation from lastScroll, so compare with a 1 pixel threshold
    if(fabsf(app->lastScroll - scrollPanel->scroll.y) > 1)
    {
        //NOTE: if user has adjusted scroll manually, deactivate auto-scroll
        app->autoScroll = false;
    }

    oc_arena_scope scratch = oc_scratch_begin();

    oc_ui_style_next(&(oc_ui_style){
                         .size.width = { OC_UI_SIZE_PARENT, 1 },
                     },
                     OC_UI_STYLE_SIZE_WIDTH);

    oc_ui_container("bytecode-view", 0)
    {
        for(u32 funcIndex = 0; funcIndex < app->instance->module->functionCount; funcIndex++)
        {
            wa_func* func = &app->instance->functions[funcIndex];
            if(func->code)
            {
                oc_str8_list list = { 0 };
                oc_str8_list_push(scratch.arena, &list, OC_STR8("func "));

                u32 funcIndex = func - app->instance->functions;
                oc_str8 funcName = find_function_export_name(app, funcIndex);
                if(funcName.len)
                {
                    oc_str8_list_push(scratch.arena, &list, funcName);
                }
                else
                {
                    oc_str8_list_pushf(scratch.arena, &list, "%i", funcIndex);
                }
                oc_str8_list_push(scratch.arena, &list, OC_STR8(" "));
                push_func_type_str8_list(scratch.arena, &list, func->type);

                oc_str8 funcText = oc_str8_list_join(scratch.arena, list);

                oc_ui_style_next(&(oc_ui_style){
                                     .size.width = { OC_UI_SIZE_PARENT, 1 },
                                     .layout = {
                                         .axis = OC_UI_AXIS_Y,
                                         .spacing = BOX_MARGIN_H,
                                         .margin.x = BOX_MARGIN_W,
                                         .margin.y = BOX_MARGIN_H,
                                         .align = OC_UI_ALIGN_START,
                                     },
                                 },
                                 OC_UI_STYLE_SIZE_WIDTH | OC_UI_STYLE_LAYOUT);

                oc_ui_container_str8(funcText, 0)
                {
                    oc_ui_label_str8(funcText);

                    for(u64 codeIndex = 0; codeIndex < func->codeLen; codeIndex++)
                    {
                        u64 startIndex = codeIndex;

                        wa_code* c = &func->code[codeIndex];
                        wa_instr_op opcode = c->opcode;

                        wa_breakpoint* breakpoint = wa_debugger_find_breakpoint(&app->debugger,
                                                                                &(wa_bytecode_loc){
                                                                                    .instance = app->debugger.interpreter->instance,
                                                                                    .func = func,
                                                                                    .index = codeIndex,
                                                                                });
                        if(breakpoint)
                        {
                            opcode = breakpoint->savedOpcode.opcode;
                        }

                        const wa_instr_info* info = &wa_instr_infos[opcode];

                        oc_str8 key = oc_str8_pushf(scratch.arena, "0x%08llx", codeIndex);

                        oc_ui_style_next(&(oc_ui_style){
                                             .size.width = { OC_UI_SIZE_PARENT, 1 },
                                             .layout = {
                                                 .axis = OC_UI_AXIS_X,
                                                 .spacing = BOX_MARGIN_W * 5,
                                                 .margin.y = BOX_MARGIN_H,
                                                 .align = OC_UI_ALIGN_START,
                                             },
                                         },
                                         OC_UI_STYLE_SIZE_WIDTH | OC_UI_STYLE_LAYOUT);

                        bool makeExecCursor = false;
                        if(app->instance)
                        {
                            u32 index = app->interpreter.pc - func->code;
                            wa_func* execFunc = app->interpreter.controlStack[app->interpreter.controlStackTop].func;

                            if(func == execFunc && index == codeIndex)
                            {
                                makeExecCursor = true;
                            }
                        }

                        if(makeExecCursor)
                        {
                            oc_ui_style_next(&(oc_ui_style){
                                                 .bgColor = { 0.4, 1, 0.4, 1 },
                                             },
                                             OC_UI_STYLE_BG_COLOR);
                        }

                        oc_ui_container_str8(key, OC_UI_FLAG_DRAW_BACKGROUND)
                        {
                            // address
                            oc_ui_box* label = oc_ui_label_str8(key).box;

                            if(makeExecCursor)
                            {
                                //NOTE: we compute auto-scroll on label box instead of cursor box, because the cursor box is not permanent,
                                //      so its rect might not be set every frame, resulting in brief jumps.
                                //      Maybe the cursor box shouldnt be parented to the function UI namespace and be floating to begin with...

                                if(app->autoScroll)
                                {
                                    f32 targetScroll = scrollPanel->scroll.y;

                                    f32 scrollMargin = 60;

                                    if(label->rect.y < scrollPanel->rect.y + scrollMargin)
                                    {
                                        targetScroll = scrollPanel->scroll.y
                                                     - scrollPanel->rect.y
                                                     + label->rect.y
                                                     - scrollMargin;
                                    }
                                    else if(label->rect.y + label->rect.h + scrollMargin > scrollPanel->rect.y + scrollPanel->rect.h)
                                    {
                                        targetScroll = scrollPanel->scroll.y
                                                     + label->rect.y
                                                     + label->rect.h
                                                     + scrollMargin
                                                     - scrollPanel->rect.y
                                                     - scrollPanel->rect.h;
                                    }
                                    targetScroll = oc_clamp(targetScroll, 0, scrollPanel->childrenSum[1] - scrollPanel->rect.h);

                                    scrollPanel->scroll.y += 0.1 * (targetScroll - scrollPanel->scroll.y);
                                }
                            }

                            // spacer or exec cursor
                            oc_ui_style_next(&(oc_ui_style){
                                                 .size.width = { OC_UI_SIZE_PIXELS, 10 * BOX_MARGIN_W },
                                                 .size.height = { OC_UI_SIZE_PARENT, 1 },
                                             },
                                             OC_UI_STYLE_SIZE);

                            if(breakpoint)
                            {
                                oc_ui_box* box = oc_ui_box_make("bp", OC_UI_FLAG_DRAW_PROC | OC_UI_FLAG_CLICKABLE);
                                oc_ui_box_set_draw_proc(box, draw_breakpoint_cursor_proc, 0);

                                if(oc_ui_box_sig(box).clicked)
                                {
                                    wa_debugger_remove_breakpoint(&app->debugger, breakpoint);
                                }
                            }
                            else
                            {
                                oc_ui_box* box = oc_ui_box_make("spacer", OC_UI_FLAG_CLICKABLE);
                                if(oc_ui_box_sig(box).clicked)
                                {
                                    wa_debugger_add_breakpoint(&app->debugger,
                                                               &(wa_bytecode_loc){
                                                                   .instance = app->debugger.interpreter->instance,
                                                                   .func = func,
                                                                   .index = codeIndex,
                                                               });
                                }
                            }
                            // opcode
                            oc_ui_label(wa_instr_strings[opcode]);

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
                                        case WA_OPD_GLOBAL_INDEX:
                                            s = oc_str8_pushf(scratch.arena, "g%u", opd->valU32);
                                            break;

                                        case WA_OPD_FUNC_INDEX:
                                            s = find_function_export_name(app, opd->valU32);
                                            if(s.len == 0)
                                            {
                                                s = oc_str8_pushf(scratch.arena, "%u", opd->valU32);
                                            }
                                            break;

                                        case WA_OPD_JUMP_TARGET:
                                            s = oc_str8_pushf(scratch.arena, "%+lli", opd->valI64);
                                            break;

                                        case WA_OPD_MEM_ARG:
                                            s = oc_str8_pushf(scratch.arena, "a%u:+%u", opd->memArg.align, opd->memArg.offset);
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
                        */
                    }
                    oc_ui_style_next(&(oc_ui_style){ .size.height = { OC_UI_SIZE_PIXELS, 10 * BOX_MARGIN_H } },
                                     OC_UI_STYLE_SIZE_HEIGHT);
                    oc_ui_box_make("vspacer", 0);
                }
            }
        }
    }

    app->lastScroll = scrollPanel->scroll.y;
    oc_scratch_end(scratch);
}

void source_draw_proc(oc_ui_box* box, void* data)
{
    app_data* app = (app_data*)data;

    oc_str8 source = app->source;

    u32 start = 0;
    u32 end = 0;

    f32 x = 20;
    f32 y = 20;
    f32 maxWidth = 0;

    oc_font_metrics metrics = oc_font_get_metrics(app->font, 16);
    f32 lineHeight = metrics.ascent + metrics.descent + metrics.lineGap;

    u32 line = 0;
    while(start < source.len)
    {
        oc_arena_scope scratch = oc_scratch_begin();
        oc_str8_list list = { 0 };
        oc_utf8_dec dec = { 0 };

        while(end < source.len)
        {
            dec = oc_utf8_decode_at(source, end);
            end += dec.size;

            if(dec.codepoint == '\t' || dec.codepoint == '\n' || dec.codepoint == '\0')
            {
                oc_str8_list_push(scratch.arena, &list, oc_str8_slice(source, start, end));
                start = end;
                if(dec.codepoint == '\t')
                {
                    oc_str8_list_push(scratch.arena, &list, OC_STR8("    "));
                }
                else
                {
                    break;
                }
            }
        }
        oc_str8 str = oc_str8_list_join(scratch.arena, list);

        oc_text_metrics textMetrics = oc_font_text_metrics(app->font, 16, str);
        maxWidth = oc_max(maxWidth, textMetrics.advance.x);

        oc_move_to(x, y);
        oc_text_outlines(str);
        x = 20;
        y += lineHeight;
        oc_scratch_end(scratch);
    }
    oc_set_font(app->font);
    oc_set_color_rgba(0, 0, 0, 1);
    oc_fill();

    //////////////////////////////////////////////////////////////
    // This is a gross hack
    //////////////////////////////////////////////////////////////
    box->rect.w = maxWidth;
    box->rect.h = y;
}

void build_source_ui(app_data* app, oc_ui_box* scrollPanel)
{
    oc_ui_box* box = oc_ui_box_lookup("source");
    if(box)
    {
        oc_ui_style_next(&(oc_ui_style){
                             .size = {
                                 .width = { OC_UI_SIZE_PIXELS, box->rect.w },
                                 .height = { OC_UI_SIZE_PIXELS, box->rect.h },
                             },
                         },
                         OC_UI_STYLE_SIZE);
    }
    else
    {
        oc_ui_style_next(&(oc_ui_style){
                             .size = {
                                 .width = { OC_UI_SIZE_PARENT, 1 },
                                 .height = { OC_UI_SIZE_PARENT, 1 },
                             },
                         },
                         OC_UI_STYLE_SIZE);
    }

    box = oc_ui_box_make("source", OC_UI_FLAG_DRAW_PROC);
    oc_ui_box_set_draw_proc(box, source_draw_proc, app);
}

void build_register_ui(app_data* app)
{
    oc_arena_scope scratch = oc_scratch_begin();

    if(app->instance)
    {
        wa_func* func = app->interpreter.controlStack[app->interpreter.controlStackTop].func;
        u32 regCount = func->maxRegCount;

        for(u32 regIndex = 0; regIndex < regCount; regIndex++)
        {
            wa_value* reg = &app->interpreter.locals[regIndex];
            oc_str8 key = oc_str8_pushf(scratch.arena, "r%u: 0x%08llx", regIndex, reg->valI64);

            if(app->debugger.lastFunc == func && reg->valI64 != app->debugger.cachedRegs[regIndex].valI64)
            {
                oc_ui_style_next(&(oc_ui_style){
                                     .color = { 1, 0, 0, 1 },
                                 },
                                 OC_UI_STYLE_COLOR);
            }
            oc_ui_label_str8(key);
        }
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

            oc_ui_style_next(&(oc_ui_style){
                                 .size = {
                                     .width = { OC_UI_SIZE_PARENT, 1 },
                                     .height = { OC_UI_SIZE_PARENT, 1 },
                                 },
                                 .layout = {
                                     .axis = OC_UI_AXIS_Y,
                                     .spacing = 20,
                                     .margin.x = 20,
                                     .margin.y = 20,
                                 },

                                 .bgColor = { 0.5, 0.5, 0.5, 1 },
                             },
                             OC_UI_STYLE_LAYOUT | OC_UI_STYLE_SIZE | OC_UI_STYLE_BG_COLOR);

            oc_ui_container("frame", OC_UI_FLAG_DRAW_BACKGROUND)
            {
                //TODO: oc_ui_panel internally calls oc_ui_style_next to set itself to full parent size,
                //      so we have to create an outer container to properly size it.
                //      Fix that later...

                oc_ui_style_next(&(oc_ui_style){
                                     .size = {
                                         .width = { OC_UI_SIZE_PARENT, 1 },
                                         .height = { OC_UI_SIZE_PARENT, 0.7 },
                                     },
                                     .bgColor = { 1, 1, 1, 1 },
                                 },
                                 OC_UI_STYLE_SIZE | OC_UI_STYLE_BG_COLOR);

                oc_ui_container("bytecode_outer", OC_UI_FLAG_DRAW_BACKGROUND)
                {
                    oc_ui_panel("bytecode", 0)
                    {
                        oc_ui_box* scrollPanel = oc_ui_box_top()->parent;
                        OC_ASSERT(scrollPanel);

                        switch(app->viewKind)
                        {
                            case VIEW_BYTECODE:
                                build_bytecode_ui(app, scrollPanel);
                                break;

                            case VIEW_WASM:
                                build_wasm_ui(app, scrollPanel);
                                break;

                            case VIEW_SOURCE:
                                build_source_ui(app, scrollPanel);
                                break;
                        }
                    }
                }

                oc_ui_style_next(&(oc_ui_style){
                                     .size = {
                                         .width = { OC_UI_SIZE_PARENT, 1 },
                                         .height = { OC_UI_SIZE_PARENT, 0.3 },
                                     },
                                     .bgColor = { 1, 1, 1, 1 },
                                 },
                                 OC_UI_STYLE_SIZE | OC_UI_STYLE_BG_COLOR);

                oc_ui_container("registers_outer", OC_UI_FLAG_DRAW_BACKGROUND)
                {
                    oc_ui_panel("registers", 0)
                    {
                        build_register_ui(app);
                    }
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

    if(app->instance)
    {
        wa_interpreter_cleanup(&app->interpreter);
    }

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
        else
        {
            printf("Run:\n");
            wa_func* func = wa_instance_find_function(app->instance, OC_STR8("start"));

            if(!func)
            {
                oc_log_error("Couldn't find function start.\n");
                app->instance = 0;
                oc_arena_clear(app->moduleArena);
            }
            else
            {
                wa_interpreter_init(&app->interpreter, app->instance, func, func->type, func->code, 0, 0, 0, 0);
                wa_debugger_init(&app->debugger, &app->interpreter);
                app->autoScroll = true;
            }
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

    oc_rect windowRect = { .x = 100, .y = 100, .w = 800, .h = 600 };
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

    ///////////////////////////////////////////////////////////////////////
    //TODO: this is hardcoded for a quick draft
    ///////////////////////////////////////////////////////////////////////

    oc_file sourceFile = oc_file_open(OC_STR8("test/wasm_test.c"), OC_FILE_ACCESS_READ, OC_FILE_OPEN_NONE);
    app.source.len = oc_file_size(sourceFile);
    app.source.ptr = oc_arena_push(&arena, app.source.len);
    oc_file_read(sourceFile, app.source.len, app.source.ptr);

    oc_window_bring_to_front(app.window);
    oc_window_focus(app.window);
    oc_window_center(app.window);

    while(!oc_should_quit())
    {

        oc_pump_events(0);

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

                case OC_EVENT_KEYBOARD_KEY:
                {
                    if(event->key.action == OC_KEY_PRESS && app.instance)
                    {
                        if(event->key.keyCode == OC_KEY_SPACE)
                        {
                            if(app.viewKind == VIEW_WASM)
                            {
                                wa_debugger_single_step_wasm(&app.debugger);
                            }
                            else
                            {
                                wa_debugger_single_step(&app.debugger);
                            }
                            app.autoScroll = true;
                        }
                        else if(event->key.keyCode == OC_KEY_C)
                        {
                            wa_debugger_continue(&app.debugger);
                            app.autoScroll = true;
                        }
                        else if(event->key.keyCode == OC_KEY_TAB)
                        {
                            app.viewKind = (app.viewKind + 1) % 3;
                            app.autoScroll = true;
                        }
                        else if(event->key.keyCode == OC_KEY_ENTER)
                        {
                            printf("!\n");
                        }
                    }
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
