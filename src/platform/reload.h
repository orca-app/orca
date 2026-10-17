/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "util/strings.h"

typedef void* (*oc_reload_init_proc)();
typedef void (*oc_reload_update_proc)(void* user);
typedef void (*oc_reload_begin_proc)(void* user);
typedef void (*oc_reload_end_proc)(void* user);
typedef void (*oc_reload_deinit_proc)(void* user);

typedef enum oc_reload_names
{
    OC_RELOAD_INIT,
    OC_RELOAD_UPDATE,
    OC_RELOAD_BEGIN,
    OC_RELOAD_END,
    OC_RELOAD_DEINIT,
    OC_RELOAD_PROC_COUNT,
} oc_reload_names;

typedef struct oc_reload_info
{
    const char* symbolNames[5];

    union
    {
        struct
        {
            oc_reload_init_proc init;
            oc_reload_update_proc update;
            oc_reload_begin_proc begin;
            oc_reload_end_proc end;
            oc_reload_deinit_proc deinit;
        };

        void* procs[5];
    };

    oc_str8 path;

    void* module;
    f64 modTime;

} oc_reload_info;

int oc_reload_if_needed(oc_reload_info* info);
