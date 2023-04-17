//*****************************************************************
//
//	$file: typedefs.h $
//	$author: Martin Fouilleul $
//	$date: 23/36/2015 $
//	$revision: $
//	$note: (C) 2015 by Martin Fouilleul - all rights reserved $
//
//*****************************************************************
#ifndef __TYPEDEFS_H_
#define __TYPEDEFS_H_

#include<stddef.h>
#include<stdint.h>
#include<float.h>	//FLT_MAX/MIN etc...

#ifndef __cplusplus
#include<stdbool.h>
#endif //__cplusplus

typedef uint8_t	 byte;
typedef uint8_t	 u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t	i8;
typedef int16_t	i16;
typedef int32_t	i32;
typedef int64_t	i64;

typedef float	f32;
typedef double	f64;

typedef union
{
	struct
	{
		f32 x;
		f32 y;
	};
	f32 c[2];
} vec2;

typedef union
{
	struct
	{
		i32 x;
		i32 y;
	};
	i32 c[2];
} ivec2;

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
} vec4;

#define vec4_expand_xyz(v) (v).x, (v).y, (v).z

typedef union
{
	struct
	{
		f32 x;
		f32 y;
		f32 w;
		f32 h;
	};
	f32 c[4];
} mp_rect;

#endif //__TYPEDEFS_H_
