/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "surface.h"

//---------------------------------------------------------------
// typed handles functions
//---------------------------------------------------------------

oc_surface oc_surface_handle_alloc(oc_surface_base* surface)
{
    oc_surface handle = { .h = oc_graphics_handle_alloc(OC_GRAPHICS_HANDLE_SURFACE, (void*)surface) };
    return (handle);
}

oc_surface_base* oc_surface_from_handle(oc_surface handle)
{
    oc_surface_base* data = oc_graphics_data_from_handle(OC_GRAPHICS_HANDLE_SURFACE, handle.h);
    return (data);
}

//---------------------------------------------------------------
// surface API
//---------------------------------------------------------------

void oc_surface_destroy(oc_surface handle)
{
    OC_DEBUG_ASSERT(oc_graphicsData.init);
    oc_surface_base* surface = oc_surface_from_handle(handle);
    if(surface)
    {
        surface->destroy(surface);
        oc_graphics_handle_recycle(handle.h);
    }
}

oc_vec2 oc_surface_get_size(oc_surface handle)
{
    OC_DEBUG_ASSERT(oc_graphicsData.init);
    oc_vec2 size = { 0 };
    oc_surface_base* surface = oc_surface_from_handle(handle);
    if(surface && surface->getSize)
    {
        size = surface->getSize(surface);
    }
    return (size);
}

oc_vec2 oc_surface_contents_scaling(oc_surface handle)
{
    OC_DEBUG_ASSERT(oc_graphicsData.init);
    oc_vec2 scaling = { 1, 1 };
    oc_surface_base* surface = oc_surface_from_handle(handle);
    if(surface && surface->contentsScaling)
    {
        scaling = surface->contentsScaling(surface);
    }
    return (scaling);
}

void oc_surface_bring_to_front(oc_surface handle)
{
    oc_surface_base* surface = oc_surface_from_handle(handle);
    if(surface && surface->bringToFront)
    {
        surface->bringToFront(surface);
    }
}

void oc_surface_send_to_back(oc_surface handle)
{
    oc_surface_base* surface = oc_surface_from_handle(handle);
    if(surface && surface->sendToBack)
    {
        surface->sendToBack(surface);
    }
}

void oc_surface_set_hidden(oc_surface handle, bool hidden)
{
    OC_DEBUG_ASSERT(oc_graphicsData.init);
    oc_surface_base* surface = oc_surface_from_handle(handle);
    if(surface && surface->setHidden)
    {
        surface->setHidden(surface, hidden);
    }
}

bool oc_surface_get_hidden(oc_surface handle)
{
    OC_DEBUG_ASSERT(oc_graphicsData.init);
    bool res = false;
    oc_surface_base* surface = oc_surface_from_handle(handle);
    if(surface && surface->getHidden)
    {
        res = surface->getHidden(surface);
    }
    return (res);
}
