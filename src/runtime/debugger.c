/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "warm/debug_info.h"

//------------------------------------------------------------------------
// Debug Overlay UI
//------------------------------------------------------------------------

void debug_overlay_toggle(oc_debug_overlay* overlay)
{
    overlay->show = !overlay->show;

    if(overlay->show)
    {
        overlay->logScrollToLast = true;
    }
}

void log_entry_ui(oc_debug_overlay* overlay, log_entry* entry)
{
    oc_arena_scope scratch = oc_scratch_begin();

    static const char* levelNames[] = { "Error: ", "Warning: ", "Info: " };
    static const oc_color levelColors[] = { { 0.8, 0, 0, 1 },
                                            { 1, 0.5, 0, 1 },
                                            { 0, 0.8, 0, 1 } };

    static const oc_color bgColors[3][2] = { //errors
                                             { { 0.6, 0, 0, 0.5 }, { 0.8, 0, 0, 0.5 } },
                                             //warning
                                             { { 0.4, 0.4, 0.4, 0.5 }, { 0.5, 0.5, 0.5, 0.5 } },
                                             //info
                                             { { 0.4, 0.4, 0.4, 0.5 }, { 0.5, 0.5, 0.5, 0.5 } }
    };

    oc_str8 key = oc_str8_pushf(scratch.arena, "%ull", entry->recordIndex);

    oc_ui_box_str8(key)
    {
        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
        oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
        oc_ui_style_set_f32(OC_UI_MARGIN_X, 10);
        oc_ui_style_set_f32(OC_UI_MARGIN_Y, 5);
        oc_ui_style_set_color(OC_UI_BG_COLOR, bgColors[entry->level][entry->recordIndex & 1]);

        oc_ui_style_set_i32(OC_UI_CLICK_THROUGH, 1);

        oc_ui_box("header")
        {
            oc_ui_style_set_i32(OC_UI_CLICK_THROUGH, 1);

            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
            oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
            oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_X);

            oc_ui_style_rule("level")
            {
                oc_ui_style_set_i32(OC_UI_CLICK_THROUGH, 1);

                oc_ui_style_set_color(OC_UI_COLOR, levelColors[entry->level]);
                oc_ui_style_set_font(OC_UI_FONT, overlay->fontBold);
            }
            oc_ui_label("level", levelNames[entry->level]);

            oc_str8 loc = oc_str8_pushf(scratch.arena,
                                        "%.*s() in %.*s:%i:",
                                        oc_str8_ip(entry->function),
                                        oc_str8_ip(entry->file),
                                        entry->line);
            oc_ui_label_str8(OC_STR8("loc"), loc);
        }
        oc_ui_label_str8(OC_STR8("msg"), entry->msg);
    }
    oc_scratch_end(scratch);
}

void overlay_ui(oc_debug_overlay* overlay)
{
    //////////////////////////////////////////////////////////////////////////////
    //TODO: we should probably pump new log entries from a ring buffer here
    //////////////////////////////////////////////////////////////////////////////

    oc_arena_scope scratch = oc_scratch_begin();

    oc_ui_set_context(overlay->ui);
    oc_canvas_context_select(overlay->context);

    if(overlay->show)
    {
        //TODO: only move if it's not already on the front?
        oc_surface_bring_to_front(overlay->surface);

        oc_vec2 frameSize = oc_surface_get_size(overlay->surface);

        oc_ui_frame(frameSize)
        {
            oc_ui_style_set_color(OC_UI_BG_COLOR, (oc_color){ 0 });
            oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);

            oc_ui_box("overlay-area")
            {
                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1, 1 });
                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 0.6, 1 });
            }

            oc_ui_box("log console")
            {
                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 0.4 });
                oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                oc_ui_style_set_i32(OC_UI_CONSTRAIN_Y, 1);
                oc_ui_style_set_color(OC_UI_BG_COLOR, (oc_color){ 0, 0, 0, 0.5 });

                oc_ui_box("log toolbar")
                {
                    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
                    oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_X);
                    oc_ui_style_set_f32(OC_UI_SPACING, 10);
                    oc_ui_style_set_f32(OC_UI_MARGIN_X, 10);
                    oc_ui_style_set_f32(OC_UI_MARGIN_Y, 10);

                    if(oc_ui_button("clear", "Clear").clicked)
                    {
                        oc_list_for_safe(overlay->logEntries, entry, log_entry, listElt)
                        {
                            oc_list_remove(&overlay->logEntries, &entry->listElt);
                            oc_list_push_front(&overlay->logFreeList, &entry->listElt);
                            overlay->entryCount--;
                        }
                    }
                }

                f32 scrollY = 0;

                oc_ui_box* panel = oc_ui_box("log-view")
                {
                    scrollY = panel->scroll.y;

                    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1, 1 });
                    oc_ui_style_set_i32(OC_UI_OVERFLOW_Y, OC_UI_OVERFLOW_SCROLL);

                    oc_ui_box("contents")
                    {
                        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
                        oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                        oc_ui_style_set_f32(OC_UI_MARGIN_Y, 5);

                        oc_list_for(overlay->logEntries, entry, log_entry, listElt)
                        {
                            log_entry_ui(overlay, entry);
                        }
                    }
                }

                if(overlay->logScrollToLast)
                {
                    if(panel->scroll.y >= scrollY)
                    {
                        panel->scroll.y = oc_clamp_low(panel->childrenSum[1] - panel->rect.h, 0);
                    }
                    else
                    {
                        overlay->logScrollToLast = false;
                    }
                }
                else if(panel->scroll.y >= (panel->childrenSum[1] - panel->rect.h) - 1)
                {
                    overlay->logScrollToLast = true;
                }
            }
        }

        oc_ui_draw();
    }
    else
    {
        //TODO: only move if it's not already on the back?
        oc_surface_send_to_back(overlay->surface);
        oc_set_color_rgba(0, 0, 0, 0);
        oc_clear();
    }

    oc_canvas_render(overlay->renderer, overlay->context, overlay->surface);
    oc_canvas_present(overlay->renderer, overlay->surface);

    oc_scratch_end(scratch);
}

//------------------------------------------------------------------------
// Source tree
//------------------------------------------------------------------------

void oc_debugger_print_source_tree(wa_source_node* node, int indent)
{
    for(int i = 0; i < indent; i++)
    {
        printf("  ");
    }
    printf("%.*s\n", oc_str8_ip(node->name));
    oc_list_for(node->children, child, wa_source_node, listElt)
    {
        oc_debugger_print_source_tree(child, indent + 1);
    }
}

void oc_debugger_build_source_tree(oc_arena* arena, wa_source_node* sourceTree, u64 fileCount, wa_source_file* files)
{
    //NOTE: build the source tree from the source files array
    u64 nodeCount = 0;
    for(u64 fileIndex = 1; fileIndex < fileCount; fileIndex++)
    {
        wa_source_file* file = &files[fileIndex];

        oc_arena_scope scratch = oc_scratch_begin_next(arena);

        //NOTE: add the file's root to the source tree
        wa_source_node* root = 0;

        oc_list_for(sourceTree->children, child, wa_source_node, listElt)
        {
            if(!oc_str8_cmp(child->name, file->rootPath))
            {
                root = child;
                break;
            }
        }

        if(!root)
        {
            root = oc_arena_push_type(arena, wa_source_node);

            root->name = file->rootPath;
            root->id = nodeCount;
            nodeCount++;
            oc_list_push_back(&sourceTree->children, &root->listElt);
        }

        wa_source_node* currentNode = root;

        //NOTE: traverse the root's subtree and add nodes as needed
        oc_str8 relativePath = oc_str8_slice(file->fullPath, file->rootPath.len, file->fullPath.len);
        oc_str8_list pathElements = oc_path_split(scratch.arena, relativePath);

        oc_str8_list_for(pathElements, eltName)
        {
            wa_source_node* found = 0;
            oc_list_for(currentNode->children, child, wa_source_node, listElt)
            {
                if(!oc_str8_cmp(child->name, eltName->string))
                {
                    found = child;
                    break;
                }
            }
            if(!found)
            {
                //add the element
                found = oc_arena_push_type(arena, wa_source_node);
                memset(found, 0, sizeof(wa_source_node));

                found->parent = currentNode;
                //TODO: don't set file index for directory nodes
                found->index = fileIndex;
                found->name = eltName->string;
                found->id = nodeCount;
                nodeCount++;
                oc_list_push_back(&currentNode->children, &found->listElt);
            }
            else
            {
                //just continue
            }
            currentNode = found;
        }
        oc_scratch_end(scratch);
    }

    //DEBUG: oc_debugger_print_source_tree(sourceTree, 0);
}

wa_source_node* find_source_node(wa_source_node* node, u64 fileIndex)
{
    if(oc_list_empty(node->children) && node->index == fileIndex)
    {
        return node;
    }

    oc_list_for(node->children, child, wa_source_node, listElt)
    {
        wa_source_node* found = find_source_node(child, fileIndex);
        if(found)
        {
            return found;
        }
    }
    return 0;
}

oc_debugger_value* debugger_build_value_tree(oc_arena* arena, oc_str8 name, wa_debug_type* type, oc_str8 data, oc_list oldValues)
{
    oc_debugger_value* value = oc_arena_push_type(arena, oc_debugger_value);
    value->name = name;
    value->type = type;
    value->data = data;

    wa_debug_type* strippedType = wa_debug_type_strip(type);

    //NOTE: by defaut, expand struct, union, and small arrays
    if(strippedType->kind == WA_DEBUG_TYPE_STRUCT
       || strippedType->kind == WA_DEBUG_TYPE_UNION
       || (strippedType->kind == WA_DEBUG_TYPE_ARRAY && strippedType->array.count <= 6))
    {
        value->expanded = true;
    }

    oc_list oldChildren = { 0 };
    oc_list_for(oldValues, oldValue, oc_debugger_value, listElt)
    {
        if(!oc_str8_cmp(oldValue->name, value->name)
           && oldValue->type == value->type)
        {
            //NOTE: if value was present in previous value tree, carry over its persistant state
            value->expanded = oldValue->expanded;
            oldChildren = oldValue->children;
            break;
        }
    }

    if(strippedType->kind == WA_DEBUG_TYPE_STRUCT || strippedType->kind == WA_DEBUG_TYPE_UNION)
    {
        oc_list_for(strippedType->fields, field, wa_debug_type_field, listElt)
        {
            wa_debug_type* fieldStrippedType = wa_debug_type_strip(field->type);
            u64 fieldSize = fieldStrippedType->size;
            oc_str8 fieldData = oc_str8_slice(data, field->offset, field->offset + fieldSize);
            oc_debugger_value* fieldVal = debugger_build_value_tree(arena, field->name, field->type, fieldData, oldChildren);

            oc_list_push_back(&value->children, &fieldVal->listElt);
        }
    }
    else if(strippedType->kind == WA_DEBUG_TYPE_ARRAY)
    {
        wa_debug_type* eltType = wa_debug_type_strip(strippedType->array.type);

        for(u64 i = 0; i < strippedType->array.count; i++)
        {
            oc_str8 eltData = oc_str8_slice(data, i * eltType->size, (i + 1) * eltType->size);
            oc_debugger_value* eltVal = debugger_build_value_tree(arena, (oc_str8){ 0 }, eltType, eltData, oldChildren);

            oc_list_push_back(&value->children, &eltVal->listElt);
        }
    }

    return value;
}

oc_list debugger_build_locals_tree(oc_arena* arena, wa_debug_function* funcInfo, wa_interpreter* interpreter, oc_list oldValues)
{
    oc_list list = { 0 };

    oc_arena_scope scratch = oc_scratch_begin_next(arena);
    wa_debug_variable** shadow = oc_arena_push_array(scratch.arena, wa_debug_variable*, funcInfo->totalVarDecl);
    u64 shadowCount = 0;

    wa_debug_scope* scope = wa_debug_get_current_scope(interpreter);

    while(scope)
    {
        //NOTE: process vars in scope
        for(u64 varIndex = 0; varIndex < scope->count; varIndex++)
        {
            wa_debug_variable* var = &scope->vars[varIndex];

            //NOTE: dumb n^2 shadow check
            bool shadowed = false;
            for(u64 shadowIndex = 0; shadowIndex < shadowCount; shadowIndex++)
            {
                if(!oc_str8_cmp(shadow[shadowIndex]->name, var->name))
                {
                    shadowed = true;
                }
            }

            if(!shadowed)
            {
                oc_str8 data = wa_debug_variable_get_value(arena, interpreter, funcInfo, var);

                oc_debugger_value* value = debugger_build_value_tree(arena, var->name, var->type, data, oldValues);
                oc_list_push_back(&list, &value->listElt);

                shadow[shadowCount] = var;
                shadowCount++;
            }
        }
        scope = scope->parent;
    }

    oc_scratch_end(scratch);

    return (list);
}

oc_list debugger_build_globals_tree(oc_arena* arena, wa_debug_unit* unit, wa_interpreter* interpreter, oc_list oldValues)
{
    oc_list list = { 0 };

    for(u64 globalIndex = 0; globalIndex < unit->globalCount; globalIndex++)
    {
        wa_debug_variable* var = &unit->globals[globalIndex];

        oc_str8 data = wa_debug_variable_get_value(arena, interpreter, 0, var);
        oc_debugger_value* value = debugger_build_value_tree(arena, var->name, var->type, data, oldValues);
        oc_list_push_back(&list, &value->listElt);
    }

    return list;
}

//------------------------------------------------------------------------
// Debugger Window
//------------------------------------------------------------------------

i32 create_debug_window_callback(void* user)
{
    oc_runtime* app = (oc_runtime*)user;

    oc_rect rect = oc_window_get_frame_rect(app->window);
    rect.x += 100;
    rect.y += 100;
    rect.w = 1100;
    rect.h = 800;

    app->debugger.window = oc_window_create(rect, OC_STR8("Orca Debugger"), 0);
    oc_window_bring_to_front(app->debugger.window);
    oc_window_focus(app->debugger.window);

    return (0);
}

void oc_debugger_open(oc_runtime* app)
{
    oc_debugger* debugger = &app->debugger;
    if(!debugger->init)
    {
        //NOTE: window needs to be created on main thread
        oc_dispatch_on_main_thread_sync(create_debug_window_callback, app);

        debugger->renderer = oc_canvas_renderer_create();
        debugger->surface = oc_canvas_surface_create_for_window(debugger->renderer, debugger->window);

        debugger->canvas = oc_canvas_context_create();
        debugger->ui = oc_ui_context_create(app->debugOverlay.fontReg);

        wa_source_info* sourceInfo = &app->env.module->debugInfo->sourceInfo;
        oc_debugger_build_source_tree(&app->env.arena, &app->debugger.sourceTree, sourceInfo->fileCount, sourceInfo->files);

        for(u32 i = 0; i < 2; i++)
        {
            oc_arena_init(&debugger->valuesArena[i]);
        }

        debugger->init = true;
    }
}

void oc_debugger_close(oc_runtime* app)
{
    oc_debugger* debugger = &app->debugger;
    if(debugger->init)
    {
        oc_ui_context_destroy(debugger->ui);
        oc_canvas_context_destroy(debugger->canvas);
        oc_surface_destroy(debugger->surface);
        oc_canvas_renderer_destroy(debugger->renderer);
        oc_window_destroy(debugger->window);

        for(u32 i = 0; i < 2; i++)
        {
            oc_arena_cleanup(&debugger->valuesArena[i]);
        }
        memset(debugger, 0, sizeof(oc_debugger));
    }
}

//------------------------------------------------------------------------
// Debugger UI
//------------------------------------------------------------------------

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

////////////////////////////////////////////////////////////////////////////:
//TODO: declare this guy somewhere more appropriate
wa_instr_op wa_breakpoint_saved_opcode(wa_breakpoint* bp);

/////////////////////////////////////////////////////////////////////////////

void source_tree_ui(oc_debugger* debugger, oc_ui_box* panel, wa_source_node* node, int indent)
{
    //TODO: use full path to disambiguate similarly named root dirs
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 id = oc_str8_pushf(scratch.arena, "path-%llu", node->id);

    oc_ui_box_str8(id)
    {
        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
        oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_X);
        oc_ui_style_set_f32(OC_UI_MARGIN_X, 5);
        oc_ui_style_set_f32(OC_UI_MARGIN_Y, 5);

        oc_ui_box("indent")
        {
            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 10 * indent });
            oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
        }

        if(!oc_list_empty(node->children))
        {
            if(node->expanded)
            {
                oc_ui_label("expand-icon", "- ");
            }
            else
            {
                oc_ui_label("expand-icon", "+ ");
            }
        }
        else
        {
            oc_ui_label("expand-icon", "  ");
        }
        oc_ui_label_str8(OC_STR8("label"), oc_path_slice_filename(node->name));

        oc_ui_sig sig = oc_ui_get_sig();
        if(sig.hover)
        {
            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_BG_3);
        }
        if(sig.pressed)
        {
            if(oc_list_empty(node->children))
            {
                debugger->selectedFile = node;
                debugger->autoScroll = false;
                debugger->freshScroll = true;
            }
            else
            {
                node->expanded = !node->expanded;
            }
        }
        if(node == debugger->selectedFile)
        {
            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_PRIMARY);

            //NOTE: auto-scroll
            f32 lineY = oc_ui_box_top()->rect.y + panel->scroll.y;
            f32 lineH = oc_ui_box_top()->rect.h;

            f32 targetScroll = panel->scroll.y;

            if(debugger->autoScroll)
            {
                f32 scrollMargin = panel->rect.h * 0.3;

                if(lineY - targetScroll < scrollMargin)
                {
                    targetScroll = lineY - scrollMargin;
                }
                else if(lineY + lineH - targetScroll > panel->rect.h - scrollMargin)
                {
                    targetScroll = lineY + lineH - panel->rect.h + scrollMargin;
                }
            }
            panel->scroll.y += 0.3 * (targetScroll - panel->scroll.y);
        }
    }

    oc_scratch_end(scratch);

    if(node->expanded)
    {
        oc_list_for(node->children, child, wa_source_node, listElt)
        {
            source_tree_ui(debugger, panel, child, indent + 1);
        }
    }
}

void debugger_show_value(oc_str8 name, oc_debugger_value* value, u32 indent, u64* uid, bool showType, wa_interpreter* interpreter, oc_debugger* debugger)
{
    wa_debug_type* strippedType = wa_debug_type_strip(value->type);
    OC_ASSERT(strippedType);

    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 uidStr = oc_str8_pushf(scratch.arena, "%llu", *uid);
    (*uid)++;

    oc_ui_box_str8(uidStr)
    {
        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
        oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
        oc_ui_style_set_f32(OC_UI_SPACING, 5);

        oc_ui_box("object")
        {
            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
            oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
            oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_X);

            oc_ui_box("indent")
            {
                //TODO: make this an integer number of spaces
                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 20 * indent });
            }

            if(strippedType->kind == WA_DEBUG_TYPE_STRUCT
               || strippedType->kind == WA_DEBUG_TYPE_UNION
               || strippedType->kind == WA_DEBUG_TYPE_ARRAY
               || strippedType->kind == WA_DEBUG_TYPE_POINTER)
            {
                if(oc_ui_box_get_sig(oc_ui_box_top()).pressed)
                {
                    value->expanded = !value->expanded;
                }

                if(value->expanded)
                {
                    oc_ui_label("expand", "- ");
                }
                else
                {
                    oc_ui_label("expand", "+ ");
                }
            }
            else
            {
                oc_ui_label("spacer", "  ");
            }

            if(name.len)
            {
                oc_str8 nameStr = oc_str8_pushf(scratch.arena, "%.*s = ", oc_str8_ip(name));
                oc_ui_label_str8(OC_STR8("name"), nameStr);
            }

            if(showType)
            {
                if(value->type->name.len)
                {
                    oc_str8 typeStr = oc_str8_pushf(scratch.arena, "(%.*s) ", oc_str8_ip(value->type->name));
                    oc_ui_label_str8(OC_STR8("type"), typeStr);
                }
                else if(strippedType->kind == WA_DEBUG_TYPE_UNION)
                {
                    oc_ui_label("type", "(anonymous union) ");
                }
                else if(strippedType->kind == WA_DEBUG_TYPE_STRUCT)
                {
                    oc_ui_label("type", "(anonymous struct) ");
                }
                else if(strippedType->kind == WA_DEBUG_TYPE_ARRAY)
                {
                    wa_debug_type* eltType = strippedType->array.type;
                    oc_str8 typeStr = oc_str8_pushf(scratch.arena, "((%.*s)[%llu]) ", oc_str8_ip(eltType->name), strippedType->array.count);
                    oc_ui_label_str8(OC_STR8("type"), typeStr);
                }
            }

            if(!value->data.len)
            {
                oc_ui_label("unavailable", "unavailable");
            }
            else if(strippedType->kind == WA_DEBUG_TYPE_BASIC)
            {
                switch(strippedType->encoding)
                {
                    case WA_DEBUG_TYPE_BOOL:
                    {
                        //TODO
                    }
                    break;

                    case WA_DEBUG_TYPE_UNSIGNED:
                    {
                        oc_str8 valStr = { 0 };
                        switch(strippedType->size)
                        {
                            case 1:
                            {
                                valStr = oc_str8_pushf(scratch.arena, "%hhu", *value->data.ptr);
                            }
                            break;

                            case 2:
                            {
                                u16 u = 0;
                                memcpy(&u, value->data.ptr, sizeof(u16));
                                valStr = oc_str8_pushf(scratch.arena, "%hu", u);
                            }
                            break;

                            case 4:
                            {
                                u32 u = 0;
                                memcpy(&u, value->data.ptr, sizeof(u32));
                                valStr = oc_str8_pushf(scratch.arena, "%u", u);
                            }
                            break;

                            case 8:
                            {
                                u64 u = 0;
                                memcpy(&u, value->data.ptr, sizeof(u64));
                                valStr = oc_str8_pushf(scratch.arena, "%llu", u);
                            }
                            break;

                            default:
                                valStr = oc_str8_pushf(scratch.arena, "unsupported size %llu", strippedType->size);
                                break;
                        }
                        oc_ui_label_str8(OC_STR8("value"), valStr);
                    }
                    break;

                    case WA_DEBUG_TYPE_SIGNED:
                    {
                        oc_str8 valStr = { 0 };
                        switch(strippedType->size)
                        {
                            case 1:
                            {
                                valStr = oc_str8_pushf(scratch.arena, "%hhi", *value->data.ptr);
                            }
                            break;

                            case 2:
                            {
                                i16 i = 0;
                                memcpy(&i, value->data.ptr, sizeof(i16));
                                valStr = oc_str8_pushf(scratch.arena, "%hi", i);
                            }
                            break;

                            case 4:
                            {
                                i32 i = 0;
                                memcpy(&i, value->data.ptr, sizeof(i32));
                                valStr = oc_str8_pushf(scratch.arena, "%i", i);
                            }
                            break;

                            case 8:
                            {
                                i64 i = 0;
                                memcpy(&i, value->data.ptr, sizeof(i64));
                                valStr = oc_str8_pushf(scratch.arena, "%lli", i);
                            }
                            break;

                            default:
                                valStr = oc_str8_pushf(scratch.arena, "unsupported size %llu", strippedType->size);
                                break;
                        }
                        oc_ui_label_str8(OC_STR8("value"), valStr);
                    }
                    break;

                    case WA_DEBUG_TYPE_FLOAT:
                    {
                        oc_str8 valStr = { 0 };
                        if(strippedType->size == 4)
                        {
                            f32 f = 0;
                            memcpy(&f, value->data.ptr, strippedType->size);
                            valStr = oc_str8_pushf(scratch.arena, "%f", f);
                        }
                        else if(strippedType->size == 8)
                        {
                            f64 f = 0;
                            memcpy(&f, value->data.ptr, strippedType->size);
                            valStr = oc_str8_pushf(scratch.arena, "%f", f);
                        }

                        oc_ui_label_str8(OC_STR8("value"), valStr);
                    }
                    break;
                }
            }
            else if(strippedType->kind == WA_DEBUG_TYPE_POINTER)
            {
                u32 addr = 0;
                memcpy(&addr, value->data.ptr, sizeof(u32));
                oc_str8 valStr = oc_str8_pushf(scratch.arena, "0x%08x", addr);
                oc_ui_label_str8(OC_STR8("value"), valStr);
            }
        }

        if(value->data.len && value->expanded)
        {
            if(strippedType->kind == WA_DEBUG_TYPE_STRUCT || strippedType->kind == WA_DEBUG_TYPE_UNION)
            {
                oc_list_for(value->children, child, oc_debugger_value, listElt)
                {
                    debugger_show_value(child->name, child, indent + 1, uid, true, interpreter, debugger);
                }
            }
            else if(strippedType->kind == WA_DEBUG_TYPE_ARRAY)
            {
                oc_list_for_indexed(value->children, it, oc_debugger_value, listElt)
                {
                    oc_str8 indexStr = oc_str8_pushf(scratch.arena, "[%llu]", it.index);
                    debugger_show_value(indexStr, it.elt, indent + 1, uid, false, interpreter, debugger);
                }
            }
            else if(strippedType->kind == WA_DEBUG_TYPE_POINTER)
            {
                if(oc_list_empty(value->children))
                {
                    wa_debug_type* pointeeType = wa_debug_type_strip(strippedType->type);
                    u32 size = pointeeType->size;
                    u32 addr = 0;
                    memcpy(&addr, value->data.ptr, sizeof(addr));

                    wa_memory* memory = interpreter->instance->memories[0];

                    if(addr + size >= memory->limits.min * WA_PAGE_SIZE || addr + size < addr)
                    {
                        //NOTE: bogus address, don't load.
                        //TODO: maybe later create a special error node?
                    }
                    else
                    {
                        oc_arena* valuesArena = &debugger->valuesArena[debugger->valuesArenaIndex];
                        oc_str8 data = oc_str8_push_copy(valuesArena,
                                                         (oc_str8){
                                                             .ptr = memory->ptr + addr,
                                                             .len = pointeeType->size,
                                                         });

                        oc_debugger_value* pointee = debugger_build_value_tree(valuesArena, (oc_str8){ 0 }, strippedType->type, data, (oc_list){ 0 });

                        oc_list_push_back(&value->children, &pointee->listElt);
                    }
                }

                oc_list_for(value->children, child, oc_debugger_value, listElt)
                {
                    debugger_show_value(child->name, child, indent + 1, uid, true, interpreter, debugger);
                }
            }
        }
    }

    oc_scratch_end(scratch);
}

void debugger_ui_update(oc_debugger* debugger, oc_wasm_env* env)
{

    wa_interpreter* interpreter = env->interpreter;

    oc_arena_scope scratch = oc_scratch_begin();

    oc_ui_set_context(debugger->ui);
    oc_canvas_context_select(debugger->canvas);

    oc_vec2 frameSize = oc_surface_get_size(debugger->surface);

    oc_ui_frame(frameSize)
    {
        const f32 panelSpacing = 2;
        oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_X);
        oc_ui_style_set_f32(OC_UI_SPACING, panelSpacing);
        oc_ui_style_set_i32(OC_UI_CONSTRAIN_X, 1);

        oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_BG_4);

        static i64 selectedFunction = -1;
        static i64 selectedFrame = 0;

        f32 scrollSpeed = 0.5;
        debugger->freshScroll = false;

        wa_source_info* sourceInfo = &env->module->debugInfo->sourceInfo;

        //NOTE: if paused == true here, vm thread can not unpause until next frame.
        //      if paused == false, vm thread can become paused during the frame, but we
        //      don't really care (we render cached state for this frame and will update next frame)
        bool paused = atomic_load(&env->paused);

        u32 oldSelectedFunction = selectedFunction;

        if(paused != env->prevPaused)
        {
            if(paused == true)
            {
                //NOTE: vm thread has become paused since last frame. We set autoscroll and autoselect the function
                debugger->autoScroll = true;

                wa_source_node* oldSelectedFile = debugger->selectedFile;
                u64 oldSelectedFunction = selectedFunction;
                //NOTE: select new function and file

                selectedFrame = 0;

                wa_func* execFunc = interpreter->controlStack[interpreter->controlStackTop].func;
                selectedFunction = execFunc - env->instance->functions;

                wa_warm_loc warmLoc = {
                    env->instance->module,
                    selectedFunction,
                    interpreter->pc - execFunc->code,
                };
                wa_line_loc lineLoc = wa_line_loc_from_warm_loc(env->module, warmLoc);
                if(lineLoc.fileIndex)
                {
                    //TODO: faster lookup
                    debugger->selectedFile = find_source_node(&debugger->sourceTree, lineLoc.fileIndex);

                    //NOTE: expand all parents of selected file
                    wa_source_node* parent = debugger->selectedFile->parent;
                    while(parent)
                    {
                        parent->expanded = true;
                        parent = parent->parent;
                    }
                }

                if((debugger->showSymbols == true && selectedFunction != oldSelectedFunction)
                   || (debugger->showSymbols == false && debugger->selectedFile != oldSelectedFile))
                {
                    scrollSpeed = 1;
                    debugger->freshScroll = true;
                }

                //NOTE: build value locals value tree
                wa_debug_function* debugFunc = &env->module->debugInfo->functionLocals[selectedFunction];

                debugger->valuesArenaIndex ^= 1;
                oc_arena* valuesArena = &debugger->valuesArena[debugger->valuesArenaIndex];
                oc_arena_clear(valuesArena);

                oc_list oldLocals = { 0 };
                if(selectedFunction == oldSelectedFunction)
                {
                    oldLocals = debugger->locals;
                }

                debugger->locals = debugger_build_locals_tree(valuesArena, debugFunc, interpreter, oldLocals);
                debugger->globals = debugger_build_globals_tree(valuesArena, debugFunc->unit, interpreter, debugger->globals);
            }
            env->prevPaused = paused;
        }

        //TODO: make these adjustable
        static f32 procPanelSize = 400;
        static f32 bottomPanelHeight = 350;

        oc_ui_box("left-panel")
        {
            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, procPanelSize });
            oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
            oc_ui_style_set_i32(OC_UI_CONSTRAIN_Y, 1);
            oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);

            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_BG_4);
            oc_ui_style_set_f32(OC_UI_SPACING, panelSpacing);

            oc_ui_box("browser")
            {
                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1, 1 });
                oc_ui_style_set_i32(OC_UI_CONSTRAIN_Y, 1);
                oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);

                oc_ui_box("browser-tabs")
                {
                    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });

                    oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_BG_2);

                    oc_ui_box* optFiles = oc_ui_box("option-files")
                    {
                        oc_ui_set_text(OC_STR8("Files"));

                        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_TEXT });
                        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_TEXT });
                        oc_ui_style_set_i32(OC_UI_ALIGN_X, OC_UI_ALIGN_CENTER);
                        oc_ui_style_set_i32(OC_UI_ALIGN_Y, OC_UI_ALIGN_CENTER);
                        oc_ui_style_set_var_str8(OC_UI_MARGIN_X, OC_UI_THEME_SPACING_TIGHT);
                        oc_ui_style_set_var_str8(OC_UI_MARGIN_Y, OC_UI_THEME_SPACING_EXTRA_TIGHT);

                        oc_ui_style_set_var_str8(OC_UI_FONT, OC_UI_THEME_FONT_REGULAR);
                        oc_ui_style_set_f32(OC_UI_TEXT_SIZE, 12);
                        oc_ui_style_set_var_str8(OC_UI_COLOR, OC_UI_THEME_TEXT_0);

                        if(oc_ui_get_sig().pressed)
                        {
                            debugger->showSymbols = false;
                        }
                    }

                    oc_ui_box* optSymbols = oc_ui_box("option-symbols")
                    {
                        oc_ui_set_text(OC_STR8("Symbols"));

                        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_TEXT });
                        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_TEXT });
                        oc_ui_style_set_i32(OC_UI_ALIGN_X, OC_UI_ALIGN_CENTER);
                        oc_ui_style_set_i32(OC_UI_ALIGN_Y, OC_UI_ALIGN_CENTER);
                        oc_ui_style_set_var_str8(OC_UI_MARGIN_X, OC_UI_THEME_SPACING_TIGHT);
                        oc_ui_style_set_var_str8(OC_UI_MARGIN_Y, OC_UI_THEME_SPACING_EXTRA_TIGHT);

                        oc_ui_style_set_var_str8(OC_UI_FONT, OC_UI_THEME_FONT_REGULAR);
                        oc_ui_style_set_f32(OC_UI_TEXT_SIZE, 12);
                        oc_ui_style_set_var_str8(OC_UI_COLOR, OC_UI_THEME_TEXT_0);

                        if(oc_ui_get_sig().pressed)
                        {
                            debugger->showSymbols = true;
                        }
                    }

                    oc_ui_style_rule(debugger->showSymbols ? "option-symbols" : "option-files")
                    {
                        oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_PRIMARY);
                    }
                }

                oc_ui_box("browser-contents")
                {
                    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1, 1 });
                    oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_BG_1);
                    oc_ui_style_set_i32(OC_UI_OVERFLOW_Y, OC_UI_OVERFLOW_SCROLL);

                    oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                    oc_ui_style_set_f32(OC_UI_MARGIN_Y, 5);

                    if(debugger->showSymbols)
                    {
                        for(u32 funcIndex = 0; funcIndex < env->module->functionCount; funcIndex++)
                        {
                            wa_func* func = &env->instance->functions[funcIndex];
                            if(func->codeLen)
                            {
                                oc_str8 name = wa_module_get_function_name(env->module, funcIndex);

                                oc_ui_box* box = oc_ui_box_str8(name)
                                {
                                    oc_ui_set_text(name);

                                    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                                    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_TEXT });
                                    oc_ui_style_set_f32(OC_UI_MARGIN_X, 10);
                                    oc_ui_style_set_f32(OC_UI_MARGIN_Y, 5);

                                    oc_ui_style_set_var_str8(OC_UI_FONT, OC_UI_THEME_FONT_REGULAR);
                                    oc_ui_style_set_f32(OC_UI_TEXT_SIZE, 12);
                                    oc_ui_style_set_var_str8(OC_UI_COLOR, OC_UI_THEME_TEXT_0);

                                    if(selectedFunction == funcIndex)
                                    {
                                        oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_PRIMARY);
                                    }
                                    else
                                    {
                                        oc_ui_style_rule(".hover")
                                        {
                                            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_BG_3);
                                        }
                                    }

                                    oc_ui_sig sig = oc_ui_get_sig();
                                    if(sig.pressed)
                                    {
                                        selectedFunction = funcIndex;
                                        debugger->autoScroll = false;
                                        debugger->freshScroll = true;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        oc_list_for(debugger->sourceTree.children, child, wa_source_node, listElt)
                        {
                            source_tree_ui(debugger, oc_ui_box_top(), child, 0);
                        }
                    }
                }
            }

            oc_ui_box("callstack")
            {
                //TODO: review layout once we have a simpler way of expressing a list of equal rows from unequal items

                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, bottomPanelHeight });

                oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                oc_ui_style_set_i32(OC_UI_CONSTRAIN_Y, 1);

                oc_ui_style_set_f32(OC_UI_SPACING, 5);

                oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_BG_1);

                if(env->paused)
                {

                    oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_BG_1);

                    oc_ui_box("title")
                    {
                        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
                        oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                        oc_ui_style_set_f32(OC_UI_MARGIN_X, 5);
                        oc_ui_style_set_f32(OC_UI_MARGIN_Y, 5);

                        oc_ui_label("title", "Callstack:");
                        oc_ui_label("spacer", " ");
                    }

                    oc_ui_box* callstackScrollPanel = oc_ui_box("callstack-scroll-panel")
                    {
                        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1, 1 });

                        oc_ui_style_set_i32(OC_UI_OVERFLOW_X, OC_UI_OVERFLOW_SCROLL);
                        oc_ui_style_set_i32(OC_UI_OVERFLOW_Y, OC_UI_OVERFLOW_SCROLL);

                        oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);

                        oc_ui_box* callstackContents = oc_ui_box("callstackContents")
                        {
                            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
                            oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });

                            oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                            oc_ui_style_set_f32(OC_UI_MARGIN_Y, 5);

                            for(u32 level = 0; level <= interpreter->controlStackTop; level++)
                            {
                                wa_func* func = interpreter->controlStack[level].func;
                                u64 addr = 0;
                                if(level == interpreter->controlStackTop)
                                {
                                    addr = interpreter->pc - func->code;
                                }
                                else
                                {
                                    addr = interpreter->controlStack[level + 1].returnPC - 2 - func->code;
                                }
                                u32 functionIndex = func - interpreter->instance->functions;
                                oc_str8 name = wa_module_get_function_name(interpreter->instance->module, functionIndex);

                                oc_str8 label = oc_str8_pushf(scratch.arena, "label-%i", level);
                                oc_str8 text = oc_str8_pushf(scratch.arena, "[%i] %.*s + 0x%08llx", level, oc_str8_ip(name), addr);

                                oc_ui_style_rule(".label.hover")
                                {
                                    oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_BG_3);
                                }

                                oc_ui_style_rule(".label")
                                {
                                    oc_ui_style_set_f32(OC_UI_MARGIN_X, 5);
                                    oc_ui_style_set_f32(OC_UI_MARGIN_Y, 2.5);

                                    //TODO: this is a hack because we don't have another option to grow labels to the parent for now
                                    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_TEXT, .minSize = callstackContents->rect.w });
                                }

                                if(interpreter->controlStackTop - level == selectedFrame)
                                {
                                    oc_ui_style_rule_str8(label)
                                    {
                                        oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_PRIMARY);
                                    }
                                }

                                if(oc_ui_label_str8(label, text).pressed)
                                {
                                    selectedFrame = interpreter->controlStackTop - level;
                                }
                            }
                        }
                    }
                }
            }
        }

        oc_ui_box* codePanel = oc_ui_box("code-panel")
        {
            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1, 1 });
            oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
            oc_ui_style_set_i32(OC_UI_CONSTRAIN_Y, 1);
            oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
            oc_ui_style_set_f32(OC_UI_SPACING, panelSpacing);

            oc_ui_box* codeView = oc_ui_box("code-view")
            {
                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1, 1 });
                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1, 1 });
                oc_ui_style_set_i32(OC_UI_OVERFLOW_Y, OC_UI_OVERFLOW_SCROLL);
                oc_ui_style_set_i32(OC_UI_OVERFLOW_X, OC_UI_OVERFLOW_SCROLL);

                oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_BG_1);

                const i32 BOX_MARGIN_H = 2;
                const i32 BOX_MARGIN_W = 2;

                if(debugger->freshScroll)
                {
                    oc_log_info("[%f] freshScroll, set scroll to 0\n", oc_ui_frame_time());
                    codeView->scroll.y = 0;
                }

                if(debugger->showSymbols)
                {
                    if(selectedFunction >= 0)
                    {
                        wa_func* func = &env->instance->functions[selectedFunction];
                        oc_str8 funcName = wa_module_get_function_name(env->module, selectedFunction);
                        /*
                    if(funcName.len)
                    {
                        oc_str8_list_push(scratch.arena, &list, funcName);
                    }
                    else
                    {
                        oc_str8_list_pushf(scratch.arena, &list, "%i", selectedFunction);
                    }

                    oc_str8_list_push(scratch.arena, &list, OC_STR8(" "));
                    push_func_type_str8_list(scratch.arena, &list, func->type);


                    oc_str8 funcText = oc_str8_list_join(scratch.arena, list);
                */
                        oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                        oc_ui_style_set_f32(OC_UI_MARGIN_X, 5);
                        oc_ui_style_set_f32(OC_UI_MARGIN_Y, 5);

                        oc_ui_box* funcLabel = oc_ui_label_str8(OC_STR8("func-label"), funcName).box;

                        //TODO: compute line height from font.
                        f32 lineH = funcLabel->rect.h;
                        f32 lineY = funcLabel->rect.h + 10;
                        f32 lineMargin = 2;

                        oc_ui_box_str8(funcName)
                        {
                            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                            oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                            oc_ui_style_set_f32(OC_UI_MARGIN_X, 5);
                            oc_ui_style_set_f32(OC_UI_MARGIN_Y, 5);

                            for(u64 codeIndex = 0; codeIndex < func->codeLen; codeIndex++)
                            {
                                u64 startIndex = codeIndex;

                                wa_code* c = &func->code[codeIndex];
                                wa_instr_op opcode = c->opcode;

                                wa_breakpoint* breakpoint = wa_interpreter_find_breakpoint(
                                    interpreter,
                                    &(wa_warm_loc){
                                        .module = interpreter->instance->module,
                                        .funcIndex = selectedFunction,
                                        .codeIndex = codeIndex,
                                    });

                                //TODO: should probably not intertwine modified bytecode and UI like that?
                                {
                                    wa_breakpoint* anyBreakpoint = wa_interpreter_find_breakpoint_any(
                                        interpreter,
                                        &(wa_warm_loc){
                                            .module = interpreter->instance->module,
                                            .funcIndex = selectedFunction,
                                            .codeIndex = codeIndex,
                                        });

                                    if(anyBreakpoint)
                                    {
                                        //TODO: find _any_ breakpoint (could be a line one) that might modify opcodes
                                        opcode = wa_breakpoint_saved_opcode(anyBreakpoint);
                                    }
                                }

                                const wa_instr_info* info = &wa_instr_infos[opcode];

                                oc_str8 key = oc_str8_pushf(scratch.arena, "0x%08llx", codeIndex);

                                oc_ui_box_str8(key)
                                {
                                    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                                    oc_ui_style_set_f32(OC_UI_MARGIN_Y, lineMargin);
                                    oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);

                                    oc_ui_box("line")
                                    {
                                        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });

                                        bool makeExecCursor = false;
                                        if(env->instance)
                                        {
                                            u32 index = interpreter->pc - func->code;
                                            wa_func* execFunc = interpreter->controlStack[interpreter->controlStackTop].func;

                                            if(func == execFunc && index == codeIndex)
                                            {
                                                makeExecCursor = true;
                                            }
                                        }

                                        if(makeExecCursor)
                                        {
                                            oc_ui_style_set_color(OC_UI_BG_COLOR, (oc_color){ 0.4, 0.7, 0.1, 1, OC_COLOR_SPACE_SRGB });
                                        }

                                        // address
                                        oc_ui_box* label = oc_ui_label_str8(OC_STR8("address"), key).box;

                                        if(makeExecCursor)
                                        {
                                            //NOTE: we compute auto-scroll on label box instead of cursor box, because the cursor box is not permanent,
                                            //      so its rect might not be set every frame, resulting in brief jumps.
                                            //      Maybe the cursor box shouldnt be parented to the function UI namespace and be floating to begin with...

                                            f32 targetScroll = codeView->scroll.y;

                                            if(debugger->autoScroll)
                                            {
                                                f32 scrollMargin = 80;

                                                if(scrollSpeed == 1)
                                                {
                                                    scrollMargin = codeView->rect.h / 2;
                                                }

                                                if(lineY - targetScroll < scrollMargin)
                                                {
                                                    targetScroll = lineY - scrollMargin;
                                                }
                                                else if(lineY + lineH - targetScroll > codeView->rect.h - scrollMargin)
                                                {
                                                    targetScroll = lineY + lineH - codeView->rect.h + scrollMargin;
                                                }
                                            }
                                            if(fabsf(targetScroll - codeView->scroll.y) > 300)
                                            {
                                                f32 delta = oc_clamp(targetScroll - codeView->scroll.y, -300, 300);
                                                codeView->scroll.y = targetScroll - delta;
                                            }
                                            codeView->scroll.y += scrollSpeed * (targetScroll - codeView->scroll.y);
                                        }

                                        // spacer or breakpoint
                                        if(breakpoint)
                                        {
                                            oc_ui_box("bp")
                                            {
                                                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 40 });
                                                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });

                                                oc_ui_set_draw_proc(draw_breakpoint_cursor_proc, 0);

                                                if(oc_ui_get_sig().clicked)
                                                {
                                                    wa_interpreter_remove_breakpoint(interpreter, breakpoint);
                                                }
                                            }
                                        }
                                        else
                                        {
                                            oc_ui_box("spacer")
                                            {
                                                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 40 });
                                                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });

                                                if(oc_ui_get_sig().clicked)
                                                {
                                                    wa_interpreter_add_breakpoint(
                                                        interpreter,
                                                        &(wa_warm_loc){
                                                            .module = interpreter->instance->module,
                                                            .funcIndex = selectedFunction,
                                                            .codeIndex = codeIndex,
                                                        });
                                                }
                                            }
                                        }

                                        oc_ui_box("instruction")
                                        {
                                            oc_ui_style_set_f32(OC_UI_SPACING, 10);

                                            // opcode
                                            oc_ui_label("opcode", wa_instr_strings[opcode]);

                                            // operands
                                            for(u32 opdIndex = 0; opdIndex < info->opdCount; opdIndex++)
                                            {
                                                wa_code* opd = &func->code[codeIndex + opdIndex + 1];
                                                oc_str8 opdKey = oc_str8_pushf(scratch.arena, "opd%u", opdIndex);

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
                                                        s = wa_module_get_function_name(env->module, opd->valU32);
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
                                                        //TODO: defaulting to local index for now, review that after completing wasm tables
                                                        s = oc_str8_pushf(scratch.arena, "r%u", opd->valU32);
                                                        break;
                                                }

                                                oc_ui_label_str8(opdKey, s);
                                            }
                                        }
                                    }
                                    lineY += lineH;
                                    codeIndex += info->opdCount;

                                    if(opcode == WA_INSTR_jump_table)
                                    {
                                        oc_ui_box("jump-table")
                                        {
                                            u64 brCount = func->code[startIndex + 1].valI32;
                                            for(u64 i = 0; i < brCount; i++)
                                            {
                                                codeIndex++;
                                                oc_str8 s = oc_str8_pushf(scratch.arena, "0x%02llx ", func->code[codeIndex].valI64);
                                                oc_ui_label_str8(s, s);
                                            }
                                        }
                                        lineY += lineH;
                                    }
                                }
                                lineY += 2 * lineMargin;
                            }

                            oc_ui_box("vspacer")
                            {
                                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, 20 });
                            }
                        }
                    }
                }
                else if(debugger->selectedFile)
                {
                    oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);

                    wa_source_node* node = debugger->selectedFile;

                    if(!node->contents.len)
                    {
                        oc_str8 path = sourceInfo->files[node->index].fullPath;
                        oc_file file = oc_file_open(path, OC_FILE_ACCESS_READ, OC_FILE_OPEN_NONE);

                        if(!oc_file_is_nil(file))
                        {
                            node->contents.len = oc_file_size(file);
                            node->contents.ptr = oc_malloc_array(char, node->contents.len);
                            oc_file_read(file, node->contents.len, node->contents.ptr);
                        }
                        oc_file_close(file);
                    }

                    if(node->contents.len)
                    {
                        u64 offset = 0;
                        u64 lineNum = 1;

                        //TODO: compute line height from font.
                        f32 lineH = 0;
                        f32 lineY = 0;

                        while(offset < node->contents.len)
                        {
                            u64 lineStart = offset;
                            while(offset < node->contents.len && node->contents.ptr[offset] != '\n')
                            {
                                offset++;
                            }
                            oc_str8 line = oc_str8_slice(node->contents, lineStart, offset);
                            if(!line.len)
                            {
                                line = OC_STR8(" "); //TODO or spacer?
                            }
                            offset++;

                            oc_str8 lineNumStr = oc_str8_pushf(scratch.arena, "%i", lineNum);
                            oc_ui_box_str8(lineNumStr)
                            {
                                lineH = oc_ui_box_top()->rect.h;

                                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });

                                bool makeExecCursor = false;
                                if(env->instance)
                                {
                                    ////////////////////////////////////////////////:
                                    //TODO: haul that up
                                    ////////////////////////////////////////////////
                                    wa_func* execFunc = interpreter->controlStack[interpreter->controlStackTop].func;
                                    u64 funcIndex = execFunc - interpreter->instance->functions;
                                    u32 codeIndex = interpreter->pc - execFunc->code;

                                    wa_line_loc loc = wa_line_loc_from_warm_loc(env->instance->module,
                                                                                (wa_warm_loc){
                                                                                    .module = interpreter->instance->module,
                                                                                    .funcIndex = funcIndex,
                                                                                    .codeIndex = codeIndex,
                                                                                });

                                    if(node->index == loc.fileIndex && loc.line == lineNum)
                                    {
                                        makeExecCursor = true;
                                    }
                                }

                                if(makeExecCursor)
                                {
                                    oc_ui_style_set_color(OC_UI_BG_COLOR, (oc_color){ 0.4, 0.7, 0.1, 1, OC_COLOR_SPACE_SRGB });
                                }

                                oc_ui_box* numLabel = oc_ui_box("num")
                                {
                                    //TODO: we should count the number of lines beforehand to compute the proper max size
                                    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 30 });
                                    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                                    oc_ui_style_set_f32(OC_UI_MARGIN_X, 5);

                                    oc_ui_label_str8(OC_STR8("label"), lineNumStr);
                                }

                                if(makeExecCursor)
                                {
                                    //NOTE: we compute auto-scroll on label box instead of cursor box, because the cursor box is not permanent,
                                    //      so its rect might not be set every frame, resulting in brief jumps.
                                    //      Maybe the cursor box shouldnt be parented to the function UI namespace and be floating to begin with...

                                    f32 targetScroll = codeView->scroll.y;

                                    if(debugger->autoScroll)
                                    {
                                        f32 scrollMargin = 80;

                                        if(scrollSpeed == 1)
                                        {
                                            scrollMargin = codeView->rect.h / 2;
                                        }

                                        if(lineY - targetScroll < scrollMargin)
                                        {
                                            targetScroll = lineY - scrollMargin;
                                        }
                                        else if(lineY + lineH - targetScroll > codeView->rect.h - scrollMargin)
                                        {
                                            targetScroll = lineY + lineH - codeView->rect.h + scrollMargin;
                                        }
                                    }
                                    if(fabsf(targetScroll - codeView->scroll.y) > 200)
                                    {
                                        f32 delta = oc_clamp(targetScroll - codeView->scroll.y, -200, 200);
                                        codeView->scroll.y = targetScroll - delta;
                                    }

                                    codeView->scroll.y += scrollSpeed * (targetScroll - codeView->scroll.y);
                                }

                                wa_breakpoint* breakpoint = wa_interpreter_find_breakpoint_line(interpreter,
                                                                                                &(wa_line_loc){
                                                                                                    .fileIndex = node->index,
                                                                                                    .line = lineNum,
                                                                                                });
                                // spacer or breakpoint
                                if(breakpoint)
                                {
                                    oc_ui_box("bp")
                                    {
                                        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 40 });
                                        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });

                                        oc_ui_set_draw_proc(draw_breakpoint_cursor_proc, 0);

                                        if(oc_ui_get_sig().clicked)
                                        {
                                            wa_interpreter_remove_breakpoint(interpreter, breakpoint);
                                        }
                                    }
                                }
                                else
                                {
                                    oc_ui_box("spacer")
                                    {
                                        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 30 });
                                        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });

                                        if(oc_ui_get_sig().clicked)
                                        {
                                            wa_interpreter_add_breakpoint_line(interpreter,
                                                                               &(wa_line_loc){
                                                                                   .fileIndex = node->index,
                                                                                   .line = lineNum,
                                                                               });
                                        }
                                    }
                                }

                                oc_ui_label_str8(OC_STR8("line"), line);
                            }
                            lineY += lineH;
                            lineNum++;
                        }
                    }
                }
                debugger->lastScroll = codeView->scroll.y;
                //NOTE: scroll might change (as a result of user action) at the end of the block
            }

            if(!debugger->freshScroll && fabs(debugger->lastScroll - codeView->scroll.y) > 1)
            {
                //NOTE: if user has adjusted scroll manually, deactivate auto-scroll
                OC_ASSERT(oc_ui_box_get_sig(codeView).wheel.y != 0);
                debugger->autoScroll = false;
            }

            oc_ui_box* inspector = oc_ui_box("inspector-view")
            {
                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, bottomPanelHeight });
                oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                oc_ui_style_set_i32(OC_UI_OVERFLOW_Y, OC_UI_OVERFLOW_SCROLL);

                oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_BG_1);

                if(env->paused)
                {
                    if(debugger->showSymbols)
                    {
                        oc_ui_style_set_f32(OC_UI_MARGIN_X, 5);
                        oc_ui_style_set_f32(OC_UI_MARGIN_Y, 5);
                        oc_ui_style_set_f32(OC_UI_SPACING, 5);

                        oc_ui_label("title", "Registers:");
                        oc_ui_label("spacer", " ");

                        wa_func* execFunc = interpreter->controlStack[interpreter->controlStackTop].func;
                        u32 funcIndex = execFunc - interpreter->instance->functions;
                        u32 codeIndex = interpreter->pc - execFunc->code;

                        for(u64 regIndex = 0; regIndex < execFunc->maxRegCount; regIndex++)
                        {
                            oc_str8 regId = oc_str8_pushf(scratch.arena, "reg-%llu", regIndex);

                            if(selectedFunction != oldSelectedFunction || interpreter->cachedRegs[regIndex].valI64 != interpreter->locals[regIndex].valI64)
                            {
                                oc_ui_style_rule_str8(regId)
                                {
                                    oc_ui_style_set_var_str8(OC_UI_COLOR, OC_UI_THEME_PRIMARY);
                                }
                            }

                            oc_str8 regText = { 0 };

                            wa_register_map* map = &env->module->debugInfo->registerMaps[funcIndex][regIndex];

                            wa_value_type type = WA_TYPE_UNKNOWN;
                            for(u32 rangeIndex = 0; rangeIndex < map->count; rangeIndex++)
                            {
                                wa_register_range* range = &map->ranges[rangeIndex];
                                if(codeIndex >= range->start && codeIndex <= range->end)
                                {
                                    type = range->type;
                                    break;
                                }
                            }

                            switch(type)
                            {
                                case WA_TYPE_I32:
                                {
                                    regText = oc_str8_pushf(scratch.arena,
                                                            "r%llu (i32) = %i",
                                                            regIndex,
                                                            interpreter->locals[regIndex].valI32);
                                }
                                break;
                                case WA_TYPE_I64:
                                {
                                    regText = oc_str8_pushf(scratch.arena,
                                                            "r%llu (i64) = %lli",
                                                            regIndex,
                                                            interpreter->locals[regIndex].valI64);
                                }
                                break;
                                case WA_TYPE_F32:
                                {
                                    regText = oc_str8_pushf(scratch.arena,
                                                            "r%llu (f32) = %f",
                                                            regIndex,
                                                            interpreter->locals[regIndex].valF32);
                                }
                                break;
                                case WA_TYPE_F64:
                                {
                                    regText = oc_str8_pushf(scratch.arena,
                                                            "r%llu (f64) = %f",
                                                            regIndex,
                                                            interpreter->locals[regIndex].valF64);
                                }
                                break;
                                case WA_TYPE_FUNC_REF:
                                {
                                    regText = oc_str8_pushf(scratch.arena,
                                                            "r%llu (funcref) = 0x%016llx",
                                                            regIndex,
                                                            interpreter->locals[regIndex].valI64);
                                }
                                break;
                                case WA_TYPE_EXTERN_REF:
                                {
                                    regText = oc_str8_pushf(scratch.arena,
                                                            "r%llu (externref) = 0x%016llx",
                                                            regIndex,
                                                            interpreter->locals[regIndex].valI64);
                                }
                                break;
                                default:
                                {
                                    regText = oc_str8_pushf(scratch.arena,
                                                            "r%llu = 0x%016llx",
                                                            regIndex,
                                                            interpreter->locals[regIndex].valI64);
                                }
                                break;
                            }

                            oc_ui_label_str8(regId, regText);
                        }
                    }
                    else
                    {
                        oc_ui_style_set_f32(OC_UI_MARGIN_X, 5);
                        oc_ui_style_set_f32(OC_UI_MARGIN_Y, 5);
                        oc_ui_style_set_f32(OC_UI_SPACING, 5);

                        oc_ui_label("title", "Variables:");
                        oc_ui_label("spacer", " ");

                        oc_ui_style_rule("type.label")
                        {
                            oc_ui_style_set_var_str8(OC_UI_COLOR, OC_UI_THEME_TEXT_2);
                        }
                        oc_ui_style_rule("unavailable.label")
                        {
                            oc_ui_style_set_var_str8(OC_UI_COLOR, OC_UI_THEME_TEXT_2);
                        }

                        u64 varUID = 0;

                        oc_list_for(debugger->locals, val, oc_debugger_value, listElt)
                        {
                            debugger_show_value(val->name, val, 0, &varUID, true, env->interpreter, debugger);
                        }

                        oc_ui_label("spacer2", " ");
                        oc_ui_label("title2", "Globals:");
                        oc_ui_label("spacer3", " ");

                        oc_list_for(debugger->globals, val, oc_debugger_value, listElt)
                        {
                            debugger_show_value(val->name, val, 0, &varUID, true, env->interpreter, debugger);
                        }
                    }
                }
            }
        }
    }

    oc_ui_draw();

    oc_canvas_render(debugger->renderer, debugger->canvas, debugger->surface);
    oc_canvas_present(debugger->renderer, debugger->surface);

    oc_scratch_end(scratch);
}
