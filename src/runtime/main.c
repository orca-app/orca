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

#include "debugger.c"

//------------------------------------------------------------------------
// runtime struct
//------------------------------------------------------------------------
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

//------------------------------------------------------------------------
// env init
//------------------------------------------------------------------------

void oc_wasm_env_init(oc_wasm_env* runtime)
{
    memset(runtime, 0, sizeof(oc_wasm_env));
}

//------------------------------------------------------------------------
// event queue
//------------------------------------------------------------------------

///////////////////////////////////////////////////////////////
//TODO: fold this with app's queuing helpers?

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

//------------------------------------------------------------------------
// Wasm backends
//------------------------------------------------------------------------
#include "warm/wasm.c"
#include "warm/warm_adapter.c"
#include "warm/warm.h"
#include "warm/debug_info.h"

//------------------------------------------------------------------------
// VM runloop
//------------------------------------------------------------------------

static const char* s_test_wasm_module_path = NULL;

#include "bridges.c"
#include "bridge_io.c"
#include "runtime_clipboard.c"
#include "runtime_memory.c"

#include "wasmbind/clock_api_bind_gen.c"
#include "wasmbind/core_api_bind_gen.c"
#include "wasmbind/gles_api_bind_manual.c"
#include "wasmbind/gles_api_bind_gen.c"
#include "wasmbind/io_api_bind_gen.c"
#include "wasmbind/surface_api_bind_manual.c"
#include "wasmbind/surface_api_bind_gen.c"

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

            case OC_DEBUGGER_STEP_LINE:
            {
                status = wa_interpreter_step_line(interpreter);
            }
            break;

            case OC_DEBUGGER_QUIT:
            {
                //TODO: should instead return WA_QUIT or something, that shouldn't trigger an error in the caller
                return WA_OK;
            }
            break;
        }
    }

    return status;
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

        //TODO: change for platform layer file IO functions
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

    if(wa_module_status(app->env.module) != WA_OK)
    {
        wa_module_print_errors(app->env.module);
    }

    OC_WASM_TRAP(wa_module_status(app->env.module));

    //NOTE: bind orca APIs
    {
        oc_arena_scope scratch = oc_scratch_begin();

        wa_import_package package = {
            .name = OC_STR8("env"),
        };

        int err = 0;
        err |= bindgen_link_core_api(scratch.arena, &package);
        err |= bindgen_link_surface_api(scratch.arena, &package);
        err |= bindgen_link_clock_api(scratch.arena, &package);
        err |= bindgen_link_io_api(scratch.arena, &package);
        err |= bindgen_link_gles_api(scratch.arena, &package);
        err |= manual_link_gles_api(scratch.arena, &package);

        if(err)
        {
            OC_ABORT("The application couldn't link one or more functions to its web assembly module (see console log for more information)");
        }

        wa_instance_options options = {
            .packageCount = 1,
            .importPackages = &package,
        };
        app->env.instance = wa_instance_create(&app->env.arena, app->env.module, &options);

        OC_WASM_TRAP(wa_instance_status(app->env.instance));

        oc_scratch_end(scratch);
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
                wa_func_type info = wa_func_get_type(scratch.arena, app->env.instance, handle);

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

    //NOTE: tests
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

    //--------------------------------------------------------------------------------------------------------------------
    //NOTE: here the module was successfully loaded/initialized and all export functions and handlers are correctly bound.

    //NOTE: call init handler and frame resize
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

    //NOTE: app event loop: get events and call appropriate handlers

    while(!app->quit)
    {
        oc_arena_scope scratch = oc_scratch_begin();
        oc_event* event = 0;

        while(!app->quit && (event = queue_next_event(scratch.arena, &app->eventBuffer)) != 0)
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

    //NOTE: we exited the event loop, call the terminate handler and cleanup module/instance then quit

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

//------------------------------------------------------------------------
// Control runloop
//------------------------------------------------------------------------

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

void debugger_resume_with_command(oc_runtime* app, oc_debugger_command command)
{
    app->env.debuggerCommand = command;
    vm_thread_resume(&app->env);
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
            if(!oc_window_is_nil(event->window)
               && event->window.h == app->debugger.window.h)
            {
                oc_ui_set_context(app->debugger.ui);
                oc_ui_process_event(event);

                switch(event->type)
                {
                    case OC_EVENT_WINDOW_CLOSE:
                    {
                        oc_debugger_close(app);
                    }
                    break;

                    case OC_EVENT_QUIT:
                    {
                        //TODO: we should also unblock vm thread and abort interpreter here
                        app->quit = true;
                        debugger_resume_with_command(app, OC_DEBUGGER_QUIT);
                    }
                    break;

                    case OC_EVENT_KEYBOARD_KEY:
                    {
                        if(event->key.action == OC_KEY_PRESS)
                        {
                            if(event->key.keyCode == OC_KEY_C)
                            {
                                //NOTE: signal vm thread to continue
                                debugger_resume_with_command(app, OC_DEBUGGER_CONTINUE);
                            }
                            else if(event->key.keyCode == OC_KEY_N)
                            {
                                //NOTE: signal vm thread to step
                                if(app->debugger.showSymbols)
                                {
                                    debugger_resume_with_command(app, OC_DEBUGGER_STEP);
                                }
                                else
                                {
                                    debugger_resume_with_command(app, OC_DEBUGGER_STEP_LINE);
                                }
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
                        debugger_resume_with_command(app, OC_DEBUGGER_QUIT);
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
                                    oc_debugger_open(app);
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
        }

        //TODO: if vm has suspended, drain queue here

        if(app->debugger.init)
        {
            debugger_ui_update(&app->debugger, &app->env);
        }

        if(!app->quit)
        {
            overlay_ui(&app->debugOverlay);
        }

        oc_scratch_end(scratch);
    }

    i64 exitCode = 0;
    oc_thread_join(vmThread, &exitCode);

    return exitCode;
}

//------------------------------------------------------------------------
// Main runloop
//------------------------------------------------------------------------

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

        app->debugOverlay.ui = oc_ui_context_create(app->debugOverlay.fontReg);

        //NOTE: show window
        oc_window_bring_to_front(app->window);
        oc_window_focus(app->window);
        oc_window_center(app->window);
    }

    //NOTE: start runloop
    oc_thread* controlThread = oc_thread_create(control_runloop, app);

    while(!oc_should_quit())
    {
        oc_pump_events(-1);
    }

    //NOTE: collect threads and wind down
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
