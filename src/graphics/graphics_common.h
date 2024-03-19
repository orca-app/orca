/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#ifndef __GRAPHICS_COMMON_H_
#define __GRAPHICS_COMMON_H_

#include "graphics.h"

//------------------------------------------------------------------------
// canvas structs
//------------------------------------------------------------------------
typedef enum
{
    OC_PATH_MOVE,
    OC_PATH_LINE,
    OC_PATH_QUADRATIC,
    OC_PATH_CUBIC
} oc_path_elt_type;

typedef struct oc_path_elt
{
    oc_path_elt_type type;
    oc_vec2 p[3];

} oc_path_elt;

typedef struct oc_path_descriptor
{
    u32 startIndex;
    u32 count;
    oc_vec2 startPoint;

} oc_path_descriptor;

typedef struct oc_attributes
{
    f32 width;
    f32 tolerance;
    bool hasGradient;
    oc_color colors[4];
    oc_joint_type joint;
    f32 maxJointExcursion;
    oc_cap_type cap;

    oc_font font;
    f32 fontSize;

    oc_image image;
    oc_rect srcRegion;

    oc_mat2x3 transform;
    oc_rect clip;

} oc_attributes;

typedef enum
{
    OC_CMD_FILL = 0,
    OC_CMD_STROKE,
    OC_CMD_JUMP
} oc_primitive_cmd;

typedef struct oc_primitive
{
    oc_primitive_cmd cmd;
    oc_attributes attributes;

    union
    {
        oc_path_descriptor path;
        oc_rect rect;
        u32 jump;
    };

} oc_primitive;

ORCA_API void oc_canvas_renderer_submit(oc_canvas_renderer renderer,
                                        oc_surface surface,
                                        u32 msaaSampleCount,
                                        oc_color clearColor,
                                        u32 primitiveCount,
                                        oc_primitive* primitives,
                                        u32 eltCount,
                                        oc_path_elt* elements);

#endif //__GRAPHICS_COMMON_H_
