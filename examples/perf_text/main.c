
#include<stdio.h>
#include<stdlib.h>

#define LOG_DEFAULT_LEVEL LOG_LEVEL_MESSAGE
#define LOG_COMPILE_DEBUG

#include"milepost.h"

#define LOG_SUBSYSTEM "Main"

static const char* TEST_STRING =
"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nulla quam enim, aliquam in placerat luctus, rutrum in quam. "
"Cras urna elit, pellentesque ac ipsum at, lobortis scelerisque eros. Aenean et turpis nibh. Maecenas lectus augue, eleifend "
"nec efficitur eu, faucibus eget turpis. Suspendisse vel nulla mi. Duis imperdiet neque orci, ac ultrices orci molestie a. "
"Etiam malesuada vulputate hendrerit. Cras ultricies diam in lectus finibus, eu laoreet diam rutrum.\n"
"\n"
"Etiam dictum orci arcu, ac fermentum leo dapibus lacinia. Integer vitae elementum ex. Vestibulum tempor nunc eu hendrerit "
"ornare. Nunc pretium ligula sit amet massa pulvinar, vitae imperdiet justo bibendum. Maecenas consectetur elementum mi, sed "
"vehicula neque pulvinar sit amet. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nunc tortor erat, accumsan in laoreet "
"quis, placerat nec enim. Nulla facilisi. Morbi vitae nibh ligula. Suspendisse in molestie magna, eget aliquet mauris. Sed  "
"aliquam faucibus magna.\n"
"\n"
"Sed metus odio, imperdiet et consequat non, faucibus nec risus. Suspendisse facilisis sem neque, id scelerisque dui mattis sit "
"amet. Nullam tincidunt nisl nec dui dignissim mattis. Proin fermentum ornare ipsum. Proin eleifend, mi vitae porttitor placerat, "
"neque magna elementum turpis, eu aliquet mi urna et leo. Pellentesque interdum est mauris, sed pellentesque risus blandit in. "
"Phasellus dignissim consequat eros, at aliquam elit finibus posuere. Proin suscipit tortor leo, id vulputate odio lobortis in. "
"Vestibulum et orci ligula. Sed scelerisque nunc non nisi aliquam, vel eleifend felis suscipit. Integer posuere sapien elit,  "
"lacinia ultricies nibh sodales nec.\n"
"\n"
"Etiam aliquam purus sit amet purus ultricies tristique. Nunc maximus nunc quis magna ornare, vel interdum urna fermentum. "
"Vestibulum cursus nisl ut nulla egestas, quis mattis elit venenatis. Praesent malesuada mi non magna aliquam fringilla eget eu "
"turpis. Integer suscipit elit vel consectetur vulputate. Integer euismod, erat eget elementum tempus, magna metus consectetur "
"elit, sed feugiat urna sapien sodales sapien. Sed sit amet varius nunc. Curabitur sodales nunc justo, ac scelerisque ipsum semper "
"eget. Integer ornare, velit ut hendrerit dapibus, erat mauris commodo justo, vel semper urna justo non mauris. Proin blandit, "
"enim ut posuere placerat, leo nibh tristique eros, ut pulvinar sapien elit eget enim. Pellentesque et mauris lectus. Curabitur "
"quis lobortis leo, sit amet egestas dui. Nullam ut sapien eu justo lacinia ultrices. Ut tincidunt, sem non luctus tempus, felis "
"purus imperdiet nisi, non ultricies libero ipsum eu augue. Mauris at luctus enim.\n"
"\n"
"Aliquam sed tortor a justo pulvinar dictum consectetur eu felis. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices "
"posuere cubilia curae; Etiam vehicula porttitor volutpat. Morbi fringilla tortor nec accumsan aliquet. Aliquam in commodo neque. "
"Sed laoreet tellus in consectetur aliquet. Nullam nibh eros, feugiat sit amet aliquam non, malesuada vel urna. Ut vel egestas nunc. "
"Pellentesque vitae ante quis ante pharetra pretium. Nam quis eros commodo, mattis enim sed, finibus ante. Quisque lacinia tortor ut "
"odio laoreet, vel viverra libero porttitor. Vestibulum vitae dapibus ex. Phasellus varius lorem sed justo sollicitudin faucibus. "
"Etiam aliquam lacinia consectetur. Phasellus nulla ipsum, viverra non nulla in, rhoncus posuere nunc.\n"
"\n"
"Phasellus efficitur commodo tellus, eget lobortis erat porta quis. Aenean condimentum tortor ut neque dapibus, vitae vulputate quam "
"condimentum. Aliquam elementum vitae nulla vitae tristique. Suspendisse feugiat turpis ac magna dapibus, ut blandit diam tincidunt. "
"Integer id dui id enim ullamcorper dictum. Maecenas malesuada vitae ex pharetra iaculis. Curabitur eu dolor consectetur, tempus augue "
"sed, finibus est. Nulla facilisi. Vivamus sed lacinia turpis, in gravida dolor. Aenean interdum consectetur enim a malesuada. Sed turpis "
"nisi, lacinia et fermentum nec, pharetra id dui. Vivamus neque ligula, iaculis sed tempor eget, vehicula blandit quam. Morbi rhoncus quam "
"semper magna mollis luctus. Donec eu dolor ut ante ullamcorper porta. Mauris et est tristique libero pharetra faucibus.\n"
"\n"
"Duis ut elementum sem. Praesent commodo erat nec sem ultricies sollicitudin. Suspendisse a pellentesque sapien. Nunc ac magna a dui "
"elementum luctus non a mi. Cras elementum nunc sed nunc gravida, sit amet accumsan tortor pulvinar. Etiam elit arcu, pellentesque non ex "
"id, vestibulum pellentesque velit. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque habitant morbi tristique senectus "
"et netus et malesuada fames ac turpis egestas. Proin sit amet velit eget tellus vulputate sagittis eget non massa. Cras accumsan tempor  "
"tortor, quis rutrum neque placerat id. Nullam a egestas eros, eu porta nisi. Aenean rutrum, sapien quis fermentum tempus, dolor orci  "
"faucibus eros, vel luctus justo leo vitae ante. Curabitur aliquam condimentum ipsum sit amet ultrices. Nullam ac velit semper, dapibus urna "
"sit amet, malesuada enim. Mauris ultricies nibh orci.";


mg_font create_font()
{
	//NOTE(martin): create font
	str8 fontPath = mp_app_get_resource_path(mem_scratch(), "../resources/OpenSansLatinSubset.ttf");
	char* fontPathCString = str8_to_cstring(mem_scratch(), fontPath);

	FILE* fontFile = fopen(fontPathCString, "r");
	if(!fontFile)
	{
		LOG_ERROR("Could not load font file '%s'\n", fontPathCString);
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
	LogLevel(LOG_LEVEL_MESSAGE);
	mp_init();
	mp_clock_init();

	mp_rect rect = {.x = 100, .y = 100, .w = 980, .h = 600};
	mp_window window = mp_window_create(rect, "test", 0);

	mp_rect contentRect = mp_window_get_content_rect(window);

	//NOTE: create surface, canvas and font

	mg_surface surface = mg_surface_create_for_window(window, MG_BACKEND_DEFAULT);
	mg_surface_swap_interval(surface, 0);

	mg_canvas canvas = mg_canvas_create(surface);

	mg_font font = create_font();
	mg_font_extents extents = mg_font_get_extents(font);
	f32 fontScale = mg_font_get_scale_for_em_pixels(font, 14);

	f32 lineHeight = fontScale*(extents.ascent + extents.descent + extents.leading);

	int codePointCount = utf8_codepoint_count_for_string(str8_from_cstring((char*)TEST_STRING));
	u32* codePoints = malloc_array(utf32, codePointCount);
	utf8_to_codepoints(codePointCount, codePoints, str8_from_cstring((char*)TEST_STRING));

	u32 glyphCount = 0;
	for(int i=0; i<codePointCount; i++)
	{
		if(codePoints[i] != ' ' && codePoints[i] != '\n')
		{
			glyphCount++;
		}
	}

	// start app
	mp_window_bring_to_front(window);
	mp_window_focus(window);

	f64 frameTime = 0;

	bool tracked = false;
	vec2 trackPoint = {0};
	f32 zoom = 1;

	f32 startX = 10;
	f32 startY = (contentRect.h - lineHeight - 10);
/*
	f32 startX = -100;
	f32 startY = -100;
*/
	while(!mp_should_quit())
	{
		f64 startFrameTime = mp_get_time(MP_CLOCK_MONOTONIC);

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

				case MP_EVENT_MOUSE_BUTTON:
				{
					if(event.key.code == MP_MOUSE_LEFT)
					{
						if(event.key.action == MP_KEY_PRESS)
						{
							tracked = true;
							vec2 mousePos = mp_input_mouse_position();
							trackPoint.x = mousePos.x/zoom - startX;
							trackPoint.y = mousePos.y/zoom - startY;
						}
						else
						{
							tracked = false;
						}
					}
				} break;

				case MP_EVENT_MOUSE_WHEEL:
				{
					vec2 mousePos = mp_input_mouse_position();
					f32 trackX = mousePos.x/zoom - startX;
					f32 trackY = mousePos.y/zoom - startY;

					zoom *= 1 + event.move.deltaY * 0.01;
					zoom = Clamp(zoom, 0.2, 10);

					startX = mousePos.x/zoom - trackX;
					startY = mousePos.y/zoom - trackY;
				} break;

				default:
					break;
			}
		}

		if(tracked)
		{
			vec2 mousePos = mp_input_mouse_position();
			startX = mousePos.x/zoom - trackPoint.x;
			startY = mousePos.y/zoom - trackPoint.y;
		}

		f32 textX = startX;
		f32 textY = startY;

		mg_surface_prepare(surface);

/*
		mg_set_color_rgba(1, 1, 1, 1);
		mg_clear();
		mg_set_color_rgba(1, 0, 0, 1);
		for(int i=0; i<1000; i++)
		{
			mg_rectangle_fill(0, 0, 100, 100);
		}
*/

			mg_matrix_push((mg_mat2x3){zoom, 0, 0,
			                           0, zoom, 0});

			mg_set_color_rgba(1, 1, 1, 1);
			mg_clear();

			mg_set_font(font);
			mg_set_font_size(14);
			mg_set_color_rgba(0, 0, 0, 1);

			mg_move_to(textX, textY);

			int startIndex = 0;
			while(startIndex < codePointCount)
			{
				bool lineBreak = false;
				int subIndex = 0;
				for(; (startIndex+subIndex) < codePointCount && subIndex < 120; subIndex++)
				{
					if(codePoints[startIndex + subIndex] == '\n')
					{
						break;
					}
				}

				u32 glyphs[512];
				mg_font_get_glyph_indices(font, (str32){subIndex, codePoints+startIndex}, (str32){512, glyphs});

				mg_glyph_outlines((str32){subIndex, glyphs});
				mg_fill();

				textY -= lineHeight;
				mg_move_to(textX, textY);
				startIndex++;

				startIndex += subIndex;
			}

			mg_matrix_pop();

			mg_set_color_rgba(0, 0, 1, 1);
			mg_set_font(font);
			mg_set_font_size(14);
			mg_move_to(10, 10 + lineHeight);

			str8 text = str8_pushf(mem_scratch(),
			                      "Test program: %i glyphs, frame time = %fs, fps = %f",
			                      glyphCount,
			                      frameTime,
			                      1./frameTime);
			mg_text_outlines(text);
			mg_fill();


			f64 startFlushTime = mp_get_time(MP_CLOCK_MONOTONIC);
			mg_flush();

		f64 startPresentTime = mp_get_time(MP_CLOCK_MONOTONIC);
		mg_surface_present(surface);

		f64 endFrameTime = mp_get_time(MP_CLOCK_MONOTONIC);

		frameTime = (endFrameTime - startFrameTime);

		printf("frame time: %.2fms (%.2fFPS), draw = %f.2ms, flush = %.2fms, present = %.2fms\n",
		      	frameTime*1000,
		      	1./frameTime,
		      	(startFlushTime - startFrameTime)*1000,
		      	(startPresentTime - startFlushTime)*1000,
		      	(endFrameTime - startPresentTime)*1000);

		mem_arena_clear(mem_scratch());
	}


	mg_font_destroy(font);
	mg_canvas_destroy(canvas);
	mg_surface_destroy(surface);
	mp_window_destroy(window);
	mp_terminate();

	return(0);
}
