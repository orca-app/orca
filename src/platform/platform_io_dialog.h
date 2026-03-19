/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "io.h"
#include "app/app.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct oc_file_open_with_dialog_elt
{
    oc_list_elt listElt;
    oc_file file;
} oc_file_open_with_dialog_elt;

typedef struct oc_file_open_with_dialog_result
{
    oc_file_dialog_button button;
    oc_file file;
    oc_list selection;
} oc_file_open_with_dialog_result;

ORCA_API oc_file_open_with_dialog_result oc_file_open_with_dialog(oc_arena* arena, oc_file_access rights, oc_file_open_flags flags, oc_file_dialog_desc* desc);

#ifdef __cplusplus
}
#endif
