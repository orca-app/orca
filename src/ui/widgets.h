/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "ui/ui.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------
// Label
//------------------------------------------------------------------------
ORCA_API oc_ui_sig oc_ui_label(const char* key, const char* label);
ORCA_API oc_ui_sig oc_ui_label_str8(oc_str8 key, oc_str8 label);

//------------------------------------------------------------------------
// Button
//------------------------------------------------------------------------
ORCA_API oc_ui_sig oc_ui_button(const char* key, const char* text);
ORCA_API oc_ui_sig oc_ui_button_str8(oc_str8 key, oc_str8 text);

//------------------------------------------------------------------------
// Checkbox
//------------------------------------------------------------------------
ORCA_API oc_ui_sig oc_ui_checkbox(const char* key, bool* checked);
ORCA_API oc_ui_sig oc_ui_checkbox_str8(oc_str8 key, bool* checked);

//------------------------------------------------------------------------
// Slider
//------------------------------------------------------------------------
ORCA_API oc_ui_box* oc_ui_slider(const char* name, f32* value);
ORCA_API oc_ui_box* oc_ui_slider_str8(oc_str8 name, f32* value);

//------------------------------------------------------------------------
// Tooltip
//------------------------------------------------------------------------
ORCA_API void oc_ui_tooltip(const char* key, const char* text);
ORCA_API void oc_ui_tooltip_str8(oc_str8 key, oc_str8 text);

//------------------------------------------------------------------------
// Menus
//------------------------------------------------------------------------
ORCA_API void oc_ui_menu_bar_begin(const char* key);
ORCA_API void oc_ui_menu_bar_begin_str8(oc_str8 key);
ORCA_API void oc_ui_menu_bar_end(void);
#define oc_ui_menu_bar(key) oc_defer_loop(oc_ui_menu_bar_begin(key), oc_ui_menu_bar_end())

ORCA_API void oc_ui_menu_begin(const char* key, const char* name);
ORCA_API void oc_ui_menu_begin_str8(oc_str8 key, oc_str8 name);
ORCA_API void oc_ui_menu_end(void);
#define oc_ui_menu(key, text) oc_defer_loop(oc_ui_menu_begin(key, text), oc_ui_menu_end())

ORCA_API oc_ui_sig oc_ui_menu_button(const char* key, const char* text);
ORCA_API oc_ui_sig oc_ui_menu_button_str8(oc_str8 key, oc_str8 text);

//------------------------------------------------------------------------
// Text Box
//------------------------------------------------------------------------
typedef struct oc_ui_text_box_result
{
    bool changed;
    bool accepted;
    oc_str8 text;
    oc_ui_box* frame;
    oc_ui_box* textBox;
} oc_ui_text_box_result;

typedef enum
{
    OC_UI_EDIT_MOVE_NONE = 0,
    OC_UI_EDIT_MOVE_CHAR,
    OC_UI_EDIT_MOVE_WORD,
    OC_UI_EDIT_MOVE_LINE
} oc_ui_edit_move;

typedef struct oc_ui_text_box_info
{
    oc_str8 text;

    i32 cursor;
    i32 mark;

    oc_ui_edit_move selectionMode;
    i32 wordSelectionInitialCursor;
    i32 wordSelectionInitialMark;

    i32 firstDisplayedChar;
    f64 cursorBlinkStart;

} oc_ui_text_box_info;

ORCA_API oc_ui_text_box_result oc_ui_text_box(const char* key, oc_arena* arena, oc_ui_text_box_info* info);
ORCA_API oc_ui_text_box_result oc_ui_text_box_str8(oc_str8 key, oc_arena* arena, oc_ui_text_box_info* info);

//------------------------------------------------------------------------
// Select Popup
//------------------------------------------------------------------------
typedef struct oc_ui_select_popup_info
{
    bool changed;
    int selectedIndex; // -1 if nothing is selected
    int optionCount;
    oc_str8* options;
    oc_str8 placeholder;
} oc_ui_select_popup_info;

ORCA_API oc_ui_select_popup_info oc_ui_select_popup(const char* key, oc_ui_select_popup_info* info);
ORCA_API oc_ui_select_popup_info oc_ui_select_popup_str8(oc_str8 key, oc_ui_select_popup_info* info);

//------------------------------------------------------------------------
// Radio Group
//------------------------------------------------------------------------
typedef struct oc_ui_radio_group_info
{
    bool changed;
    int selectedIndex; // -1 if nothing is selected
    int optionCount;
    oc_str8* options;
} oc_ui_radio_group_info;

ORCA_API oc_ui_radio_group_info oc_ui_radio_group(const char* key, oc_ui_radio_group_info* info);
ORCA_API oc_ui_radio_group_info oc_ui_radio_group_str8(oc_str8 key, oc_ui_radio_group_info* info);

#ifdef __cplusplus
} // extern "C"
#endif
