/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "strings.h"
#include "typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

ORCA_API u64 oc_hash_xx64_string_seed(oc_str8 string, u64 seed);
ORCA_API u64 oc_hash_xx64_string(oc_str8 string);

#ifdef __cplusplus
} // extern "C"
#endif
