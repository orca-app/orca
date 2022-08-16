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

typedef struct ui_layout
{
	ui_axis axis;
	ui_align align[UI_AXIS_COUNT];

} ui_layout;

typedef enum
{
	UI_SIZE_TEXT,
	UI_SIZE_PIXELS,
	UI_SIZE_CHILDREN,
	UI_SIZE_PARENT_RATIO,

} ui_size_kind;

typedef struct ui_size
{
	ui_size_kind kind;
	f32 value;
	f32 strictness;
} ui_size;

typedef enum { UI_STYLE_SEL_NORMAL = 1<<0,
               UI_STYLE_SEL_HOT    = 1<<1,
               UI_STYLE_SEL_ACTIVE = 1<<2,
               UI_STYLE_SEL_ANY    = UI_STYLE_SEL_NORMAL|UI_STYLE_SEL_HOT|UI_STYLE_SEL_ACTIVE,
             } ui_style_selector;

typedef u32 ui_style_tag;
#define UI_STYLE_TAG_ANY (ui_style_tag)0

typedef enum { UI_STYLE_ANIMATE_SIZE         = 1<<1,
               UI_STYLE_ANIMATE_BG_COLOR     = 1<<2,
               UI_STYLE_ANIMATE_FG_COLOR     = 1<<3,
               UI_STYLE_ANIMATE_BORDER_COLOR = 1<<4,
               UI_STYLE_ANIMATE_FONT_COLOR   = 1<<5,
               UI_STYLE_ANIMATE_FONT_SIZE    = 1<<6,
               UI_STYLE_ANIMATE_BORDER_SIZE  = 1<<7,
               UI_STYLE_ANIMATE_ROUNDNESS    = 1<<8,
               UI_STYLE_ANIMATE_POS          = 1<<9,
             } ui_style_animation_flags;

typedef struct ui_style
{
	ui_size size[UI_AXIS_COUNT];
	mg_color bgColor;
	mg_color fgColor;
	mg_color borderColor;
	mg_color fontColor;
	mg_font font;
	f32 fontSize;
	f32 borderSize;
	f32 roundness;
	f32 animationTime;
	ui_style_animation_flags animationFlags;
} ui_style;

typedef struct ui_box ui_box;

typedef struct ui_sig
{
	ui_box* box;

	vec2 mouse;
	vec2 delta;
	vec2 wheel;

	bool pressed;
	bool released;
	bool triggered;
	bool clicked;
	bool doubleClicked;
	bool rightClicked;
	bool dragging;
	bool hovering;

} ui_sig;

typedef void(*ui_box_render_proc)(mg_canvas canvas, ui_box* box, void* data);

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

	// styling and layout
	ui_box_render_proc renderProc;
	void* renderData;

	ui_style_tag tag;
	ui_style* targetStyle;
	ui_style computedStyle;
	u32 z;
	bool floating[UI_AXIS_COUNT];
	vec2 floatPos;
	vec2 floatTarget;
	ui_layout layout;
	f32 childrenSum[2];
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

void ui_init();
ui_context* ui_get_context();
void ui_set_context(ui_context* context);

void ui_begin_frame(u32 width, u32 height, ui_style defaultStyle);
void ui_end_frame();
void ui_draw(mg_canvas canvas);

ui_box* ui_box_make(const char* string, ui_flags flags);
ui_box* ui_box_begin(const char* string, ui_flags flags);
ui_box* ui_box_make_str8(str8 string, ui_flags flags);
ui_box* ui_box_begin_str8(str8 string, ui_flags flags);
ui_box* ui_box_end();
#define ui_container(name, flags) defer_loop(ui_box_begin(name, flags), ui_box_end())

void ui_box_push(ui_box* box);
void ui_box_pop();
ui_box* ui_box_top();

void ui_box_set_render_proc(ui_box* box, ui_box_render_proc proc, void* data);

void ui_box_set_layout(ui_box* box, ui_axis axis, ui_align alignX, ui_align alignY);
void ui_box_set_size(ui_box* box, ui_axis axis, ui_size_kind kind, f32 value, f32 strictness);
void ui_box_set_floating(ui_box* box, ui_axis axis, f32 pos);

void ui_box_set_style_selector(ui_box* box, ui_style_selector selector);

ui_sig ui_box_sig(ui_box* box);

void ui_push_size(ui_axis axis, ui_size_kind kind, f32 value, f32 strictness);
void ui_push_size_ext(ui_style_tag tag, ui_style_selector selector, ui_axis axis, ui_size_kind kind, f32 value, f32 strictness);
void ui_pop_size(ui_axis axis);

void ui_push_bg_color(mg_color color);
void ui_push_fg_color(mg_color color);
void ui_push_font(mg_font font);
void ui_push_font_size(f32 size);
void ui_push_font_color(mg_color color);
void ui_push_border_size(f32 size);
void ui_push_border_color(mg_color color);
void ui_push_roundness(f32 roundness);
void ui_push_animation_time(f32 time);
void ui_push_animation_flags(u32 flags);

void ui_push_bg_color_ext(ui_style_tag tag, ui_style_selector selector, mg_color color);
void ui_push_fg_color_ext(ui_style_tag tag, ui_style_selector selector, mg_color color);
void ui_push_font_ext(ui_style_tag tag, ui_style_selector selector, mg_font font);
void ui_push_font_size_ext(ui_style_tag tag, ui_style_selector selector, f32 size);
void ui_push_font_color_ext(ui_style_tag tag, ui_style_selector selector, mg_color color);
void ui_push_border_size_ext(ui_style_tag tag, ui_style_selector selector, f32 size);
void ui_push_border_color_ext(ui_style_tag tag, ui_style_selector selector, mg_color color);
void ui_push_roundness_ext(ui_style_tag tag, ui_style_selector selector, f32 roundness);
void ui_push_animation_time_ext(ui_style_tag tag, ui_style_selector selector, f32 time);
void ui_push_animation_flags_ext(ui_style_tag tag, ui_style_selector selector, u32 flags);

void ui_pop_bg_color();
void ui_pop_fg_color();
void ui_pop_font();
void ui_pop_font_size();
void ui_pop_font_color();
void ui_pop_border_size();
void ui_pop_border_color();
void ui_pop_roundness();
void ui_pop_animation_time();
void ui_pop_animation_flags();
// Basic helpers

enum {
	UI_STYLE_TAG_USER_MAX = 1<<16,
	UI_STYLE_TAG_LABEL,
	UI_STYLE_TAG_BUTTON,
	UI_STYLE_TAG_SCROLLBAR,
	UI_STYLE_TAG_PANEL,
	UI_STYLE_TAG_TOOLTIP,
	UI_STYLE_TAG_MENU
};

ui_sig ui_label(const char* label);
ui_sig ui_button(const char* label);
ui_box* ui_scrollbar(const char* label, f32 thumbRatio, f32* scrollValue);

ui_box* ui_panel_begin(const char* name);
void ui_panel_end();
#define ui_panel(name) defer_loop(ui_panel_begin(name), ui_panel_end())

ui_sig ui_tooltip_begin(const char* name);
void ui_tooltip_end();
#define ui_tooltip(name) defer_loop(ui_tooltip_begin(name), ui_tooltip_end())

void ui_menu_bar_begin(const char* label);
void ui_menu_bar_end();
#define ui_menu_bar(name) defer_loop(ui_menu_bar_begin(name), ui_menu_bar_end())

void ui_menu_begin(const char* label);
void ui_menu_end();
#define ui_menu(name) defer_loop(ui_menu_begin(name), ui_menu_end())

typedef struct ui_text_box_result
{
	bool changed;
	bool accepted;
	str8 text;

}ui_text_box_result;

ui_text_box_result ui_text_box(const char* name, mem_arena* arena, str8 text);

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__UI_H_
