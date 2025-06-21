/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "platform.h"
#include "util/typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum
{
    OC_CLOCK_MONOTONIC, // clock that increment monotonically
    OC_CLOCK_UPTIME,    // clock that increment monotonically during uptime
    OC_CLOCK_DATE       // clock that is driven by the platform time
} oc_clock_kind;

#if !OC_PLATFORM_ORCA
ORCA_API void oc_clock_init(void); // initialize the clock subsystem
#endif

ORCA_API f64 oc_clock_time(oc_clock_kind clock);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
