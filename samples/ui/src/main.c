#include"orca.h"

vec2 frameSize = {100, 100};

mg_surface surface;
mg_canvas canvas;
mg_font font;
ui_context ui;
mem_arena textArena = {0};

ORCA_EXPORT void OnInit(void)
{
	//TODO create surface for main window
	surface = mg_surface_canvas();
	canvas = mg_canvas_create();
	ui_init(&ui);

	//NOTE: load font
	{
		file_handle file = file_open(STR8("/OpenSansLatinSubset.ttf"), FILE_ACCESS_READ, 0);
		if(file_last_error(file) != IO_OK)
		{
			log_error("Couldn't open file OpenSansLatinSubset.ttf\n");
		}
		u64 size = file_size(file);
		char* buffer = mem_arena_alloc(mem_scratch(), size);
		file_read(file, size, buffer);
		file_close(file);
		unicode_range ranges[5] = {UNICODE_RANGE_BASIC_LATIN,
		                       UNICODE_RANGE_C1_CONTROLS_AND_LATIN_1_SUPPLEMENT,
		                       UNICODE_RANGE_LATIN_EXTENDED_A,
		                       UNICODE_RANGE_LATIN_EXTENDED_B,
		                       UNICODE_RANGE_SPECIALS};
		// TODO: Decide whether we're using strings or explicit pointer + length
		font = mg_font_create_from_memory(size, (byte*)buffer, 5, ranges);
	}

	mem_arena_clear(mem_scratch());
	mem_arena_init(&textArena);
}

ORCA_EXPORT void OnFrameResize(u32 width, u32 height)
{
	log_info("frame resize %u, %u", width, height);
	frameSize.x = width;
	frameSize.y = height;
}

ORCA_EXPORT void OnRawEvent(mp_event *event)
{
	ui_process_event(event);
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

ORCA_EXPORT void OnFrameRefresh(void)
{
	ui_style defaultStyle = {.bgColor = {0},
	                         .color = {1, 1, 1, 1},
	                         .font = font,
	                         .fontSize = 16,
	                         .borderColor = {0.278, 0.333, 0.412, 1},
		                     .borderSize = 2};

	ui_style_mask defaultMask = UI_STYLE_BG_COLOR
	                          | UI_STYLE_COLOR
	                          | UI_STYLE_BORDER_COLOR
	                          | UI_STYLE_BORDER_SIZE
	                          | UI_STYLE_FONT
	                          | UI_STYLE_FONT_SIZE;

	ui_frame(frameSize, &defaultStyle, defaultMask)
	{
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
			ui_container("title", 0)
			{
				ui_style_next(&(ui_style){.fontSize = 26}, UI_STYLE_FONT_SIZE);
				ui_label("Orca UI Demo");

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
						log_info("Pressed option 1.1\n");
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
			ui_container("contents", 0)
			{
				ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 0.5},
				                          .size.height = {UI_SIZE_PARENT, 1}},
				              UI_STYLE_SIZE);

				ui_container("left", 0)
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

					ui_container("up", 0)
					{
						ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 0.5},
						                          .size.height = {UI_SIZE_PARENT, 1}},
						             UI_STYLE_SIZE);
						widget_view("Buttons")
						{
							if(ui_button("Button A").clicked)
							{
								log_info("A clicked");
							}

							if(ui_button("Button B").clicked)
							{
								log_info("B clicked");
							}

							if(ui_button("Button C").clicked)
							{
								log_info("C clicked");
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

					ui_container("down", 0)
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

				ui_container("right", 0)
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


	mg_canvas_set_current(canvas);
    mg_surface_prepare(surface);
	ui_draw();
    mg_render(surface, canvas);
    mg_surface_present(surface);
}
