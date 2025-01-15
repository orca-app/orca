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

#define OC_GRAPHICS_INCLUDE_GL_API 1
#include "graphics/graphics_common.h"
#include "orca.h"

//TODO: wgpu-renderer: figure out graphics backends include selection
#include "graphics/gles_surface.h"

#include "runtime.h"
#include "runtime_clipboard.c"
#include "runtime_io.c"
#include "runtime_memory.c"

#include "wasm/wasm.c"
#if OC_WASM_BACKEND_WASM3
    #include "wasm/backend_wasm3.c"
#elif OC_WASM_BACKEND_BYTEBOX
    #include "wasm/backend_bytebox.c"
#elif OC_WASM_BACKEND_WARM
    #include "warm/warm_adapter.c"
#else
    #error "Unknown wasm backend"
#endif

static const char* s_test_wasm_module_path = NULL;

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
    return wa_instance_get_memory_str8(__orcaApp.env.instance);
}

u64 orca_check_cstring(wa_instance* instance, const char* ptr)
{
    oc_str8 memory = wa_instance_get_memory_str8(instance);

    //NOTE: Here we are guaranteed that ptr is in [ memory ; memory + memorySize [
    //      hence (memory + memorySize) - ptr is representable by size_t and <= memorySize
    size_t maxLen = (memory.ptr + memory.len) - ptr;

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
        log_entry* e = oc_list_pop_front_entry(&debug->logEntries, log_entry, listElt);
        if(e)
        {
            oc_list_push_front(&debug->logFreeList, &e->listElt);
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

void oc_bridge_exit(int ec)
{
    //TODO: send a trap exit to oc_wasm to stop interpreter,
    // then when we return from the trap, quit app
    // temporarily, we just exit here
    exit(ec);
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

    switch(data->api)
    {
        case OC_SURFACE_CANVAS:
            data->surface = oc_canvas_surface_create_for_window(__orcaApp.canvasRenderer, data->window);
            break;
        case OC_SURFACE_GLES:
        default:
            data->surface = oc_gles_surface_create_for_window(data->window);
            break;
    }

    if(data->api == OC_SURFACE_GLES)
    {
#if OC_PLATFORM_WINDOWS
        //NOTE(martin): on windows we set all surfaces to non-synced, and do a single "manual" wait here.
        //              on macOS each surface is individually synced to the monitor refresh rate but don't block each other
        oc_gles_surface_swap_interval(data->surface, 0);
#endif

        //NOTE: this will be called on main thread, so we need to deselect the surface here,
        //      and reselect it on the orca thread
        oc_gles_surface_make_current(oc_surface_nil());
    }
    //TODO: wgpu-renderer: set canvas surface swap?

    return (0);
}

oc_surface oc_bridge_gles_surface_create(void)
{
    orca_surface_create_data data = {
        .surface = oc_surface_nil(),
        .window = __orcaApp.window,
        .api = OC_SURFACE_GLES
    };

    oc_dispatch_on_main_thread_sync(__orcaApp.window, orca_surface_callback, (void*)&data);
    oc_gles_surface_make_current(data.surface);
    return (data.surface);
}

oc_canvas_renderer oc_bridge_canvas_renderer_create(void)
{
    return (__orcaApp.canvasRenderer);
}

oc_surface oc_bridge_canvas_surface_create(oc_canvas_renderer renderer)
{
    orca_surface_create_data data = {
        .surface = oc_surface_nil(),
        .window = __orcaApp.window,
        .api = OC_SURFACE_CANVAS
    };
    //TODO: check renderer

    oc_dispatch_on_main_thread_sync(__orcaApp.window, orca_surface_callback, (void*)&data);
    return (data.surface);
}

void oc_bridge_canvas_renderer_submit(oc_canvas_renderer renderer,
                                      oc_surface surface,
                                      u32 msaaSampleCount,
                                      bool clear,
                                      oc_color clearColor,
                                      u32 primitiveCount,
                                      oc_primitive* primitives,
                                      u32 eltCount,
                                      oc_path_elt* elements)
{
    oc_runtime* app = &__orcaApp;

    oc_str8 mem = wa_instance_get_memory_str8(app->env.instance);
    char* memBase = mem.ptr;
    u32 memSize = mem.len;

    oc_rect window_content_rect = oc_window_get_content_rect(app->window);

    if(((char*)primitives > memBase)
       && ((char*)primitives + primitiveCount * sizeof(oc_primitive) - memBase <= memSize)
       && ((char*)elements > memBase)
       && ((char*)elements + eltCount * sizeof(oc_path_elt) - memBase <= memSize)
       && window_content_rect.w > 0
       && window_content_rect.h > 0
       && oc_window_is_minimized(app->window) == false)
    {
        oc_canvas_renderer_submit(renderer,
                                  surface,
                                  msaaSampleCount,
                                  clear,
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

char valtype_to_tag(wa_value_type type)
{
    switch(type)
    {
        case WA_TYPE_I32:
            return ('i');
        case WA_TYPE_I64:
            return ('I');
        case WA_TYPE_F32:
            return ('f');
        case WA_TYPE_F64:
            return ('F');
        default:
            return ('!');
    }
}

void oc_wasm_env_init(oc_wasm_env* runtime)
{
    memset(runtime, 0, sizeof(oc_wasm_env));
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
    {
        oc_arena_scope scratch = oc_scratch_begin();

        oc_str8 modulePath = oc_path_executable_relative(scratch.arena, OC_STR8("../app/wasm/module.wasm"));
        if(s_test_wasm_module_path)
        {
            modulePath = oc_str8_push_copy(scratch.arena, OC_STR8(s_test_wasm_module_path));
        }

        FILE* file = fopen(modulePath.ptr, "rb");
        if(!file)
        {
            OC_ABORT("The application couldn't load: web assembly module '%s' not found", modulePath.ptr);
        }
        oc_scratch_end(scratch);

        fseek(file, 0, SEEK_END);
        u64 wasmSize = ftell(file);
        rewind(file);

        app->env.wasmBytecode.len = wasmSize;
        app->env.wasmBytecode.ptr = oc_malloc_array(char, wasmSize);
        fread(app->env.wasmBytecode.ptr, 1, app->env.wasmBytecode.len, file);
        fclose(file);
    }

    oc_arena_init(&app->env.arena);
    app->env.module = wa_module_create(&app->env.arena, app->env.wasmBytecode);

    ///////////////////////////////////////////////////////////////////////
    //TODO: check module's status
    // OC_WASM_TRAP(oc_wasm_decode(app->env.wasm, app->env.wasmBytecode));
    ///////////////////////////////////////////////////////////////////////

    //NOTE: bind orca APIs
    wa_import_package package = {
        .name = OC_STR8("env"),
    };

    {
        oc_arena_scope scratch = oc_scratch_begin();

        int err = 0;
        err |= bindgen_link_core_api(scratch.arena, &package);
        err |= bindgen_link_surface_api(scratch.arena, &package);
        err |= bindgen_link_clock_api(scratch.arena, &package);
        err |= bindgen_link_io_api(scratch.arena, &package);
        err |= bindgen_link_gles_api(scratch.arena, &package);
        err |= manual_link_gles_api(scratch.arena, &package);

        oc_scratch_end(scratch);

        if(err)
        {
            OC_ABORT("The application couldn't link one or more functions to its web assembly module (see console log for more information)");
        }
    }

    {
        wa_instance_options options = {
            .packageCount = 1,
            .importPackages = &package,
        };
        app->env.instance = wa_instance_create(&app->env.arena, app->env.module, &options);
        ///////////////////////////////////////////////////////////////////////
        //TODO: check module's status
        //    OC_WASM_TRAP(oc_wasm_instantiate(app->env.wasm, OC_STR8("module"), &package));
        ///////////////////////////////////////////////////////////////////////
    }

    //NOTE: Find and type check event handlers.
    {
        oc_arena_scope scratch = oc_scratch_begin();

        for(int i = 0; i < OC_EXPORT_COUNT; i++)
        {
            const oc_export_desc* desc = &OC_EXPORT_DESC[i];

            wa_func* handle = wa_instance_find_function(app->env.instance, desc->name);
            if(handle)
            {
                wa_func_type info = wa_function_get_type(scratch.arena, app->env.instance, handle);

                bool checked = false;

                //NOTE: check function signature
                if(info.returnCount == desc->retTags.len && info.paramCount == desc->argTags.len)
                {
                    checked = true;

                    for(int retIndex = 0; retIndex < info.returnCount && checked; retIndex++)
                    {
                        char tag = valtype_to_tag(info.returns[retIndex]);
                        if(tag != desc->retTags.ptr[retIndex])
                        {
                            checked = false;
                        }
                    }

                    for(int argIndex = 0; argIndex < info.paramCount && checked; argIndex++)
                    {
                        char tag = valtype_to_tag(info.params[argIndex]);
                        if(tag != desc->argTags.ptr[argIndex])
                        {
                            checked = false;
                        }
                    }
                }

                if(checked)
                {
                    app->env.exports[i] = handle;
                }
                else
                {
                    oc_log_error("type mismatch for event handler %.*s\n", (int)desc->name.len, desc->name.ptr);
                }
            }
        }

        oc_scratch_end(scratch);
    }

    //NOTE: get location of the raw event slot
    {
        wa_global* global = wa_instance_find_global(app->env.instance, OC_STR8("oc_rawEvent"));
        if(!global)
        {
            oc_abort_ext_dialog(__FILE__, __FUNCTION__, __LINE__, "Failed to find raw event global - was this module linked with the Orca wasm runtime?");
        }
        app->env.rawEventOffset = wa_global_get(app->env.instance, global).valI32;
    }

    //NOTE: preopen the app local root dir
    {
        oc_arena_scope scratch = oc_scratch_begin();

        oc_str8 localRootPath = oc_path_executable_relative(scratch.arena, OC_STR8("../app/data"));

        oc_io_req req = { .op = OC_IO_OPEN_AT,
                          .open.rights = OC_FILE_ACCESS_READ | OC_FILE_ACCESS_WRITE,
                          .size = localRootPath.len,
                          .buffer = localRootPath.ptr };
        oc_io_cmp cmp = oc_io_wait_single_req_for_table(&req, &app->fileTable);
        app->rootDir = cmp.handle;

        oc_scratch_end(scratch);
    }

    wa_func** exports = app->env.exports;

    if(s_test_wasm_module_path)
    {
        wa_value returnCode = { 0 };
        if(exports[OC_EXPORT_ON_TEST])
        {
            wa_status status = wa_instance_invoke(app->env.instance, exports[OC_EXPORT_ON_TEST], 0, NULL, 1, &returnCode);
            OC_WASM_TRAP(status);

            if(returnCode.valI32 != 0)
            {
                oc_log_error("Tests failed. Exit code: %d\n", returnCode.valI32);
            }
        }
        else
        {
            returnCode.valI32 = 1;
            oc_log_error("Failed to find oc_on_test() hook - unable to run tests.\n");
        }
        oc_request_quit();
        return returnCode.valI32;
    }

    //NOTE: call init handler
    if(exports[OC_EXPORT_ON_INIT])
    {
        wa_status status = wa_instance_invoke(app->env.instance, exports[OC_EXPORT_ON_INIT], 0, NULL, 0, NULL);
        OC_WASM_TRAP(status);
    }

    if(exports[OC_EXPORT_FRAME_RESIZE])
    {
        oc_rect content = oc_window_get_content_rect(app->window);

        wa_value params[2];
        params[0].valI32 = (i32)content.w;
        params[1].valI32 = (i32)content.h;

        wa_status status = wa_instance_invoke(app->env.instance, exports[OC_EXPORT_FRAME_RESIZE], oc_array_size(params), params, 0, NULL);
        OC_WASM_TRAP(status);
    }

    oc_ui_set_context(&app->debugOverlay.ui);

    while(!app->quit)
    {
        oc_arena_scope scratch = oc_scratch_begin();
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
                    if(oc_is_little_endian())
                    {
                        oc_event* eventPtr = (oc_event*)oc_wasm_address_to_ptr(app->env.rawEventOffset, sizeof(oc_event));
                        memcpy(eventPtr, events[i], sizeof(*events[i]));

                        wa_value eventOffset = { .valI32 = (i32)app->env.rawEventOffset };
                        wa_status status = wa_instance_invoke(app->env.instance, exports[OC_EXPORT_RAW_EVENT], 1, &eventOffset, 0, NULL);
                        OC_WASM_TRAP(status);
                    }
                    else
                    {
                        oc_log_error("oc_on_raw_event() is not supported on big endian platforms");
                    }
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
                        wa_value params[2];
                        params[0].valI32 = (i32)event->move.content.w;
                        params[1].valI32 = (i32)event->move.content.h;

                        wa_status status = wa_instance_invoke(app->env.instance, exports[OC_EXPORT_FRAME_RESIZE], oc_array_size(params), params, 0, NULL);
                        OC_WASM_TRAP(status);
                    }
                }
                break;

                case OC_EVENT_MOUSE_BUTTON:
                {
                    if(event->key.action == OC_KEY_PRESS)
                    {
                        if(exports[OC_EXPORT_MOUSE_DOWN])
                        {
                            wa_value button = { .valI32 = event->key.button };

                            wa_status status = wa_instance_invoke(app->env.instance, exports[OC_EXPORT_MOUSE_DOWN], 1, &button, 0, NULL);
                            OC_WASM_TRAP(status);
                        }
                    }
                    else
                    {
                        if(exports[OC_EXPORT_MOUSE_UP])
                        {
                            wa_value button = { .valI32 = event->key.button };

                            wa_status status = wa_instance_invoke(app->env.instance, exports[OC_EXPORT_MOUSE_UP], 1, &button, 0, NULL);
                            OC_WASM_TRAP(status);
                        }
                    }
                }
                break;

                case OC_EVENT_MOUSE_WHEEL:
                {
                    if(exports[OC_EXPORT_MOUSE_WHEEL])
                    {
                        wa_value params[2];
                        params[0].valF32 = event->mouse.deltaX;
                        params[1].valF32 = event->mouse.deltaY;

                        wa_status status = wa_instance_invoke(app->env.instance, exports[OC_EXPORT_MOUSE_WHEEL], oc_array_size(params), params, 0, NULL);
                        OC_WASM_TRAP(status);
                    }
                }
                break;

                case OC_EVENT_MOUSE_MOVE:
                {
                    if(exports[OC_EXPORT_MOUSE_MOVE])
                    {
                        wa_value params[4];
                        params[0].valF32 = event->mouse.x;
                        params[1].valF32 = event->mouse.y;
                        params[2].valF32 = event->mouse.deltaX;
                        params[3].valF32 = event->mouse.deltaY;

                        wa_status status = wa_instance_invoke(app->env.instance, exports[OC_EXPORT_MOUSE_MOVE], oc_array_size(params), params, 0, NULL);
                        OC_WASM_TRAP(status);
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
                            wa_value params[2];
                            params[0].valI32 = event->key.scanCode;
                            params[1].valI32 = event->key.keyCode;

                            wa_status status = wa_instance_invoke(app->env.instance, exports[OC_EXPORT_KEY_DOWN], oc_array_size(params), params, 0, NULL);
                            OC_WASM_TRAP(status);
                        }
                    }
                    else if(event->key.action == OC_KEY_RELEASE)
                    {
                        if(exports[OC_EXPORT_KEY_UP])
                        {
                            wa_value params[2];
                            params[0].valI32 = event->key.scanCode;
                            params[1].valI32 = event->key.keyCode;

                            wa_status status = wa_instance_invoke(app->env.instance, exports[OC_EXPORT_KEY_UP], oc_array_size(params), params, 0, NULL);
                            OC_WASM_TRAP(status);
                        }
                    }
                }
                break;

                default:
                    break;
            }
        }

        if(exports[OC_EXPORT_FRAME_REFRESH])
        {
            wa_status status = wa_instance_invoke(app->env.instance, exports[OC_EXPORT_FRAME_REFRESH], 0, NULL, 0, NULL);
            OC_WASM_TRAP(status);
        }

        oc_canvas_context_select(app->debugOverlay.context);

        if(app->debugOverlay.show)
        {
            //TODO: only move if it's not already on the front?
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
                                oc_list_push_front(&app->debugOverlay.logFreeList, &entry->listElt);
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
            //TODO: only move if it's not already on the back?
            oc_surface_send_to_back(app->debugOverlay.surface);
            oc_set_color_rgba(0, 0, 0, 0);
            oc_clear();
        }

        oc_canvas_render(app->canvasRenderer, app->debugOverlay.context, app->debugOverlay.surface);
        oc_canvas_present(app->canvasRenderer, app->debugOverlay.surface);

        oc_scratch_end(scratch);

#if OC_PLATFORM_WINDOWS
        //NOTE(martin): on windows we set all surfaces to non-synced, and do a single "manual" wait here.
        //              on macOS each surface is individually synced to the monitor refresh rate but don't block each other
        oc_vsync_wait(app->window);
#endif
    }

    if(exports[OC_EXPORT_TERMINATE])
    {
        wa_status status = wa_instance_invoke(app->env.instance, exports[OC_EXPORT_TERMINATE], 0, NULL, 0, NULL);
        OC_WASM_TRAP(status);
    }

    wa_instance_destroy(app->env.instance);
    wa_module_destroy(app->env.module);

    oc_request_quit();

    return (0);
}

int main(int argc, char** argv)
{
    if(argc > 1)
    {
        if(strstr(argv[1], "--test="))
        {
            s_test_wasm_module_path = argv[1] + sizeof("--test=") - 1;
        }
    }

    oc_log_set_level(OC_LOG_LEVEL_INFO);

    oc_init();
    oc_clock_init();

    oc_runtime* app = &__orcaApp;

    app->debugOverlay.maxEntries = 200;
    oc_arena_init(&app->debugOverlay.logArena);

    if(s_test_wasm_module_path == NULL)
    {
        //NOTE: create window and surfaces
        oc_rect windowRect = { .x = 100, .y = 100, .w = 810, .h = 610 };
        app->window = oc_window_create(windowRect, OC_STR8("orca"), 0);

        app->canvasRenderer = oc_canvas_renderer_create();

        app->debugOverlay.show = false;
        app->debugOverlay.surface = oc_canvas_surface_create_for_window(app->canvasRenderer, app->window);
        app->debugOverlay.context = oc_canvas_context_create();
        app->debugOverlay.fontReg = orca_font_create("../resources/Menlo.ttf");
        app->debugOverlay.fontBold = orca_font_create("../resources/Menlo Bold.ttf");

        /*TODO: wgpu-renderer: set swap interval?

#if OC_PLATFORM_WINDOWS
        //NOTE(martin): on windows we set all surfaces to non-synced, and do a single "manual" wait here.
        //              on macOS each surface is individually synced to the monitor refresh rate but don't block each other
        oc_surface_swap_interval(app->debugOverlay.surface, 0);
#else
        oc_surface_swap_interval(app->debugOverlay.surface, 1);
#endif
    */
        oc_ui_init(&app->debugOverlay.ui);

        //NOTE: show window and start runloop
        oc_window_bring_to_front(app->window);
        oc_window_focus(app->window);
        oc_window_center(app->window);
    }

    oc_thread* runloopThread = oc_thread_create(orca_runloop, 0);

    while(!oc_should_quit())
    {
        oc_pump_events(-1);
        //TODO: what to do with mem scratch here?
    }

    i64 exitCode = 0;
    oc_thread_join(runloopThread, &exitCode);

    if(s_test_wasm_module_path == NULL)
    {
        oc_canvas_context_destroy(app->debugOverlay.context);
        oc_surface_destroy(app->debugOverlay.surface);
        oc_canvas_renderer_destroy(app->canvasRenderer);
        oc_window_destroy(app->window);
    }

    oc_terminate();
    return (int)(exitCode);
}
