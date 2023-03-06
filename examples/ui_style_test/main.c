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
	for_each_in_list(&rule->pattern.l, selector, ui_selector, listElt)
	{
		switch(selector->kind)
		{
			case UI_SEL_TEXT:
				printf("text='%.*s': ", (int)selector->text.len, selector->text.ptr);
				break;

			case UI_SEL_TAG:
				printf("tag=0x%llx: ", selector->tag.hash);
				break;

			default:
				printf("unknown: ");
				break;
		}
	}
	printf("=> font size = %f\n", rule->style->fontSize);
}

void debug_print_styles(ui_box* box, int indent)
{
	debug_print_indent(indent);
	printf("### box '%.*s'\n", (int)box->string.len, box->string.ptr);
	indent++;

	debug_print_indent(indent);
	printf("font size: %f\n", box->targetStyle->fontSize);

	if(!ListEmpty(&box->beforeRules))
	{
		debug_print_indent(indent);
		printf("before rules:\n");
		for_each_in_list(&box->beforeRules, rule, ui_style_rule, boxElt)
		{
			debug_print_indent(indent+1);
			debug_print_rule(rule);
		}
	}

	if(!ListEmpty(&box->afterRules))
	{
		debug_print_indent(indent);
		printf("after rules:\n");
		for_each_in_list(&box->afterRules, rule, ui_style_rule, boxElt)
		{
			debug_print_indent(indent+1);
			debug_print_rule(rule);
		}
	}

	if(!ListEmpty(&box->children))
	{
		debug_print_indent(indent);
		printf("children:\n");
		indent++;
		for_each_in_list(&box->children, child, ui_box, listElt)
		{
			debug_print_styles(child, indent);
		}
	}
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


	//TEST UI
	ui_style defaultStyle = {.bgColor = {0.9, 0.9, 0.9, 1},
		                        .borderSize = 2,
		                        .borderColor = {0, 0, 1, 1},
		                        .color = {0, 0, 0, 1},
		                        .fontSize = 32};

	ui_box* root = 0;
	ui_frame(800, 800, defaultStyle)
	{
		root = ui_box_top();

		ui_pattern pattern = {0};
		ui_pattern_push(mem_scratch(), &pattern, (ui_selector){.kind = UI_SEL_TEXT, .text = str8_lit("b")});
		ui_pattern_push(mem_scratch(), &pattern, (ui_selector){.kind = UI_SEL_TAG, .tag = ui_tag_make(str8_lit("foo"))});
		ui_style_next(pattern,
		              &(ui_style){.fontSize = 12},
		              UI_STYLE_FONT_SIZE);

		ui_container("a", 0)
		{
			ui_pattern pattern = {0};
			ui_pattern_push(mem_scratch(), &pattern, (ui_selector){.kind = UI_SEL_TEXT, .text = str8_lit("b")});
			ui_style_next(pattern,
			          	  &(ui_style){.fontSize = 20},
			          	  UI_STYLE_FONT_SIZE);

			ui_container("b", 0)
			{
				ui_container("c", 0)
				{
					ui_box_make("d", 0);
				}

				ui_container("e", 0)
				{
					ui_tag_next(str8_lit("foo"));
					ui_box_make("f", 0);
				}
			}
			ui_tag_next(str8_lit("foo"));
			ui_box_make("d", 0);
		}

		pattern = (ui_pattern){0};
		ui_pattern_push(mem_scratch(), &pattern, (ui_selector){.kind = UI_SEL_TEXT, .text = str8_lit("c")});
		ui_pattern_push(mem_scratch(), &pattern, (ui_selector){.kind = UI_SEL_TEXT, .text = str8_lit("d")});
		ui_style_prev(pattern,
		              &(ui_style){.fontSize = 30},
		              UI_STYLE_FONT_SIZE);
	}
	debug_print_styles(root, 0);

	mp_terminate();

	return(0);
}
