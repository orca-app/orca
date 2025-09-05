/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "wrapped_types.h"

oc_thread_local bool oc_lastCatchResult = false;

void oc_set_last_catch_result(bool r)
{
    oc_lastCatchResult = r;
}

bool oc_get_last_catch_result(void)
{
    return (oc_lastCatchResult);
}
