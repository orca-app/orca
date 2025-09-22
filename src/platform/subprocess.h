/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "util/typedefs.h"
#include "util/memory.h"
#include "util/strings.h"
#include "util/wrapped_types.h"

typedef enum oc_subprocess_error
{
    OC_SUBPROCESS_OK,
    OC_SUBPROCESS_NOT_EXECUTABLE,
    OC_SUBPROCESS_NOT_FOUND,
    OC_SUBPROCESS_NO_CHILD,
    OC_SUBPROCESS_INTERRUPTED,
    OC_SUBPROCESS_PIPE,
    OC_SUBPROCESS_UNKNOWN,

} oc_subprocess_error;

typedef struct oc_subprocess_run_options
{
    oc_arena* captureArena;
    //...
} oc_subprocess_run_options;

typedef struct oc_subprocess_spawn_options
{
    //...
} oc_subprocess_spawn_options;

typedef struct oc_subprocess_completion
{
    int returnCode;
    int signal;
    oc_str8 capturedStdout;
    oc_str8 capturedStderr;

} oc_subprocess_completion;

typedef oc_result(oc_subprocess_completion, oc_subprocess_error) oc_subprocess_result;

typedef struct oc_subprocess_info oc_subprocess_info;
typedef oc_subprocess_info* oc_subprocess;
typedef oc_result(oc_subprocess, oc_subprocess_error) oc_subprocess_spawn_result;

oc_subprocess_result oc_subprocess_run(int argc, char** argv, oc_subprocess_run_options* options);

oc_subprocess_spawn_result oc_subprocess_spawn(int argc, char** argv, oc_subprocess_spawn_options* options);
oc_subprocess_result oc_subprocess_wait(oc_subprocess subprocess);
oc_subprocess_result oc_subprocess_read_and_wait(oc_arena* arena, oc_subprocess subprocess);
oc_subprocess_result oc_subprocess_kill(oc_subprocess subprocess);
