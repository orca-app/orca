/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#ifndef __OSX_VIEW_H_
#define __OSX_VIEW_H_

#ifdef __OBJC__
    #import <Cocoa/Cocoa.h>
#else
    #define CALayer void
#endif

#include "util/lists.h"

typedef struct oc_window_data oc_window_data;

typedef struct oc_view
{
    oc_list_elt listElt;
    oc_window_data* window;
    CALayer* layer;
} oc_view;

#endif //__OSX_VIEW_H_
