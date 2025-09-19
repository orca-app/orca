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

#include <sys/stat.h>
#include <unistd.h>
#include <copyfile.h>
#include "ext/libzip/lib/zip.h"

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

static bool s_is_test_module = false;

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

            case OC_DEBUGGER_INSTR_STEP_IN:
            {
                status = wa_interpreter_instr_step_in(interpreter);
            }
            break;

            case OC_DEBUGGER_LINE_STEP_IN:
            {
                status = wa_interpreter_line_step_in(interpreter);
            }
            break;

            case OC_DEBUGGER_INSTR_STEP_OVER:
            {
                status = wa_interpreter_instr_step_over(interpreter);
            }
            break;

            case OC_DEBUGGER_LINE_STEP_OVER:
            {
                status = wa_interpreter_line_step_over(interpreter);
            }
            break;

            case OC_DEBUGGER_STEP_OUT:
            {
                status = wa_interpreter_step_out(interpreter);
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

int make_dirs(oc_str8 path)
{
    int result = 0;
    oc_arena_scope scratch = oc_scratch_begin();

    oc_str8_list elements = oc_path_split(scratch.arena, path);

    oc_str8_list acc = { 0 };
    if(path.len && path.ptr[0] == '/')
    {
        ////////////////////////////////////////////////////
        //NOTE: should oc_path_split return root? otherwise we can't
        // differentiate between abs an relative path...
        ////////////////////////////////////////////////////
        oc_str8_list_push(scratch.arena, &acc, OC_STR8("/"));
    }

    oc_str8_list_for(elements, elt)
    {
        oc_str8_list_push(scratch.arena, &acc, elt->string);
        oc_str8 accPath = oc_path_join(scratch.arena, acc);

        struct stat st = { 0 };
        if(stat(accPath.ptr, &st) != 0)
        {
            // create the directory
            mkdir(accPath.ptr, 0700);
        }
        else if(!(st.st_mode & S_IFDIR))
        {
            // file exists, but not a directory. Error out.
            result = -1;
            goto end;
        }
    }

end:
    oc_scratch_end(scratch);
    return result;
}

/*
int extract_zip(oc_str8 src, oc_str8 dst)
{
    oc_arena_scope scratch = oc_scratch_begin();
    const char* srcCStr = oc_str8_to_cstring(scratch.arena, src);

    zip_t* zip = zip_open(srcCStr, ZIP_RDONLY, 0);
    if(!zip)
    {
        goto error;
    }

    i64 count = zip_get_num_entries(zip, 0);
    for(i64 entryIndex = 0; entryIndex < count; entryIndex++)
    {
        oc_str8 name = OC_STR8(zip_get_name(zip, entryIndex, 0));
        if(!name.ptr)
        {
            goto error;
        }
        else
        {
            oc_str8_list list = { 0 };
            oc_str8_list_push(scratch.arena, &list, dst);
            oc_str8_list_push(scratch.arena, &list, name);
            oc_str8 dstPath = oc_path_join(scratch.arena, list);

            if(name.ptr[name.len - 1] == '/')
            {
                //NOTE: directory. Ignore since we create dirs when extracting files
            }
            else if(!oc_str8_cmp(oc_str8_slice(name, 0, 8), OC_STR8("__MACOSX")))
            {
                //NOTE: apple archiver's way of storing resource forks. Ignore
            }
            else
            {
                //NOTE: normal file
                // first make the leading directories if they don't exist
                oc_str8 dir = oc_path_slice_directory(dstPath);
                if(make_dirs(dir) != 0)
                {
                    goto error;
                }

                zip_file_t* srcFile = zip_fopen_index(zip, entryIndex, 0);
                if(!srcFile)
                {
                    goto error;
                }
                FILE* dstFile = fopen(dstPath.ptr, "w");
                if(!dstFile)
                {
                    goto error;
                }

                char chunk[1024];
                while(1)
                {
                    i64 n = zip_fread(srcFile, chunk, 1024);
                    if(n == 0)
                    {
                        break;
                    }
                    else if(n == -1)
                    {
                        fclose(dstFile);
                        zip_fclose(srcFile);
                        goto error;
                    }
                    else
                    {
                        fwrite(chunk, 1, n, dstFile);
                    }
                }

                fclose(dstFile);
                zip_fclose(srcFile);
            }
        }
    }

    zip_close(zip);
    oc_scratch_end(scratch);
    return 0;

error:
    if(zip)
    {
        zip_close(zip);
    }
    oc_scratch_end(scratch);
    return -1;
}
*/

oc_str8 get_orca_home_dir(oc_arena* arena)
{
    oc_arena_scope scratch = oc_scratch_begin_next(arena);

    char* home = getenv("HOME");

    oc_str8_list list = { 0 };
    oc_str8_list_push(scratch.arena, &list, OC_STR8(home));
    oc_str8_list_push(scratch.arena, &list, OC_STR8(".orca"));

    oc_str8 path = oc_path_join(arena, list);

    oc_scratch_end(scratch);

    return path;
}

/*
int copy_dir_from_archive(oc_str8 archive, oc_str8 src, oc_str8 dst)
{
    oc_arena_scope scratch = oc_scratch_begin();
    const char* archiveCStr = oc_str8_to_cstring(scratch.arena, archive);

    zip_t* zip = zip_open(archiveCStr, ZIP_RDONLY, 0);
    if(!zip)
    {
        goto error;
    }

    i64 count = zip_get_num_entries(zip, 0);
    for(i64 entryIndex = 0; entryIndex < count; entryIndex++)
    {
        oc_str8 name = OC_STR8(zip_get_name(zip, entryIndex, 0));
        if(!name.ptr)
        {
            goto error;
        }
        else
        {
            oc_str8_list list = { 0 };
            oc_str8_list_push(scratch.arena, &list, dst);
            oc_str8_list_push(scratch.arena, &list, name);
            oc_str8 dstPath = oc_path_join(scratch.arena, list);

            if(name.ptr[name.len - 1] == '/')
            {
                //NOTE: directory. Ignore since we create dirs when extracting files
            }
            else if(!oc_str8_cmp(oc_str8_slice(name, 0, src.len), src))
            {
                //NOTE: normal file inside src dir
                // first make the leading directories if they don't exist
                oc_str8 dir = oc_path_slice_directory(dstPath);
                if(make_dirs(dir) != 0)
                {
                    goto error;
                }

                zip_file_t* srcFile = zip_fopen_index(zip, entryIndex, 0);
                if(!srcFile)
                {
                    goto error;
                }
                FILE* dstFile = fopen(dstPath.ptr, "w");
                if(!dstFile)
                {
                    goto error;
                }

                char chunk[1024];
                while(1)
                {
                    i64 n = zip_fread(srcFile, chunk, 1024);
                    if(n == 0)
                    {
                        break;
                    }
                    else if(n == -1)
                    {
                        fclose(dstFile);
                        zip_fclose(srcFile);
                        goto error;
                    }
                    else
                    {
                        fwrite(chunk, 1, n, dstFile);
                    }
                }

                fclose(dstFile);
                zip_fclose(srcFile);
            }
        }
    }

    zip_close(zip);
    oc_scratch_end(scratch);
    return 0;

error:
    if(zip)
    {
        zip_close(zip);
    }
    oc_scratch_end(scratch);
    return -1;
}
*/

/*
oc_str8 load_module_from_archive(oc_arena* arena, oc_str8 archive, oc_str8 name)
{
    oc_str8 res = { 0 };

    oc_arena_scope scratch = oc_scratch_begin_next(arena);
    const char* archiveCStr = oc_str8_to_cstring(scratch.arena, archive);

    zip_t* zip = zip_open(archiveCStr, ZIP_RDONLY, 0);
    if(zip)
    {
        oc_str8_list list = { 0 };
        oc_str8_list_push(scratch.arena, &list, OC_STR8("modules"));
        oc_str8_list_push(scratch.arena, &list, name);
        oc_str8 modulePath = oc_path_join(scratch.arena, list);

        zip_file_t* zipFile = zip_fopen(zip, modulePath.ptr, 0);
        if(zipFile)
        {
            oc_str8_list chunks = { 0 };
            while(1)
            {
                char* chunk = oc_arena_push_array(scratch.arena, char, 1024);
                i64 n = zip_fread(zipFile, chunk, 1024);
                if(n > 0)
                {
                    oc_str8_list_push(scratch.arena, &chunks, oc_str8_from_buffer(n, chunk));
                }
                else
                {
                    break;
                }
            }
            res = oc_str8_list_join(arena, chunks);
            zip_fclose(zipFile);
        }

        zip_close(zip);
    }
    return res;
}
*/

void load_app(oc_runtime* app)
{
    oc_arena_scope scratch = oc_scratch_begin();

    oc_str8 orcaDir = get_orca_home_dir(scratch.arena);

    //NOTE: copy data from app dir to user's dir
    oc_str8 dataDirSrc = { 0 };
    {
        oc_str8_list list = { 0 };
        oc_str8_list_push(scratch.arena, &list, app->path);
        oc_str8_list_push(scratch.arena, &list, OC_STR8("data"));

        dataDirSrc = oc_path_join(scratch.arena, list);
    }
    oc_str8 dataDirDest = { 0 };
    {
        //TODO: need a function to get extension / stem from path
        oc_str8 appname = oc_path_slice_filename(app->path);

        oc_str8_list list = { 0 };
        oc_str8_list_push(scratch.arena, &list, orcaDir);
        oc_str8_list_push(scratch.arena, &list, OC_STR8("userdata"));
        oc_str8_list_push(scratch.arena, &list, appname);

        dataDirDest = oc_path_join(scratch.arena, list);
    }
    copyfile(dataDirSrc.ptr, dataDirDest.ptr, NULL, COPYFILE_DATA | COPYFILE_RECURSIVE);

    //NOTE: loads wasm module
    {
        oc_str8_list list = { 0 };
        oc_str8_list_push(scratch.arena, &list, app->path);
        oc_str8_list_push(scratch.arena, &list, OC_STR8("modules/main.wasm"));
        oc_str8 modulePath = oc_path_join(scratch.arena, list);

        //TODO: change for platform layer file IO functions
        FILE* file = fopen(modulePath.ptr, "rb");
        if(!file)
        {
            OC_ABORT("The application couldn't load: web assembly module '%s' not found", modulePath.ptr);
        }

        fseek(file, 0, SEEK_END);
        u64 wasmSize = ftell(file);
        rewind(file);

        app->env.wasmBytecode.len = wasmSize;
        app->env.wasmBytecode.ptr = oc_malloc_array(char, wasmSize);
        fread(app->env.wasmBytecode.ptr, 1, app->env.wasmBytecode.len, file);
        fclose(file);
    }

    oc_scratch_end(scratch);

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

                bool checked = (info.paramCount == desc->paramCount)
                            && (info.returnCount == desc->returnCount);

                //NOTE: check function signature
                if(checked)
                {
                    for(int paramIndex = 0; paramIndex < info.paramCount; paramIndex++)
                    {
                        if(info.params[paramIndex] != desc->params[paramIndex])
                        {
                            checked = false;
                            break;
                        }
                    }

                    for(int retIndex = 0; retIndex < info.returnCount; retIndex++)
                    {
                        if(info.returns[retIndex] != desc->returns[retIndex])
                        {
                            checked = false;
                            break;
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
}

i32 vm_runloop(void* user)
{
    oc_runtime* app = &__orcaApp;

    load_app(app);

    wa_func** exports = app->env.exports;

    //NOTE: tests
    if(s_is_test_module)
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
        app->quit = true;
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
                                //NOTE: signal vm thread to step over
                                if(app->debugger.selectedTab && app->debugger.selectedTab->mode == OC_DEBUGGER_CODE_TAB_ASSEMBLY)
                                {
                                    debugger_resume_with_command(app, OC_DEBUGGER_INSTR_STEP_OVER);
                                }
                                else
                                {
                                    debugger_resume_with_command(app, OC_DEBUGGER_LINE_STEP_OVER);
                                }
                            }
                            else if(event->key.keyCode == OC_KEY_I)
                            {
                                if(app->debugger.selectedTab && app->debugger.selectedTab->mode == OC_DEBUGGER_CODE_TAB_ASSEMBLY)
                                {
                                    debugger_resume_with_command(app, OC_DEBUGGER_INSTR_STEP_IN);
                                }
                                else
                                {
                                    debugger_resume_with_command(app, OC_DEBUGGER_LINE_STEP_IN);
                                }
                            }
                            else if(event->key.keyCode == OC_KEY_O)
                            {
                                debugger_resume_with_command(app, OC_DEBUGGER_STEP_OUT);
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
#if WA_ENABLE_DEBUGGER //-------------------------------------------------------------
                                    oc_debugger_open(app);
#endif //-----------------------------------------------------------------------------
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
            debugger_ui(&app->debugger, &app->env);
        }

        //TODO: properly check if running in test mode here (instead of testing if ui context is not null)
        if(!app->quit && app->debugOverlay.ui)
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
    oc_runtime* app = &__orcaApp;

    for(i32 i = 1; i < argc; i++)
    {
        if(strstr(argv[i], "--test"))
        {
            s_is_test_module = true;
        }
        else
        {
            app->path = OC_STR8(argv[i]);
        }
    }

    oc_arena_init(&app->env.arena);
    if(!app->path.len)
    {
        app->path = oc_path_executable_relative(&app->env.arena, OC_STR8("../app.orca"));
    }

    oc_log_set_level(OC_LOG_LEVEL_INFO);
    oc_init();
    oc_clock_init();

    app->debugOverlay.maxEntries = 200;
    oc_arena_init(&app->debugOverlay.logArena);

    if(s_is_test_module == false)
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

    if(s_is_test_module == false)
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
