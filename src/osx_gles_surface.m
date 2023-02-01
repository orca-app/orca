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

#include<GLES3/gl3.h>
#define EGL_EGLEXT_PROTOTYPES
#include<EGL/egl.h>
#include<EGL/eglext.h>

#include"graphics_internal.h"
#include"osx_gles_surface.h"


void mg_gles_surface_destroy(mg_surface_data* interface)
{
	//////////////////////////////////////////////////
	//TODO
	//////////////////////////////////////////////////
}

void mg_gles_surface_prepare(mg_surface_data* interface)
{
	mg_gles_surface* surface = (mg_gles_surface*)interface;
	eglMakeCurrent(surface->eglDisplay, surface->eglSurface, surface->eglSurface, surface->eglContext);
}

void mg_gles_surface_present(mg_surface_data* interface)
{

	//TODO: eglSwapBuffers seem to never block in macOS (ie eglSwapInterval doesn't seem to have any effect)
	//      We need to use a CVDisplayLink to time this if we want surface present to block

	mg_gles_surface* surface = (mg_gles_surface*)interface;
	eglSwapBuffers(surface->eglDisplay, surface->eglSurface);
}

void mg_gles_surface_set_frame(mg_surface_data* interface, mp_rect frame)
{
	mg_gles_surface* surface = (mg_gles_surface*)interface;
	f32 scale = surface->layer.contentsScale;
	CGRect bounds = (CGRect){{frame.x * scale, frame.y * scale}, {.width = frame.w*scale, .height = frame.h*scale}};
	[surface->layer setBounds: bounds];
}

mp_rect mg_gles_surface_get_frame(mg_surface_data* interface)
{
	mg_gles_surface* surface = (mg_gles_surface*)interface;
	f32 scale = surface->layer.contentsScale;
	CGRect bounds = [surface->layer bounds];
	mp_rect rect = {bounds.origin.x / scale, bounds.origin.y / scale, bounds.size.width / scale, bounds.size.height / scale};
	return(rect);
}


void mg_gles_surface_set_hidden(mg_surface_data* interface, bool hidden)
{
	//TODO: doesn't make sense for an offscreen surface?
}

bool mg_gles_surface_get_hidden(mg_surface_data* interface)
{
	//TODO: doesn't make sense for an offscreen surface?
	return(false);
}

vec2 mg_gles_surface_get_size(mg_surface_data* interface)
{
	mg_gles_surface* surface = (mg_gles_surface*)interface;
	CGRect bounds = [surface->layer bounds];
	f32 scale = surface->layer.contentsScale;
	vec2 res = {bounds.size.width/scale, bounds.size.height/scale};
	return(res);
}

mg_surface mg_gles_surface_create_with_view(u32 width, u32 height, NSView* view)
{
	mg_gles_surface* surface = malloc_type(mg_gles_surface);
	memset(surface, 0, sizeof(mg_gles_surface));

	surface->interface.backend = MG_BACKEND_GLES;
	surface->interface.destroy = mg_gles_surface_destroy;
	surface->interface.prepare = mg_gles_surface_prepare;
	surface->interface.present = mg_gles_surface_present;
	surface->interface.getFrame = mg_gles_surface_get_frame;
	surface->interface.setFrame = mg_gles_surface_set_frame;
	surface->interface.getHidden = mg_gles_surface_get_hidden;
	surface->interface.setHidden = mg_gles_surface_set_hidden;

	surface->view = view;

	@autoreleasepool
	{
		surface->layer = [[CALayer alloc] init];
		[surface->layer retain];
		[surface->layer setBounds:CGRectMake(0, 0, width, height)];

		if(surface->view)
		{
			[surface->view setWantsLayer: YES];
			surface->view.layer = surface->layer;
		}
	}


	EGLAttrib displayAttribs[] = {
		//NOTE: we need to explicitly set EGL_PLATFORM_ANGLE_TYPE_ANGLE to EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE, because
		// EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE defaults to using CGL, and eglSetSwapInterval is broken for this backend
		EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE,
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
		EGL_CONTEXT_MINOR_VERSION_KHR, 0, //NOTE: Angle can't create a GLES 3.1 context on macOS
		EGL_CONTEXT_BIND_GENERATES_RESOURCE_CHROMIUM, EGL_TRUE,
		EGL_CONTEXT_CLIENT_ARRAYS_ENABLED_ANGLE, EGL_TRUE,
		EGL_CONTEXT_OPENGL_BACKWARDS_COMPATIBLE_ANGLE, EGL_FALSE,
		EGL_NONE};

	surface->eglContext = eglCreateContext(surface->eglDisplay, surface->eglConfig, EGL_NO_CONTEXT, contextAttributes);
	eglMakeCurrent(surface->eglDisplay, surface->eglSurface, surface->eglSurface, surface->eglContext);

	eglSwapInterval(surface->eglDisplay, 1);

	mg_surface handle = mg_surface_alloc_handle((mg_surface_data*)surface);
	return(handle);
}

mg_surface mg_gles_surface_create_offscreen(u32 width, u32 height)
{
	return(mg_gles_surface_create_with_view(width, height, 0));
}

mg_surface mg_gles_surface_create_for_window(mp_window window)
{
	mg_surface res = mg_surface_nil();
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		@autoreleasepool
		{
			NSRect frame = [[windowData->osx.nsWindow contentView] frame];
			NSView* view = [[NSView alloc] initWithFrame: frame];

			res = mg_gles_surface_create_with_view(frame.size.width, frame.size.height, view);

			if(!mg_surface_is_nil(res))
			{
				[[windowData->osx.nsWindow contentView] addSubview: view];
			}
		}
	}
	return(res);
}
