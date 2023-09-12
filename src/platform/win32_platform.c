/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

oc_host_platform oc_get_host_platform(void)
{
    return OC_HOST_PLATFORM_WINDOWS;
}

#ifdef __cplusplus
} // extern "C"
#endif