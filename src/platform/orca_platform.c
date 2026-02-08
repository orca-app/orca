/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "platform.h"
#include "wasmbind/hostcalls.h"

oc_host_platform oc_get_host_platform()
{
    return oc_hostcall_get_host_platform();
}
