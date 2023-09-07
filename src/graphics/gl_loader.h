/********************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
*********************************************************/
#ifndef __GL_LOADER_H__
#define __GL_LOADER_H__

#include "gl_api.h"

typedef void* (*oc_gl_load_proc)(const char* name);

void oc_gl_load_gl41(oc_gl_api* api, oc_gl_load_proc loadProc);
void oc_gl_load_gl43(oc_gl_api* api, oc_gl_load_proc loadProc);
void oc_gl_load_gl44(oc_gl_api* api, oc_gl_load_proc loadProc);
void oc_gl_load_gles30(oc_gl_api* api, oc_gl_load_proc loadProc);
void oc_gl_load_gles31(oc_gl_api* api, oc_gl_load_proc loadProc);

void oc_gl_select_api(oc_gl_api* api);

#endif // __GL_LOADER_H__
