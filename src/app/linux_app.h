/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#ifndef __LINUX_APP_H_
#define __LINUX_APP_H_

#include "app.h"

typedef enum x11_success
{
    X11_FAILED = 0,
    X11_SUCCESS = 1,
    X11_AUTHENTICATE = 2,
} x11_success;

typedef enum x11_op
{
    X11_OP_NONE = 0,
    X11_OP_CREATE_WINDOW = 1,
    X11_OP_MAP_WINDOW = 8,
    X11_OP_SET_INPUT_FOCUS = 42,
    X11_OP_GET_INPUT_FOCUS = 43,
} x11_op;

typedef enum x11_backing_stores
{
    X11_BACKING_STORES_NEVER = 0,
    X11_BACKING_STORES_WHEN_MAPPED = 1,
    X11_BACKING_STORES_ALWAYS = 2,
} x11_backing_stores;
#define X11_BACKING_STORES_NOT_USEFUL  X11_BACKING_STORES_NEVER

typedef enum x11_visual_class
{
    X11_VISUAL_CLASS_STATIC_GRAY = 0,
    X11_VISUAL_CLASS_GRAY_SCALE = 1,
    X11_VISUAL_CLASS_STATIC_COLOR = 2,
    X11_VISUAL_CLASS_PSEUDO_COLOR = 3,
    X11_VISUAL_CLASS_TRUE_COLOR = 4,
    X11_VISUAL_CLASS_DIRECT_COLOR = 5,
} x11_visual_class;

typedef enum x11_image_byte_order
{
    X11_IMAGE_BYTE_ORDER_LSB_FIRST = 0,
    X11_IMAGE_BYTE_ORDER_MSB_FIRST = 1,
} x11_image_byte_order;

typedef enum x11_bitmap_bit_order
{
    X11_BITMAP_BIT_ORDER_LEAST_SIGNIFICANT = 0,
    X11_BITMAP_BIT_ORDER_MOST_SIGNIFICANT = 1,
} x11_bitmap_bit_order;

typedef enum x11_window_class
{
    X11_WINDOW_CLASS_COPY_FROM_PARENT = 0,
    X11_WINDOW_CLASS_INPUT_OUTPUT = 1,
    X11_WINDOW_CLASS_INPUT_ONLY = 2,
} x11_window_class;

typedef enum x11_window_attr
{
    X11_WINDOW_ATTR_BACKGROUND_PIXMAP = 1 << 0,
    X11_WINDOW_ATTR_BACKGROUND_PIXEL = 1 << 1,
    X11_WINDOW_ATTR_BORDER_PIXMAP = 1 << 2,
    X11_WINDOW_ATTR_BORDER_PIXEL = 1 << 3,
    X11_WINDOW_ATTR_BIT_GRAVITY = 1 << 4,
    X11_WINDOW_ATTR_WIN_GRAVITY = 1 << 5,
    X11_WINDOW_ATTR_BACKING_STORE = 1 << 6,
    X11_WINDOW_ATTR_BACKING_PLANES = 1 << 7,
    X11_WINDOW_ATTR_BACKING_PIXEL = 1 << 8,
    X11_WINDOW_ATTR_OVERRIDE_REDIRECT = 1 << 9,
    X11_WINDOW_ATTR_SAVE_UNDER = 1 << 10,
    X11_WINDOW_ATTR_EVENT_MASK = 1 << 11,
    X11_WINDOW_ATTR_DO_NOT_PROPAGATE_MASK = 1 << 12,
    X11_WINDOW_ATTR_COLOMAP = 1 << 13,
    X11_WINDOW_ATTR_CURSOR = 1 << 14,
} x11_window_attr;

typedef enum x11_bit_gravity
{
    X11_BIT_GRAVITY_FORGET = 0,
    X11_BIT_GRAVITY_NORTH_WEST = 1,
    X11_BIT_GRAVITY_NORTH = 2,
    X11_BIT_GRAVITY_NORTH_EAST = 3,
    X11_BIT_GRAVITY_WEST = 4,
    X11_BIT_GRAVITY_CENTER = 5,
    X11_BIT_GRAVITY_EAST = 6,
    X11_BIT_GRAVITY_SOUTH_WEST = 7,
    X11_BIT_GRAVITY_SOUTH = 8,
    X11_BIT_GRAVITY_SOUTH_EAST = 9,
    X11_BIT_GRAVITY_STATIC = 10,
} x11_bit_gravity;

typedef enum x11_win_gravity
{
    X11_WIN_GRAVITY_UNMAP = 0,
    X11_WIN_GRAVITY_NORTH_WEST = 1,
    X11_WIN_GRAVITY_NORTH = 2,
    X11_WIN_GRAVITY_NORTH_EAST = 3,
    X11_WIN_GRAVITY_WEST = 4,
    X11_WIN_GRAVITY_CENTER = 5,
    X11_WIN_GRAVITY_EAST = 6,
    X11_WIN_GRAVITY_SOUTH_WEST = 7,
    X11_WIN_GRAVITY_SOUTH = 8,
    X11_WIN_GRAVITY_SOUTH_EAST = 9,
    X11_WIN_GRAVITY_STATIC = 10,
} x11_win_gravity;

typedef enum x11_focus_revert_to
{
    X11_FOCUS_REVERT_TO_NONE = 0,
    X11_FOCUS_REVERT_TO_POINTER_ROOT = 1,
    X11_FOCUS_REVERT_TO_PARENT = 2,
} x11_focus_revert_to;

#define X11_WINDOW_VISUAL_COPY_FROM_PARENT  0
#define X11_WINDOW_ATTR_BACKGROUND_PIXMAP_NONE  0
#define X11_WINDOW_ATTR_BACKGROUND_PIXMAP_PARENT_RELATIVE  1
#define X11_WINDOW_ATTR_BORDER_PIXMAP_COPY_FROM_PARENT  0
#define X11_WINDOW_ATTR_COLORMAP_COPY_FROM_PARENT  0
#define X11_WINDOW_ATTR_CURSOR_NONE  0
#define X11_FOCUS_WINDOW_NONE  0
#define X11_TIMESTAMP_CURRENT_TIME  0

typedef struct x11_visual
{
    u32 id;
    x11_visual_class class;
    u32 bpp;
    u32 colormap_entries;
    u32 red_mask;
    u32 green_mask;
    u32 blue_mask;
} x11_visual;

typedef struct x11_depth
{
    u32 depth;
    u64 visuals_len;
    x11_visual *visuals;
} x11_depth;

typedef struct x11_screen
{
    u32 root;
    u32 def_colormap;
    u32 white_pixel;
    u32 black_pixel;
    u32 current_input_masks;
    u32 width;
    u32 height;
    u32 width_mm;
    u32 height_mm;
    u32 min_installed_maps;
    u32 max_installed_maps;
    u32 root_visual;
    x11_backing_stores backing_stores;
    bool save_unders;
    u32 root_depth;
    u64 allowed_depths_len;
    x11_depth *allowed_depths;
} x11_screen;

typedef struct x11_format
{
    u32 depth;
    u32 bpp;
    u32 scanline_pad_bits;
} x11_format;

typedef struct x11_setup
{
    u32 major;
    u32 minor;
    u32 release;
    u32 id_base;
    u32 id_mask;
    u32 motion_buffer_size;
    u64 req_cap;
    x11_image_byte_order image_byte_order;
    x11_bitmap_bit_order bitmap_bit_order;
    u32 bitmap_scanline_unit;
    u32 bitmap_scanline_pad;
    u32 min_keycode;
    u32 max_keycode;
    oc_str8 vendor;
    u64 formats_len;
    x11_format *formats;
    u64 roots_len;
    x11_screen *roots;
} x11_setup;

typedef struct oc_linux_x11
{
    int sock;
    oc_arena arena;
    oc_arena req_arena;
    oc_arena res_arena;
    oc_list req_list;
    oc_list res_list;
    u64 committed;
    u64 sent;
    u64 received;
    x11_setup setup;
    u32 gen_id;
} oc_linux_x11;

typedef struct oc_linux_app_data
{
    oc_linux_x11 x11;
} oc_linux_app_data;

#define OC_PLATFORM_WINDOW_DATA int linux;
#define OC_PLATFORM_APP_DATA oc_linux_app_data linux;

typedef struct oc_layer
{
    int unimplemented;
} oc_layer;

#endif // __LINUX_APP_H_

