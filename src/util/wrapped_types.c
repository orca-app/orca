/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "wrapped_types.h"

oc_thread_local bool oc_optLastResult = false;

void oc_option_set_last_result(bool b)
{
    oc_optLastResult = b;
}

bool oc_option_get_last_result(void)
{
    return (oc_optLastResult);
}

oc_thread_local i32 oc_lastError = 0;

void oc_set_last_error(i32 e)
{
    oc_lastError = e;
}

i32 oc_last_error(void)
{
    return oc_lastError;
}
