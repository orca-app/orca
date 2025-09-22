/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include <float.h> //FLT_MAX/MIN etc...
#include <stddef.h>
#include <stdint.h>

#ifndef __cplusplus
    #include <stdbool.h>
#endif //__cplusplus

typedef uint8_t byte;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
_Static_assert(sizeof(unsigned long long) == 8, "");
typedef unsigned long long u64;
typedef intptr_t uptr;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
_Static_assert(sizeof(signed long long) == 8, "");
typedef signed long long i64;
typedef uintptr_t iptr;
typedef ptrdiff_t dptr;

typedef float f32;
typedef double f64;

typedef union
{
    struct
    {
        f32 x;
        f32 y;
    };

    f32 c[2];
} oc_vec2;

typedef union
{
    struct
    {
        f32 x;
        f32 y;
        f32 z;
    };

    f32 c[3];
} oc_vec3;

typedef union
{
    struct
    {
        i32 x;
        i32 y;
    };

    i32 c[2];
} oc_vec2i;

typedef union
{
    struct
    {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };

    f32 c[4];
} oc_vec4;

typedef struct oc_mat2x3
{
    f32 m[6];
} oc_mat2x3;

typedef union
{
    struct
    {
        f32 x;
        f32 y;
        f32 w;
        f32 h;
    };

    struct
    {
        oc_vec2 xy;
        oc_vec2 wh;
    };

    f32 c[4];
} oc_rect;
