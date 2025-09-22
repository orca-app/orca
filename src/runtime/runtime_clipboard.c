/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "runtime_clipboard.h"

#include <assert.h>

#if OC_PLATFORM_WINDOWS || OC_PLATFORM_MACOS

oc_wasm_str8 oc_runtime_clipboard_get_string(oc_runtime_clipboard* clipboard, oc_wasm_addr wasmArena)
{
    oc_wasm_str8 result = { 0 };
    if(clipboard->isGetAllowed)
    {
        oc_arena_scope scratch = oc_scratch_begin();
        oc_str8 value = oc_clipboard_get_string(scratch.arena);
        oc_wasm_addr valueAddr = oc_wasm_arena_push(wasmArena, value.len + 1);
        char* valuePtr = (char*)oc_wasm_address_to_ptr(valueAddr, value.len + 1);
        memcpy(valuePtr, value.ptr, value.len);
        valuePtr[value.len] = '\0';
        oc_scratch_end(scratch);
        result = (oc_wasm_str8){ .ptr = valueAddr,
                                 .len = value.len };
    }
    else
    {
        oc_log_warning("Clipboard contents can only be get from within the paste event handler\n");
    }
    return (result);
}

static f64 OC_CLIPBOARD_SET_TIMEOUT = 1;

void oc_runtime_clipboard_set_string(oc_runtime_clipboard* clipboard, oc_wasm_str8 value)
{
    f64 time = oc_clock_time(OC_CLOCK_MONOTONIC);
    if(time < clipboard->setAllowedUntil)
    {
        oc_str8 nativeValue = oc_wasm_str8_to_native(value);
        oc_clipboard_set_string(nativeValue);
    }
    else
    {
        oc_log_warning("Clipboard contents can only be set within %f second(s) of a paste shortcut press\n", OC_CLIPBOARD_SET_TIMEOUT);
    }
}

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

#elif OC_PLATFORM_LINUX

oc_wasm_str8 oc_runtime_clipboard_get_string(oc_runtime_clipboard* clipboard, oc_wasm_addr wasmArena)
{
    assert(0 && "Unimplemented");
    return (oc_wasm_str8){0};
}

void oc_runtime_clipboard_set_string(oc_runtime_clipboard* clipboard, oc_wasm_str8 value)
{
    assert(0 && "Unimplemented");
}

oc_event* oc_runtime_clipboard_process_event_begin(oc_arena* arena, oc_runtime_clipboard* clipboard, oc_event* origEvent)
{
    assert(0 && "Unimplemented");
    return NULL;
}

void oc_runtime_clipboard_process_event_end(oc_runtime_clipboard* clipboard)
{
    assert(0 && "Unimplemented");
}

#else
    #error Default clipboard handling is not supported on this platform
#endif
