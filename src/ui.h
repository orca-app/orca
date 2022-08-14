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

typedef enum { UI_STYLE_NORMAL,
               UI_STYLE_HOT,
               UI_STYLE_ACTIVE,
               UI_STYLE_SELECTOR_COUNT } ui_style_selector;

typedef struct ui_style
{
	mg_color backgroundColor;
	mg_color foregroundColor;
	mg_color borderColor;
	mg_color textColor;
	mg_font font;
	f32 fontSize;
	f32 borderSize;
	f32 roundness;
	f32 animationTime;
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

	// layout
	u32 z;
	bool floating[UI_AXIS_COUNT];
	ui_size desiredSize[UI_AXIS_COUNT];
	ui_layout layout;
	mp_rect rect;
	f32 childrenSum[2];

	// styling
	ui_style* styles[UI_STYLE_SELECTOR_COUNT];
	ui_style computedStyle;
	ui_style_selector styleSelector;

	// signals
	ui_sig* sig;

	// stateful behaviour
	bool closed;
	bool parentClosed;
	bool dragging;
	bool active;
	vec2 scroll;

	// animation data
	f32 hotTransition;
	f32 activeTransition;
};

void ui_init();

void ui_begin_frame(u32 width, u32 height, ui_style defaultStyle);
void ui_end_frame();
void ui_draw(mg_canvas canvas);

ui_box* ui_box_make(const char* string, ui_flags flags);
ui_box* ui_box_begin(const char* string, ui_flags flags);
ui_box* ui_box_make_str8(str8 string, ui_flags flags);
ui_box* ui_box_begin_str8(str8 string, ui_flags flags);
ui_box* ui_box_end();
#define ui_container(name, flags) defer_loop(ui_box_begin(name, flags), ui_box_end())

void ui_box_set_layout(ui_box* box, ui_axis axis, ui_align alignX, ui_align alignY);
void ui_box_set_size(ui_box* box, ui_axis axis, ui_size_kind kind, f32 value, f32 strictness);
void ui_box_set_floating(ui_box* box, ui_axis axis, f32 pos);

void ui_box_set_style_selector(ui_box* box, ui_style_selector selector);

ui_sig ui_box_sig(ui_box* box);

void ui_size_push(ui_axis axis, ui_size_kind kind, f32 value, f32 strictness);
void ui_size_pop(ui_axis axis);
void ui_style_push(ui_style_selector selector, ui_style style);
void ui_style_pop(ui_style_selector selector);

// Basic helpers
ui_sig ui_label(const char* label);

ui_sig ui_button(const char* label);
ui_box* ui_scrollbar(const char* label, f32 thumbRatio, f32* scrollValue);

void ui_panel_begin(const char* name);
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

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__UI_H_
