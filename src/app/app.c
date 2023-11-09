/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "app_internal.h"
#include "platform/platform_debug.h"

oc_app oc_appData = { 0 };

//---------------------------------------------------------------
// Window handles
//---------------------------------------------------------------

void oc_init_window_handles()
{
    oc_list_init(&oc_appData.windowFreeList);
    for(int i = 0; i < OC_APP_MAX_WINDOWS; i++)
    {
        oc_appData.windowPool[i].generation = 1;
        oc_list_append(&oc_appData.windowFreeList, &oc_appData.windowPool[i].freeListElt);
    }
}

bool oc_window_handle_is_null(oc_window window)
{
    return (window.h == 0);
}

oc_window oc_window_null_handle()
{
    return ((oc_window){ .h = 0 });
}

oc_window_data* oc_window_alloc()
{
    return (oc_list_pop_entry(&oc_appData.windowFreeList, oc_window_data, freeListElt));
}

oc_window_data* oc_window_ptr_from_handle(oc_window handle)
{
    u32 index = handle.h >> 32;
    u32 generation = handle.h & 0xffffffff;
    if(index >= OC_APP_MAX_WINDOWS)
    {
        return (0);
    }
    oc_window_data* window = &oc_appData.windowPool[index];
    if(window->generation != generation)
    {
        return (0);
    }
    else
    {
        return (window);
    }
}

oc_window oc_window_handle_from_ptr(oc_window_data* window)
{
    OC_DEBUG_ASSERT((window - oc_appData.windowPool) >= 0
                    && (window - oc_appData.windowPool) < OC_APP_MAX_WINDOWS);

    u64 h = ((u64)(window - oc_appData.windowPool)) << 32
          | ((u64)window->generation);

    return ((oc_window){ h });
}

void oc_window_recycle_ptr(oc_window_data* window)
{
    window->generation++;
    oc_list_push(&oc_appData.windowFreeList, &window->freeListElt);
}

//---------------------------------------------------------------
// Init
//---------------------------------------------------------------

static void oc_init_common()
{
    oc_init_window_handles();
    oc_ringbuffer_init(&oc_appData.eventQueue, 16);
}

static void oc_terminate_common()
{
    oc_ringbuffer_cleanup(&oc_appData.eventQueue);
}

//---------------------------------------------------------------
// Event handling
//---------------------------------------------------------------

void oc_set_event_callback(oc_event_proc callback, void* userData)
{
    oc_appData.eventCallback = callback;
    oc_appData.eventCallbackData = userData;
}

void oc_queue_event(oc_event* event)
{
    oc_ringbuffer* queue = &oc_appData.eventQueue;

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

void oc_dispatch_event(oc_event* event)
{
    if(oc_appData.eventCallback)
    {
        oc_appData.eventCallback(event, oc_appData.eventCallbackData);
    }
    else
    {
        oc_queue_event(event);
    }
}

oc_event* oc_next_event(oc_arena* arena)
{
    //NOTE: pop and return event from queue
    oc_event* event = 0;
    oc_ringbuffer* queue = &oc_appData.eventQueue;

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

//---------------------------------------------------------------
// key / scan codes
//---------------------------------------------------------------

oc_key_code oc_scancode_to_keycode(oc_scan_code scanCode)
{
    return (oc_appData.keyMap[scanCode]);
}

#define OC_DEFAULT_KEYMAP_ENTRY(sc, sv, kc, ...) [(int) sc] = OC_VA_NOPT(sv, ##__VA_ARGS__) __VA_ARGS__,

oc_key_code oc_defaultKeyMap[OC_SCANCODE_COUNT] = {
    OC_KEY_TABLE(OC_DEFAULT_KEYMAP_ENTRY)
};

#undef OC_DEFAULT_KEYMAP_ENTRY

//---------------------------------------------------------------
// window rects helpers
//---------------------------------------------------------------

void oc_window_set_content_position(oc_window window, oc_vec2 position)
{
    oc_rect rect = oc_window_get_content_rect(window);
    rect.x = position.x;
    rect.y = position.y;
    oc_window_set_content_rect(window, rect);
}

void oc_window_set_content_size(oc_window window, oc_vec2 size)
{
    oc_rect rect = oc_window_get_content_rect(window);
    rect.w = size.x;
    rect.h = size.y;
    oc_window_set_content_rect(window, rect);
}

void oc_window_set_frame_position(oc_window window, oc_vec2 position)
{
    oc_rect frame = oc_window_get_frame_rect(window);
    frame.x = position.x;
    frame.y = position.y;
    oc_window_set_frame_rect(window, frame);
}

void oc_window_set_frame_size(oc_window window, oc_vec2 size)
{
    oc_rect frame = oc_window_get_frame_rect(window);
    frame.w = size.x;
    frame.h = size.y;
    oc_window_set_frame_rect(window, frame);
}

//---------------------------------------------------------------
// dialogs
//---------------------------------------------------------------

oc_file_dialog_result oc_file_dialog(oc_arena* arena, oc_file_dialog_desc* desc)
{
    return (oc_file_dialog_for_table(arena, desc, oc_file_table_get_global()));
}
