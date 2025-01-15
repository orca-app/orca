/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "util/lists.h"

typedef struct oc_window_data oc_window_data;

typedef struct oc_view
{
    oc_list_elt listElt;
    oc_window_data* parent;
    IDCompositionVisual* dcompVisual;
    HWND hWnd;

} oc_view;
