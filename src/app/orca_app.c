/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "orca.h"
#include "wasmbind/hostcalls.h"

//This is used to pass raw events from the runtime
ORCA_EXPORT oc_event oc_rawEvent;

ORCA_EXPORT void* oc_arena_push_aligned_stub(oc_arena* arena, u64 size, u32 alignment)
{
    return (oc_arena_push_aligned(arena, size, alignment));
}

void oc_window_set_title(oc_str8 title)
{
    oc_hostcall_window_set_title(&title);
}

oc_str8 oc_clipboard_get_string(oc_arena* arena)
{
    oc_str8 ret = { 0 };
    oc_hostcall_clipboard_get_string(arena, &ret);
    return ret;
}

void oc_clipboard_set_string(oc_str8 string)
{
    oc_hostcall_clipboard_set_string(&string);
}
