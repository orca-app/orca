
#include<stdio.h>
#include<stdlib.h>

#define LOG_DEFAULT_LEVEL LOG_LEVEL_MESSAGE
#define LOG_COMPILE_DEBUG

#include"milepost.h"

#define LOG_SUBSYSTEM "Main"

static const char* TEST_STRING =
"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nulla quam enim, aliquam in placerat luctus, rutrum in quam.\n" \
"Cras urna elit, pellentesque ac ipsum at, lobortis scelerisque eros. Aenean et turpis nibh. Maecenas lectus augue, eleifend\n" \
"nec efficitur eu, faucibus eget turpis. Suspendisse vel nulla mi. Duis imperdiet neque orci, ac ultrices orci molestie a.\n"
"Etiam malesuada vulputate hendrerit. Cras ultricies diam in lectus finibus, eu laoreet diam rutrum.\n" \
"\n" \
"Etiam dictum orci arcu, ac fermentum leo dapibus lacinia. Integer vitae elementum ex. Vestibulum tempor nunc eu hendrerit\n" \
"ornare. Nunc pretium ligula sit amet massa pulvinar, vitae imperdiet justo bibendum. Maecenas consectetur elementum mi, sed\n" \
"vehicula neque pulvinar sit amet. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nunc tortor erat, accumsan in laoreet\n" \
"quis, placerat nec enim. Nulla facilisi. Morbi vitae nibh ligula. Suspendisse in molestie magna, eget aliquet mauris. Sed \n" \
"aliquam faucibus magna.\n" \
"\n" \
"Sed metus odio, imperdiet et consequat non, faucibus nec risus. Suspendisse facilisis sem neque, id scelerisque dui mattis sit\n" \
"amet. Nullam tincidunt nisl nec dui dignissim mattis. Proin fermentum ornare ipsum. Proin eleifend, mi vitae porttitor placerat,\n" \
"neque magna elementum turpis, eu aliquet mi urna et leo. Pellentesque interdum est mauris, sed pellentesque risus blandit in.\n" \
"Phasellus dignissim consequat eros, at aliquam elit finibus posuere. Proin suscipit tortor leo, id vulputate odio lobortis in.\n" \
"Vestibulum et orci ligula. Sed scelerisque nunc non nisi aliquam, vel eleifend felis suscipit. Integer posuere sapien elit, \n" \
"lacinia ultricies nibh sodales nec.\n" \
"\n" \
"Etiam aliquam purus sit amet purus ultricies tristique. Nunc maximus nunc quis magna ornare, vel interdum urna fermentum.\n" \
"Vestibulum cursus nisl ut nulla egestas, quis mattis elit venenatis. Praesent malesuada mi non magna aliquam fringilla eget eu\n" \
"turpis. Integer suscipit elit vel consectetur vulputate. Integer euismod, erat eget elementum tempus, magna metus consectetur\n" \
"elit, sed feugiat urna sapien sodales sapien. Sed sit amet varius nunc. Curabitur sodales nunc justo, ac scelerisque ipsum semper\n" \
"eget. Integer ornare, velit ut hendrerit dapibus, erat mauris commodo justo, vel semper urna justo non mauris. Proin blandit,\n" \
"enim ut posuere placerat, leo nibh tristique eros, ut pulvinar sapien elit eget enim. Pellentesque et mauris lectus. Curabitur\n" \
"quis lobortis leo, sit amet egestas dui. Nullam ut sapien eu justo lacinia ultrices. Ut tincidunt, sem non luctus tempus, felis\n" \
"purus imperdiet nisi, non ultricies libero ipsum eu augue. Mauris at luctus enim.";


mg_font create_font()
{
	//NOTE(martin): create font
/*	str8 fontPath = mp_app_get_resource_path(mem_scratch(), "../resources/OpenSansLatinSubset.ttf");
	char* fontPathCString = str8_to_cstring(mem_scratch(), fontPath);
*/
	char* fontPathCString = "resources/OpenSansLatinSubset.ttf";

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

	//NOTE: create surface, canvas and font

#if defined(OS_MACOS)
	mg_surface surface = mg_metal_surface_create_for_window(window);
#elif defined(OS_WIN64)
	mg_surface surface = mg_gles_surface_create_for_window(window);
#else
	#error "unsupported OS"
#endif

	mg_canvas canvas = mg_canvas_create(surface);

	mg_font font = create_font();
	mg_font_extents extents = mg_font_get_extents(font);
	f32 fontScale = mg_font_get_scale_for_em_pixels(font, 12);

	f32 lineHeight = fontScale*(extents.ascent + extents.descent + extents.leading);

	int codePointCount = utf8_codepoint_count_for_string(str8_from_cstring((char*)TEST_STRING));
	u32* codePoints = malloc_array(utf32, codePointCount);
	utf8_to_codepoints(codePointCount, codePoints, str8_from_cstring((char*)TEST_STRING));

	// start app
	mp_window_bring_to_front(window);
	mp_window_focus(window);

	f64 frameTime = 0;

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

				default:
					break;
			}
		}

		f32 textX = 10;
		f32 textY = 600 - lineHeight;

		mg_surface_prepare(surface);
			mg_set_color_rgba(1, 1, 1, 1);
			mg_clear();

			mg_set_font(font);
			mg_set_font_size(12);
			mg_set_color_rgba(0, 0, 0, 1);

			mg_move_to(textX, textY);

			int startIndex = 0;
			while(startIndex < codePointCount)
			{
				bool lineBreak = false;
				int subIndex = 0;
				for(; (startIndex+subIndex) < codePointCount && subIndex < 512; subIndex++)
				{
					if(codePoints[startIndex + subIndex] == '\n')
					{
						lineBreak = true;
						break;
					}
				}
				ASSERT(subIndex < 512 && (startIndex+subIndex)<=codePointCount);
				u32 glyphs[512];
				mg_font_get_glyph_indices(font, (str32){subIndex, codePoints+startIndex}, (str32){512, glyphs});

				mg_glyph_outlines((str32){subIndex, glyphs});
				mg_fill();

				if(lineBreak)
				{
					textY -= lineHeight;
					mg_move_to(textX, textY);
					startIndex++;
				}
				startIndex += subIndex;
			}

			f64 startFlushTime = mp_get_time(MP_CLOCK_MONOTONIC);

			mg_set_color_rgba(0, 0, 1, 1);
			mg_set_font(font);
			mg_set_font_size(12);
			mg_move_to(50, 50);

			str8 text = str8_pushf(mem_scratch(),
			                      "Milepost vector graphics test program (frame time = %fs, fps = %f)...",
			                      frameTime,
			                      1./frameTime);
			mg_text_outlines(text);
			mg_fill();

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

	mp_terminate();

	return(0);
}
