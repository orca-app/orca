/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <orca.h>

static oc_vec2 frameSize = { 100, 100 };
static u64 frame = 0;

ORCA_EXPORT void oc_on_init(void)
{
    oc_window_set_title(OC_STR8("Orca Linux playground"));
    oc_window_set_size((oc_vec2){ .x = 20, .y = 20 });
}

ORCA_EXPORT void oc_on_resize(u32 width, u32 height)
{
    frameSize.x = width;
    frameSize.y = height;
}

ORCA_EXPORT void oc_on_frame_refresh(void)
{
    frame++;
}
