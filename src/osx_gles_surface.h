/************************************************************//**
*
*	@file: osx_gles_surface.cpp
*	@author: Martin Fouilleul
*	@date: 18/08/2022
*	@revision:
*
*****************************************************************/
//#include<Cocoa/Cocoa.h>
//#include <Foundation/Foundation.h>
#include <QuartzCore/QuartzCore.h>

#include<GLES3/gl32.h>
#define EGL_EGLEXT_PROTOTYPES
#include<EGL/egl.h>
#include<EGL/eglext.h>

#include"graphics_internal.h"

typedef struct mg_gles_surface
{
	mg_surface_data interface;

	NSView* view;
	CALayer* layer;
	EGLDisplay eglDisplay;
	EGLConfig eglConfig;
	EGLContext eglContext;
	EGLSurface eglSurface;
	EGLint numConfigs;

} mg_gles_surface;
