/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "util/debug.h"

_Noreturn void abort(void)
{
    OC_ABORT();
}
