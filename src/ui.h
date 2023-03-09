/************************************************************//**
*
*	@file: ui.h
*	@author: Martin Fouilleul
*	@date: 08/08/2022
*	@revision:
*
*****************************************************************/
#ifndef __UI_H_
#define __UI_H_

#include"typedefs.h"
#include"lists.h"
#include"graphics.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	UI_FLAG_CLICKABLE        = (1<<0),
	UI_FLAG_SCROLLABLE       = (1<<1),
	UI_FLAG_BLOCK_MOUSE      = (1<<2),
	UI_FLAG_HOT_ANIMATION    = (1<<3),
	UI_FLAG_ACTIVE_ANIMATION = (1<<4),
	UI_FLAG_CLIP             = (1<<5),
	UI_FLAG_DRAW_BACKGROUND  = (1<<6),
	UI_FLAG_DRAW_FOREGROUND  = (1<<7),
	UI_FLAG_DRAW_BORDER      = (1<<8),
	UI_FLAG_DRAW_TEXT        = (1<<9),
	UI_FLAG_DRAW_RENDER_PROC = (1<<10),

} ui_flags;

typedef struct ui_key
{
	u64 hash;
} ui_key;

typedef enum
{
	UI_AXIS_X,
	UI_AXIS_Y,
	UI_AXIS_COUNT
} ui_axis;

typedef enum
{
	UI_ALIGN_START,
	UI_ALIGN_END,
	UI_ALIGN_CENTER,
} ui_align;

typedef union ui_layout_align
{
	struct
	{
		ui_align x;
		ui_align y;
	};
	ui_align c[UI_AXIS_COUNT];
} ui_layout_align;

typedef struct ui_layout
{
	ui_axis axis;
	f32 spacing;
	union
	{
		struct
		{
			f32 x;
			f32 y;
		};
		f32 c[UI_AXIS_COUNT];
	} margin;
	ui_layout_align align;

} ui_layout;

typedef enum ui_size_kind
{
	UI_SIZE_TEXT,
	UI_SIZE_PIXELS,
	UI_SIZE_CHILDREN,
	UI_SIZE_PARENT,
	UI_SIZE_PARENT_MINUS_PIXELS,

} ui_size_kind;

typedef struct ui_size
{
	ui_size_kind kind;
	f32 value;
	f32 relax;
} ui_size;

typedef union ui_box_size
{
	struct
	{
		ui_size width;
		ui_size height;
	};
	ui_size c[UI_AXIS_COUNT];
} ui_box_size;

typedef union ui_box_floating
{
	struct
	{
		bool x;
		bool y;
	};
	bool c[UI_AXIS_COUNT];
} ui_box_floating;

typedef enum { UI_STYLE_ANIMATE_SIZE_WIDTH   = 1<<1,
               UI_STYLE_ANIMATE_SIZE_HEIGHT  = 1<<2,
               UI_STYLE_ANIMATE_COLOR        = 1<<3,
               UI_STYLE_ANIMATE_BG_COLOR     = 1<<4,
               UI_STYLE_ANIMATE_BORDER_COLOR = 1<<5,
               UI_STYLE_ANIMATE_BORDER_SIZE  = 1<<6,
               UI_STYLE_ANIMATE_FONT_SIZE    = 1<<7,
               UI_STYLE_ANIMATE_ROUNDNESS    = 1<<8,
               UI_STYLE_ANIMATE_POS          = 1<<9,
             } ui_style_animation_flags;

typedef struct ui_style
{
	ui_box_size size;
	ui_layout layout;
	ui_box_floating floating;
	vec2 floatTarget;
	mg_color color;
	mg_color bgColor;
	mg_color borderColor;
	mg_font font;
	f32 fontSize;
	f32 borderSize;
	f32 roundness;
	f32 animationTime;
	ui_style_animation_flags animationFlags;
} ui_style;

typedef u64 ui_style_mask;
enum
{
	UI_STYLE_NONE            = 0,
	UI_STYLE_SIZE_WIDTH      = 1<<1,
	UI_STYLE_SIZE_HEIGHT     = 1<<2,
	UI_STYLE_LAYOUT_AXIS     = 1<<3,
	UI_STYLE_LAYOUT_ALIGN_X  = 1<<4,
	UI_STYLE_LAYOUT_ALIGN_Y  = 1<<5,
	UI_STYLE_LAYOUT_SPACING  = 1<<6,
	UI_STYLE_LAYOUT_MARGIN_X = 1<<7,
	UI_STYLE_LAYOUT_MARGIN_Y = 1<<8,
	UI_STYLE_FLOAT_X         = 1<<9,
	UI_STYLE_FLOAT_Y         = 1<<10,
	UI_STYLE_COLOR           = 1<<11,
	UI_STYLE_BG_COLOR        = 1<<12,
	UI_STYLE_BORDER_COLOR    = 1<<13,
	UI_STYLE_BORDER_SIZE     = 1<<14,
	UI_STYLE_ROUNDNESS       = 1<<15,
	UI_STYLE_FONT            = 1<<16,
	UI_STYLE_FONT_SIZE       = 1<<17,
	UI_STYLE_ANIMATION_TIME  = 1<<18,
	UI_STYLE_ANIMATION_FLAGS = 1<<19,

	//masks
	UI_STYLE_SIZE = UI_STYLE_SIZE_WIDTH
	              | UI_STYLE_SIZE_HEIGHT,

	UI_STYLE_LAYOUT = UI_STYLE_LAYOUT_AXIS
	                | UI_STYLE_LAYOUT_ALIGN_X
	                | UI_STYLE_LAYOUT_ALIGN_Y
	                | UI_STYLE_LAYOUT_SPACING
	                | UI_STYLE_LAYOUT_MARGIN_X
	                | UI_STYLE_LAYOUT_MARGIN_Y,

	UI_STYLE_FLOAT = UI_STYLE_FLOAT_X
	               | UI_STYLE_FLOAT_Y,

	UI_STYLE_MASK_INHERITED = UI_STYLE_COLOR
	                        | UI_STYLE_FONT
	                        | UI_STYLE_FONT_SIZE
	                        | UI_STYLE_ANIMATION_TIME
	                        | UI_STYLE_ANIMATION_FLAGS,
};


typedef struct ui_tag { u64 hash; } ui_tag;

typedef enum
{
	UI_SEL_ANY,
	UI_SEL_OWNER,
	UI_SEL_TEXT,
	UI_SEL_TAG,
	UI_SEL_STATUS,
	UI_SEL_KEY,
	//...
} ui_selector_kind;

typedef u8 ui_status;
enum
{
	UI_NONE     = 0,
	UI_HOVER    = 1<<1,
	UI_ACTIVE   = 1<<2,
	UI_DRAGGING = 1<<3,
};

typedef enum
{
	UI_SEL_DESCENDANT = 0,
	UI_SEL_AND = 1,
	//...
} ui_selector_op;

typedef struct ui_selector
{
	list_elt listElt;
	ui_selector_kind kind;
	ui_selector_op op;
	union
	{
		str8 text;
		ui_key key;
		ui_tag tag;
		ui_status status;
		//...
	};
} ui_selector;

typedef struct ui_pattern { list_info l; } ui_pattern;

typedef struct ui_box ui_box;

typedef struct ui_style_rule
{
	list_elt boxElt;
	list_elt buildElt;
	list_elt tmpElt;

	ui_box* owner;
	ui_pattern pattern;
	ui_style_mask mask;
	ui_style* style;
} ui_style_rule;

typedef struct ui_sig
{
	ui_box* box;

	vec2 mouse;
	vec2 delta;
	vec2 wheel;

	bool pressed;
	bool released;
	bool clicked;
	bool doubleClicked;
	bool rightPressed;

	bool dragging;
	bool hovering;

} ui_sig;

typedef void(*ui_box_render_proc)(ui_box* box, void* data);

struct ui_box
{
	// hierarchy
	list_elt listElt;
	list_info children;
	ui_box* parent;

	// keying and caching
	list_elt bucketElt;
	ui_key key;
	u64 frameCounter;

	// builder-provided info
	ui_flags flags;
	str8 string;
	list_info tags;

	ui_box_render_proc renderProc;
	void* renderData;

	// styling
	list_info beforeRules;
	list_info afterRules;

	//ui_style_tag tag;
	ui_style* targetStyle;
	ui_style style;
	u32 z;

	vec2 floatPos;
	f32 childrenSum[2];
	f32 spacing[2];
	mp_rect rect;

	// signals
	ui_sig* sig;

	// stateful behaviour
	bool fresh;
	bool closed;
	bool parentClosed;
	bool dragging;
	bool hot;
	bool active;
	vec2 scroll;

	// animation data
	f32 hotTransition;
	f32 activeTransition;
};

typedef struct ui_context ui_context;

//-------------------------------------------------------------------------------------
// UI context initialization and frame cycle
//-------------------------------------------------------------------------------------
MP_API void ui_init(void);
MP_API ui_context* ui_get_context(void);
MP_API void ui_set_context(ui_context* context);

MP_API void ui_begin_frame(ui_style* defaultStyle, ui_style_mask mask);
MP_API void ui_end_frame(void);
MP_API void ui_draw(void);

#define ui_frame(s, m) defer_loop(ui_begin_frame((s), (m)), ui_end_frame())

//-------------------------------------------------------------------------------------
// Box keys
//-------------------------------------------------------------------------------------
MP_API ui_key ui_key_make_str8(str8 string);
MP_API ui_key ui_key_make_path(str8_list path);

MP_API ui_box* ui_box_lookup_key(ui_key key);
MP_API ui_box* ui_box_lookup_str8(str8 string);

// C-string helper
#define ui_key_make(s) ui_key_make_str8(STR8(s))
#define ui_box_lookup(s) ui_box_lookup_str8(STR8(s))

//-------------------------------------------------------------------------------------
// Box hierarchy building
//-------------------------------------------------------------------------------------
MP_API ui_box* ui_box_make_str8(str8 string, ui_flags flags);
MP_API ui_box* ui_box_begin_str8(str8 string, ui_flags flags);

MP_API ui_box* ui_box_end(void);
#define ui_container(name, flags) defer_loop(ui_box_begin(name, flags), ui_box_end())

MP_API void ui_box_push(ui_box* box);
MP_API void ui_box_pop(void);
MP_API ui_box* ui_box_top(void);

MP_API void ui_box_set_render_proc(ui_box* box, ui_box_render_proc proc, void* data);

// C-string helpers
#define ui_box_lookup(s) ui_box_lookup_str8(STR8(s))
#define ui_box_make(s, flags) ui_box_make_str8(STR8(s), flags)
#define ui_box_begin(s, flags) ui_box_begin_str8(STR8(s), flags)

//-------------------------------------------------------------------------------------
// Box status and signals
//-------------------------------------------------------------------------------------
MP_API bool ui_box_closed(ui_box* box);
MP_API void ui_box_set_closed(ui_box* box, bool closed);

MP_API bool ui_box_active(ui_box* box);
MP_API void ui_box_activate(ui_box* box);
MP_API void ui_box_deactivate(ui_box* box);

MP_API bool ui_box_hot(ui_box* box);
MP_API void ui_box_set_hot(ui_box* box, bool hot);

MP_API ui_sig ui_box_sig(ui_box* box);

//-------------------------------------------------------------------------------------
// Tagging
//-------------------------------------------------------------------------------------
MP_API ui_tag ui_tag_make_str8(str8 string);
MP_API void ui_tag_box_str8(ui_box* box, str8 string);
MP_API void ui_tag_next_str8(str8 string);

// C-string helpers
#define ui_tag_make(s) ui_tag_make_str8(STR8(s))
#define ui_tag_box(b, s) ui_tag_box_str8(b, STR8(s))
#define ui_tag_next(s) ui_tag_next_str8(STR8(s))

//-------------------------------------------------------------------------------------
// Styling
//-------------------------------------------------------------------------------------
//NOTE: styling API
//WARN: You can use a pattern in multiple rules, but be aware that a pattern is references an underlying list of selectors,
//      hence pushing to a pattern also modifies rules in which the pattern was previously used!
MP_API void ui_apply_style_with_mask(ui_style* dst, ui_style* src, ui_style_mask mask);

MP_API void ui_pattern_push(mem_arena* arena, ui_pattern* pattern, ui_selector selector);
MP_API ui_pattern ui_pattern_all(void);
MP_API ui_pattern ui_pattern_owner(void);

MP_API void ui_style_next(ui_style* style, ui_style_mask mask);
MP_API void ui_style_match_before(ui_pattern pattern, ui_style* style, ui_style_mask mask);
MP_API void ui_style_match_after(ui_pattern pattern, ui_style* style, ui_style_mask mask);

//-------------------------------------------------------------------------
// Basic widget helpers
//-------------------------------------------------------------------------
enum {
	UI_STYLE_TAG_USER_MAX = 1<<16,
	UI_STYLE_TAG_LABEL,
	UI_STYLE_TAG_BUTTON,
	UI_STYLE_TAG_SCROLLBAR,
	UI_STYLE_TAG_PANEL,
	UI_STYLE_TAG_TOOLTIP,
	UI_STYLE_TAG_MENU
};

MP_API ui_sig ui_label(const char* label);
MP_API ui_sig ui_button(const char* label);
MP_API ui_box* ui_slider(const char* label, f32 thumbRatio, f32* scrollValue);

MP_API void ui_panel_begin(const char* name, ui_flags flags);
MP_API void ui_panel_end(void);
#define ui_panel(s, f) defer_loop(ui_panel_begin(s, f), ui_panel_end())

MP_API ui_sig ui_tooltip_begin(const char* name);
MP_API void ui_tooltip_end(void);
#define ui_tooltip(name) defer_loop(ui_tooltip_begin(name), ui_tooltip_end())

MP_API void ui_menu_bar_begin(const char* label);
MP_API void ui_menu_bar_end(void);
#define ui_menu_bar(name) defer_loop(ui_menu_bar_begin(name), ui_menu_bar_end())

MP_API void ui_menu_begin(const char* label);
MP_API void ui_menu_end(void);
#define ui_menu(name) defer_loop(ui_menu_begin(name), ui_menu_end())

MP_API ui_sig ui_menu_button(const char* name);

typedef struct ui_text_box_result
{
	bool changed;
	bool accepted;
	str8 text;

}ui_text_box_result;

MP_API ui_text_box_result ui_text_box(const char* name, mem_arena* arena, str8 text);

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__UI_H_
