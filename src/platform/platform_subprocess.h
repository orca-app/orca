/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#ifndef __PLATFORM_SUBPROCESS_H
#define __PLATFORM_SUBPROCESS_H

#include "platform.h"
#include "util/strings.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

ORCA_API oc_str8 oc_run_cmd(oc_arena* arena, oc_str8 cmd);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // __PLATFORM_SUBPROCESS_H
