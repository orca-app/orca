#include "orca.h"
#include "runtime/runtime_memory.h"

//------------------------------------------------------------------------
// bounds checking functions
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

u64 orca_image_upload_region_rgba8_length(wa_instance* instance, oc_rect* rect)
{
    u64 pixelFormatWidth = sizeof(u8) * 4;
    u64 len = rect->w * rect->h * pixelFormatWidth;
    return len;
}

//------------------------------------------------------------------------
// failure dialog boxes
//------------------------------------------------------------------------

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

//------------------------------------------------------------------------
// clock handlers
//------------------------------------------------------------------------

f32 oc_hostapi_clock_time(oc_clock_kind clock)
{
    return oc_clock_time(clock);
}

//------------------------------------------------------------------------
// log/assert handlers
//------------------------------------------------------------------------

void oc_hostapi_log(oc_log_level level, i32 functionLen, char* function, i32 fileLen, char* file, i32 line, i32 msgLen, char* msg)
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

void oc_hostapi_assert_fail(char* file, char* function, i32 line, char* src, char* note)
{
    oc_assert_fail_dialog(file, function, line, src, note);
}

void oc_hostapi_exit(i32 ec)
{
    //TODO: send a trap exit to oc_wasm to stop interpreter,
    // then when we return from the trap, quit app
    // temporarily, we just exit here
    exit(ec);
}

void oc_hostapi_abort_ext(char* file, char* function, i32 line, char* note)
{
    oc_abort_ext_dialog(file, function, line, note);
}

//------------------------------------------------------------------------
// mem grow
//------------------------------------------------------------------------

i32 oc_hostapi_mem_grow(u32 size)
{
    return oc_mem_grow(size);
}

//------------------------------------------------------------------------
// get host
//------------------------------------------------------------------------

i32 oc_hostapi_get_host_platform()
{
    return oc_get_host_platform();
}

//------------------------------------------------------------------------
// app / window handlers
//------------------------------------------------------------------------

void oc_hostapi_request_quit()
{
    oc_request_quit();
}

void oc_hostapi_window_set_title(oc_wasm_str8* title)
{
    oc_str8 nativeTitle = oc_wasm_str8_to_native(*title);
    if(nativeTitle.ptr)
    {
        oc_window_set_title(__orcaApp.window, nativeTitle);
    }
}

void oc_hostapi_window_set_size(oc_vec2* size)
{
    oc_window_set_content_size(__orcaApp.window, *size);
}

oc_key_code oc_hostapi_scancode_to_keycode(oc_scan_code scanCode)
{
    return oc_scancode_to_keycode(scanCode);
}

static f64 OC_CLIPBOARD_SET_TIMEOUT = 1;

oc_event* oc_runtime_clipboard_process_event_begin(oc_arena* arena, oc_runtime_clipboard* clipboard, oc_event* origEvent)
{
    oc_event* resultEvent = 0;
    if(origEvent->type == OC_EVENT_KEYBOARD_KEY)
    {
        bool isPressedOrRepeated = origEvent->key.action == OC_KEY_PRESS || origEvent->key.action == OC_KEY_REPEAT;
        oc_keymod_flags rawMods = origEvent->key.mods & ~OC_KEYMOD_MAIN_MODIFIER;
#if OC_PLATFORM_WINDOWS
        bool cutOrCopied = isPressedOrRepeated
                        && ((origEvent->key.keyCode == OC_KEY_X && rawMods == OC_KEYMOD_CTRL)
                            || (origEvent->key.keyCode == OC_KEY_DELETE && rawMods == OC_KEYMOD_SHIFT)
                            || (origEvent->key.keyCode == OC_KEY_C && rawMods == OC_KEYMOD_CTRL)
                            || (origEvent->key.keyCode == OC_KEY_INSERT && rawMods == OC_KEYMOD_CTRL));
        bool pasted = isPressedOrRepeated
                   && ((origEvent->key.keyCode == OC_KEY_V && rawMods == OC_KEYMOD_CTRL)
                       || (origEvent->key.keyCode == OC_KEY_INSERT && rawMods == OC_KEYMOD_SHIFT));
#elif OC_PLATFORM_MACOS
        bool cutOrCopied = isPressedOrRepeated
                        && ((origEvent->key.keyCode == OC_KEY_X && rawMods == OC_KEYMOD_CMD)
                            || (origEvent->key.keyCode == OC_KEY_C && rawMods == OC_KEYMOD_CMD));
        bool pasted = isPressedOrRepeated
                   && origEvent->key.keyCode == OC_KEY_V && rawMods == OC_KEYMOD_CMD;
#endif
        if(cutOrCopied)
        {
            clipboard->setAllowedUntil = oc_clock_time(OC_CLOCK_MONOTONIC) + OC_CLIPBOARD_SET_TIMEOUT;
        }

        if(pasted)
        {
            clipboard->isGetAllowed = true;
            resultEvent = oc_arena_push_type(arena, oc_event);
            *resultEvent = (oc_event){ .window = origEvent->window,
                                       .type = OC_EVENT_CLIPBOARD_PASTE };
        }
    }
    return (resultEvent);
}

void oc_runtime_clipboard_process_event_end(oc_runtime_clipboard* clipboard)
{
    clipboard->isGetAllowed = false;
}

void oc_hostapi_clipboard_get_string(oc_wasm_arena* wasmArena, oc_wasm_str8* returnPointer)
{
    oc_runtime_clipboard* clipboard = &__orcaApp.clipboard;

    oc_wasm_str8 result = { 0 };
    if(clipboard->isGetAllowed)
    {
        oc_arena_scope scratch = oc_scratch_begin();

        oc_str8 string = oc_clipboard_get_string(scratch.arena);

        oc_wasm_addr returnAddr = oc_wasm_arena_push(wasmArena, string.len + 1);
        char* returnPtr = (char*)oc_wasm_address_to_ptr(returnAddr, string.len + 1);

        memcpy(returnPtr, string.ptr, string.len);
        returnPtr[string.len] = '\0';

        returnPointer->len = string.len;
        returnPointer->ptr = returnAddr;

        oc_scratch_end(scratch);
    }
    else
    {
        oc_log_warning("Clipboard contents can only be get from within the paste event handler\n");
    }
}

void oc_hostapi_clipboard_set_string(oc_wasm_str8* value)
{
    oc_runtime_clipboard* clipboard = &__orcaApp.clipboard;

    f64 time = oc_clock_time(OC_CLOCK_MONOTONIC);
    if(time < clipboard->setAllowedUntil)
    {
        oc_str8 nativeValue = oc_wasm_str8_to_native(*value);
        oc_clipboard_set_string(nativeValue);
    }
    else
    {
        oc_log_warning("Clipboard contents can only be set within %f second(s) of a paste shortcut press\n", OC_CLIPBOARD_SET_TIMEOUT);
    }
}

//------------------------------------------------------------------------
// IO handlers
//------------------------------------------------------------------------

typedef struct oc_wasm_file_dialog_desc
{
    oc_file_dialog_kind kind;
    oc_file_dialog_flags flags;
    oc_wasm_str8 title;
    oc_wasm_str8 okLabel;
    oc_file startAt;
    oc_wasm_str8 startPath;
    oc_wasm_str8_list filters;

} oc_wasm_file_dialog_desc;

typedef struct oc_wasm_file_open_with_dialog_elt
{
    oc_wasm_list_elt listElt;
    oc_file file;
} oc_wasm_file_open_with_dialog_elt;

typedef struct oc_wasm_file_open_with_dialog_result
{
    oc_file_dialog_button button;
    oc_file file;
    oc_wasm_list selection;

} oc_wasm_file_open_with_dialog_result;

typedef struct oc_wasm_file_listdir_elt
{
    oc_wasm_list_elt listElt;
    oc_wasm_str8 basename;
    oc_file_type type;
} oc_wasm_file_listdir_elt;

typedef struct oc_wasm_file_list
{
    oc_wasm_list list;
    u64 eltCount;
} oc_wasm_file_list;

void oc_hostapi_io_wait_single_req(oc_io_req* wasmReq, oc_io_cmp* returnPointer)
{
    oc_runtime* orca = oc_runtime_get();

    oc_io_cmp cmp = { 0 };
    oc_io_req req = *wasmReq;

    //TODO: lookup if operation needs a buffer in a compile-time table
    oc_io_op op = wasmReq->op;
    if(op == OC_IO_OPEN
       || op == OC_IO_STAT
       || op == OC_IO_READ
       || op == OC_IO_WRITE)
    {
        //TODO have a separate oc_wasm_io_req struct, and marshall between wasm/native versions
        void* buffer = oc_wasm_address_to_ptr((oc_wasm_addr)(uintptr_t)req.buffer, req.size);
        if(buffer)
        {
            req.buffer = buffer;

            //TODO: lookup in a compile-time table which operations use a 'at' handle that must be replaced by root handle if 0.
            if(req.op == OC_IO_OPEN)
            {
                if(req.handle.h == 0)
                {
                    //NOTE: change root to app local folder
                    req.handle = orca->rootDir;
                }
            }
        }
        else
        {
            cmp.error = OC_IO_ERR_ARG;
        }
    }
    if(cmp.error == OC_IO_OK)
    {
        cmp = oc_io_wait_single_req_for_table(&req, &orca->fileTable);
    }

    *returnPointer = cmp;
}

void oc_hostapi_file_open_with_request(oc_wasm_str8* path, oc_file_access rights, oc_file_open_flags flags, oc_file* returnPointer)
{
    oc_file file = oc_file_nil();
    oc_runtime* orca = oc_runtime_get();

    oc_str8 nativePath = oc_wasm_str8_to_native(*path);

    if(nativePath.ptr)
    {
        file = oc_file_open_with_request_for_table(nativePath, rights, flags, &orca->fileTable);
    }
    *returnPointer = file;
}

void oc_hostapi_file_open_with_dialog(oc_wasm_arena* wasmArena,
                                      oc_file_access rights,
                                      oc_file_open_flags flags,
                                      oc_wasm_file_dialog_desc* desc,
                                      oc_wasm_file_open_with_dialog_result* returnPointer)
{
    oc_runtime* orca = oc_runtime_get();
    oc_arena_scope scratch = oc_scratch_begin();

    oc_file_dialog_desc nativeDesc = {
        .kind = desc->kind,
        .flags = desc->flags
    };

    nativeDesc.title.ptr = oc_wasm_address_to_ptr(desc->title.ptr, desc->title.len);
    nativeDesc.title.len = desc->title.len;

    nativeDesc.okLabel.ptr = oc_wasm_address_to_ptr(desc->okLabel.ptr, desc->okLabel.len);
    nativeDesc.okLabel.len = desc->okLabel.len;

    if(oc_file_is_nil(desc->startAt) && desc->startPath.len)
    {
        nativeDesc.startAt = orca->rootDir;
    }
    else
    {
        nativeDesc.startAt = desc->startAt;
    }
    nativeDesc.startPath.ptr = oc_wasm_address_to_ptr(desc->startPath.ptr, desc->startPath.len);
    nativeDesc.startPath.len = desc->startPath.len;

    u32 eltIndex = desc->filters.list.first;
    while(eltIndex)
    {
        oc_wasm_str8_elt* elt = oc_wasm_address_to_ptr(eltIndex, sizeof(oc_wasm_str8_elt));
        oc_str8 filter = oc_wasm_str8_to_native(elt->string);

        oc_str8_list_push(scratch.arena, &nativeDesc.filters, filter);

        oc_log_info("filter: %.*s\n", (int)filter.len, filter.ptr);

        eltIndex = elt->listElt.next;
    }

    oc_file_open_with_dialog_result nativeResult = oc_file_open_with_dialog_for_table(scratch.arena, rights, flags, &nativeDesc, &orca->fileTable);

    oc_wasm_file_open_with_dialog_result result = {
        .button = nativeResult.button,
        .file = nativeResult.file
    };

    oc_list_for(nativeResult.selection, elt, oc_file_open_with_dialog_elt, listElt)
    {
        oc_wasm_addr wasmEltAddr = oc_wasm_arena_push_type(wasmArena, oc_wasm_file_open_with_dialog_elt);
        oc_wasm_file_open_with_dialog_elt* wasmElt = oc_wasm_address_to_ptr(wasmEltAddr, sizeof(oc_wasm_file_open_with_dialog_elt));
        wasmElt->file = elt->file;

        oc_wasm_list_push_back(&result.selection, &wasmElt->listElt);
    }

    oc_scratch_end(scratch);
    *returnPointer = result;
}

void oc_hostapi_file_listdir(oc_wasm_arena* wasmArena, oc_file* directory, oc_wasm_file_list* returnPointer)
{
    oc_arena_scope scratch = oc_scratch_begin();

    oc_runtime* orca = oc_runtime_get();
    oc_file_list nativeList = oc_file_listdir_for_table(scratch.arena, *directory, &orca->fileTable);

    oc_wasm_file_list wasmList = { 0 };
    wasmList.eltCount = nativeList.eltCount;
    if(oc_file_last_error(*directory) == OC_IO_OK)
    {
        oc_file_list_for(nativeList, nativeElt)
        {
            oc_wasm_addr wasmEltAddr = oc_wasm_arena_push_type(wasmArena, oc_wasm_file_listdir_elt);
            oc_wasm_file_listdir_elt* wasmElt = oc_wasm_address_to_ptr(wasmEltAddr, sizeof(oc_wasm_file_listdir_elt));
            oc_wasm_list_push_back(&wasmList.list, &wasmElt->listElt);

            wasmElt->basename = oc_wasm_str8_from_native(wasmArena, nativeElt->basename);
            wasmElt->type = nativeElt->type;
        }
    }

    oc_scratch_end(scratch);
    *returnPointer = wasmList;
}

//------------------------------------------------------------------------
// graphics handlers
//------------------------------------------------------------------------

void oc_hostapi_image_size(oc_image* image, oc_vec2* returnPointer)
{
    *returnPointer = oc_image_size(*image);
}

void oc_hostapi_image_create(oc_canvas_renderer* renderer, u32 width, u32 height, oc_image* returnPointer)
{
    *returnPointer = oc_image_create(*renderer, width, height);
}

void oc_hostapi_image_destroy(oc_image* image)
{
    oc_image_destroy(*image);
}

void oc_hostapi_image_upload_region_rgba8(oc_image* image, oc_rect* region, u8* pixels)
{
    oc_image_upload_region_rgba8(*image, *region, pixels);
}

void oc_hostapi_surface_get_size(oc_surface* surface, oc_vec2* returnPointer)
{
    *returnPointer = oc_surface_get_size(*surface);
}

void oc_hostapi_surface_contents_scaling(oc_surface* surface, oc_vec2* returnPointer)
{
    *returnPointer = oc_surface_contents_scaling(*surface);
}

void oc_hostapi_surface_bring_to_front(oc_surface* surface)
{
    oc_surface_bring_to_front(*surface);
}

void oc_hostapi_surface_send_to_back(oc_surface* surface)
{
    oc_surface_send_to_back(*surface);
}

void oc_hostapi_canvas_renderer_create(oc_canvas_renderer* returnPointer)
{
    *returnPointer = oc_canvas_renderer_create();
}

void oc_hostapi_canvas_surface_create(oc_canvas_renderer* renderer, oc_surface* returnPointer)
{
    *returnPointer = oc_canvas_surface_create_for_window(*renderer, __orcaApp.window);
}

#include "graphics/graphics_common.h"

void oc_hostapi_canvas_renderer_submit(oc_canvas_renderer* renderer,
                                       oc_surface* surface,
                                       u32 msaaSampleCount,
                                       bool clear,
                                       oc_color* clearColor,
                                       u32 primitiveCount,
                                       oc_primitive* primitives,
                                       u32 eltCount,
                                       oc_path_elt* elements)
{
    oc_canvas_renderer_submit(*renderer,
                              *surface,
                              msaaSampleCount,
                              clear,
                              *clearColor,
                              primitiveCount,
                              primitives,
                              eltCount,
                              elements);
}

void oc_hostapi_canvas_present(oc_canvas_renderer* renderer, oc_surface* surface)
{
    oc_canvas_present(*renderer, *surface);
}

void oc_hostapi_gles_surface_create(oc_surface* returnPointer)
{
    *returnPointer = oc_gles_surface_create_for_window(__orcaApp.window);
}

void oc_hostapi_gles_surface_make_current(oc_surface* surface)
{
    oc_gles_surface_make_current(*surface);
}

void oc_hostapi_gles_surface_swap_buffers(oc_surface* surface)
{
    oc_gles_surface_swap_buffers(*surface);
}
