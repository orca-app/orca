/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "runtime.h"

//------------------------------------------------------------------------
// bounds checking and bridge functions
//------------------------------------------------------------------------
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
        char* mem = oc_arena_push_aligned(&debug->logArena, cap, _Alignof(log_entry));
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

oc_surface oc_bridge_gles_surface_create(void)
{
    oc_surface surface = oc_gles_surface_create_for_window(__orcaApp.window);
#if OC_PLATFORM_WINDOWS
    //NOTE(martin): on windows we set all surfaces to non-synced, and do a single "manual" wait.
    //              on macOS each surface is individually synced to the monitor refresh rate but don't block each other
    oc_gles_surface_swap_interval(surface, 0);
#endif
    return (surface);
}

oc_canvas_renderer oc_bridge_canvas_renderer_create(void)
{
    return (__orcaApp.canvasRenderer);
}

oc_surface oc_bridge_canvas_surface_create(oc_canvas_renderer renderer)
{
    //TODO: check renderer
    oc_surface surface = oc_canvas_surface_create_for_window(__orcaApp.canvasRenderer, __orcaApp.window);
    return (surface);
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
