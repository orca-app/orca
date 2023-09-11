/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#ifndef __UI_H_
#define __UI_H_

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

} oc_ui_layout;

typedef enum oc_ui_size_kind
{
    OC_UI_SIZE_TEXT,
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

//NOTE: flags for axis-dependent properties (e.g. OC_UI_STYLE_FLOAT_X/Y) need to be consecutive bits
//      in order to play well with axis agnostic functions
typedef u64 oc_ui_style_mask;

enum
{
    OC_UI_STYLE_NONE = 0,
    OC_UI_STYLE_SIZE_WIDTH = 1 << 1,
    OC_UI_STYLE_SIZE_HEIGHT = 1 << 2,
    OC_UI_STYLE_LAYOUT_AXIS = 1 << 3,
    OC_UI_STYLE_LAYOUT_ALIGN_X = 1 << 4,
    OC_UI_STYLE_LAYOUT_ALIGN_Y = 1 << 5,
    OC_UI_STYLE_LAYOUT_SPACING = 1 << 6,
    OC_UI_STYLE_LAYOUT_MARGIN_X = 1 << 7,
    OC_UI_STYLE_LAYOUT_MARGIN_Y = 1 << 8,
    OC_UI_STYLE_FLOAT_X = 1 << 9,
    OC_UI_STYLE_FLOAT_Y = 1 << 10,
    OC_UI_STYLE_COLOR = 1 << 11,
    OC_UI_STYLE_BG_COLOR = 1 << 12,
    OC_UI_STYLE_BORDER_COLOR = 1 << 13,
    OC_UI_STYLE_BORDER_SIZE = 1 << 14,
    OC_UI_STYLE_ROUNDNESS = 1 << 15,
    OC_UI_STYLE_FONT = 1 << 16,
    OC_UI_STYLE_FONT_SIZE = 1 << 17,
    OC_UI_STYLE_ANIMATION_TIME = 1 << 18,
    OC_UI_STYLE_ANIMATION_MASK = 1 << 19,

    //masks
    OC_UI_STYLE_SIZE = OC_UI_STYLE_SIZE_WIDTH
                     | OC_UI_STYLE_SIZE_HEIGHT,

    OC_UI_STYLE_LAYOUT_MARGINS = OC_UI_STYLE_LAYOUT_MARGIN_X
                               | OC_UI_STYLE_LAYOUT_MARGIN_Y,

    OC_UI_STYLE_LAYOUT = OC_UI_STYLE_LAYOUT_AXIS
                       | OC_UI_STYLE_LAYOUT_ALIGN_X
                       | OC_UI_STYLE_LAYOUT_ALIGN_Y
                       | OC_UI_STYLE_LAYOUT_SPACING
                       | OC_UI_STYLE_LAYOUT_MARGIN_X
                       | OC_UI_STYLE_LAYOUT_MARGIN_Y,

    OC_UI_STYLE_FLOAT = OC_UI_STYLE_FLOAT_X
                      | OC_UI_STYLE_FLOAT_Y,

    OC_UI_STYLE_MASK_INHERITED = OC_UI_STYLE_COLOR
                               | OC_UI_STYLE_FONT
                               | OC_UI_STYLE_FONT_SIZE
                               | OC_UI_STYLE_ANIMATION_TIME
                               | OC_UI_STYLE_ANIMATION_MASK,
};

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
    oc_ui_style_mask animationMask;
} oc_ui_style;

typedef struct oc_ui_palette
{
    oc_color red0;
    oc_color red1;
    oc_color red2;
    oc_color red3;
    oc_color red4;
    oc_color red5;
    oc_color red6;
    oc_color red7;
    oc_color red8;
    oc_color red9;
    oc_color orange0;
    oc_color orange1;
    oc_color orange2;
    oc_color orange3;
    oc_color orange4;
    oc_color orange5;
    oc_color orange6;
    oc_color orange7;
    oc_color orange8;
    oc_color orange9;
    oc_color amber0;
    oc_color amber1;
    oc_color amber2;
    oc_color amber3;
    oc_color amber4;
    oc_color amber5;
    oc_color amber6;
    oc_color amber7;
    oc_color amber8;
    oc_color amber9;
    oc_color yellow0;
    oc_color yellow1;
    oc_color yellow2;
    oc_color yellow3;
    oc_color yellow4;
    oc_color yellow5;
    oc_color yellow6;
    oc_color yellow7;
    oc_color yellow8;
    oc_color yellow9;
    oc_color lime0;
    oc_color lime1;
    oc_color lime2;
    oc_color lime3;
    oc_color lime4;
    oc_color lime5;
    oc_color lime6;
    oc_color lime7;
    oc_color lime8;
    oc_color lime9;
    oc_color lightGreen0;
    oc_color lightGreen1;
    oc_color lightGreen2;
    oc_color lightGreen3;
    oc_color lightGreen4;
    oc_color lightGreen5;
    oc_color lightGreen6;
    oc_color lightGreen7;
    oc_color lightGreen8;
    oc_color lightGreen9;
    oc_color green0;
    oc_color green1;
    oc_color green2;
    oc_color green3;
    oc_color green4;
    oc_color green5;
    oc_color green6;
    oc_color green7;
    oc_color green8;
    oc_color green9;
    oc_color teal0;
    oc_color teal1;
    oc_color teal2;
    oc_color teal3;
    oc_color teal4;
    oc_color teal5;
    oc_color teal6;
    oc_color teal7;
    oc_color teal8;
    oc_color teal9;
    oc_color cyan0;
    oc_color cyan1;
    oc_color cyan2;
    oc_color cyan3;
    oc_color cyan4;
    oc_color cyan5;
    oc_color cyan6;
    oc_color cyan7;
    oc_color cyan8;
    oc_color cyan9;
    oc_color lightBlue0;
    oc_color lightBlue1;
    oc_color lightBlue2;
    oc_color lightBlue3;
    oc_color lightBlue4;
    oc_color lightBlue5;
    oc_color lightBlue6;
    oc_color lightBlue7;
    oc_color lightBlue8;
    oc_color lightBlue9;
    oc_color blue0;
    oc_color blue1;
    oc_color blue2;
    oc_color blue3;
    oc_color blue4;
    oc_color blue5;
    oc_color blue6;
    oc_color blue7;
    oc_color blue8;
    oc_color blue9;
    oc_color indigo0;
    oc_color indigo1;
    oc_color indigo2;
    oc_color indigo3;
    oc_color indigo4;
    oc_color indigo5;
    oc_color indigo6;
    oc_color indigo7;
    oc_color indigo8;
    oc_color indigo9;
    oc_color violet0;
    oc_color violet1;
    oc_color violet2;
    oc_color violet3;
    oc_color violet4;
    oc_color violet5;
    oc_color violet6;
    oc_color violet7;
    oc_color violet8;
    oc_color violet9;
    oc_color purple0;
    oc_color purple1;
    oc_color purple2;
    oc_color purple3;
    oc_color purple4;
    oc_color purple5;
    oc_color purple6;
    oc_color purple7;
    oc_color purple8;
    oc_color purple9;
    oc_color pink0;
    oc_color pink1;
    oc_color pink2;
    oc_color pink3;
    oc_color pink4;
    oc_color pink5;
    oc_color pink6;
    oc_color pink7;
    oc_color pink8;
    oc_color pink9;
    oc_color grey0;
    oc_color grey1;
    oc_color grey2;
    oc_color grey3;
    oc_color grey4;
    oc_color grey5;
    oc_color grey6;
    oc_color grey7;
    oc_color grey8;
    oc_color grey9;
    oc_color black;
    oc_color white;
} oc_ui_palette;

extern oc_ui_palette OC_UI_DARK_PALETTE;
extern oc_ui_palette OC_UI_LIGHT_PALETTE;

typedef struct oc_ui_theme
{
    oc_color white;
    oc_color primary;
    oc_color primaryHover;
    oc_color primaryActive;
    oc_color fill0;
    oc_color fill1;
    oc_color fill2;
    oc_color bg0;
    oc_color bg1;
    oc_color bg2;
    oc_color bg3;
    oc_color bg4;
    oc_color text0;
    oc_color text1;
    oc_color text2;
    oc_color text3;
    oc_color sliderThumbBorder;
    oc_color elevatedBorder;

    oc_ui_palette* palette;
} oc_ui_theme;

extern oc_ui_theme OC_UI_DARK_THEME;
extern oc_ui_theme OC_UI_LIGHT_THEME;

typedef struct oc_ui_tag
{
    u64 hash;
} oc_ui_tag;

typedef enum
{
    OC_UI_SEL_ANY,
    OC_UI_SEL_OWNER,
    OC_UI_SEL_TEXT,
    OC_UI_SEL_TAG,
    OC_UI_SEL_STATUS,
    OC_UI_SEL_KEY,
    //...
} oc_ui_selector_kind;

typedef u8 oc_ui_status;

enum
{
    OC_UI_NONE = 0,
    OC_UI_HOVER = 1 << 1,
    OC_UI_ACTIVE = 1 << 2,
    OC_UI_DRAGGING = 1 << 3,
};

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

    union
    {
        oc_str8 text;
        oc_ui_key key;
        oc_ui_tag tag;
        oc_ui_status status;
        //...
    };
} oc_ui_selector;

typedef struct oc_ui_pattern
{
    oc_list l;
} oc_ui_pattern;

typedef struct oc_ui_box oc_ui_box;

typedef struct oc_ui_style_rule
{
    oc_list_elt boxElt;
    oc_list_elt buildElt;
    oc_list_elt tmpElt;

    oc_ui_box* owner;
    oc_ui_pattern pattern;
    oc_ui_style_mask mask;
    oc_ui_style* style;
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

typedef enum
{
    OC_UI_FLAG_CLICKABLE = (1 << 0),
    OC_UI_FLAG_SCROLL_WHEEL_X = (1 << 1),
    OC_UI_FLAG_SCROLL_WHEEL_Y = (1 << 2),
    OC_UI_FLAG_BLOCK_MOUSE = (1 << 3),
    OC_UI_FLAG_HOT_ANIMATION = (1 << 4),
    OC_UI_FLAG_ACTIVE_ANIMATION = (1 << 5),
    //WARN: these two following flags need to be kept as consecutive bits to
    //      play well with axis-agnostic functions
    OC_UI_FLAG_ALLOW_OVERFLOW_X = (1 << 6),
    OC_UI_FLAG_ALLOW_OVERFLOW_Y = (1 << 7),
    OC_UI_FLAG_CLIP = (1 << 8),
    OC_UI_FLAG_DRAW_BACKGROUND = (1 << 9),
    OC_UI_FLAG_DRAW_FOREGROUND = (1 << 10),
    OC_UI_FLAG_DRAW_BORDER = (1 << 11),
    OC_UI_FLAG_DRAW_TEXT = (1 << 12),
    OC_UI_FLAG_DRAW_PROC = (1 << 13),

    OC_UI_FLAG_OVERLAY = (1 << 14),
} oc_ui_flags;

struct oc_ui_box
{
    // hierarchy
    oc_list_elt listElt;
    oc_list children;
    oc_ui_box* parent;

    oc_list_elt overlayElt;

    // keying and caching
    oc_list_elt bucketElt;
    oc_ui_key key;
    u64 frameCounter;

    // builder-provided info
    oc_ui_flags flags;
    oc_str8 string;
    oc_list tags;

    oc_ui_box_draw_proc drawProc;
    void* drawData;

    // styling
    oc_list beforeRules;
    oc_list afterRules;

    //oc_ui_style_tag tag;
    oc_ui_style* targetStyle;
    oc_ui_style style;
    u32 z;

    oc_vec2 floatPos;
    f32 childrenSum[2];
    f32 spacing[2];
    oc_rect rect;

    // signals
    oc_ui_sig* sig;

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

    oc_list nextBoxBeforeRules;
    oc_list nextBoxAfterRules;
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
    
    bool clipboardRegistered;

    oc_ui_theme* theme;
} oc_ui_context;

//-------------------------------------------------------------------------------------
// UI context initialization and frame cycle
//-------------------------------------------------------------------------------------
ORCA_API void oc_ui_init(oc_ui_context* context);
ORCA_API oc_ui_context* oc_ui_get_context(void);
ORCA_API void oc_ui_set_context(oc_ui_context* context);

ORCA_API void oc_ui_process_event(oc_event* event);
ORCA_API void oc_ui_begin_frame(oc_vec2 size, oc_ui_style* defaultStyle, oc_ui_style_mask mask);
ORCA_API void oc_ui_end_frame(void);
ORCA_API void oc_ui_draw(void);
ORCA_API void oc_ui_set_theme(oc_ui_theme* theme);

#define oc_ui_frame(size, style, mask) oc_defer_loop(oc_ui_begin_frame((size), (style), (mask)), oc_ui_end_frame())

//-------------------------------------------------------------------------------------
// Box keys
//-------------------------------------------------------------------------------------
ORCA_API oc_ui_key oc_ui_key_make_str8(oc_str8 string);
ORCA_API oc_ui_key oc_ui_key_make_path(oc_str8_list path);

ORCA_API oc_ui_box* oc_ui_box_lookup_key(oc_ui_key key);
ORCA_API oc_ui_box* oc_ui_box_lookup_str8(oc_str8 string);

// C-string helper
#define oc_ui_key_make(s) oc_ui_key_make_str8(OC_STR8(s))
#define oc_ui_box_lookup(s) oc_ui_box_lookup_str8(OC_STR8(s))

//-------------------------------------------------------------------------------------
// Box hierarchy building
//-------------------------------------------------------------------------------------
ORCA_API oc_ui_box* oc_ui_box_make_str8(oc_str8 string, oc_ui_flags flags);
ORCA_API oc_ui_box* oc_ui_box_begin_str8(oc_str8 string, oc_ui_flags flags);

ORCA_API oc_ui_box* oc_ui_box_end(void);
#define oc_ui_container(name, flags) oc_defer_loop(oc_ui_box_begin(name, flags), oc_ui_box_end())
#define oc_ui_container_str8(name, flags) oc_defer_loop(oc_ui_box_begin_str8(name, flags), oc_ui_box_end())

ORCA_API void oc_ui_box_push(oc_ui_box* box);
ORCA_API void oc_ui_box_pop(void);
ORCA_API oc_ui_box* oc_ui_box_top(void);

ORCA_API void oc_ui_box_set_draw_proc(oc_ui_box* box, oc_ui_box_draw_proc proc, void* data);

// C-string helpers
#define oc_ui_box_lookup(s) oc_ui_box_lookup_str8(OC_STR8(s))
#define oc_ui_box_make(s, flags) oc_ui_box_make_str8(OC_STR8(s), flags)
#define oc_ui_box_begin(s, flags) oc_ui_box_begin_str8(OC_STR8(s), flags)

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

// C-string helpers
#define oc_ui_tag_make(s) oc_ui_tag_make_str8(OC_STR8(s))
#define oc_ui_tag_box(b, s) oc_ui_tag_box_str8(b, OC_STR8(s))
#define oc_ui_tag_next(s) oc_ui_tag_next_str8(OC_STR8(s))

//-------------------------------------------------------------------------------------
// Styling
//-------------------------------------------------------------------------------------
//NOTE: styling API
//WARN: You can use a pattern in multiple rules, but be aware that a pattern is references an underlying list of selectors,
//      hence pushing to a pattern also modifies rules in which the pattern was previously used!
ORCA_API void oc_ui_apply_style_with_mask(oc_ui_style* dst, oc_ui_style* src, oc_ui_style_mask mask);

ORCA_API void oc_ui_pattern_push(oc_arena* arena, oc_ui_pattern* pattern, oc_ui_selector selector);
ORCA_API oc_ui_pattern oc_ui_pattern_all(void);
ORCA_API oc_ui_pattern oc_ui_pattern_owner(void);

ORCA_API void oc_ui_style_next(oc_ui_style* style, oc_ui_style_mask mask);
ORCA_API void oc_ui_style_match_before(oc_ui_pattern pattern, oc_ui_style* style, oc_ui_style_mask mask);
ORCA_API void oc_ui_style_match_after(oc_ui_pattern pattern, oc_ui_style* style, oc_ui_style_mask mask);

//-------------------------------------------------------------------------
// Basic widget helpers
//-------------------------------------------------------------------------
enum
{
    OC_UI_STYLE_TAG_USER_MAX = 1 << 16,
    OC_UI_STYLE_TAG_LABEL,
    OC_UI_STYLE_TAG_BUTTON,
    OC_UI_STYLE_TAG_SCROLLBAR,
    OC_UI_STYLE_TAG_PANEL,
    OC_UI_STYLE_TAG_TOOLTIP,
    OC_UI_STYLE_TAG_MENU
};

ORCA_API oc_ui_sig oc_ui_label(const char* label);
ORCA_API oc_ui_sig oc_ui_label_str8(oc_str8 label);

ORCA_API oc_ui_sig oc_ui_button(const char* label);
ORCA_API oc_ui_sig oc_ui_checkbox(const char* name, bool* checked);
ORCA_API oc_ui_box* oc_ui_slider(const char* label, f32* value);
ORCA_API oc_ui_box* oc_ui_scrollbar(const char* label, f32 thumbRatio, f32* scrollValue);
ORCA_API void oc_ui_tooltip(const char* label);

ORCA_API void oc_ui_panel_begin(const char* name, oc_ui_flags flags);
ORCA_API void oc_ui_panel_end(void);
#define oc_ui_panel(s, f) oc_defer_loop(oc_ui_panel_begin(s, f), oc_ui_panel_end())

ORCA_API void oc_ui_menu_bar_begin(const char* label);
ORCA_API void oc_ui_menu_bar_end(void);
#define oc_ui_menu_bar(name) oc_defer_loop(oc_ui_menu_bar_begin(name), oc_ui_menu_bar_end())

ORCA_API void oc_ui_menu_begin(const char* label);
ORCA_API void oc_ui_menu_end(void);
#define oc_ui_menu(name) oc_defer_loop(oc_ui_menu_begin(name), oc_ui_menu_end())

ORCA_API oc_ui_sig oc_ui_menu_button(const char* name);

typedef struct oc_ui_text_box_result
{
    bool changed;
    bool accepted;
    oc_str8 text;

} oc_ui_text_box_result;

ORCA_API oc_ui_text_box_result oc_ui_text_box(const char* name, oc_arena* arena, oc_str8 text);

typedef struct oc_ui_select_popup_info
{
    bool changed;
    int selectedIndex;
    int optionCount;
    oc_str8* options;
} oc_ui_select_popup_info;

ORCA_API oc_ui_select_popup_info oc_ui_select_popup(const char* name, oc_ui_select_popup_info* info);

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__UI_H_
