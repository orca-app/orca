/************************************************************//**
*
*	@file: main.cpp
*	@author: Martin Fouilleul
*	@date: 30/07/2022
*	@revision:
*
*****************************************************************/
#include<stdlib.h>
#include"milepost.h"

#define LOG_SUBSYSTEM "Main"

int main()
{
	LogLevel(LOG_LEVEL_DEBUG);

	mp_init();

	mp_rect rect = {.x = 100, .y = 100, .w = 800, .h = 600};
	mp_window window = mp_window_create(rect, "test", 0);

	mg_init();
	ui_init();
/*
	mp_rect frame = {0, 0, 800, 600};
	mp_view view = mp_view_create(window, frame);
	mg_surface surface = mg_surface_create_for_view(view, MG_BACKEND_METAL);
/*/
	mg_surface surface = mg_surface_create_for_window(window, MG_BACKEND_METAL);
//*/
	mg_canvas canvas = mg_canvas_create(surface, (mp_rect){0, 0, 800, 600});
	mg_image image = mg_image_create_from_file(canvas, str8_lit("Top512.png"), true);

	u8 colors[64];
	for(int i=0; i<64; i+=4)
	{
		colors[i] = 255;
		colors[i+1] = 0;
		colors[i+2] = 0;
		colors[i+3] = 255;
	}
	mg_image image3 = mg_image_create_from_rgba8(canvas, 4, 4, colors);

	mg_image image2 = mg_image_create_from_file(canvas, str8_lit("triceratops.png"), true);

	//NOTE(martin): create font
	char* fontPath = 0;
	mp_app_get_resource_path("../resources/Andale Mono.ttf", &fontPath);
	FILE* fontFile = fopen(fontPath, "r");
	free(fontPath);
	if(!fontFile)
	{
		LOG_ERROR("Could not load font file '%s'\n", fontPath);
		return(-1);
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

	mp_window_bring_to_front_and_focus(window);

	mp_pump_events(-1);
	while(!mp_should_quit())
	{
		mp_event event = {};
		mp_pump_events(0);
		while(mp_next_event(&event))
		{
			switch(event.type)
			{
				case MP_EVENT_KEYBOARD_CHAR:
				{
					printf("entered char %s\n", event.character.sequence);
				} break;

				case MP_EVENT_WINDOW_CLOSE:
				{
					mp_do_quit();
				} break;

				case MP_EVENT_WINDOW_RESIZE:
				{
					/*
					mp_rect frame = {0, 0, event.frame.rect.w, event.frame.rect.h};
					mp_view_set_frame(view, frame);
					*/
					mg_canvas_viewport(canvas, (mp_rect){0, 0, event.frame.rect.w, event.frame.rect.h});

				} break;

				case MP_EVENT_FRAME:
				{
				} break;

				default:
					break;
			}
		}
		mg_surface_prepare(surface);

		mg_set_color_rgba(canvas, 1, 1, 1, 1);
		mg_clear(canvas);

		mg_move_to(canvas, 0, 0);
		mg_line_to(canvas, 400, 300);
		mg_set_width(canvas, 5);
		mg_set_color_rgba(canvas, 1, 0, 0, 1);
		mg_stroke(canvas);

		mg_image_draw(canvas, image, (mp_rect){400, 300, 200, 200});

		mg_rounded_image_draw(canvas, image2, (mp_rect){200, 20, 200, 200}, 50);

		mg_set_color_rgba(canvas, 0, 1, 0, 0.5);
		mg_rectangle_fill(canvas, 800, 800, 400, 200);

		mg_set_font(canvas, font);
		mg_set_font_size(canvas, 32);
		mg_move_to(canvas, 500, 500);
		mg_text_outlines(canvas, str8_lit("Hello, world!"));
		mg_set_color_rgba(canvas, 0, 0, 0, 1);
		mg_fill(canvas);

		mp_rect windowRect = mp_window_get_content_rect(window);

		ui_style defaultStyle = {.borderSize = 2,
		                         .borderColor = {0, 0, 1, 1},
		                         .textColor = {0, 0, 0, 1},
		                         .font = font,
		                         .fontSize = 32};

		ui_style buttonStyle[UI_STYLE_SELECTOR_COUNT];
		for(int i=0; i<UI_STYLE_SELECTOR_COUNT; i++)
		{
			buttonStyle[i] = (ui_style){ .backgroundColor = {0.8, 0.8, 0.8, 1},
			                             .foregroundColor = {0.5, 0.5, 0.5, 1},
			                             .borderSize = 2,
			                             .borderColor = {0, 0, 1, 1},
			                             .textColor = {0, 0, 0, 1},
			                             .font = font,
			                             .fontSize = 32 };
		}
		buttonStyle[UI_STYLE_HOT].borderColor = (mg_color){1, 0, 0, 1};
		buttonStyle[UI_STYLE_ACTIVE].borderColor = (mg_color){1, 0, 0, 1};
		buttonStyle[UI_STYLE_ACTIVE].foregroundColor = (mg_color){0.1, 0.1, 0.1, 1};

		ui_begin_frame(windowRect.w, windowRect.h, defaultStyle);
		{
			ui_size_push(UI_AXIS_X, UI_SIZE_PIXELS, 600, 0);
			ui_size_push(UI_AXIS_Y, UI_SIZE_PIXELS, 200, 0);

			for(int i=0; i<UI_STYLE_SELECTOR_COUNT; i++)
			{
				ui_style_push(i, buttonStyle[i]);
			}

			ui_menu_bar("menu")
			{
				ui_menu("File")
				{
					ui_button("Open");
					ui_button("Save");
					ui_button("Save As");
				}

				ui_menu("Edit")
				{
					ui_button("Cut");
					ui_button("Copy");
					ui_button("Paste");
				}

				ui_menu("View")
				{
					ui_button("History");
					ui_button("Bookmarks");
				}
			}

			ui_box* container = ui_box_begin("Test", UI_FLAG_DRAW_BORDER | UI_FLAG_DRAW_BACKGROUND);
			ui_box_set_layout(container, UI_AXIS_X, UI_ALIGN_END, UI_ALIGN_CENTER);
			{
				ui_size_push(UI_AXIS_X, UI_SIZE_TEXT, 0, 0);
				ui_size_push(UI_AXIS_Y, UI_SIZE_PARENT_RATIO, 0.25, 0);

				ui_sig sig1 = ui_button("Child1");
				if(sig1.hovering)
				{
					ui_tooltip("tip")
					{
						ui_label("Click me!");
					}
				}
				if(sig1.triggered)
				{
					printf("clicked child 1!\n");
				}
				if(ui_button("Child2").triggered)
				{
					printf("clicked child 2!\n");
				}

				ui_box* block = ui_box_make("block", UI_FLAG_DRAW_BACKGROUND | UI_FLAG_DRAW_BORDER | UI_FLAG_DRAW_TEXT | UI_FLAG_BLOCK_MOUSE);
				ui_box_set_size(block, UI_AXIS_X, UI_SIZE_TEXT, 0, 0);
				ui_box_set_size(block, UI_AXIS_Y, UI_SIZE_TEXT, 0, 0);
				ui_box_set_floating(block, UI_AXIS_X, 100);
				ui_box_set_floating(block, UI_AXIS_Y, 100);

				ui_size_push(UI_AXIS_X, UI_SIZE_PIXELS, 200, 0);
				ui_size_push(UI_AXIS_Y, UI_SIZE_PIXELS, 20, 0);

				static f32 scrollValue = 0.;
				ui_scrollbar("scroll", .25, &scrollValue);

				ui_size_pop(UI_AXIS_X);
				ui_size_pop(UI_AXIS_Y);

				ui_size_pop(UI_AXIS_X);
				ui_size_pop(UI_AXIS_Y);

			} ui_box_end();

			ui_size_pop(UI_AXIS_X);
			ui_size_pop(UI_AXIS_Y);

			ui_size_push(UI_AXIS_X, UI_SIZE_PIXELS, 200, 0);
			ui_size_push(UI_AXIS_Y, UI_SIZE_PIXELS, 200, 0);
			ui_panel_begin("panel");

			ui_size_push(UI_AXIS_X, UI_SIZE_TEXT, 0, 0);
			ui_size_push(UI_AXIS_Y, UI_SIZE_TEXT, 0, 0);

			ui_box* b = ui_box_begin("container", 0);
			ui_box_set_size(b, UI_AXIS_X, UI_SIZE_CHILDREN, 0, 0);
			ui_box_set_size(b, UI_AXIS_Y, UI_SIZE_PIXELS, 300, 0);
				ui_button("Hello 1");
				ui_button("Hello 2");
				ui_button("Hello 3");
			ui_box_end();

			ui_size_pop(UI_AXIS_X);
			ui_size_pop(UI_AXIS_Y);

			ui_panel_end();
			ui_size_pop(UI_AXIS_X);
			ui_size_pop(UI_AXIS_Y);

			for(int i=0; i<UI_STYLE_SELECTOR_COUNT; i++)
			{
				ui_style_pop(i);
			}

		} ui_end_frame();

		ui_draw(canvas);

		mg_canvas_flush(canvas);

		mg_surface_present(surface);
	}

	mg_surface_destroy(surface);
	mp_terminate();
	return(0);
}
