/************************************************************//**
*
*	@file: main.cpp
*	@author: Martin Fouilleul
*	@date: 30/07/2022
*	@revision:
*
*****************************************************************/
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<errno.h>

#define _USE_MATH_DEFINES //NOTE: necessary for MSVC
#include<math.h>

#include"milepost.h"

void debug_print_indent(int indent)
{
	for(int i=0; i<indent; i++)
	{
		printf("  ");
	}
}

void debug_print_rule(ui_style_rule* rule)
{
	for_list(&rule->pattern.l, selector, ui_selector, listElt)
	{
		switch(selector->kind)
		{
			case UI_SEL_ANY:
				printf("any: ");
				break;

			case UI_SEL_OWNER:
				printf("owner: ");
				break;

			case UI_SEL_TEXT:
				printf("text='%.*s': ", (int)selector->text.len, selector->text.ptr);
				break;

			case UI_SEL_TAG:
				printf("tag=0x%llx: ", selector->tag.hash);
				break;

			case UI_SEL_STATUS:
			{
				if(selector->status & UI_HOVER)
				{
					printf("hover: ");
				}
				if(selector->status & UI_ACTIVE)
				{
					printf("active: ");
				}
				if(selector->status & UI_DRAGGING)
				{
					printf("dragging: ");
				}
			} break;

			case UI_SEL_KEY:
				printf("key=0x%llx: ", selector->key.hash);
				break;

			default:
				printf("unknown: ");
				break;
		}
	}
	printf("=> font size = %f\n", rule->style->fontSize);
}
void debug_print_size(ui_box* box, ui_axis axis, int indent)
{
	debug_print_indent(indent);
	printf("size %s: ", axis == UI_AXIS_X ? "x" : "y");
	f32 value = box->targetStyle->size.c[axis].value;
	switch(box->targetStyle->size.c[axis].kind)
	{
		case UI_SIZE_TEXT:
			printf("text\n");
			break;

		case UI_SIZE_CHILDREN:
			printf("children\n");
			break;

		case UI_SIZE_PARENT:
			printf("parent: %f\n", value);
			break;

		case UI_SIZE_PARENT_MINUS_PIXELS:
			printf("parent minus pixels: %f\n", value);
			break;

		case UI_SIZE_PIXELS:
			printf("pixels: %f\n", value);
			break;
	}

}

void debug_print_styles(ui_box* box, int indent)
{
	debug_print_indent(indent);
	printf("### box '%.*s'\n", (int)box->string.len, box->string.ptr);
	indent++;

	debug_print_indent(indent);
	printf("font size: %f\n", box->targetStyle->fontSize);

	debug_print_size(box, UI_AXIS_X, indent);
	debug_print_size(box, UI_AXIS_Y, indent);

	if(!list_empty(&box->beforeRules))
	{
		debug_print_indent(indent);
		printf("before rules:\n");
		for_list(&box->beforeRules, rule, ui_style_rule, boxElt)
		{
			debug_print_indent(indent+1);
			debug_print_rule(rule);
		}
	}

	if(!list_empty(&box->afterRules))
	{
		debug_print_indent(indent);
		printf("after rules:\n");
		for_list(&box->afterRules, rule, ui_style_rule, boxElt)
		{
			debug_print_indent(indent+1);
			debug_print_rule(rule);
		}
	}

	if(!list_empty(&box->children))
	{
		debug_print_indent(indent);
		printf("children:\n");
		indent++;
		for_list(&box->children, child, ui_box, listElt)
		{
			debug_print_styles(child, indent);
		}
	}
}

mg_font create_font()
{
	//NOTE(martin): create font
	str8 fontPath = path_find_resource(mem_scratch(), STR8("../resources/OpenSansLatinSubset.ttf"));
	char* fontPathCString = str8_to_cstring(mem_scratch(), fontPath);

	FILE* fontFile = fopen(fontPathCString, "r");
	if(!fontFile)
	{
		log_error("Could not load font file '%s': %s\n", fontPathCString, strerror(errno));
		return(mg_font_nil());
	}
	unsigned char* fontData = 0;
	fseek(fontFile, 0, SEEK_END);
	u32 fontDataSize = ftell(fontFile);
	rewind(fontFile);
	fontData = (unsigned char*)malloc(fontDataSize);
	fread(fontData, 1, fontDataSize, fontFile);
	fclose(fontFile);

	unicode_range ranges[5] = {UNICODE_RANGE_BASIC_LATIN,
	                           UNICODE_RANGE_C1_CONTROLS_AND_LATIN_1_SUPPLEMENT,
	                           UNICODE_RANGE_LATIN_EXTENDED_A,
	                           UNICODE_RANGE_LATIN_EXTENDED_B,
	                           UNICODE_RANGE_SPECIALS};

	mg_font font = mg_font_create_from_memory(fontDataSize, fontData, 5, ranges);
	free(fontData);

	return(font);
}

void widget_begin_view(char* str)
{
	ui_style_next(&(ui_style){.layout.axis = UI_AXIS_Y,
	                          .layout.spacing = 10,
	                          .layout.margin.x = 10,
	                          .layout.margin.y = 10,
	                          .layout.align.x = UI_ALIGN_CENTER,
	                          .layout.align.y = UI_ALIGN_START},
	               UI_STYLE_LAYOUT);

	ui_box_begin(str, UI_FLAG_DRAW_BORDER);
	ui_label(str);

}

void widget_end_view(void)
{
	ui_box_end();
}

#define widget_view(s) defer_loop(widget_begin_view(s), widget_end_view())

int main()
{
	mp_init();
	mp_clock_init(); //TODO put that in mp_init()?

	ui_context context;
	ui_init(&context);
	ui_set_context(&context);

	mp_rect windowRect = {.x = 100, .y = 100, .w = 810, .h = 610};
	mp_window window = mp_window_create(windowRect, "test", 0);

	mp_rect contentRect = mp_window_get_content_rect(window);

	//NOTE: create surface
	mg_surface surface = mg_surface_create_for_window(window, MG_CANVAS);
	mg_surface_swap_interval(surface, 0);

	//TODO: create canvas
	mg_canvas canvas = mg_canvas_create();

	if(mg_canvas_is_nil(canvas))
	{
		printf("Error: couldn't create canvas\n");
		return(-1);
	}

	mg_font font = create_font();

	mem_arena textArena = {0};
	mem_arena_init(&textArena);

	// start app
	mp_window_bring_to_front(window);
	mp_window_focus(window);

	while(!mp_should_quit())
	{
		bool printDebugStyle = false;

		f64 startTime = mp_get_time(MP_CLOCK_MONOTONIC);

		mp_pump_events(0);
		mp_event* event = 0;
		while((event = mp_next_event(mem_scratch())) != 0)
		{
			ui_process_event(event);

			switch(event->type)
			{
				case MP_EVENT_WINDOW_CLOSE:
				{
					mp_request_quit();
				} break;


				case MP_EVENT_KEYBOARD_KEY:
				{
					if(event->key.action == MP_KEY_PRESS && event->key.code == MP_KEY_P)
					{
						printDebugStyle = true;
					}
				} break;

				default:
					break;
			}
		}

		//TEST UI
		ui_style defaultStyle = {.bgColor = {0},
		                         .color = {1, 1, 1, 1},
		                         .font = font,
		                         .fontSize = 16,
		                         .borderColor = {1, 0, 0, 1},
			                     .borderSize = 2};

		ui_style_mask defaultMask = UI_STYLE_BG_COLOR
		                          | UI_STYLE_COLOR
		                          | UI_STYLE_BORDER_COLOR
		                          | UI_STYLE_BORDER_SIZE
		                          | UI_STYLE_FONT
		                          | UI_STYLE_FONT_SIZE;

		ui_flags debugFlags = UI_FLAG_DRAW_BORDER;

		ui_box* root = 0;

		mp_rect frameRect = mg_surface_get_frame(surface);
		vec2 frameSize = {frameRect.w, frameRect.h};

		ui_frame(frameSize, &defaultStyle, defaultMask)
		{
			root = ui_box_top();
			ui_style_match_before(ui_pattern_all(), &defaultStyle, defaultMask);

			ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 1},
			                          .size.height = {UI_SIZE_PARENT, 1},
			                          .layout.axis = UI_AXIS_Y,
			                          .layout.align.x = UI_ALIGN_CENTER,
			                          .layout.align.y = UI_ALIGN_START,
			                          .layout.spacing = 10,
			                          .layout.margin.x = 10,
			                          .layout.margin.y = 10,
			                          .bgColor = {0.11, 0.11, 0.11, 1}},
			                UI_STYLE_SIZE
			              | UI_STYLE_LAYOUT
			              | UI_STYLE_BG_COLOR);

			ui_container("background", UI_FLAG_DRAW_BACKGROUND)
			{
				ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 1},
				                          .size.height = {UI_SIZE_CHILDREN},
				                          .layout.align.x = UI_ALIGN_CENTER},
				               UI_STYLE_SIZE
				              |UI_STYLE_LAYOUT_ALIGN_X);
				ui_container("title", debugFlags)
				{
					ui_style_next(&(ui_style){.fontSize = 26}, UI_STYLE_FONT_SIZE);
					ui_label("Milepost UI Demo");

					if(ui_box_sig(ui_box_top()).hovering)
					{
						ui_tooltip("tooltip")
						{
							ui_style_next(&(ui_style){.bgColor = {1, 0.99, 0.82, 1}},
							               UI_STYLE_BG_COLOR);

							ui_container("background", UI_FLAG_DRAW_BACKGROUND)
							{
								ui_style_next(&(ui_style){.color = {0, 0, 0, 1}},
								               UI_STYLE_COLOR);

								ui_label("That is a tooltip!");
							}
						}
					}
				}

				ui_menu_bar("Menu bar")
				{
					ui_menu("Menu 1")
					{
						if(ui_menu_button("Option 1.1").pressed)
						{
							printf("Pressed option 1.1\n");
						}
						ui_menu_button("Option 1.2");
						ui_menu_button("Option 1.3");
						ui_menu_button("Option 1.4");
					}

					ui_menu("Menu 2")
					{
						ui_menu_button("Option 2.1");
						ui_menu_button("Option 2.2");
						ui_menu_button("Option 2.3");
						ui_menu_button("Option 2.4");
					}

					ui_menu("Menu 3")
					{
						ui_menu_button("Option 3.1");
						ui_menu_button("Option 3.2");
						ui_menu_button("Option 3.3");
						ui_menu_button("Option 3.4");
					}
				}

				ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 1},
				                          .size.height = {UI_SIZE_PARENT, 1, 1}},
				                          UI_STYLE_SIZE);

				ui_style_next(&(ui_style){.layout.axis = UI_AXIS_X}, UI_STYLE_LAYOUT_AXIS);
				ui_container("contents", debugFlags)
				{
					ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 0.5},
					                          .size.height = {UI_SIZE_PARENT, 1},
					                          .borderColor = {0, 0, 1, 1}},
					              UI_STYLE_SIZE
					              |UI_STYLE_BORDER_COLOR);

					ui_container("left", debugFlags)
					{
						ui_style_next(&(ui_style){.layout.axis = UI_AXIS_X,
						                          .layout.spacing = 10,
						                          .layout.margin.x = 10,
						                          .layout.margin.y = 10,
						                          .size.width = {UI_SIZE_PARENT, 1},
						                          .size.height = {UI_SIZE_PARENT, 0.5}},
						               UI_STYLE_LAYOUT_AXIS
						              |UI_STYLE_LAYOUT_SPACING
						              |UI_STYLE_LAYOUT_MARGIN_X
						              |UI_STYLE_LAYOUT_MARGIN_Y
						              |UI_STYLE_SIZE);

						ui_container("up", debugFlags)
						{
							ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 0.5},
							                          .size.height = {UI_SIZE_PARENT, 1}},
							             UI_STYLE_SIZE);
							widget_view("Buttons")
							{
								if(ui_button("Test Dialog").clicked)
								{
									char* options[] = {"Accept", "Reject"};
									int res = mp_alert_popup("test dialog", "dialog message", 2, options);
									if(res >= 0)
									{
										printf("selected options %i: %s\n", res, options[res]);
									}
									else
									{
										printf("no options selected\n");
									}
								}

								if(ui_button("Open").clicked)
								{
									char* filters[] = {"md"};
									str8 file = mp_open_dialog(mem_scratch(), "Open File", "C:\\Users", 1, filters, false);
									printf("selected file %.*s\n", (int)file.len, file.ptr);
								}

								if(ui_button("Save").clicked)
								{
									str8 file = mp_save_dialog(mem_scratch(), "Save File", "C:\\Users", 0, 0);
									printf("selected file %.*s\n", (int)file.len, file.ptr);
								}
							}

							ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 0.5},
							                          .size.height = {UI_SIZE_PARENT, 1}},
							             UI_STYLE_SIZE);


							ui_pattern pattern = {0};
							ui_pattern_push(mem_scratch(), &pattern, (ui_selector){.kind = UI_SEL_TAG, .tag = ui_tag_make("checkbox")});
							ui_style_match_after(pattern,
							                     &(ui_style){.bgColor = {0, 1, 0, 1},
							                                 .color = {1, 1, 1, 1}},
							                     UI_STYLE_COLOR | UI_STYLE_BG_COLOR);

							widget_view("checkboxes")
							{
								static bool check1 = true;
								static bool check2 = false;
								static bool check3 = false;

								ui_checkbox("check1", &check1);
								ui_checkbox("check2", &check2);
								ui_checkbox("check3", &check3);
							}
						}

						ui_style_next(&(ui_style){.layout.axis = UI_AXIS_X,
						                          .size.width = {UI_SIZE_PARENT, 1},
						                          .size.height = {UI_SIZE_PARENT, 0.5}},
						               UI_STYLE_LAYOUT_AXIS
						              |UI_STYLE_SIZE);

						ui_container("down", debugFlags)
						{
							widget_view("Vertical Sliders")
							{
								ui_style_next(&(ui_style){.layout.axis = UI_AXIS_X,
								                          .layout.spacing = 10},
								               UI_STYLE_LAYOUT_AXIS
								              |UI_STYLE_LAYOUT_SPACING);
								ui_container("contents", 0)
								{
									ui_style_next(&(ui_style){.size.width = {UI_SIZE_PIXELS, 20},
								                          	.size.height = {UI_SIZE_PIXELS, 200}},
								              	UI_STYLE_SIZE);
									static f32 slider1 = 0;
									ui_slider("slider1", 0.2, &slider1);

									ui_style_next(&(ui_style){.size.width = {UI_SIZE_PIXELS, 20},
								                          	.size.height = {UI_SIZE_PIXELS, 200}},
								              	UI_STYLE_SIZE);
									static f32 slider2 = 0;
									ui_slider("slider2", 0.2, &slider2);

									ui_style_next(&(ui_style){.size.width = {UI_SIZE_PIXELS, 20},
								                          	.size.height = {UI_SIZE_PIXELS, 200}},
								              	UI_STYLE_SIZE);
									static f32 slider3 = 0;
									ui_slider("slider3", 0.2, &slider3);
								}
							}

							widget_view("Horizontal Sliders")
							{
								ui_style_next(&(ui_style){.size.width = {UI_SIZE_PIXELS, 200},
								                          .size.height = {UI_SIZE_PIXELS, 20}},
								              UI_STYLE_SIZE);
								static f32 slider1 = 0;
								ui_slider("slider1", 0.2, &slider1);

								ui_style_next(&(ui_style){.size.width = {UI_SIZE_PIXELS, 200},
								                          .size.height = {UI_SIZE_PIXELS, 20}},
								              UI_STYLE_SIZE);
								static f32 slider2 = 0;
								ui_slider("slider2", 0.2, &slider2);

								ui_style_next(&(ui_style){.size.width = {UI_SIZE_PIXELS, 200},
								                          .size.height = {UI_SIZE_PIXELS, 20}},
								              UI_STYLE_SIZE);
								static f32 slider3 = 0;
								ui_slider("slider3", 0.2, &slider3);
							}
						}
					}

					ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 0.5},
					                          .size.height = {UI_SIZE_PARENT, 1}},
					              UI_STYLE_SIZE);

					ui_container("right", debugFlags)
					{

						ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 1},
						                          .size.height = {UI_SIZE_PARENT, 0.33}},
						             UI_STYLE_SIZE);
						widget_view("Text box")
						{
							ui_style_next(&(ui_style){.size.width = {UI_SIZE_PIXELS, 300},
					                          		.size.height = {UI_SIZE_TEXT}},
					              		UI_STYLE_SIZE);
							static str8 text = {0};
							ui_text_box_result res = ui_text_box("textbox", mem_scratch(), text);
							if(res.changed)
							{
								mem_arena_clear(&textArena);
								text = str8_push_copy(&textArena, res.text);
							}
						}

						ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 1},
						                          .size.height = {UI_SIZE_PARENT, 0.33}},
						             UI_STYLE_SIZE);
						widget_view("Test")
						{
							ui_pattern pattern = {0};
							ui_pattern_push(mem_scratch(), &pattern, (ui_selector){.kind = UI_SEL_TEXT, .text = STR8("panel")});
							ui_style_match_after(pattern, &(ui_style){.bgColor = {0.3, 0.3, 1, 1}}, UI_STYLE_BG_COLOR);

							static int selected = 0;
							str8 options[] = {STR8("option 1"),
							                  STR8("option 2"),
							                  STR8("long option 3"),
							                  STR8("option 4"),
							                  STR8("option 5")};
							ui_select_popup_info info = {.selectedIndex = selected,
							                             .optionCount = 5,
							                             .options = options};

							ui_select_popup_info result = ui_select_popup("popup", &info);
							selected = result.selectedIndex;
						}

						ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 1},
						                          .size.height = {UI_SIZE_PARENT, 0.33}},
						             UI_STYLE_SIZE);
						widget_view("Color")
						{
							ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 1},
							                          .size.height = {UI_SIZE_PARENT, 0.7},
							                          .layout.axis = UI_AXIS_X},
							               UI_STYLE_SIZE
							              |UI_STYLE_LAYOUT_AXIS);

							ui_panel("Panel", UI_FLAG_DRAW_BORDER)
							{
								ui_style_next(&(ui_style){.layout.axis = UI_AXIS_X},
							                  UI_STYLE_LAYOUT_AXIS);
								ui_container("contents", 0)
								{
									ui_style_next(&(ui_style){.layout.spacing = 20},
							                  	UI_STYLE_LAYOUT_SPACING);
							    	ui_container("buttons", 0)
							    	{
										ui_button("Button A");
										ui_button("Button B");
										ui_button("Button C");
										ui_button("Button D");
									}

									ui_style_next(&(ui_style){.layout.axis = UI_AXIS_X,
								                          	.layout.spacing = 20},
							                  	UI_STYLE_LAYOUT_SPACING
							                  	|UI_STYLE_LAYOUT_AXIS);

									ui_container("buttons2", 0)
									{
										ui_button("Button A");
										ui_button("Button B");
										ui_button("Button C");
										ui_button("Button D");
									}
								}
							}
						}

					}
				}
			}
		}
		if(printDebugStyle)
		{
			debug_print_styles(root, 0);
		}

		mg_surface_prepare(surface);

		ui_draw();

		mg_render(surface, canvas);
		mg_surface_present(surface);

		mem_arena_clear(mem_scratch());
	}

	mg_surface_destroy(surface);
	mp_terminate();

	return(0);
}
