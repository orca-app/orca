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
#include"osx_gles_surface.h"


void mg_gles_surface_prepare(mg_surface_info* interface)
{
	mg_gles_surface* surface = (mg_gles_surface*)interface;
	eglMakeCurrent(surface->eglDisplay, surface->eglSurface, surface->eglSurface, surface->eglContext);
}

void mg_gles_surface_present(mg_surface_info* interface)
{
	mg_gles_surface* surface = (mg_gles_surface*)interface;
	eglSwapBuffers(surface->eglDisplay, surface->eglSurface);
}

void mg_gles_surface_resize(mg_surface_info* interface, u32 width, u32 height)
{
	mg_gles_surface* surface = (mg_gles_surface*)interface;
	f32 scale = surface->layer.contentsScale;
	CGRect bounds = (CGRect){{0, 0}, {.width = width*scale, .height = height*scale}};
	[surface->layer setBounds: bounds];
}

void mg_gles_surface_set_hidden(mg_surface_info* interface, bool hidden)
{
	//NOTE: doesn't make sense for an offscreen surface?
}

vec2 mg_gles_surface_get_size(mg_surface_info* interface)
{
	mg_gles_surface* surface = (mg_gles_surface*)interface;
	CGRect bounds = [surface->layer bounds];
	f32 scale = surface->layer.contentsScale;
	vec2 res = {bounds.size.width/scale, bounds.size.height/scale};
	return(res);
}

void mg_gles_surface_destroy(mg_surface_info* interface)
{
	//TODO
}

void* mg_gles_surface_get_os_resource(mg_surface_info* interface)
{
	mg_gles_surface* surface = (mg_gles_surface*)interface;
	return((void*)surface->layer);
}

mg_surface mg_gles_surface_create_offscreen(u32 width, u32 height)
{
	mg_gles_surface* surface = malloc_type(mg_gles_surface);
	surface->interface.backend = MG_BACKEND_GLES;
	surface->interface.destroy = mg_gles_surface_destroy;
	surface->interface.prepare = mg_gles_surface_prepare;
	surface->interface.present = mg_gles_surface_present;
	surface->interface.resize = mg_gles_surface_resize;
	surface->interface.setHidden = mg_gles_surface_set_hidden;
	surface->interface.getSize = mg_gles_surface_get_size;
	surface->interface.getOSResource = mg_gles_surface_get_os_resource;

	@autoreleasepool
	{
		surface->layer = [[CALayer alloc] init];
		[surface->layer retain];
		[surface->layer setBounds:CGRectMake(0, 0, width, height)];
	}


	EGLAttrib displayAttribs[] = {
		EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE,
	    EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE,
	    EGL_NONE};

	surface->eglDisplay = eglGetPlatformDisplay(EGL_PLATFORM_ANGLE_ANGLE, (void*)EGL_DEFAULT_DISPLAY, displayAttribs);
	eglInitialize(surface->eglDisplay, NULL, NULL);

	EGLint const configAttributes[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_DEPTH_SIZE, 24,
		EGL_STENCIL_SIZE, 8,
		EGL_SAMPLE_BUFFERS, 0,
		EGL_SAMPLES, EGL_DONT_CARE,
		EGL_COLOR_COMPONENT_TYPE_EXT, EGL_COLOR_COMPONENT_TYPE_FIXED_EXT,
		EGL_NONE };

	eglChooseConfig(surface->eglDisplay, configAttributes, &surface->eglConfig, 1, &surface->numConfigs);

	EGLint const surfaceAttributes[] = {EGL_NONE};
	surface->eglSurface = eglCreateWindowSurface(surface->eglDisplay, surface->eglConfig, surface->layer, surfaceAttributes);

	eglBindAPI(EGL_OPENGL_ES_API);
	EGLint contextAttributes[] = {
		EGL_CONTEXT_MAJOR_VERSION_KHR, 3,
		EGL_CONTEXT_MINOR_VERSION_KHR, 0,
		EGL_CONTEXT_BIND_GENERATES_RESOURCE_CHROMIUM, EGL_TRUE,
		EGL_CONTEXT_CLIENT_ARRAYS_ENABLED_ANGLE, EGL_TRUE,
		EGL_CONTEXT_OPENGL_BACKWARDS_COMPATIBLE_ANGLE, EGL_FALSE,
		EGL_NONE};

	surface->eglContext = eglCreateContext(surface->eglDisplay, surface->eglConfig, EGL_NO_CONTEXT, contextAttributes);
	eglMakeCurrent(surface->eglDisplay, surface->eglSurface, surface->eglSurface, surface->eglContext);

	mg_surface handle = mg_surface_alloc_handle((mg_surface_info*)surface);
	return(handle);
}
