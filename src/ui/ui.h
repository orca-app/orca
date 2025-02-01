/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "graphics/graphics.h"
#include "input_state.h"
#include "util/lists.h"
#include "util/typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct oc_ui_key
{
    u64 hash;
} oc_ui_key;

typedef enum
{
    OC_UI_AXIS_X,
    OC_UI_AXIS_Y,
    OC_UI_AXIS_COUNT
} oc_ui_axis;

typedef enum
{
    OC_UI_ALIGN_START,
    OC_UI_ALIGN_END,
    OC_UI_ALIGN_CENTER,
} oc_ui_align;

typedef union oc_ui_layout_align
{
    struct
    {
        oc_ui_align x;
        oc_ui_align y;
    };

    oc_ui_align c[OC_UI_AXIS_COUNT];
} oc_ui_layout_align;

typedef enum oc_ui_overflow
{
    OC_UI_OVERFLOW_CLIP,
    OC_UI_OVERFLOW_ALLOW,
    OC_UI_OVERFLOW_SCROLL,
} oc_ui_overflow;

typedef struct oc_ui_layout
{
    oc_ui_axis axis;
    f32 spacing;

    union
    {
        struct
        {
            f32 x;
            f32 y;
        };

        f32 c[OC_UI_AXIS_COUNT];
    } margin;

    oc_ui_layout_align align;

    union
    {
        struct
        {
            oc_ui_overflow x;
            oc_ui_overflow y;
        };

        oc_ui_overflow c[OC_UI_AXIS_COUNT];
    } overflow;

    union
    {
        struct
        {
            bool x;
            bool y;
        };

        bool c[OC_UI_AXIS_COUNT];
    } constrain;

} oc_ui_layout;

typedef enum oc_ui_size_kind
{
    OC_UI_SIZE_TEXT = 0,
    OC_UI_SIZE_PIXELS,
    OC_UI_SIZE_CHILDREN,
    OC_UI_SIZE_PARENT,
    OC_UI_SIZE_PARENT_MINUS_PIXELS,

} oc_ui_size_kind;

typedef struct oc_ui_size
{
    oc_ui_size_kind kind;
    f32 value;
    f32 relax;
    f32 minSize;
} oc_ui_size;

typedef union oc_ui_box_size
{
    struct
    {
        oc_ui_size width;
        oc_ui_size height;
    };

    oc_ui_size c[OC_UI_AXIS_COUNT];
} oc_ui_box_size;

typedef union oc_ui_box_floating
{
    struct
    {
        bool x;
        bool y;
    };

    bool c[OC_UI_AXIS_COUNT];
} oc_ui_box_floating;

typedef enum oc_ui_attr
{
    OC_UI_WIDTH,  // WIDTH?
    OC_UI_HEIGHT, // HEIGHT?
    OC_UI_AXIS,
    OC_UI_MARGIN_X,
    OC_UI_MARGIN_Y,
    OC_UI_SPACING,
    OC_UI_ALIGN_X,
    OC_UI_ALIGN_Y,
    OC_UI_FLOATING_X,
    OC_UI_FLOATING_Y,
    OC_UI_FLOAT_TARGET_X,
    OC_UI_FLOAT_TARGET_Y,
    OC_UI_OVERFLOW_X,
    OC_UI_OVERFLOW_Y,
    OC_UI_CONSTRAIN_X,
    OC_UI_CONSTRAIN_Y,
    OC_UI_COLOR,
    OC_UI_BG_COLOR,
    OC_UI_BORDER_COLOR,
    OC_UI_FONT,
    OC_UI_TEXT_SIZE,
    OC_UI_BORDER_SIZE,
    OC_UI_ROUNDNESS,
    OC_UI_ANIMATION_TIME,
    OC_UI_ANIMATION_MASK,
    OC_UI_CLICK_THROUGH,

    OC_UI_ATTR_COUNT,
} oc_ui_attr;

//NOTE: flags for axis-dependent properties (e.g. OC_UI_STYLE_FLOAT_X/Y) need to be consecutive bits
//      in order to play well with axis agnostic functions

typedef enum oc_ui_attr_mask
{
    OC_UI_MASK_NONE = 0,
    OC_UI_MASK_SIZE_WIDTH = 1 << OC_UI_WIDTH,
    OC_UI_MASK_SIZE_HEIGHT = 1 << OC_UI_HEIGHT,
    OC_UI_MASK_LAYOUT_AXIS = 1 << OC_UI_AXIS,
    OC_UI_MASK_LAYOUT_ALIGN_X = 1 << OC_UI_ALIGN_X,
    OC_UI_MASK_LAYOUT_ALIGN_Y = 1 << OC_UI_ALIGN_Y,
    OC_UI_MASK_LAYOUT_SPACING = 1 << OC_UI_SPACING,
    OC_UI_MASK_LAYOUT_MARGIN_X = 1 << OC_UI_MARGIN_X,
    OC_UI_MASK_LAYOUT_MARGIN_Y = 1 << OC_UI_MARGIN_Y,
    OC_UI_MASK_FLOATING_X = 1 << OC_UI_FLOATING_X,
    OC_UI_MASK_FLOATING_Y = 1 << OC_UI_FLOATING_Y,
    OC_UI_MASK_FLOAT_TARGET_X = 1 << OC_UI_FLOAT_TARGET_X,
    OC_UI_MASK_FLOAT_TARGET_Y = 1 << OC_UI_FLOAT_TARGET_Y,
    OC_UI_MASK_OVERFLOW_X = 1 << OC_UI_OVERFLOW_X,
    OC_UI_MASK_OVERFLOW_Y = 1 << OC_UI_OVERFLOW_Y,
    OC_UI_MASK_CONSTRAIN_X = 1 << OC_UI_CONSTRAIN_X,
    OC_UI_MASK_CONSTRAIN_Y = 1 << OC_UI_CONSTRAIN_Y,
    OC_UI_MASK_COLOR = 1 << OC_UI_COLOR,
    OC_UI_MASK_BG_COLOR = 1 << OC_UI_BG_COLOR,
    OC_UI_MASK_BORDER_COLOR = 1 << OC_UI_BORDER_COLOR,
    OC_UI_MASK_BORDER_SIZE = 1 << OC_UI_BORDER_SIZE,
    OC_UI_MASK_ROUNDNESS = 1 << OC_UI_ROUNDNESS,
    OC_UI_MASK_FONT = 1 << OC_UI_FONT,
    OC_UI_MASK_FONT_SIZE = 1 << OC_UI_TEXT_SIZE,
    OC_UI_MASK_ANIMATION_TIME = 1 << OC_UI_ANIMATION_TIME,
    OC_UI_MASK_ANIMATION_MASK = 1 << OC_UI_ANIMATION_MASK,
    OC_UI_MASK_CLICK_THROUGH = 1 << OC_UI_CLICK_THROUGH,

    //masks
    OC_UI_MASK_SIZE = OC_UI_MASK_SIZE_WIDTH
                    | OC_UI_MASK_SIZE_HEIGHT,

    OC_UI_MASK_LAYOUT_MARGINS = OC_UI_MASK_LAYOUT_MARGIN_X
                              | OC_UI_MASK_LAYOUT_MARGIN_Y,

    OC_UI_MASK_LAYOUT = OC_UI_MASK_LAYOUT_AXIS
                      | OC_UI_MASK_LAYOUT_ALIGN_X
                      | OC_UI_MASK_LAYOUT_ALIGN_Y
                      | OC_UI_MASK_LAYOUT_SPACING
                      | OC_UI_MASK_LAYOUT_MARGIN_X
                      | OC_UI_MASK_LAYOUT_MARGIN_Y,

    OC_UI_MASK_FLOAT = OC_UI_MASK_FLOATING_X
                     | OC_UI_MASK_FLOATING_Y
                     | OC_UI_MASK_FLOAT_TARGET_X
                     | OC_UI_MASK_FLOAT_TARGET_Y,

    OC_UI_MASK_MASK_INHERITED = OC_UI_MASK_COLOR
                              | OC_UI_MASK_FONT
                              | OC_UI_MASK_FONT_SIZE
                              | OC_UI_MASK_ANIMATION_TIME
                              | OC_UI_MASK_ANIMATION_MASK,
} oc_ui_attr_mask;

typedef struct oc_ui_style
{
    oc_ui_box_size size;
    oc_ui_layout layout;
    oc_ui_box_floating floating;
    oc_vec2 floatTarget;
    oc_color color;
    oc_color bgColor;
    oc_color borderColor;
    oc_font font;
    f32 fontSize;
    f32 borderSize;
    f32 roundness;
    f32 animationTime;
    oc_ui_attr_mask animationMask;
    bool clickThrough;
} oc_ui_style;

typedef struct oc_ui_tag
{
    u64 hash;
} oc_ui_tag;

typedef enum
{
    OC_UI_SEL_ID = 0,
    OC_UI_SEL_TAG,

} oc_ui_selector_kind;

typedef enum
{
    OC_UI_SEL_DESCENDANT = 0,
    OC_UI_SEL_AND = 1,
    //...
} oc_ui_selector_op;

typedef struct oc_ui_selector
{
    oc_list_elt listElt;
    oc_ui_selector_kind kind;
    oc_ui_selector_op op;

    oc_str8 string;
    u64 hash;

} oc_ui_selector;

typedef union oc_ui_pattern_specificity
{
    struct
    {
        u32 id;
        u32 tag;
    };

    u32 s[2];
} oc_ui_pattern_specificity;

typedef struct oc_ui_pattern
{
    oc_list l;
    oc_ui_pattern_specificity specificity;
} oc_ui_pattern;

typedef struct oc_ui_box oc_ui_box;

typedef struct oc_ui_style_rule
{
    oc_list_elt boxElt;
    oc_list_elt rulesetElt;

    oc_ui_box* owner;
    oc_ui_pattern pattern;
    oc_ui_attr_mask mask;
    oc_ui_style style;
} oc_ui_style_rule;

typedef struct oc_ui_sig
{
    oc_ui_box* box;

    oc_vec2 mouse;
    oc_vec2 delta;
    oc_vec2 wheel;

    bool pressed;
    bool released;
    bool clicked;
    bool doubleClicked;
    bool tripleClicked;
    bool rightPressed;

    bool dragging;
    bool hovering;

    bool pasted;

} oc_ui_sig;

typedef void (*oc_ui_box_draw_proc)(oc_ui_box* box, void* data);

struct oc_ui_box
{
    // hierarchy
    oc_list_elt listElt;
    oc_list children;
    oc_ui_box* parent;

    oc_list_elt overlayElt;
    bool overlay;

    // keying and caching
    oc_list_elt bucketElt;
    oc_ui_key key;
    u64 frameCounter;

    // builder-provided info
    oc_str8 keyString;
    oc_str8 text;
    oc_list tags;

    oc_ui_box_draw_proc drawProc;
    void* drawData;

    // styling
    oc_list rules;

    oc_ui_style* targetStyle;
    oc_ui_style style;
    u32 z;

    oc_vec2 floatPos;
    f32 childrenSum[2];
    f32 spacing[2];
    f32 minSize[2];
    oc_rect rect;

    oc_list styleVariables;

    // signals
    oc_ui_sig sig;

    // stateful behaviour
    bool fresh;
    bool closed;
    bool parentClosed;
    bool dragging;
    bool hot;
    bool active;
    oc_vec2 scroll;
    oc_vec2 pressedMouse;

    // animation data
    f32 hotTransition;
    f32 activeTransition;
};

//-----------------------------------------------------------------------------
// context
//-----------------------------------------------------------------------------

enum
{
    OC_UI_MAX_INPUT_CHAR_PER_FRAME = 64
};

typedef struct oc_ui_input_text
{
    u8 count;
    oc_utf32 codePoints[OC_UI_MAX_INPUT_CHAR_PER_FRAME];

} oc_ui_input_text;

typedef struct oc_ui_stack_elt oc_ui_stack_elt;

struct oc_ui_stack_elt
{
    oc_ui_stack_elt* parent;

    union
    {
        oc_ui_box* box;
        oc_ui_size size;
        oc_rect clip;
    };
};

typedef struct oc_ui_tag_elt
{
    oc_list_elt listElt;
    oc_ui_tag tag;
} oc_ui_tag_elt;

enum
{
    OC_UI_BOX_MAP_BUCKET_COUNT = 1024
};

typedef enum
{
    OC_UI_EDIT_MOVE_NONE = 0,
    OC_UI_EDIT_MOVE_CHAR,
    OC_UI_EDIT_MOVE_WORD,
    OC_UI_EDIT_MOVE_LINE
} oc_ui_edit_move;

//TODO: WIP /////////////////////////////////////////////////////

typedef struct oc_ui_var_stack
{
    oc_list_elt bucketElt;
    oc_str8 name;
    u64 hash;
    oc_list vars;
} oc_ui_var_stack;

typedef enum oc_ui_var_kind
{
    OC_UI_VAR_I32,
    OC_UI_VAR_F32,
    OC_UI_VAR_SIZE,
    OC_UI_VAR_COLOR,
    OC_UI_VAR_FONT,
} oc_ui_var_kind;

typedef struct oc_ui_value
{
    oc_ui_var_kind kind;

    union
    {
        oc_ui_size size;
        oc_color color;
        oc_font font;
        f32 f;
        i32 i;
    };
} oc_ui_value;

typedef struct oc_ui_var
{
    oc_list_elt boxElt;
    oc_list_elt stackElt;
    oc_ui_var_stack* stack;
    oc_ui_value value;

} oc_ui_var;

typedef struct oc_ui_var_map
{
    u64 mask;
    oc_list* buckets;
} oc_ui_var_map;

///////////////////////////////////////////////////////

typedef struct oc_ui_context
{
    bool init;

    oc_input_state input;

    u64 frameCounter;
    f64 frameTime;
    f64 lastFrameDuration;

    oc_arena frameArena;
    oc_pool boxPool;
    oc_list boxMap[OC_UI_BOX_MAP_BUCKET_COUNT];

    oc_ui_box* root;
    oc_ui_box* overlay;
    oc_list overlayList;
    oc_ui_stack_elt* boxStack;
    oc_ui_stack_elt* clipStack;

    oc_list nextBoxTags;

    u32 z;
    oc_ui_box* hovered;

    oc_ui_box* focus;
    i32 editCursor;
    i32 editMark;
    i32 editFirstDisplayedChar;
    f64 editCursorBlinkStart;
    oc_ui_edit_move editSelectionMode;
    i32 editWordSelectionInitialCursor;
    i32 editWordSelectionInitialMark;

    //TODO: reorganize
    oc_ui_var_map styleVariables;
    oc_ui_style_rule* workingRule;
    oc_font defaultFont;

} oc_ui_context;

//-------------------------------------------------------------------------------------
// UI context initialization and frame cycle
//-------------------------------------------------------------------------------------
ORCA_API void oc_ui_init(oc_ui_context* context, oc_font defaultFont);
ORCA_API oc_ui_context* oc_ui_get_context(void);
ORCA_API void oc_ui_set_context(oc_ui_context* context);

ORCA_API void oc_ui_process_event(oc_event* event);
ORCA_API void oc_ui_begin_frame(oc_vec2 size);
ORCA_API void oc_ui_end_frame(void);
ORCA_API void oc_ui_draw(void);

#define oc_ui_frame(size) oc_defer_loop(oc_ui_begin_frame(size), oc_ui_end_frame())

//-------------------------------------------------------------------------------------
// Box keys
//-------------------------------------------------------------------------------------
ORCA_API oc_ui_key oc_ui_key_make_str8(oc_str8 string);
ORCA_API oc_ui_key oc_ui_key_make_path(oc_str8_list path);

// C-string helper
#define oc_ui_key_make(s) oc_ui_key_make_str8(OC_STR8(s))

//-------------------------------------------------------------------------------------
// Box hierarchy building
//-------------------------------------------------------------------------------------
ORCA_API oc_ui_box* oc_ui_box_make_str8(oc_str8 string);
ORCA_API oc_ui_box* oc_ui_box_begin_str8(oc_str8 string);

ORCA_API oc_ui_box* oc_ui_box_end(void);

#define oc_ui_box_str8(name)    \
    oc_ui_box_begin_str8(name); \
    oc_defer_loop(, oc_ui_box_end())

#define oc_ui_box(name) oc_ui_box_str8(OC_STR8(name))

ORCA_API void oc_ui_box_push(oc_ui_box* box);
ORCA_API void oc_ui_box_pop(void);
ORCA_API oc_ui_box* oc_ui_box_top(void);

ORCA_API oc_ui_box* oc_ui_box_lookup_key(oc_ui_key key);
ORCA_API oc_ui_box* oc_ui_box_lookup_str8(oc_str8 string);

ORCA_API void oc_ui_box_set_draw_proc(oc_ui_box* box, oc_ui_box_draw_proc proc, void* data);

// C-string helpers
#define oc_ui_box_lookup(s) oc_ui_box_lookup_str8(OC_STR8(s))
#define oc_ui_box_make(s) oc_ui_box_make_str8(OC_STR8(s))
#define oc_ui_box_begin(s) oc_ui_box_begin_str8(OC_STR8(s))

//TODO: find its place
ORCA_API void oc_ui_set_text(oc_str8 text);
ORCA_API void oc_ui_set_overlay(bool overlay);
//-------------------------------------------------------------------------------------
// Box status and signals
//-------------------------------------------------------------------------------------
ORCA_API bool oc_ui_box_closed(oc_ui_box* box);
ORCA_API void oc_ui_box_set_closed(oc_ui_box* box, bool closed);

ORCA_API bool oc_ui_box_active(oc_ui_box* box);
ORCA_API void oc_ui_box_activate(oc_ui_box* box);
ORCA_API void oc_ui_box_deactivate(oc_ui_box* box);

ORCA_API bool oc_ui_box_hot(oc_ui_box* box);
ORCA_API void oc_ui_box_set_hot(oc_ui_box* box, bool hot);

ORCA_API oc_ui_sig oc_ui_box_sig(oc_ui_box* box);

//-------------------------------------------------------------------------------------
// Tagging
//-------------------------------------------------------------------------------------
ORCA_API oc_ui_tag oc_ui_tag_make_str8(oc_str8 string);
ORCA_API void oc_ui_tag_box_str8(oc_ui_box* box, oc_str8 string);
ORCA_API void oc_ui_tag_next_str8(oc_str8 string);

ORCA_API void oc_ui_tag_str8(oc_str8 string);

#define oc_ui_tag(s) oc_ui_tag_str8(OC_STR8(s));

// C-string helpers
#define oc_ui_tag_make(s) oc_ui_tag_make_str8(OC_STR8(s))
#define oc_ui_tag_box(b, s) oc_ui_tag_box_str8(b, OC_STR8(s))
#define oc_ui_tag_next(s) oc_ui_tag_next_str8(OC_STR8(s))

//-------------------------------------------------------------------------------------
// Styling
//-------------------------------------------------------------------------------------
//NOTE: styling API

//[WIP] ///////////////////////////////////////////////////////////

ORCA_API void oc_ui_style_rule_begin(oc_str8 pattern);
ORCA_API void oc_ui_style_rule_end();

#define oc_ui_style_rule(p) oc_defer_loop(oc_ui_style_rule_begin(OC_STR8(p)), oc_ui_style_rule_end())
#define oc_ui_style_rule_str8(p) oc_defer_loop(oc_ui_style_rule_begin(p), oc_ui_style_rule_end())

ORCA_API void oc_ui_style_set_i32(oc_ui_attr attr, i32 i);
ORCA_API void oc_ui_style_set_f32(oc_ui_attr attr, f32 f);
ORCA_API void oc_ui_style_set_color(oc_ui_attr attr, oc_color color);
ORCA_API void oc_ui_style_set_font(oc_ui_attr attr, oc_font font);
ORCA_API void oc_ui_style_set_size(oc_ui_attr attr, oc_ui_size size);
ORCA_API void oc_ui_style_set_var_str8(oc_ui_attr attr, oc_str8 var);
ORCA_API void oc_ui_style_set_var(oc_ui_attr attr, const char* var);

///////////////////////////////////////////////////////////////////
// style varibles

//NOTE: declaring variable with default value
ORCA_API void oc_ui_var_default_i32_str8(oc_str8 name, i32 i);
ORCA_API void oc_ui_var_default_f32_str8(oc_str8 name, f32 f);
ORCA_API void oc_ui_var_default_size_str8(oc_str8 name, oc_ui_size size);
ORCA_API void oc_ui_var_default_color_str8(oc_str8 name, oc_color color);
ORCA_API void oc_ui_var_default_font_str8(oc_str8 name, oc_font font);
ORCA_API void oc_ui_var_default_str8(oc_str8 name, oc_str8 src);

//C-string versions
ORCA_API void oc_ui_var_default_i32(const char* name, i32 i);
ORCA_API void oc_ui_var_default_f32(const char* name, f32 f);
ORCA_API void oc_ui_var_default_size(const char* name, oc_ui_size size);
ORCA_API void oc_ui_var_default_color(const char* name, oc_color color);
ORCA_API void oc_ui_var_default_font(const char* name, oc_font font);
ORCA_API void oc_ui_var_default(const char* name, const char* src);

//NOTE: setting variable value
ORCA_API void oc_ui_var_set_i32_str8(oc_str8 name, i32 i);
ORCA_API void oc_ui_var_set_f32_str8(oc_str8 name, f32 f);
ORCA_API void oc_ui_var_set_size_str8(oc_str8 name, oc_ui_size size);
ORCA_API void oc_ui_var_set_color_str8(oc_str8 name, oc_color color);
ORCA_API void oc_ui_var_set_font_str8(oc_str8 name, oc_font font);
ORCA_API void oc_ui_var_set_str8(oc_str8 name, oc_str8 src);

//C-string versions
ORCA_API void oc_ui_var_set_i32(const char* name, i32 i);
ORCA_API void oc_ui_var_set_f32(const char* name, f32 f);
ORCA_API void oc_ui_var_set_size(const char* name, oc_ui_size size);
ORCA_API void oc_ui_var_set_color(const char* name, oc_color color);
ORCA_API void oc_ui_var_set_font(const char* name, oc_font font);
ORCA_API void oc_ui_var_set(const char* name, const char* src);

//NOTE: getting variable value
ORCA_API i32 oc_ui_var_get_i32_str8(oc_str8 name);
ORCA_API f32 oc_ui_var_get_f32_str8(oc_str8 name);
ORCA_API oc_ui_size oc_ui_var_get_size_str8(oc_str8 name);
ORCA_API oc_color oc_ui_var_get_color_str8(oc_str8 name);
ORCA_API oc_font oc_ui_var_get_font_str8(oc_str8 name);

//C-string versions
ORCA_API i32 oc_ui_var_get_i32(const char* name);
ORCA_API f32 oc_ui_var_get_f32(const char* name);
ORCA_API oc_ui_size oc_ui_var_get_size(const char* name);
ORCA_API oc_color oc_ui_var_get_color(const char* name);
ORCA_API oc_font oc_ui_var_get_font(const char* name);

//NOTE: default themes
#define OC_UI_DEFAULT_THEME_DATA(_)                               \
    _(OC_UI_THEME_PRIMARY, "primary")                             \
    _(OC_UI_THEME_PRIMARY_HOVER, "primary-hover")                 \
    _(OC_UI_THEME_PRIMARY_ACTIVE, "primary-active")               \
    _(OC_UI_THEME_PRIMARY_DISABLED, "primary-disabled")           \
    _(OC_UI_THEME_TEXT_0, "text-0")                               \
    _(OC_UI_THEME_TEXT_1, "text-1")                               \
    _(OC_UI_THEME_TEXT_2, "text-2")                               \
    _(OC_UI_THEME_TEXT_3, "text-3")                               \
    _(OC_UI_THEME_BG_0, "bg-0")                                   \
    _(OC_UI_THEME_BG_1, "bg-1")                                   \
    _(OC_UI_THEME_BG_2, "bg-2")                                   \
    _(OC_UI_THEME_BG_3, "bg-3")                                   \
    _(OC_UI_THEME_BG_4, "bg-4")                                   \
    _(OC_UI_THEME_FILL_0, "fill-0")                               \
    _(OC_UI_THEME_FILL_1, "fill-1")                               \
    _(OC_UI_THEME_FILL_2, "fill-2")                               \
    _(OC_UI_THEME_BORDER, "border")                               \
    _(OC_UI_THEME_ROUNDNESS_SMALL, "roundness-small")             \
    _(OC_UI_THEME_ROUNDNESS_REGULAR, "roundness-regular")         \
    _(OC_UI_THEME_ROUNDNESS_LARGE, "roundness-large")             \
    _(OC_UI_THEME_TEXT_SIZE_SMALL, "text-size-small")             \
    _(OC_UI_THEME_TEXT_SIZE_REGULAR, "text-size-regular")         \
    _(OC_UI_THEME_TEXT_SIZE_HEADER_0, "text-size-header-0")       \
    _(OC_UI_THEME_TEXT_SIZE_HEADER_1, "text-size-header-1")       \
    _(OC_UI_THEME_TEXT_SIZE_HEADER_2, "text-size-header-2")       \
    _(OC_UI_THEME_TEXT_SIZE_HEADER_3, "text-size-header-3")       \
    _(OC_UI_THEME_TEXT_SIZE_HEADER_4, "text-size-header-4")       \
    _(OC_UI_THEME_FONT_REGULAR, "font-regular")                   \
    _(OC_UI_THEME_SPACING_EXTRA_TIGHT, "spacing-extra-tight")     \
    _(OC_UI_THEME_SPACING_TIGHT, "spacing-tight")                 \
    _(OC_UI_THEME_SPACING_REGULAR_TIGHT, "spacing-regular-tight") \
    _(OC_UI_THEME_SPACING_REGULAR, "spacing-regular")             \
    _(OC_UI_THEME_SPACING_REGULAR_LOOSE, "spacing-regular-loose") \
    _(OC_UI_THEME_SPACING_LOOSE, "spacing-loose")                 \
    _(OC_UI_THEME_SPACING_EXTRA_LOOSE, "spacing-extra-loose")

#define OC_UI_THEME_NAME(n, s) static const oc_str8 n = OC_STR8_LIT(s);
OC_UI_DEFAULT_THEME_DATA(OC_UI_THEME_NAME)
#undef OC_UI_THEME_NAME

ORCA_API void oc_ui_theme_dark();
ORCA_API void oc_ui_theme_light();
///////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------
// Basic widget helpers
//-------------------------------------------------------------------------
ORCA_API oc_ui_sig oc_ui_label(const char* key, const char* label);
ORCA_API oc_ui_sig oc_ui_label_str8(oc_str8 key, oc_str8 label);

ORCA_API oc_ui_sig oc_ui_button(const char* key, const char* text);

ORCA_API oc_ui_sig oc_ui_checkbox(const char* name, bool* checked);
ORCA_API oc_ui_box* oc_ui_slider(const char* name, f32* value);

ORCA_API oc_ui_box* oc_ui_scrollbar(const char* name, oc_rect rect, f32 thumbRatio, f32* scrollValue, bool horizontal);

ORCA_API void oc_ui_tooltip_str8(oc_str8 label);
ORCA_API void oc_ui_tooltip(const char* label);

ORCA_API void oc_ui_menu_bar_begin(const char* name);
ORCA_API void oc_ui_menu_bar_end(void);
#define oc_ui_menu_bar(name) oc_defer_loop(oc_ui_menu_bar_begin(name), oc_ui_menu_bar_end())

ORCA_API void oc_ui_menu_begin(const char* label);
ORCA_API void oc_ui_menu_end(void);
#define oc_ui_menu(label) oc_defer_loop(oc_ui_menu_begin(label), oc_ui_menu_end())

ORCA_API oc_ui_sig oc_ui_menu_button(const char* label);

typedef struct oc_ui_text_box_result
{
    bool changed;
    bool accepted;
    oc_str8 text;
    oc_ui_box* frame;
    oc_ui_box* textBox;
} oc_ui_text_box_result;

ORCA_API oc_ui_text_box_result oc_ui_text_box(const char* name, oc_arena* arena, oc_str8 text);

typedef struct oc_ui_select_popup_info
{
    bool changed;
    int selectedIndex; // -1 if nothing is selected
    int optionCount;
    oc_str8* options;
    oc_str8 placeholder;
} oc_ui_select_popup_info;

ORCA_API oc_ui_select_popup_info oc_ui_select_popup(const char* name, oc_ui_select_popup_info* info);

typedef struct oc_ui_radio_group_info
{
    bool changed;
    int selectedIndex; // -1 if nothing is selected
    int optionCount;
    oc_str8* options;
} oc_ui_radio_group_info;

ORCA_API oc_ui_radio_group_info oc_ui_radio_group(const char* name, oc_ui_radio_group_info* info);

#ifdef __cplusplus
} // extern "C"
#endif
