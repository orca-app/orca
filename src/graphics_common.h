/************************************************************//**
*
*	@file: graphics_common.h
*	@author: Martin Fouilleul
*	@date: 26/04/2023
*
*****************************************************************/
#ifndef __GRAPHICS_COMMON_H_
#define __GRAPHICS_COMMON_H_

#include"graphics.h"

//------------------------------------------------------------------------
// canvas structs
//------------------------------------------------------------------------
typedef enum { MG_PATH_MOVE,
               MG_PATH_LINE,
	           MG_PATH_QUADRATIC,
	           MG_PATH_CUBIC } mg_path_elt_type;

typedef struct mg_path_elt
{
	mg_path_elt_type type;
	vec2 p[3];

} mg_path_elt;

typedef struct mg_path_descriptor
{
	u32 startIndex;
	u32 count;
	vec2 startPoint;

} mg_path_descriptor;

typedef struct mg_attributes
{
	f32 width;
	f32 tolerance;
	mg_color color;
	mg_joint_type joint;
	f32 maxJointExcursion;
	mg_cap_type cap;

	mg_font font;
	f32 fontSize;

	mg_image image;
	mp_rect srcRegion;

	mg_mat2x3 transform;
	mp_rect clip;

} mg_attributes;

typedef enum { MG_CMD_FILL,
	           MG_CMD_STROKE,
	           MG_CMD_JUMP
	     } mg_primitive_cmd;

typedef struct mg_primitive
{
	mg_primitive_cmd cmd;
	mg_attributes attributes;

	union
	{
		mg_path_descriptor path;
		mp_rect rect;
		u32 jump;
	};

} mg_primitive;

MP_API void mg_surface_render_commands(mg_surface surface,
                                       mg_color clearColor,
                                       u32 primitiveCount,
                                       mg_primitive* primitives,
                                       u32 eltCount,
                                       mg_path_elt* elements);

#endif //__GRAPHICS_COMMON_H_
