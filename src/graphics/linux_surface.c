/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "linux_surface.h"
#include "surface.h"

void oc_surface_base_init_for_window(oc_surface_base* surface, oc_window_data* window)
{
    oc_unimplemented();
}

void oc_surface_base_cleanup(oc_surface_base* surface)
{
    oc_unimplemented();
}

oc_vec2 oc_surface_base_get_size(oc_surface_base* surface)
{
    oc_unimplemented();
    return (oc_vec2){ 0, 0 };
}

oc_vec2 oc_surface_base_contents_scaling(oc_surface_base* surface)
{
    oc_unimplemented();
    return (oc_vec2){ 0, 0 };
}

void oc_surface_base_bring_to_front(oc_surface_base* surface)
{
    oc_unimplemented();
}

void oc_surface_base_send_to_back(oc_surface_base* surface)
{
    oc_unimplemented();
}

bool oc_surface_base_get_hidden(oc_surface_base* surface)
{
    oc_unimplemented();
    return false;
}

void oc_surface_base_set_hidden(oc_surface_base* surface, bool hidden)
{
    oc_unimplemented();
}

