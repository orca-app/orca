/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "app/app_internal.h"

void oc_osx_update_surfaces(oc_window_data* window)
{
    @autoreleasepool
    {
        int z = 0;
        oc_list_for(window->osx.surfaces, surface, oc_surface_base, view.listElt)
        {
            surface->view.layer.zPosition = (CGFloat)z;
            z++;
        }
    }
}

oc_vec2 oc_surface_base_contents_scaling(oc_surface_base* surface)
{
    @autoreleasepool
    {
        f32 contentsScale = [surface->view.layer contentsScale];
        oc_vec2 res = { contentsScale, contentsScale };
        return (res);
    }
}

oc_vec2 oc_surface_base_get_size(oc_surface_base* surface)
{
    @autoreleasepool
    {
        CGRect bounds = surface->view.layer.bounds;
        oc_vec2 res = { bounds.size.width, bounds.size.height };
        return (res);
    }
}

void oc_surface_base_bring_to_front(oc_surface_base* surface)
{
    dispatch_block_t block = ^{
      @autoreleasepool
      {
          oc_window_data* window = surface->view.window;
          if(window)
          {
              oc_list_remove(&window->osx.surfaces, &surface->view.listElt);
              oc_list_push_back(&window->osx.surfaces, &surface->view.listElt);
              oc_osx_update_surfaces(window);
          }
      }
    };

    if([NSThread isMainThread])
    {
        block();
    }
    else
    {
        dispatch_sync(dispatch_get_main_queue(), block);
    }
}

void oc_surface_base_send_to_back(oc_surface_base* surface)
{
    dispatch_block_t block = ^{
      @autoreleasepool
      {
          oc_window_data* window = surface->view.window;
          if(window)
          {
              oc_list_remove(&window->osx.surfaces, &surface->view.listElt);
              oc_list_push_front(&window->osx.surfaces, &surface->view.listElt);
              oc_osx_update_surfaces(window);
          }
      }
    };

    if([NSThread isMainThread])
    {
        block();
    }
    else
    {
        dispatch_sync(dispatch_get_main_queue(), block);
    }
}

bool oc_surface_base_get_hidden(oc_surface_base* surface)
{
    @autoreleasepool
    {
        return ([surface->view.layer isHidden]);
    }
}

void oc_surface_base_set_hidden(oc_surface_base* surface, bool hidden)
{
    @autoreleasepool
    {
        [CATransaction begin];
        [CATransaction setDisableActions:YES];
        [surface->view.layer setHidden:hidden];
        [CATransaction commit];
    }
}

void oc_surface_base_cleanup(oc_surface_base* surface)
{
    @autoreleasepool
    {
        oc_window_data* window = surface->view.window;
        if(window)
        {
            oc_list_remove(&window->osx.surfaces, &surface->view.listElt);
            [surface->view.layer removeFromSuperlayer];
            oc_osx_update_surfaces(window);
        }
        [surface->view.layer release];
    }
}

void oc_surface_base_init_for_window(oc_surface_base* surface, oc_window_data* window)
{
    @autoreleasepool
    {
        surface->view.window = window;

        surface->contentsScaling = oc_surface_base_contents_scaling;
        surface->getSize = oc_surface_base_get_size;
        surface->bringToFront = oc_surface_base_bring_to_front;
        surface->sendToBack = oc_surface_base_send_to_back;
        surface->getHidden = oc_surface_base_get_hidden;
        surface->setHidden = oc_surface_base_set_hidden;

        surface->view.layer = [[CALayer alloc] init];
        [surface->view.layer retain];

        NSRect frame = [[window->osx.nsWindow contentView] frame];
        CGSize size = frame.size;
        surface->view.layer.frame = (CGRect){ { 0, 0 }, size };
        surface->view.layer.contentsScale = window->osx.nsView.layer.contentsScale;
        surface->view.layer.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;

        [window->osx.nsView.layer addSublayer:surface->view.layer];

        oc_list_push_back(&window->osx.surfaces, &surface->view.listElt);
        oc_osx_update_surfaces(window);
    }
}
