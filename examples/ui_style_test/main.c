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
#include<errno.h>

#define _USE_MATH_DEFINES //NOTE: necessary for MSVC
#include<math.h>

#include"milepost.h"

#define LOG_SUBSYSTEM "Main"

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
	switch(box->targetStyle->size.s[axis].kind)
	{
		case UI_SIZE_TEXT:
			printf("text\n");
			break;

		case UI_SIZE_CHILDREN:
			printf("children\n");
			break;

		case UI_SIZE_PARENT:
			printf("parent\n");
			break;

		case UI_SIZE_PIXELS:
			printf("pixels\n");
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
	str8 fontPath = mp_app_get_resource_path(mem_scratch(), "../resources/OpenSansLatinSubset.ttf");
	char* fontPathCString = str8_to_cstring(mem_scratch(), fontPath);

	FILE* fontFile = fopen(fontPathCString, "r");
	if(!fontFile)
	{
		LOG_ERROR("Could not load font file '%s': %s\n", fontPathCString, strerror(errno));
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

int main()
{
	LogLevel(LOG_LEVEL_WARNING);

	mp_init();
	mp_clock_init(); //TODO put that in mp_init()?

	ui_init();

	mp_rect windowRect = {.x = 100, .y = 100, .w = 810, .h = 610};
	mp_window window = mp_window_create(windowRect, "test", 0);

	mp_rect contentRect = mp_window_get_content_rect(window);

	//NOTE: create surface
	mg_surface surface = mg_surface_create_for_window(window, MG_BACKEND_DEFAULT);
	mg_surface_swap_interval(surface, 0);

	//TODO: create canvas
	mg_canvas canvas = mg_canvas_create(surface);

	if(mg_canvas_is_nil(canvas))
	{
		printf("Error: couldn't create canvas\n");
		return(-1);
	}

	mg_font font = create_font();

	// start app
	mp_window_bring_to_front(window);
	mp_window_focus(window);

	while(!mp_should_quit())
	{
		bool printDebugStyle = false;

		f64 startTime = mp_get_time(MP_CLOCK_MONOTONIC);

		mp_pump_events(0);
		mp_event event = {0};
		while(mp_next_event(&event))
		{
			switch(event.type)
			{
				case MP_EVENT_WINDOW_CLOSE:
				{
					mp_request_quit();
				} break;


				case MP_EVENT_KEYBOARD_KEY:
				{
					if(event.key.action == MP_KEY_PRESS && event.key.code == MP_KEY_P)
					{
						printDebugStyle = true;
					}
				} break;

				default:
					break;
			}
		}

		//TEST UI
		ui_style defaultStyle = {.size.width = {UI_SIZE_CHILDREN},
		                         .size.height = {UI_SIZE_CHILDREN},
		                         .layout.axis = UI_AXIS_Y,
		                         .layout.spacing = 10,
		                         .layout.margin.x = 10,
		                         .layout.margin.y = 10,
		                         .bgColor = {0.9, 0.9, 0.9, 1},
		                         .color = {0, 0, 0, 1},
		                         .borderSize = 2,
		                         .borderColor = {0, 0, 1, 1},
		                         .font = font,
		                         .fontSize = 32};

		ui_flags defaultFlags = UI_FLAG_DRAW_BORDER | UI_FLAG_DRAW_BACKGROUND;

		ui_box* root = 0;
		ui_frame()
		{
			root = ui_box_top();

			ui_pattern pattern = {0};
			ui_pattern_push(mem_scratch(), &pattern, (ui_selector){.kind = UI_SEL_TEXT, .text = STR8("b")});
			ui_pattern_push(mem_scratch(), &pattern, (ui_selector){.kind = UI_SEL_TAG, .tag = ui_tag_make("foo")});
			ui_style_match_before(pattern, &(ui_style){.fontSize = 36}, UI_STYLE_FONT_SIZE);

			ui_style_match_before(ui_pattern_all(),
			                           &defaultStyle,
			                           UI_STYLE_FONT
			                           |UI_STYLE_FONT_SIZE
			                           |UI_STYLE_COLOR
			                           |UI_STYLE_BORDER_SIZE
			                           |UI_STYLE_BORDER_COLOR
			                           |UI_STYLE_SIZE_WIDTH
			                           |UI_STYLE_SIZE_HEIGHT
			                           |UI_STYLE_LAYOUT);


			pattern = (ui_pattern){0};
			ui_pattern_push(mem_scratch(), &pattern, (ui_selector){.kind = UI_SEL_TEXT, .text = STR8("c")});
			ui_pattern_push(mem_scratch(), &pattern, (ui_selector){.kind = UI_SEL_TAG, .tag = ui_tag_make("button")});
			ui_style_match_after(pattern,
			                          &(ui_style){.bgColor = {1, 0.5, 0.5, 1}},
			                          UI_STYLE_BG_COLOR);

			pattern = (ui_pattern){0};
			ui_pattern_push(mem_scratch(), &pattern, (ui_selector){.kind = UI_SEL_TEXT, .text = STR8("c")});
			ui_pattern_push(mem_scratch(), &pattern, (ui_selector){.kind = UI_SEL_TAG, .tag = ui_tag_make("button")});
			ui_pattern_push(mem_scratch(), &pattern, (ui_selector){.kind = UI_SEL_STATUS, .op = UI_SEL_AND, .status = UI_ACTIVE|UI_HOVER});
			ui_style_match_after(pattern,
			                          &(ui_style){.bgColor = {0.5, 1, 0.5, 1}},
			                          UI_STYLE_BG_COLOR);

			ui_style_next(&(ui_style){.bgColor = {0.7, 0.7, 0.7, 1}}, UI_STYLE_BG_COLOR);
			ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 1}, .size.height = {UI_SIZE_PARENT, 1}}, UI_STYLE_SIZE);

			ui_container("a", defaultFlags)
			{
				ui_pattern pattern = {0};
				ui_pattern_push(mem_scratch(), &pattern, (ui_selector){.kind = UI_SEL_TEXT, .text = STR8("b")});
				ui_style_match_before(pattern, &(ui_style){.fontSize = 22}, UI_STYLE_FONT_SIZE);

				ui_container("b", defaultFlags)
				{
					ui_container("c", defaultFlags)
					{
						if(ui_button("button d").clicked)
						{
							printf("clicked button d\n");
						}
					}

					ui_container("e", defaultFlags)
					{
						ui_tag_next("foo");
						if(ui_button("button f").clicked)
						{
							printf("clicked button f\n");
						}

						ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 1},
						                          .size.height = {UI_SIZE_PIXELS, 20, 0}},
						              UI_STYLE_SIZE);
						static f32 slider1 = 0;
						ui_slider("slider1", 0.3, &slider1);


						ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 1},
						                          .size.height = {UI_SIZE_PIXELS, 20, 0}},
						              UI_STYLE_SIZE);
						static f32 slider2 = 0;
						ui_slider("slider2", 0.3, &slider2);

						ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 1},
						                          .size.height = {UI_SIZE_PIXELS, 20}},
						              UI_STYLE_SIZE);
						static f32 slider3 = 0;
						ui_slider("slider3", 0.3, &slider3);


						ui_style_next(&(ui_style){.size.width = {UI_SIZE_PIXELS, 20},
						                          .size.height = {UI_SIZE_PIXELS, 200}},
						              UI_STYLE_SIZE);
						static f32 slider4 = 0;
						ui_slider("slider4", 0.3, &slider4);


					}
				}
				ui_tag_next("foo");
				ui_label("label d");
			}
		}
		if(printDebugStyle)
		{
			debug_print_styles(root, 0);
		}

		mg_surface_prepare(surface);

//		mg_set_color_rgba(1, 0, 0, 1);
//		mg_rectangle_fill(100, 100, 400, 200);

		ui_draw();

/*
		mg_mat2x3 transform = {1, 0, 0,
	                       	0, -1, 800};

		bool oldTextFlip = mg_get_text_flip();
		mg_set_text_flip(true);

		mg_matrix_push(transform);

		mg_set_font(font);
		mg_set_font_size(20);
		mg_set_color_rgba(0, 0, 0, 1);

		mg_move_to(0, 38);
		mg_text_outlines(STR8("hello, world"));
		mg_fill();

		mg_matrix_pop();

		mg_set_text_flip(oldTextFlip);
*/
		mg_flush();
		mg_surface_present(surface);

		mem_arena_clear(mem_scratch());
	}

	mp_terminate();

	return(0);
}
