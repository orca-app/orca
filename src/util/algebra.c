/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <math.h>
#include "algebra.h"

bool oc_vec2_equal(oc_vec2 v0, oc_vec2 v1)
{
    return (v0.x == v1.x && v0.y == v1.y);
}

oc_vec2 oc_vec2_mul(f32 f, oc_vec2 v)
{
    return ((oc_vec2){ f * v.x, f * v.y });
}

oc_vec2 oc_vec2_add(oc_vec2 v0, oc_vec2 v1)
{
    return ((oc_vec2){ v0.x + v1.x, v0.y + v1.y });
}

oc_mat2x3 oc_mat2x3_mul_m(oc_mat2x3 lhs, oc_mat2x3 rhs)
{
    oc_mat2x3 res;
    res.m[0] = lhs.m[0] * rhs.m[0] + lhs.m[1] * rhs.m[3];
    res.m[1] = lhs.m[0] * rhs.m[1] + lhs.m[1] * rhs.m[4];
    res.m[2] = lhs.m[0] * rhs.m[2] + lhs.m[1] * rhs.m[5] + lhs.m[2];
    res.m[3] = lhs.m[3] * rhs.m[0] + lhs.m[4] * rhs.m[3];
    res.m[4] = lhs.m[3] * rhs.m[1] + lhs.m[4] * rhs.m[4];
    res.m[5] = lhs.m[3] * rhs.m[2] + lhs.m[4] * rhs.m[5] + lhs.m[5];

    return (res);
}

oc_mat2x3 oc_mat2x3_inv(oc_mat2x3 x)
{
    oc_mat2x3 res;
    res.m[0] = x.m[4] / (x.m[0] * x.m[4] - x.m[1] * x.m[3]);
    res.m[1] = x.m[1] / (x.m[1] * x.m[3] - x.m[0] * x.m[4]);
    res.m[3] = x.m[3] / (x.m[1] * x.m[3] - x.m[0] * x.m[4]);
    res.m[4] = x.m[0] / (x.m[0] * x.m[4] - x.m[1] * x.m[3]);
    res.m[2] = -(x.m[2] * res.m[0] + x.m[5] * res.m[1]);
    res.m[5] = -(x.m[2] * res.m[3] + x.m[5] * res.m[4]);
    return (res);
}

oc_vec2 oc_mat2x3_mul(oc_mat2x3 m, oc_vec2 p)
{
    f32 x = p.x * m.m[0] + p.y * m.m[1] + m.m[2];
    f32 y = p.x * m.m[3] + p.y * m.m[4] + m.m[5];
    return ((oc_vec2){ x, y });
}

oc_mat2x3 oc_mat2x3_rotate(f32 radians)
{
    const f32 sinRot = sinf(radians);
    const f32 cosRot = cosf(radians);
    oc_mat2x3 rot = {
        cosRot, -sinRot, 0,
        sinRot, cosRot, 0
    };
    return rot;
}

oc_mat2x3 oc_mat2x3_translate(f32 x, f32 y)
{
    oc_mat2x3 translate = {
        1, 0, x,
        0, 1, y
    };
    return translate;
}
