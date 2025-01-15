/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "platform/platform.h"
#include "typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

ORCA_API bool oc_vec2_equal(oc_vec2 v0, oc_vec2 v1);
ORCA_API oc_vec2 oc_vec2_mul(f32 f, oc_vec2 v);
ORCA_API oc_vec2 oc_vec2_add(oc_vec2 v0, oc_vec2 v1);

ORCA_API oc_vec2 oc_mat2x3_mul(oc_mat2x3 m, oc_vec2 p);
ORCA_API oc_mat2x3 oc_mat2x3_mul_m(oc_mat2x3 lhs, oc_mat2x3 rhs);
ORCA_API oc_mat2x3 oc_mat2x3_inv(oc_mat2x3 x);

ORCA_API oc_mat2x3 oc_mat2x3_rotate(f32 radians);
ORCA_API oc_mat2x3 oc_mat2x3_translate(f32 x, f32 y);

//TODO: complete

#ifdef __cplusplus
}
#endif
