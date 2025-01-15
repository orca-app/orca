/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "runtime_memory.h"

typedef struct oc_runtime_clipboard
{
    bool isGetAllowed;
    f64 setAllowedUntil;
} oc_runtime_clipboard;

#if OC_PLATFORM_WINDOWS || OC_PLATFORM_MACOS

oc_wasm_str8 oc_runtime_clipboard_get_string(oc_runtime_clipboard* clipboard, oc_wasm_addr wasmArena);
void oc_runtime_clipboard_set_string(oc_runtime_clipboard* clipboard, oc_wasm_str8 value);
oc_event* oc_runtime_clipboard_process_event_begin(oc_arena* arena, oc_runtime_clipboard* clipboard, oc_event* origEvent);
void oc_runtime_clipboard_process_event_end(oc_runtime_clipboard* clipboard);

#else
    #error Default clipboard handling is not supported on this platform"
#endif
