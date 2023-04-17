#include"graphics.h"


void g_set_color_argptr_stub(g_color* color);

void g_set_color(g_color color)
{
	g_set_color_argptr_stub(&color);
}


void g_matrix_push_argptr_stub(g_mat2x3* m);

void g_matrix_push(g_mat2x3 m)
{
	g_matrix_push_argptr_stub(&m);
}


void g_font_create_default_argptr_stub(g_font* __retArg);

g_font g_font_create_default()
{
	g_font __ret;
	g_font_create_default_argptr_stub(&__ret);
	return(__ret);
}


void g_set_font_argptr_stub(g_font* font);

void g_set_font(g_font font)
{
	g_set_font_argptr_stub(&font);
}


void g_text_outlines_argptr_stub(str8* text);

void g_text_outlines(str8 text)
{
	g_text_outlines_argptr_stub(&text);
}


