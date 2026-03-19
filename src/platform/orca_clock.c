/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "platform_clock.h"
#include "util/typedefs.h"
#include "wasmbind/hostcalls.h"

f64 oc_clock_time(oc_clock_kind clock)
{
    return oc_hostcall_clock_time(clock);
}
