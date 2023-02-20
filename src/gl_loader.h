/********************************************************
*
*	@file: gl_loader.h
*	@note: auto-generated by glapi.py from gl.xml
*	@date: 20/022023
*
*********************************************************/
#ifndef __GL_LOADER_H__
#define __GL_LOADER_H__

#include"gl_api.h"

typedef void*(*mg_gl_load_proc)(const char* name);

void mg_gl_load_gl41(mg_gl_api* api, mg_gl_load_proc loadProc);
void mg_gl_load_gl43(mg_gl_api* api, mg_gl_load_proc loadProc);
void mg_gl_load_gles31(mg_gl_api* api, mg_gl_load_proc loadProc);
void mg_gl_load_gles32(mg_gl_api* api, mg_gl_load_proc loadProc);

void mg_gl_select_api(mg_gl_api* api);
mg_gl_api* mg_gl_get_api(void);

#endif // __GL_LOADER_H__
