/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <errno.h>
#include <math.h>
#include <stdio.h>

#define OC_INCLUDE_GL_API
#include "graphics/graphics_common.h"
#include "orca.h"

#include "runtime.h"
#include "runtime_clipboard.c"
#include "runtime_io.c"
#include "runtime_memory.c"

oc_font orca_font_create(const char* resourcePath)
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

oc_runtime __orcaApp = { 0 };

oc_runtime* oc_runtime_get()
{
    return (&__orcaApp);
}

oc_wasm_env* oc_runtime_get_env()
{
    return (&__orcaApp.env);
}

oc_str8 oc_runtime_get_wasm_memory()
{
    oc_str8 mem = { 0 };
    u32 size = 0;
    mem.ptr = (char*)m3_GetMemory(__orcaApp.env.m3Runtime, &size, 0);
    mem.len = size;
    return (mem);
}

u64 orca_check_cstring(IM3Runtime runtime, const char* ptr)
{
    uint32_t memorySize = 0;
    char* memory = (char*)m3_GetMemory(runtime, &memorySize, 0);

    //NOTE: Here we are guaranteed that ptr is in [ memory ; memory + memorySize [
    //      hence (memory + memorySize) - ptr is representable by size_t and <= memorySize
    size_t maxLen = (memory + memorySize) - ptr;

    u64 len = strnlen(ptr, maxLen);

    if(len == maxLen)
    {
        //NOTE: string overflows wasm memory, return a length that will trigger the bounds check
        len = maxLen + 1;
    }
    return (len + 1); //include null-terminator
}

void oc_bridge_window_set_title(oc_wasm_str8 title)
{
    oc_str8 nativeTitle = oc_wasm_str8_to_native(title);
    if(nativeTitle.ptr)
    {
        oc_window_set_title(__orcaApp.window, nativeTitle);
    }
}

void oc_bridge_window_set_size(oc_vec2 size)
{
    oc_window_set_content_size(__orcaApp.window, size);
}

oc_wasm_str8 oc_bridge_clipboard_get_string(oc_wasm_addr wasmArena)
{
    return oc_runtime_clipboard_get_string(&__orcaApp.clipboard, wasmArena);
}

void oc_bridge_clipboard_set_string(oc_wasm_str8 value)
{
    oc_runtime_clipboard_set_string(&__orcaApp.clipboard, value);
}

void oc_assert_fail_dialog(const char* file,
                           const char* function,
                           int line,
                           const char* test,
                           const char* fmt,
                           ...)
{
    oc_arena_scope scratch = oc_scratch_begin();

    va_list ap;
    va_start(ap, fmt);
    oc_str8 note = oc_str8_pushfv(scratch.arena, fmt, ap);
    va_end(ap);

    oc_str8 msg = oc_str8_pushf(scratch.arena,
                                "Assertion failed in function %s() in file \"%s\", line %i:\n%s\nNote: %.*s\n",
                                function,
                                file,
                                line,
                                test,
                                oc_str8_ip(note));

    oc_log_error(msg.ptr);

    oc_str8_list options = { 0 };
    oc_str8_list_push(scratch.arena, &options, OC_STR8("OK"));

    oc_alert_popup(OC_STR8("Assertion Failed"), msg, options);
    exit(-1);

    oc_scratch_end(scratch);
}

void oc_abort_ext_dialog(const char* file, const char* function, int line, const char* fmt, ...)
{
    oc_arena_scope scratch = oc_scratch_begin();

    va_list ap;
    va_start(ap, fmt);
    oc_str8 note = oc_str8_pushfv(scratch.arena, fmt, ap);
    va_end(ap);

    oc_str8 msg = oc_str8_pushf(scratch.arena,
                                "Fatal error in function %s() in file \"%s\", line %i:\n%.*s\n",
                                function,
                                file,
                                line,
                                oc_str8_ip(note));

    oc_log_error(msg.ptr);

    oc_str8_list options = { 0 };
    oc_str8_list_push(scratch.arena, &options, OC_STR8("OK"));

    oc_alert_popup(OC_STR8("Fatal Error"), msg, options);
    exit(-1);

    oc_scratch_end(scratch);
}

void oc_wasm3_trap(IM3Runtime runtime, M3Result res, const char* file, const char* function, int line, const char* msg)
{
    M3ErrorInfo errInfo = { 0 };
    m3_GetErrorInfo(runtime, &errInfo);
    if(errInfo.message && res == errInfo.result)
    {
        oc_abort_ext_dialog(file, function, line, "%s: %s (%s)", msg, res, errInfo.message);
    }
    else
    {
        oc_abort_ext_dialog(file, function, line, "%s: %s", msg, res);
    }
}

void oc_bridge_log(oc_log_level level,
                   int functionLen,
                   char* function,
                   int fileLen,
                   char* file,
                   int line,
                   int msgLen,
                   char* msg)
{
    oc_debug_overlay* debug = &__orcaApp.debugOverlay;

    //NOTE: recycle first entry if we exceeded the max entry count
    debug->entryCount++;
    if(debug->entryCount > debug->maxEntries)
    {
        log_entry* e = oc_list_pop_entry(&debug->logEntries, log_entry, listElt);
        if(e)
        {
            oc_list_push(&debug->logFreeList, &e->listElt);
            debug->entryCount--;
        }
    }

    u64 cap = sizeof(log_entry) + fileLen + functionLen + msgLen;

    //NOTE: allocate a new entry
    //TODO: should probably use a buddy allocator over the arena or something
    log_entry* entry = 0;
    oc_list_for(debug->logFreeList, elt, log_entry, listElt)
    {
        if(elt->cap >= cap)
        {
            oc_list_remove(&debug->logFreeList, &elt->listElt);
            entry = elt;
            break;
        }
    }

    if(!entry)
    {
        char* mem = oc_arena_push(&debug->logArena, cap);
        entry = (log_entry*)mem;
        entry->cap = cap;
    }
    char* payload = (char*)entry + sizeof(log_entry);

    entry->function.len = functionLen;
    entry->function.ptr = payload;
    payload += entry->function.len;

    entry->file.len = fileLen;
    entry->file.ptr = payload;
    payload += entry->file.len;

    entry->msg.len = msgLen;
    entry->msg.ptr = payload;
    payload += entry->msg.len;

    memcpy(entry->file.ptr, file, fileLen);
    memcpy(entry->function.ptr, function, functionLen);
    memcpy(entry->msg.ptr, msg, msgLen);

    entry->level = level;
    entry->line = line;
    entry->recordIndex = debug->logEntryTotalCount;
    debug->logEntryTotalCount++;

    oc_list_push_back(&debug->logEntries, &entry->listElt);

    oc_log_ext(level,
               function,
               file,
               line,
               "%.*s\n",
               msgLen,
               msg);
}

void oc_bridge_request_quit(void)
{
    __orcaApp.quit = true;
}

typedef struct orca_surface_create_data
{
    oc_window window;
    oc_surface_api api;
    oc_surface surface;

} orca_surface_create_data;

i32 orca_surface_callback(void* user)
{
    orca_surface_create_data* data = (orca_surface_create_data*)user;
    data->surface = oc_surface_create_for_window(data->window, data->api);

#if OC_PLATFORM_WINDOWS
    //NOTE(martin): on windows we set all surfaces to non-synced, and do a single "manual" wait here.
    //              on macOS each surface is individually synced to the monitor refresh rate but don't block each other
    oc_surface_swap_interval(data->surface, 0);
#endif

    //NOTE: this will be called on main thread, so we need to deselect the surface here,
    //      and reselect it on the orca thread
    oc_surface_deselect();

    return (0);
}

oc_surface orca_surface_canvas(void)
{
    orca_surface_create_data data = {
        .surface = oc_surface_nil(),
        .window = __orcaApp.window,
        .api = OC_CANVAS
    };

    oc_dispatch_on_main_thread_sync(__orcaApp.window, orca_surface_callback, (void*)&data);
    oc_surface_select(data.surface);
    return (data.surface);
}

oc_surface orca_surface_gles(void)
{
    orca_surface_create_data data = {
        .surface = oc_surface_nil(),
        .window = __orcaApp.window,
        .api = OC_GLES
    };

    oc_dispatch_on_main_thread_sync(__orcaApp.window, orca_surface_callback, (void*)&data);
    oc_surface_select(data.surface);
    return (data.surface);
}

void orca_surface_render_commands(oc_surface surface,
                                  oc_color clearColor,
                                  u32 primitiveCount,
                                  oc_primitive* primitives,
                                  u32 eltCount,
                                  oc_path_elt* elements)
{
    oc_runtime* app = &__orcaApp;

    char* memBase = app->env.wasmMemory.ptr;
    u32 memSize = app->env.wasmMemory.committed;
    oc_rect window_content_rect = oc_window_get_content_rect(app->window);

    if(((char*)primitives > memBase)
       && ((char*)primitives + primitiveCount * sizeof(oc_primitive) - memBase <= memSize)
       && ((char*)elements > memBase)
       && ((char*)elements + eltCount * sizeof(oc_path_elt) - memBase <= memSize)
       && window_content_rect.w > 0
       && window_content_rect.h > 0
       && oc_window_is_minimized(app->window) == false)
    {
        oc_surface_render_commands(surface,
                                   clearColor,
                                   primitiveCount,
                                   primitives,
                                   eltCount,
                                   elements);
    }
}

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

    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                     .size.height = { OC_UI_SIZE_CHILDREN },
                                     .layout.axis = OC_UI_AXIS_Y,
                                     .layout.margin.x = 10,
                                     .layout.margin.y = 5,
                                     .bgColor = bgColors[entry->level][entry->recordIndex & 1] },
                     OC_UI_STYLE_SIZE
                         | OC_UI_STYLE_LAYOUT_AXIS
                         | OC_UI_STYLE_LAYOUT_MARGINS
                         | OC_UI_STYLE_BG_COLOR);

    oc_str8 key = oc_str8_pushf(scratch.arena, "%ull", entry->recordIndex);

    oc_ui_container_str8(key, OC_UI_FLAG_DRAW_BACKGROUND)
    {
        oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                         .size.height = { OC_UI_SIZE_CHILDREN },
                                         .layout.axis = OC_UI_AXIS_X },
                         OC_UI_STYLE_SIZE
                             | OC_UI_STYLE_LAYOUT_AXIS);

        oc_ui_container("header", 0)
        {
            oc_ui_style_next(&(oc_ui_style){ .color = levelColors[entry->level],
                                             .font = overlay->fontBold },
                             OC_UI_STYLE_COLOR
                                 | OC_UI_STYLE_FONT);
            oc_ui_label(levelNames[entry->level]);

            oc_str8 loc = oc_str8_pushf(scratch.arena,
                                        "%.*s() in %.*s:%i:",
                                        oc_str8_ip(entry->function),
                                        oc_str8_ip(entry->file),
                                        entry->line);
            oc_ui_label_str8(loc);
        }
        oc_ui_label_str8(entry->msg);
    }
    oc_scratch_end(scratch);
}

char m3_type_to_tag(M3ValueType type)
{
    switch(type)
    {
        case c_m3Type_none:
            return ('v');
        case c_m3Type_i32:
            return ('i');
        case c_m3Type_i64:
            return ('I');
        case c_m3Type_f32:
            return ('f');
        case c_m3Type_f64:
            return ('d');

        case c_m3Type_unknown:
        default:
            return ('!');
    }
}

void oc_wasm_env_init(oc_wasm_env* runtime)
{
    memset(runtime, 0, sizeof(oc_wasm_env));
    oc_base_allocator* allocator = oc_base_allocator_default();
    runtime->wasmMemory.committed = 0;
    runtime->wasmMemory.reserved = 4ULL << 30;
    runtime->wasmMemory.ptr = oc_base_reserve(allocator, runtime->wasmMemory.reserved);
}

#include "wasmbind/clock_api_bind_gen.c"
#include "wasmbind/core_api_bind_gen.c"
#include "wasmbind/gles_api_bind_manual.c"
#include "wasmbind/gles_api_bind_gen.c"
#include "wasmbind/io_api_bind_gen.c"
#include "wasmbind/surface_api_bind_manual.c"
#include "wasmbind/surface_api_bind_gen.c"

i32 orca_runloop(void* user)
{
    oc_runtime* app = &__orcaApp;

    oc_wasm_env_init(&app->env);

    //NOTE: loads wasm module
    oc_arena_scope scratch = oc_scratch_begin();

    const char* bundleNameCString = "module";
    oc_str8 modulePath = oc_path_executable_relative(scratch.arena, OC_STR8("../app/wasm/module.wasm"));

    FILE* file = fopen(modulePath.ptr, "rb");
    if(!file)
    {
        OC_ABORT("The application couldn't load: web assembly module not found");
    }

    fseek(file, 0, SEEK_END);
    u64 wasmSize = ftell(file);
    rewind(file);

    app->env.wasmBytecode.len = wasmSize;
    app->env.wasmBytecode.ptr = oc_malloc_array(char, wasmSize);
    fread(app->env.wasmBytecode.ptr, 1, app->env.wasmBytecode.len, file);
    fclose(file);

    u32 stackSize = 65536;
    app->env.m3Env = m3_NewEnvironment();

    app->env.m3Runtime = m3_NewRuntime(app->env.m3Env, stackSize, NULL);
    //NOTE: host memory will be freed when runtime is freed.
    m3_RuntimeSetMemoryCallbacks(app->env.m3Runtime, oc_wasm_memory_resize_callback, oc_wasm_memory_free_callback, &app->env.wasmMemory);

    M3Result res = m3_ParseModule(app->env.m3Env, &app->env.m3Module, (u8*)app->env.wasmBytecode.ptr, app->env.wasmBytecode.len);
    if(res)
    {
        OC_WASM3_TRAP(app->env.m3Runtime, res, "The application couldn't parse its web assembly module");
    }

    res = m3_LoadModule(app->env.m3Runtime, app->env.m3Module);
    if(res)
    {
        OC_WASM3_TRAP(app->env.m3Runtime, res, "The application couldn't load its web assembly module into the runtime");
    }
    m3_SetModuleName(app->env.m3Module, bundleNameCString);

    oc_scratch_end(scratch);

    //NOTE: bind orca APIs
    {
        int err = 0;
        err |= bindgen_link_core_api(app->env.m3Module);
        err |= bindgen_link_surface_api(app->env.m3Module);
        err |= bindgen_link_clock_api(app->env.m3Module);
        err |= bindgen_link_io_api(app->env.m3Module);
        err |= bindgen_link_gles_api(app->env.m3Module);
        err |= manual_link_gles_api(app->env.m3Module);

        if(err)
        {
            OC_ABORT("The application couldn't link one or more functions to its web assembly module (see console log for more information)");
        }
    }
    //NOTE: compile
    res = m3_CompileModule(app->env.m3Module);
    if(res)
    {
        OC_WASM3_TRAP(app->env.m3Runtime, res, "The application couldn't compile its web assembly module");
    }

    //NOTE: Find and type check event handlers.
    for(int i = 0; i < OC_EXPORT_COUNT; i++)
    {
        const oc_export_desc* desc = &OC_EXPORT_DESC[i];
        IM3Function handler = 0;
        m3_FindFunction(&handler, app->env.m3Runtime, desc->name.ptr);

        if(handler)
        {
            bool checked = false;

            //NOTE: check function signature
            int retCount = m3_GetRetCount(handler);
            int argCount = m3_GetArgCount(handler);
            if(retCount == desc->retTags.len && argCount == desc->argTags.len)
            {
                checked = true;
                for(int retIndex = 0; retIndex < retCount; retIndex++)
                {
                    M3ValueType m3Type = m3_GetRetType(handler, retIndex);
                    char tag = m3_type_to_tag(m3Type);

                    if(tag != desc->retTags.ptr[retIndex])
                    {
                        checked = false;
                        break;
                    }
                }
                if(checked)
                {
                    for(int argIndex = 0; argIndex < argCount; argIndex++)
                    {
                        M3ValueType m3Type = m3_GetArgType(handler, argIndex);
                        char tag = m3_type_to_tag(m3Type);

                        if(tag != desc->argTags.ptr[argIndex])
                        {
                            checked = false;
                            break;
                        }
                    }
                }
            }

            if(checked)
            {
                app->env.exports[i] = handler;
            }
            else
            {
                oc_log_error("type mismatch for event handler %.*s\n", (int)desc->name.len, desc->name.ptr);
            }
        }
    }

    //NOTE: get location of the raw event slot
    IM3Global rawEventGlobal = m3_FindGlobal(app->env.m3Module, "oc_rawEvent");
    app->env.rawEventOffset = (u32)rawEventGlobal->intValue;

    //NOTE: preopen the app local root dir
    {
        scratch = oc_scratch_begin();

        oc_str8 localRootPath = oc_path_executable_relative(scratch.arena, OC_STR8("../app/data"));

        oc_io_req req = { .op = OC_IO_OPEN_AT,
                          .open.rights = OC_FILE_ACCESS_READ | OC_FILE_ACCESS_WRITE,
                          .size = localRootPath.len,
                          .buffer = localRootPath.ptr };
        oc_io_cmp cmp = oc_io_wait_single_req_for_table(&req, &app->fileTable);
        app->rootDir = cmp.handle;

        oc_scratch_end(scratch);
    }

    IM3Function* exports = app->env.exports;

    //NOTE: call init handler
    if(exports[OC_EXPORT_ON_INIT])
    {
        M3Result res = m3_Call(exports[OC_EXPORT_ON_INIT], 0, 0);
        if(res)
        {
            OC_WASM3_TRAP(app->env.m3Runtime, res, "Runtime error");
        }
    }

    if(exports[OC_EXPORT_FRAME_RESIZE])
    {
        oc_rect content = oc_window_get_content_rect(app->window);
        u32 width = (u32)content.w;
        u32 height = (u32)content.h;
        const void* args[2] = { &width, &height };
        M3Result res = m3_Call(exports[OC_EXPORT_FRAME_RESIZE], 2, args);
        if(res)
        {
            OC_WASM3_TRAP(app->env.m3Runtime, res, "Runtime error");
        }
    }

    oc_ui_set_context(&app->debugOverlay.ui);

    while(!app->quit)
    {
        scratch = oc_scratch_begin();
        oc_event* event = 0;

        while((event = oc_next_event(scratch.arena)) != 0)
        {
            if(app->debugOverlay.show)
            {
                oc_ui_process_event(event);
            }

            if(exports[OC_EXPORT_RAW_EVENT])
            {
                oc_event* clipboardEvent = oc_runtime_clipboard_process_event_begin(scratch.arena, &__orcaApp.clipboard, event);
                oc_event* events[2];
                u64 eventsCount;
                if(clipboardEvent != 0)
                {
                    events[0] = clipboardEvent;
                    events[1] = event;
                    eventsCount = 2;
                }
                else
                {
                    events[0] = event;
                    eventsCount = 1;
                }

                for(int i = 0; i < eventsCount; i++)
                {
#ifndef M3_BIG_ENDIAN
                    oc_event* eventPtr = (oc_event*)oc_wasm_address_to_ptr(app->env.rawEventOffset, sizeof(oc_event));
                    memcpy(eventPtr, events[i], sizeof(*events[i]));

                    const void* args[1] = { &app->env.rawEventOffset };
                    M3Result res = m3_Call(exports[OC_EXPORT_RAW_EVENT], 1, args);
                    if(res)
                    {
                        OC_WASM3_TRAP(app->env.m3Runtime, res, "Runtime error");
                    }
#else
                    oc_log_error("oc_on_raw_event() is not supported on big endian platforms");
#endif
                }

                oc_runtime_clipboard_process_event_end(&__orcaApp.clipboard);
            }

            switch(event->type)
            {
                case OC_EVENT_WINDOW_CLOSE:
                case OC_EVENT_QUIT:
                {
                    app->quit = true;
                }
                break;

                case OC_EVENT_WINDOW_RESIZE:
                {
                    oc_rect frame = { 0, 0, event->move.frame.w, event->move.frame.h };

                    if(exports[OC_EXPORT_FRAME_RESIZE])
                    {
                        u32 width = (u32)event->move.content.w;
                        u32 height = (u32)event->move.content.h;
                        const void* args[2] = { &width, &height };
                        M3Result res = m3_Call(exports[OC_EXPORT_FRAME_RESIZE], 2, args);
                        if(res)
                        {
                            OC_WASM3_TRAP(app->env.m3Runtime, res, "Runtime error");
                        }
                    }
                }
                break;

                case OC_EVENT_MOUSE_BUTTON:
                {
                    if(event->key.action == OC_KEY_PRESS)
                    {
                        if(exports[OC_EXPORT_MOUSE_DOWN])
                        {
                            oc_mouse_button button = event->key.button;
                            const void* args[1] = { &button };
                            M3Result res = m3_Call(exports[OC_EXPORT_MOUSE_DOWN], 1, args);
                            if(res)
                            {
                                OC_WASM3_TRAP(app->env.m3Runtime, res, "Runtime error");
                            }
                        }
                    }
                    else
                    {
                        if(exports[OC_EXPORT_MOUSE_UP])
                        {
                            oc_mouse_button button = event->key.button;
                            const void* args[1] = { &button };
                            M3Result res = m3_Call(exports[OC_EXPORT_MOUSE_UP], 1, args);
                            if(res)
                            {
                                OC_WASM3_TRAP(app->env.m3Runtime, res, "Runtime error");
                            }
                        }
                    }
                }
                break;

                case OC_EVENT_MOUSE_WHEEL:
                {
                    if(exports[OC_EXPORT_MOUSE_WHEEL])
                    {
                        const void* args[2] = { &event->mouse.deltaX, &event->mouse.deltaY };
                        M3Result res = m3_Call(exports[OC_EXPORT_MOUSE_WHEEL], 2, args);
                        if(res)
                        {
                            OC_WASM3_TRAP(app->env.m3Runtime, res, "Runtime error");
                        }
                    }
                }
                break;

                case OC_EVENT_MOUSE_MOVE:
                {
                    if(exports[OC_EXPORT_MOUSE_MOVE])
                    {
                        const void* args[4] = { &event->mouse.x, &event->mouse.y, &event->mouse.deltaX, &event->mouse.deltaY };
                        M3Result res = m3_Call(exports[OC_EXPORT_MOUSE_MOVE], 4, args);
                        if(res)
                        {
                            OC_WASM3_TRAP(app->env.m3Runtime, res, "Runtime error");
                        }
                    }
                }
                break;

                case OC_EVENT_KEYBOARD_KEY:
                {
                    if(event->key.action == OC_KEY_PRESS)
                    {
                        if(event->key.keyCode == OC_KEY_D
                           && (event->key.mods & OC_KEYMOD_SHIFT)
                           && (event->key.mods & OC_KEYMOD_MAIN_MODIFIER))
                        {
                            debug_overlay_toggle(&app->debugOverlay);
                        }

                        if(exports[OC_EXPORT_KEY_DOWN])
                        {
                            const void* args[2] = { &event->key.scanCode, &event->key.keyCode };
                            M3Result res = m3_Call(exports[OC_EXPORT_KEY_DOWN], 2, args);
                            if(res)
                            {
                                OC_WASM3_TRAP(app->env.m3Runtime, res, "Runtime error");
                            }
                        }
                    }
                    else if(event->key.action == OC_KEY_RELEASE)
                    {
                        if(exports[OC_EXPORT_KEY_UP])
                        {
                            const void* args[2] = { &event->key.scanCode, &event->key.keyCode };
                            M3Result res = m3_Call(exports[OC_EXPORT_KEY_UP], 2, args);
                            if(res)
                            {
                                OC_WASM3_TRAP(app->env.m3Runtime, res, "Runtime error");
                            }
                        }
                    }
                }
                break;

                default:
                    break;
            }
        }

        oc_surface_deselect();

        if(exports[OC_EXPORT_FRAME_REFRESH])
        {
            M3Result res = m3_Call(exports[OC_EXPORT_FRAME_REFRESH], 0, 0);
            if(res)
            {
                OC_WASM3_TRAP(app->env.m3Runtime, res, "Runtime error");
            }
        }

        oc_surface_select(app->debugOverlay.surface);
        oc_canvas_select(app->debugOverlay.canvas);

        if(app->debugOverlay.show)
        {
            oc_surface_bring_to_front(app->debugOverlay.surface);

            oc_ui_style debugUIDefaultStyle = { .bgColor = { 0 },
                                                .color = { 1, 1, 1, 1 },
                                                .font = app->debugOverlay.fontReg,
                                                .fontSize = 16,
                                                .borderColor = { 1, 0, 0, 1 },
                                                .borderSize = 2 };

            oc_ui_style_mask debugUIDefaultMask = OC_UI_STYLE_BG_COLOR
                                                | OC_UI_STYLE_COLOR
                                                | OC_UI_STYLE_BORDER_COLOR
                                                | OC_UI_STYLE_BORDER_SIZE
                                                | OC_UI_STYLE_FONT
                                                | OC_UI_STYLE_FONT_SIZE;

            oc_vec2 frameSize = oc_surface_get_size(app->debugOverlay.surface);

            oc_ui_frame(frameSize, &debugUIDefaultStyle, debugUIDefaultMask)
            {
                oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                                 .size.height = { OC_UI_SIZE_PARENT, 1, 1 } },
                                 OC_UI_STYLE_SIZE);

                oc_ui_container("overlay area", 0)
                {
                    //...
                }

                oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                                 .size.height = { OC_UI_SIZE_PARENT, 0.4 },
                                                 .layout.axis = OC_UI_AXIS_Y,
                                                 .bgColor = { 0, 0, 0, 0.5 } },
                                 OC_UI_STYLE_SIZE
                                     | OC_UI_STYLE_LAYOUT_AXIS
                                     | OC_UI_STYLE_BG_COLOR);

                oc_ui_container("log console", OC_UI_FLAG_DRAW_BACKGROUND)
                {
                    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                                     .size.height = { OC_UI_SIZE_CHILDREN },
                                                     .layout.axis = OC_UI_AXIS_X,
                                                     .layout.spacing = 10,
                                                     .layout.margin.x = 10,
                                                     .layout.margin.y = 10 },
                                     OC_UI_STYLE_SIZE
                                         | OC_UI_STYLE_LAYOUT);

                    oc_ui_container("log toolbar", 0)
                    {
                        oc_ui_style buttonStyle = { .layout.margin.x = 4,
                                                    .layout.margin.y = 4,
                                                    .roundness = 2,
                                                    .bgColor = { 0, 0, 0, 0.5 },
                                                    .color = { 1, 1, 1, 1 } };

                        oc_ui_style_mask buttonStyleMask = OC_UI_STYLE_LAYOUT_MARGINS
                                                         | OC_UI_STYLE_ROUNDNESS
                                                         | OC_UI_STYLE_BG_COLOR
                                                         | OC_UI_STYLE_COLOR;

                        oc_ui_style_match_after(oc_ui_pattern_all(), &buttonStyle, buttonStyleMask);
                        if(oc_ui_button("Clear").clicked)
                        {
                            oc_list_for_safe(app->debugOverlay.logEntries, entry, log_entry, listElt)
                            {
                                oc_list_remove(&app->debugOverlay.logEntries, &entry->listElt);
                                oc_list_push(&app->debugOverlay.logFreeList, &entry->listElt);
                                app->debugOverlay.entryCount--;
                            }
                        }
                    }

                    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                                     .size.height = { OC_UI_SIZE_PARENT, 1, 1 } },
                                     OC_UI_STYLE_SIZE);

                    //TODO: this is annoying to have to do that. Basically there's another 'contents' box inside oc_ui_panel,
                    //      and we need to change that to size according to its parent (whereas the default is sizing according
                    //      to its children)
                    oc_ui_pattern pattern = { 0 };
                    oc_ui_pattern_push(scratch.arena, &pattern, (oc_ui_selector){ .kind = OC_UI_SEL_OWNER });
                    oc_ui_pattern_push(scratch.arena, &pattern, (oc_ui_selector){ .kind = OC_UI_SEL_TEXT, .text = OC_STR8("contents") });
                    oc_ui_style_match_after(pattern, &(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 } }, OC_UI_STYLE_SIZE_WIDTH);

                    oc_ui_box* panel = oc_ui_box_lookup("log view");
                    f32 scrollY = 0;
                    if(panel)
                    {
                        scrollY = panel->scroll.y;
                    }

                    oc_ui_panel("log view", OC_UI_FLAG_SCROLL_WHEEL_Y)
                    {
                        panel = oc_ui_box_top()->parent;

                        oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                                         .size.height = { OC_UI_SIZE_CHILDREN },
                                                         .layout.axis = OC_UI_AXIS_Y,
                                                         .layout.margin.y = 5 },
                                         OC_UI_STYLE_SIZE
                                             | OC_UI_STYLE_LAYOUT_AXIS);

                        oc_ui_container("contents", 0)
                        {
                            oc_list_for(app->debugOverlay.logEntries, entry, log_entry, listElt)
                            {
                                log_entry_ui(&app->debugOverlay, entry);
                            }
                        }
                    }
                    if(app->debugOverlay.logScrollToLast)
                    {
                        if(panel->scroll.y >= scrollY)
                        {
                            panel->scroll.y = oc_clamp_low(panel->childrenSum[1] - panel->rect.h, 0);
                        }
                        else
                        {
                            app->debugOverlay.logScrollToLast = false;
                        }
                    }
                    else if(panel->scroll.y >= (panel->childrenSum[1] - panel->rect.h) - 1)
                    {
                        app->debugOverlay.logScrollToLast = true;
                    }
                }
            }

            oc_ui_draw();
        }
        else
        {
            oc_set_color_rgba(0, 0, 0, 0);
            oc_clear();
        }

        oc_render(app->debugOverlay.canvas);
        oc_surface_present(app->debugOverlay.surface);

        oc_scratch_end(scratch);

#if OC_PLATFORM_WINDOWS
        //NOTE(martin): on windows we set all surfaces to non-synced, and do a single "manual" wait here.
        //              on macOS each surface is individually synced to the monitor refresh rate but don't block each other
        oc_vsync_wait(app->window);
#endif
    }

    if(exports[OC_EXPORT_TERMINATE])
    {
        M3Result res = m3_Call(exports[OC_EXPORT_TERMINATE], 0, 0);
        if(res)
        {
            OC_WASM3_TRAP(app->env.m3Runtime, res, "Runtime error");
        }
    }

    oc_request_quit();

    return (0);
}

int main(int argc, char** argv)
{
    oc_log_set_level(OC_LOG_LEVEL_INFO);

    oc_init();
    oc_clock_init();

    oc_runtime* app = &__orcaApp;

    //NOTE: create window and surfaces
    oc_rect windowRect = { .x = 100, .y = 100, .w = 810, .h = 610 };
    app->window = oc_window_create(windowRect, OC_STR8("orca"), 0);

    app->debugOverlay.show = false;
    app->debugOverlay.surface = oc_surface_create_for_window(app->window, OC_CANVAS);
    app->debugOverlay.canvas = oc_canvas_create();
    app->debugOverlay.fontReg = orca_font_create("../resources/Menlo.ttf");
    app->debugOverlay.fontBold = orca_font_create("../resources/Menlo Bold.ttf");
    app->debugOverlay.maxEntries = 200;
    oc_arena_init(&app->debugOverlay.logArena);

#if OC_PLATFORM_WINDOWS
    //NOTE(martin): on windows we set all surfaces to non-synced, and do a single "manual" wait here.
    //              on macOS each surface is individually synced to the monitor refresh rate but don't block each other
    oc_surface_swap_interval(app->debugOverlay.surface, 0);
#else
    oc_surface_swap_interval(app->debugOverlay.surface, 1);
#endif

    oc_surface_deselect();

    oc_ui_init(&app->debugOverlay.ui);

    //NOTE: show window and start runloop
    oc_window_bring_to_front(app->window);
    oc_window_focus(app->window);
    oc_window_center(app->window);

    oc_thread* runloopThread = oc_thread_create(orca_runloop, 0);

    while(!oc_should_quit())
    {
        oc_pump_events(-1);
        //TODO: what to do with mem scratch here?
    }

    oc_thread_join(runloopThread, NULL);

    oc_canvas_destroy(app->debugOverlay.canvas);
    oc_surface_destroy(app->debugOverlay.surface);
    oc_window_destroy(app->window);

    oc_terminate();
    return (0);
}
