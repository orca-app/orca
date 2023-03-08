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
	switch(box->targetStyle->size.c[axis].kind)
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


void panel_begin(char* str, ui_flags flags)
{
	ui_box* box = ui_box_begin(str, flags | UI_FLAG_CLIP | UI_FLAG_BLOCK_MOUSE);

	ui_style contentsStyle = {
	    .size.width = {UI_SIZE_CHILDREN},
	    .size.height = {UI_SIZE_CHILDREN},
		.floating.x = true,
		.floating.y = true,
		.floatTarget = {-box->scroll.x, -box->scroll.y}};

	ui_style_next(&contentsStyle, UI_STYLE_FLOAT);
	ui_box_begin("contents", 0);

}

void panel_end(void)
{
	ui_box* contents = ui_box_top();
	ui_box_end();

	ui_box* panel = ui_box_top();
	ui_sig sig = ui_box_sig(panel);

	f32 contentsW = ClampLowBound(contents->rect.w, panel->rect.w);
	f32 contentsH = ClampLowBound(contents->rect.h, panel->rect.h);

	contentsW = ClampLowBound(contentsW, 1);
	contentsH = ClampLowBound(contentsH, 1);

	ui_box* scrollBarX = 0;
	ui_box* scrollBarY = 0;

	if(contentsW > panel->rect.w)
	{
		f32 thumbRatioX = panel->rect.w / contentsW;
		f32 sliderX = panel->scroll.x /(contentsW - panel->rect.w);

		ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 1., 0},
		                          .size.height = {UI_SIZE_PIXELS, 10, 0},
		                          .floating.x = true,
		                          .floating.y = true,
		                          .floatTarget = {0, panel->rect.h - 12}},
		              UI_STYLE_SIZE);

		scrollBarX = ui_slider("scrollerX", thumbRatioX, &sliderX);

		panel->scroll.x = sliderX * (contentsW - panel->rect.w);
		if(sig.hovering)
		{
			panel->scroll.x += sig.wheel.x;
			ui_box_activate(scrollBarX);
		}
	}

	if(contentsH > panel->rect.h)
	{
		f32 thumbRatioY = panel->rect.h / contentsH;
		f32 sliderY = panel->scroll.y /(contentsH - panel->rect.h);

		ui_style_next(&(ui_style){.size.width = {UI_SIZE_PIXELS, 10, 0},
		                          .size.height = {UI_SIZE_PARENT, 1., 0},
		                          .floating.x = true,
		                          .floating.y = true,
		                          .floatTarget = {panel->rect.w - 12, 0}},
		               UI_STYLE_SIZE
		              |UI_STYLE_FLOAT);

		scrollBarY = ui_slider("scrollerY", thumbRatioY, &sliderY);

		panel->scroll.y = sliderY * (contentsH - panel->rect.h);
		if(sig.hovering)
		{
			panel->scroll.y += sig.wheel.y;
			ui_box_activate(scrollBarY);
		}
	}

	panel->scroll.x = Clamp(panel->scroll.x, 0, contentsW - panel->rect.w);
	panel->scroll.y = Clamp(panel->scroll.y, 0, contentsH - panel->rect.h);

	if(scrollBarX)
	{
//		ui_box_set_floating(scrollBarX, UI_AXIS_X, panel->scroll.x);
//		ui_box_set_floating(scrollBarX, UI_AXIS_Y, panel->scroll.y + panel->rect.h - 12);
	}

	if(scrollBarY)
	{
//		ui_box_set_floating(scrollBarY, UI_AXIS_X, panel->scroll.x + panel->rect.w - 12);
//		ui_box_set_floating(scrollBarY, UI_AXIS_Y, panel->scroll.y);
	}

	ui_box_end();
}

#define panel(s, f) defer_loop(panel_begin(s, f), panel_end())

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
		ui_frame()
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
				}

				ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 1},
				                          .size.height = {UI_SIZE_PIXELS, 500}},
				                          UI_STYLE_SIZE);

				ui_style_next(&(ui_style){.layout.axis = UI_AXIS_X}, UI_STYLE_LAYOUT_AXIS);
				ui_container("contents", debugFlags)
				{
					ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 0.5},
					                          .size.height = {UI_SIZE_PARENT, 1}},
					              UI_STYLE_SIZE);

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
								ui_button("Button 1");
								ui_button("Button 2");
								ui_button("Button 3");
							}

							ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 0.5},
							                          .size.height = {UI_SIZE_PARENT, 1}},
							             UI_STYLE_SIZE);
							widget_view("checkboxes")
							{

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
							static str8 text = {};
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
						widget_view("Menus")
						{}

						ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 1},
						                          .size.height = {UI_SIZE_PARENT, 0.33}},
						             UI_STYLE_SIZE);
						widget_view("Color")
						{
							ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 1},
							                          .size.height = {UI_SIZE_PARENT, 0.7}},
							              UI_STYLE_SIZE);

							panel("Panel", UI_FLAG_DRAW_BORDER)
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
							}
						}
					}
				}

			}

			/*
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
				ui_style_next(&(ui_style){.layout.axis = UI_AXIS_X}, UI_STYLE_LAYOUT_AXIS);
				ui_container("f", defaultFlags)
				{
					ui_tag_next("foo");
					ui_label("label d");

					ui_style_next(&(ui_style){.size.width = {UI_SIZE_PIXELS, 300},
					                          .size.height = {UI_SIZE_TEXT}},
					              UI_STYLE_SIZE);
					static str8 text = {};
					ui_text_box_result res = ui_text_box("textbox", mem_scratch(), text);
					if(res.changed)
					{
						mem_arena_clear(&textArena);
						text = str8_push_copy(&textArena, res.text);
					}
				}
			}
			*/
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
