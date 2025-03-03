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

#include "util/ringbuffer.h"

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

///////////////////////////////////////////////////////////////
//TODO: fold this with app's queuing helpers

void queue_event(oc_ringbuffer* queue, oc_event* event)
{
    if(oc_ringbuffer_write_available(queue) < sizeof(oc_event))
    {
        oc_log_error("event queue full\n");
    }
    else
    {
        bool error = false;
        oc_ringbuffer_reserve(queue, sizeof(oc_event), (u8*)event);

        if(event->type == OC_EVENT_PATHDROP)
        {
            oc_list_for(event->paths.list, elt, oc_str8_elt, listElt)
            {
                oc_str8* path = &elt->string;
                if(oc_ringbuffer_write_available(queue) < (sizeof(u64) + path->len))
                {
                    oc_log_error("event queue full\n");
                    error = true;
                    break;
                }
                else
                {
                    oc_ringbuffer_reserve(queue, sizeof(u64), (u8*)&path->len);
                    oc_ringbuffer_reserve(queue, path->len, (u8*)path->ptr);
                }
            }
        }
        if(error)
        {
            oc_ringbuffer_rewind(queue);
        }
        else
        {
            oc_ringbuffer_commit(queue);
        }
    }
}

oc_event* queue_next_event(oc_arena* arena, oc_ringbuffer* queue)
{
    //NOTE: pop and return event from queue
    oc_event* event = 0;

    if(oc_ringbuffer_read_available(queue) >= sizeof(oc_event))
    {
        event = oc_arena_push_type(arena, oc_event);
        u64 read = oc_ringbuffer_read(queue, sizeof(oc_event), (u8*)event);
        OC_DEBUG_ASSERT(read == sizeof(oc_event));

        if(event->type == OC_EVENT_PATHDROP)
        {
            u64 pathCount = event->paths.eltCount;
            event->paths = (oc_str8_list){ 0 };

            for(int i = 0; i < pathCount; i++)
            {
                if(oc_ringbuffer_read_available(queue) < sizeof(u64))
                {
                    oc_log_error("malformed path payload: no string size\n");
                    break;
                }

                u64 len = 0;
                oc_ringbuffer_read(queue, sizeof(u64), (u8*)&len);
                if(oc_ringbuffer_read_available(queue) < len)
                {
                    oc_log_error("malformed path payload: string shorter than expected\n");
                    break;
                }

                char* buffer = oc_arena_push_array(arena, char, len);
                oc_ringbuffer_read(queue, len, (u8*)buffer);

                oc_str8_list_push(arena, &event->paths, oc_str8_from_buffer(len, buffer));
            }
        }
    }
    return (event);
}

#include "wasmbind/clock_api_bind_gen.c"
#include "wasmbind/core_api_bind_gen.c"
#include "wasmbind/gles_api_bind_manual.c"
#include "wasmbind/gles_api_bind_gen.c"
#include "wasmbind/io_api_bind_gen.c"
#include "wasmbind/surface_api_bind_manual.c"
#include "wasmbind/surface_api_bind_gen.c"

#if OC_WASM_BACKEND_WARM
    #define OC_WASM_DEBUGGER
#endif

#ifdef OC_WASM_DEBUGGER
wa_status orca_invoke(wa_interpreter* interpreter, wa_instance* instance, wa_func* function, u32 argCount, wa_value* args, u32 retCount, wa_value* returns)
{
    oc_wasm_env* env = oc_runtime_get_env();

    wa_status status = wa_interpreter_invoke(interpreter, instance, function, argCount, args, retCount, returns);

    while(status == WA_TRAP_SUSPENDED || status == WA_TRAP_BREAKPOINT || status == WA_TRAP_STEP)
    {
        oc_mutex_lock(env->suspendMutex);
        {
            env->paused = true;
            while(env->paused)
            {
                oc_condition_wait(env->suspendCond, env->suspendMutex);
            }
        }
        oc_mutex_unlock(env->suspendMutex);

        OC_DEBUG_ASSERT(env->paused == false);

        switch(env->debuggerCommand)
        {
            case OC_DEBUGGER_CONTINUE:
            {
                status = wa_interpreter_continue(interpreter);
            }
            break;

            case OC_DEBUGGER_STEP:
            {
                status = wa_interpreter_step(interpreter);
            }
            break;
        }
    }

    return status;
}

#else // OC_WASM_DEBUGGER
    #define orca_invoke wa_interpreter_invoke
#endif // OC_WASM_DEBUGGER

i32 vm_runloop(void* user)
{
    oc_runtime* app = &__orcaApp;

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

    OC_WASM_TRAP(wa_module_status(app->env.module));

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

        OC_WASM_TRAP(wa_instance_status(app->env.instance));
    }

    app->env.interpreter = wa_interpreter_create(&app->env.arena);

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
            wa_status status = orca_invoke(app->env.interpreter, app->env.instance, exports[OC_EXPORT_ON_TEST], 0, NULL, 1, &returnCode);
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
        wa_status status = orca_invoke(app->env.interpreter, app->env.instance, exports[OC_EXPORT_ON_INIT], 0, NULL, 0, NULL);
        OC_WASM_TRAP(status);
    }

    if(exports[OC_EXPORT_FRAME_RESIZE])
    {
        oc_rect content = oc_window_get_content_rect(app->window);

        wa_value params[2];
        params[0].valI32 = (i32)content.w;
        params[1].valI32 = (i32)content.h;

        wa_status status = orca_invoke(app->env.interpreter, app->env.instance, exports[OC_EXPORT_FRAME_RESIZE], oc_array_size(params), params, 0, NULL);
        OC_WASM_TRAP(status);
    }

    while(!app->quit)
    {
        oc_arena_scope scratch = oc_scratch_begin();
        oc_event* event = 0;

        while((event = queue_next_event(scratch.arena, &app->eventBuffer)) != 0)
        {
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
                        wa_status status = orca_invoke(app->env.interpreter, app->env.instance, exports[OC_EXPORT_RAW_EVENT], 1, &eventOffset, 0, NULL);
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
                case OC_EVENT_WINDOW_RESIZE:
                {
                    oc_rect frame = { 0, 0, event->move.frame.w, event->move.frame.h };

                    if(exports[OC_EXPORT_FRAME_RESIZE])
                    {
                        wa_value params[2];
                        params[0].valI32 = (i32)event->move.content.w;
                        params[1].valI32 = (i32)event->move.content.h;

                        wa_status status = orca_invoke(app->env.interpreter, app->env.instance, exports[OC_EXPORT_FRAME_RESIZE], oc_array_size(params), params, 0, NULL);
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

                            wa_status status = orca_invoke(app->env.interpreter, app->env.instance, exports[OC_EXPORT_MOUSE_DOWN], 1, &button, 0, NULL);
                            OC_WASM_TRAP(status);
                        }
                    }
                    else
                    {
                        if(exports[OC_EXPORT_MOUSE_UP])
                        {
                            wa_value button = { .valI32 = event->key.button };

                            wa_status status = orca_invoke(app->env.interpreter, app->env.instance, exports[OC_EXPORT_MOUSE_UP], 1, &button, 0, NULL);
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

                        wa_status status = orca_invoke(app->env.interpreter, app->env.instance, exports[OC_EXPORT_MOUSE_WHEEL], oc_array_size(params), params, 0, NULL);
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

                        wa_status status = orca_invoke(app->env.interpreter, app->env.instance, exports[OC_EXPORT_MOUSE_MOVE], oc_array_size(params), params, 0, NULL);
                        OC_WASM_TRAP(status);
                    }
                }
                break;

                case OC_EVENT_KEYBOARD_KEY:
                {
                    if(event->key.action == OC_KEY_PRESS)
                    {
                        if(exports[OC_EXPORT_KEY_DOWN])
                        {
                            wa_value params[2];
                            params[0].valI32 = event->key.scanCode;
                            params[1].valI32 = event->key.keyCode;

                            wa_status status = orca_invoke(app->env.interpreter, app->env.instance, exports[OC_EXPORT_KEY_DOWN], oc_array_size(params), params, 0, NULL);
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

                            wa_status status = orca_invoke(app->env.interpreter, app->env.instance, exports[OC_EXPORT_KEY_UP], oc_array_size(params), params, 0, NULL);
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
            wa_status status = orca_invoke(app->env.interpreter, app->env.instance, exports[OC_EXPORT_FRAME_REFRESH], 0, NULL, 0, NULL);
            OC_WASM_TRAP(status);
        }

        oc_scratch_end(scratch);

#if OC_PLATFORM_WINDOWS
        //NOTE(martin): on windows we set all surfaces to non-synced, and do a single "manual" wait here.
        //              on macOS each surface is individually synced to the monitor refresh rate but don't block each other
        oc_vsync_wait(app->window);
#endif
    }

    if(exports[OC_EXPORT_TERMINATE])
    {
        wa_status status = orca_invoke(app->env.interpreter, app->env.instance, exports[OC_EXPORT_TERMINATE], 0, NULL, 0, NULL);
        OC_WASM_TRAP(status);
    }

    wa_instance_destroy(app->env.instance);
    wa_module_destroy(app->env.module);

    oc_request_quit();

    return (0);
}

void overlay_ui(oc_runtime* app)
{
    //////////////////////////////////////////////////////////////////////////////
    //TODO: we should probably pump new log entries from a ring buffer here
    //////////////////////////////////////////////////////////////////////////////

    oc_arena_scope scratch = oc_scratch_begin();

    oc_ui_set_context(app->debugOverlay.ui);
    oc_canvas_context_select(app->debugOverlay.context);

    if(app->debugOverlay.show)
    {
        //TODO: only move if it's not already on the front?
        oc_surface_bring_to_front(app->debugOverlay.surface);

        oc_vec2 frameSize = oc_surface_get_size(app->debugOverlay.surface);

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
                        oc_list_for_safe(app->debugOverlay.logEntries, entry, log_entry, listElt)
                        {
                            oc_list_remove(&app->debugOverlay.logEntries, &entry->listElt);
                            oc_list_push_front(&app->debugOverlay.logFreeList, &entry->listElt);
                            app->debugOverlay.entryCount--;
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

    oc_canvas_render(app->debugOverlay.renderer, app->debugOverlay.context, app->debugOverlay.surface);
    oc_canvas_present(app->debugOverlay.renderer, app->debugOverlay.surface);

    oc_scratch_end(scratch);
}

#ifdef OC_WASM_DEBUGGER

i32 create_debug_window_callback(void* user)
{
    oc_runtime* app = (oc_runtime*)user;

    oc_rect rect = oc_window_get_frame_rect(app->window);
    rect.x += 100;
    rect.y += 100;
    rect.w = 1000;
    rect.h = 800;

    app->debuggerUI.window = oc_window_create(rect, OC_STR8("Orca Debugger"), 0);
    oc_window_bring_to_front(app->debuggerUI.window);
    oc_window_focus(app->debuggerUI.window);

    return (0);
}

void oc_debugger_ui_open(oc_runtime* app)
{
    oc_debugger_ui* debuggerUI = &app->debuggerUI;
    if(!debuggerUI->init)
    {
        //NOTE: window needs to be created on main thread
        oc_dispatch_on_main_thread_sync(app->window, create_debug_window_callback, app);

        debuggerUI->renderer = oc_canvas_renderer_create();

        {
            //NOTE: surface also needs to be created on main thread
            orca_surface_create_data data = {
                .surface = oc_surface_nil(),
                .window = app->debuggerUI.window,
                .api = OC_SURFACE_CANVAS,
            };

            oc_dispatch_on_main_thread_sync(app->debuggerUI.window, orca_surface_callback, (void*)&data);
            debuggerUI->surface = data.surface;
        }

        debuggerUI->canvas = oc_canvas_context_create();
        debuggerUI->ui = oc_ui_context_create(app->debugOverlay.fontReg);
        debuggerUI->init = true;
    }
}

void oc_debugger_ui_close(oc_runtime* app)
{
    oc_debugger_ui* debuggerUI = &app->debuggerUI;
    if(debuggerUI->init)
    {
        oc_ui_context_destroy(debuggerUI->ui);
        oc_canvas_context_destroy(debuggerUI->canvas);
        oc_surface_destroy(debuggerUI->surface);
        oc_canvas_renderer_destroy(debuggerUI->renderer);
        oc_window_destroy(debuggerUI->window);

        memset(debuggerUI, 0, sizeof(oc_debugger_ui));
    }
}

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

void source_tree_ui(oc_runtime* app, wa_source_node* node, int indent)
{
    //TODO: use full path to disambiguate similarly named root dirs
    oc_ui_box_str8(node->path)
    {
        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
        oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_X);

        oc_ui_box("indent")
        {
            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, 10 * indent });
            oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
        }

        oc_ui_label_str8(OC_STR8("label"), node->name);
    }

    oc_list_for(node->children, child, wa_source_node, listElt)
    {
        source_tree_ui(app, child, indent + 1);
    }
}

void debugger_ui_update(oc_runtime* app)
{
    oc_arena_scope scratch = oc_scratch_begin();

    oc_ui_set_context(app->debuggerUI.ui);
    oc_canvas_context_select(app->debuggerUI.canvas);

    oc_vec2 frameSize = oc_surface_get_size(app->debuggerUI.surface);

    oc_ui_frame(frameSize)
    {
        oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_X);
        oc_ui_style_set_i32(OC_UI_CONSTRAIN_X, 1);

        static i64 selectedFunction = -1;
        f32 scrollSpeed = 0.3;
        bool freshScroll = false;

        //NOTE: if paused == true here, vm thread can not unpause until next frame.
        //      if paused == false, vm thread can become paused during the frame, but we
        //      don't really care (we render cached state for this frame and will update next frame)
        bool paused = atomic_load(&app->env.paused);
        if(paused != app->env.prevPaused)
        {
            if(paused == true)
            {
                //NOTE: vm thread has become paused since last frame. We set autoscroll and autoselect the function
                app->env.autoScroll = true;

                u32 newIndex = app->env.interpreter->controlStack[app->env.interpreter->controlStackTop].func - app->env.instance->functions;
                if(selectedFunction != newIndex)
                {
                    scrollSpeed = 1;
                    freshScroll = true;
                }
                selectedFunction = newIndex;
            }
            app->env.prevPaused = paused;
        }

        static f32 procPanelSize = 300;

        oc_ui_box("browser")
        {
            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, procPanelSize });
            oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
            oc_ui_style_set_i32(OC_UI_CONSTRAIN_Y, 1);
            oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);

            static bool showSymbols = true;

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
                        showSymbols = false;
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
                        showSymbols = true;
                    }
                }

                oc_ui_style_rule(showSymbols ? "option-symbols" : "option-files")
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

                if(showSymbols)
                {
                    for(u32 funcIndex = 0; funcIndex < app->env.module->functionCount; funcIndex++)
                    {
                        wa_func* func = &app->env.instance->functions[funcIndex];
                        if(func->codeLen)
                        {
                            oc_str8 name = wa_module_get_function_name(app->env.module, funcIndex);

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
                                    app->env.autoScroll = false;
                                    freshScroll = true;
                                }
                            }
                        }
                    }
                }
                else
                {
                    wa_source_node* sourceTree = wa_module_get_source_tree(app->env.module);
                    oc_list_for(sourceTree->children, child, wa_source_node, listElt)
                    {
                        source_tree_ui(app, child, 0);
                    }
                }
            }
        }

        oc_ui_box* codePanel = oc_ui_box("code-panel")
        {
            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1, 1 });
            oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
            oc_ui_style_set_i32(OC_UI_OVERFLOW_Y, OC_UI_OVERFLOW_SCROLL);

            const i32 BOX_MARGIN_H = 2;
            const i32 BOX_MARGIN_W = 2;

            if(freshScroll)
            {
                oc_log_info("[%f] freshScroll, set scroll to 0\n", oc_ui_frame_time());
                codePanel->scroll.y = 0;
            }

            if(selectedFunction >= 0)
            {
                wa_func* func = &app->env.instance->functions[selectedFunction];
                oc_str8 funcName = wa_module_get_function_name(app->env.module, selectedFunction);
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
                            app->env.interpreter,
                            &(wa_bytecode_loc){
                                .instance = app->env.interpreter->instance,
                                .func = func,
                                .index = codeIndex,
                            });

                        //TODO: should probably not intertwine modified bytecode and UI like that?
                        if(breakpoint)
                        {
                            opcode = wa_breakpoint_saved_opcode(breakpoint);
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
                                if(app->env.instance)
                                {
                                    u32 index = app->env.interpreter->pc - func->code;
                                    wa_func* execFunc = app->env.interpreter->controlStack[app->env.interpreter->controlStackTop].func;

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

                                    f32 targetScroll = codePanel->scroll.y;

                                    if(app->env.autoScroll)
                                    {
                                        f32 scrollMargin = 60;

                                        if(lineY - targetScroll < scrollMargin)
                                        {
                                            targetScroll = lineY - scrollMargin;
                                        }
                                        else if(lineY + lineH - targetScroll > codePanel->rect.h - scrollMargin)
                                        {
                                            targetScroll = lineY + lineH - codePanel->rect.h + scrollMargin;
                                        }
                                    }
                                    codePanel->scroll.y += scrollSpeed * (targetScroll - codePanel->scroll.y);
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
                                            wa_interpreter_remove_breakpoint(app->env.interpreter, breakpoint);
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
                                                app->env.interpreter,
                                                &(wa_bytecode_loc){
                                                    .instance = app->env.interpreter->instance,
                                                    .func = func,
                                                    .index = codeIndex,
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
                                                s = wa_module_get_function_name(app->env.module, opd->valU32);
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
            app->env.lastScroll = codePanel->scroll.y;
            //NOTE: scroll might change (as a result of user action) at the end of the block
        }

        if(!freshScroll && fabs(app->env.lastScroll - codePanel->scroll.y) > 1)
        {
            //NOTE: if user has adjusted scroll manually, deactivate auto-scroll
            OC_ASSERT(oc_ui_box_get_sig(codePanel).wheel.y != 0);
            app->env.autoScroll = false;
        }
    }

    oc_ui_draw();

    oc_canvas_render(app->debuggerUI.renderer, app->debuggerUI.canvas, app->debuggerUI.surface);
    oc_canvas_present(app->debuggerUI.renderer, app->debuggerUI.surface);

    oc_scratch_end(scratch);
}

#endif // OC_WASM_DEBUGGER

void vm_thread_resume(oc_wasm_env* env)
{
    oc_mutex_lock(env->suspendMutex);
    {
        env->paused = false;
        env->prevPaused = false;
        oc_condition_signal(env->suspendCond);
    }
    oc_mutex_unlock(env->suspendMutex);
}

i32 control_runloop(void* user)
{
    oc_runtime* app = (oc_runtime*)user;

    oc_ringbuffer_init(&app->eventBuffer, 16);

    oc_wasm_env_init(&app->env);

    app->env.suspendCond = oc_condition_create();
    app->env.suspendMutex = oc_mutex_create();

    oc_thread* vmThread = oc_thread_create(vm_runloop, app);

    while(!app->quit)
    {
        oc_arena_scope scratch = oc_scratch_begin();
        oc_event* event = 0;

        while((event = oc_next_event(scratch.arena)) != 0)
        {
#ifdef OC_WASM_DEBUGGER //---------------------------------------------------------------------------------------------
            if(!oc_window_is_nil(event->window)
               && event->window.h == app->debuggerUI.window.h)
            {
                oc_ui_set_context(app->debuggerUI.ui);
                oc_ui_process_event(event);

                switch(event->type)
                {
                    case OC_EVENT_WINDOW_CLOSE:
                    {
                        oc_debugger_ui_close(app);
                    }
                    break;

                    case OC_EVENT_QUIT:
                    {
                        //TODO: we should also unblock vm thread and abort interpreter here
                        app->quit = true;
                        vm_thread_resume(&app->env);
                    }
                    break;

                    case OC_EVENT_KEYBOARD_KEY:
                    {
                        if(event->key.action == OC_KEY_PRESS)
                        {
                            if(event->key.keyCode == OC_KEY_C)
                            {
                                //NOTE: signal vm thread to continue
                                app->env.debuggerCommand = OC_DEBUGGER_CONTINUE;
                                vm_thread_resume(&app->env);
                            }
                            else if(event->key.keyCode == OC_KEY_N)
                            {
                                //NOTE: signal vm thread to step
                                app->env.debuggerCommand = OC_DEBUGGER_STEP;
                                vm_thread_resume(&app->env);
                            }
                            else if(event->key.keyCode == OC_KEY_P
                                    && (event->key.mods & OC_KEYMOD_MAIN_MODIFIER))
                            {
                                //TODO: if we're running, signal vm thread to suspend
                                wa_interpreter_suspend(app->env.interpreter);
                            }
                        }
                    }
                    break;

                    default:
                        break;
                }
            }
            else
            {
                if(app->debugOverlay.show)
                {
                    oc_ui_set_context(app->debugOverlay.ui);
                    oc_ui_process_event(event);
                }

                switch(event->type)
                {
                    case OC_EVENT_WINDOW_CLOSE:
                    case OC_EVENT_QUIT:
                    {
                        app->quit = true;
                        vm_thread_resume(&app->env);
                    }
                    break;

                    case OC_EVENT_KEYBOARD_KEY:
                    {
                        if(event->key.action == OC_KEY_PRESS)
                        {
                            if(event->key.keyCode == OC_KEY_D
                               && (event->key.mods & OC_KEYMOD_MAIN_MODIFIER))
                            {
                                if(event->key.mods & OC_KEYMOD_SHIFT)
                                {
    #ifdef OC_WASM_DEBUGGER //---------------------------------------------------------------------------------------------
                                    oc_debugger_ui_open(app);
    #endif // OC_WASM_DEBUGGER ---------------------------------------------------------------------------------------------
                                }
                                else
                                {
                                    debug_overlay_toggle(&app->debugOverlay);
                                }
                            }
                        }
                    }
                    break;

                    default:
                        break;
                }
                if(!app->env.paused)
                {
                    queue_event(&app->eventBuffer, event);
                }
            }
#endif // OC_WASM_DEBUGGER ---------------------------------------------------------------------------------------------
        }

        //TODO: if vm has suspended, drain queue here

#ifdef OC_WASM_DEBUGGER //----------------------------------------------------------------------------------------------
        if(app->debuggerUI.init)
        {
            debugger_ui_update(app);
        }
#endif // OC_WASM_DEBUGGER ---------------------------------------------------------------------------------------------

        if(!app->quit)
        {
            overlay_ui(app);
        }

        oc_scratch_end(scratch);
    }

    i64 exitCode = 0;
    oc_thread_join(vmThread, &exitCode);

    return exitCode;
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
        app->debugOverlay.renderer = oc_canvas_renderer_create();
        app->debugOverlay.surface = oc_canvas_surface_create_for_window(app->debugOverlay.renderer, app->window);
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
        app->debugOverlay.ui = oc_ui_context_create(app->debugOverlay.fontReg);

        //NOTE: show window and start runloop
        oc_window_bring_to_front(app->window);
        oc_window_focus(app->window);
        oc_window_center(app->window);
    }

    oc_thread* controlThread = oc_thread_create(control_runloop, app);

    while(!oc_should_quit())
    {
        oc_pump_events(-1);
    }

    i64 exitCode = 0;
    oc_thread_join(controlThread, &exitCode);

    if(s_test_wasm_module_path == NULL)
    {
        oc_canvas_context_destroy(app->debugOverlay.context);
        oc_surface_destroy(app->debugOverlay.surface);
        oc_canvas_renderer_destroy(app->debugOverlay.renderer);
        oc_canvas_renderer_destroy(app->canvasRenderer);
        oc_window_destroy(app->window);
    }

    oc_terminate();
    return (int)(exitCode);
}
