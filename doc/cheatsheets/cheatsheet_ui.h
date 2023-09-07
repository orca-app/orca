/************************************************************/ /**
*
*	@file: cheatsheet_ui.h
*	@author: Martin Fouilleul
*	@date: 05/09/2023
*
*****************************************************************/

//----------------------------------------------------------------
// Context and frame lifecycle
//----------------------------------------------------------------

void oc_ui_init(oc_ui_context* context);
oc_ui_context* oc_ui_get_context(void);
void oc_ui_set_context(oc_ui_context* context);

void oc_ui_process_event(oc_event* event);
void oc_ui_begin_frame(oc_vec2 size, oc_ui_style* defaultStyle, oc_ui_style_mask mask);
void oc_ui_end_frame(void);
void oc_ui_draw(void);

#define oc_ui_frame(size, style, mask)

//----------------------------------------------------------------
// Common widget helpers
//----------------------------------------------------------------

oc_ui_sig oc_ui_label(const char* label);
oc_ui_sig oc_ui_label_str8(oc_str8 label);
oc_ui_sig oc_ui_button(const char* label);
oc_ui_sig oc_ui_checkbox(const char* name, bool* checked);
oc_ui_box* oc_ui_slider(const char* label, f32 thumbRatio, f32* scrollValue);
oc_ui_text_box_result oc_ui_text_box(const char* name, oc_arena* arena, oc_str8 text);
oc_ui_select_popup_info oc_ui_select_popup(const char* name, oc_ui_select_popup_info* info);

void oc_ui_panel_begin(const char* name, oc_ui_flags flags);
void oc_ui_panel_end(void);
#define oc_ui_panel(s, f)

void oc_ui_menu_bar_begin(const char* label);
void oc_ui_menu_bar_end(void);
#define oc_ui_menu_bar(name)

void oc_ui_menu_begin(const char* label);
void oc_ui_menu_end(void);
#define oc_ui_menu(name)

oc_ui_sig oc_ui_menu_button(const char* name);

oc_ui_sig oc_ui_tooltip_begin(const char* name);
void oc_ui_tooltip_end(void);
#define oc_ui_tooltip(name)

//-------------------------------------------------------------------------------------
// Styling
//-------------------------------------------------------------------------------------
void oc_ui_style_next(oc_ui_style* style, oc_ui_style_mask mask);

void oc_ui_pattern_push(oc_arena* arena, oc_ui_pattern* pattern, oc_ui_selector selector);
oc_ui_pattern oc_ui_pattern_all(void);
oc_ui_pattern oc_ui_pattern_owner(void);

void oc_ui_style_match_before(oc_ui_pattern pattern, oc_ui_style* style, oc_ui_style_mask mask);
void oc_ui_style_match_after(oc_ui_pattern pattern, oc_ui_style* style, oc_ui_style_mask mask);
