/************************************************************//**
*
*	@file: win32_gl_loader.h
*	@author: Martin Fouilleul
*	@date: 01/08/2022
*	@revision:
*
*****************************************************************/

#define WIN32_LEAN_AND_MEAN
#include<windows.h>
#include<GL/gl.h>
#include<GL/glext.h>
#include<GL/wglext.h>

#include"macro_helpers.h"

#define GL_PROC_LIST \
	GL_PROC(WGLCHOOSEPIXELFORMATARB, wglChoosePixelFormatARB) \
	GL_PROC(WGLCREATECONTEXTATTRIBSARB, wglCreateContextAttribsARB) \
	GL_PROC(WGLMAKECONTEXTCURRENTARB, wglMakeContextCurrentARB) \
	GL_PROC(WGLSWAPINTERVALEXT, wglSwapIntervalEXT) \
	GL_PROC(GLCREATESHADER, glCreateShader) \
	GL_PROC(GLCREATEPROGRAM, glCreateProgram) \
	GL_PROC(GLATTACHSHADER, glAttachShader) \
	GL_PROC(GLCOMPILESHADER, glCompileShader) \
	GL_PROC(GLGETSHADERIV, glGetShaderiv) \
	GL_PROC(GLGETSHADERINFOLOG, glGetShaderInfoLog) \
	GL_PROC(GLSHADERSOURCE, glShaderSource) \
	GL_PROC(GLLINKPROGRAM, glLinkProgram) \
	GL_PROC(GLGETPROGRAMIV, glGetProgramiv) \
	GL_PROC(GLGETPROGRAMINFOLOG, glGetProgramInfoLog) \
	GL_PROC(GLUSEPROGRAM, glUseProgram) \
	GL_PROC(GLGENVERTEXARRAYS, glGenVertexArrays) \
	GL_PROC(GLBINDVERTEXARRAY, glBindVertexArray) \
	GL_PROC(GLGENBUFFERS, glGenBuffers) \
	GL_PROC(GLBINDBUFFER, glBindBuffer) \
	GL_PROC(GLBUFFERDATA, glBufferData) \
	GL_PROC(GLUNIFORMMATRIX4FV, glUniformMatrix4fv) \
	GL_PROC(GLVERTEXATTRIBPOINTER, glVertexAttribPointer) \
	GL_PROC(GLENABLEVERTEXATTRIBARRAY, glEnableVertexAttribArray) \
	GL_PROC(GLBINDBUFFERBASE, glBindBufferBase) \
	GL_PROC(GLDISPATCHCOMPUTE, glDispatchCompute) \
	GL_PROC(GLUNIFORM1UI, glUniform1ui) \
	GL_PROC(GLUNIFORM2UI, glUniform2ui) \
	GL_PROC(GLBINDIMAGETEXTURE, glBindImageTexture) \
	GL_PROC(GLACTIVETEXTURE, glActiveTexture) \
	GL_PROC(GLUNIFORM1I, glUniform1i) \
	GL_PROC(GLTEXSTORAGE2D, glTexStorage2D) \

#ifdef WIN32_GL_LOADER_API
	//NOTE: pointer declarations
	#define GL_PROC(type, name) extern _cat3_(PFN, type, PROC) name;
	GL_PROC_LIST
	#undef GL_PROC
#endif


#ifdef WIN32_GL_LOADER_IMPL
#define GL_PROC(type, name) _cat3_(PFN, type, PROC) name = 0;
	GL_PROC_LIST
#undef GL_PROC

void mp_gl_load_procs()
{
	#define GL_PROC(type, name) name = (_cat3_(PFN, type, PROC))wglGetProcAddress( #name );
		GL_PROC_LIST
	#undef GL_PROC
}

#endif

#undef GL_PROC_LIST
