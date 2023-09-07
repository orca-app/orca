/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#ifndef __HASH_H_
#define __HASH_H_

#include "strings.h"
#include "typedefs.h"

#ifdef __cplusplus
extern "C"
{
#endif

    ORCA_API u64 oc_hash_aes_u64(u64 x);
    ORCA_API u64 oc_hash_aes_u64_x2(u64 x, u64 y);
    ORCA_API u64 oc_hash_aes_string(oc_str8 string);
    ORCA_API u64 oc_hash_aes_string_seed(oc_str8 string, u64 seed);

    ORCA_API u64 oc_hash_xx64_string_seed(oc_str8 string, u64 seed);
    ORCA_API u64 oc_hash_xx64_string(oc_str8 string);

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__HASH_H_
