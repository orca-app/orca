/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "orca.h"

//This is used to pass raw events from the runtime
ORCA_EXPORT oc_event oc_rawEvent;

ORCA_EXPORT void* oc_arena_push_stub(oc_arena* arena, u64 size)
{
    return (oc_arena_push(arena, size));
}
