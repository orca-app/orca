/************************************************************/ /**
*
*	@file: algebra.h
*	@author: Martin Fouilleul
*	@date: 15/08/2023
*
*****************************************************************/
#ifndef __ALGEBRA_H_
#define __ALGEBRA_H_

#include "typedefs.h"

bool oc_vec2_equal(oc_vec2 v0, oc_vec2 v1);
oc_vec2 oc_vec2_mul(f32 f, oc_vec2 v);
oc_vec2 oc_vec2_add(oc_vec2 v0, oc_vec2 v1);

oc_vec2 oc_mat2x3_mul(oc_mat2x3 m, oc_vec2 p);
oc_mat2x3 oc_mat2x3_mul_m(oc_mat2x3 lhs, oc_mat2x3 rhs);
oc_mat2x3 oc_mat2x3_inv(oc_mat2x3 x);

oc_mat2x3 oc_mat2x3_rotate(f32 radians);
oc_mat2x3 oc_mat2x3_translate(f32 x, f32 y);

//TODO: complete

#endif //__ALGEBRA_H_
