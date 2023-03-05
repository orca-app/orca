/************************************************************//**
*
*	@file: milepost.m
*	@author: Martin Fouilleul
*	@date: 13/02/2021
*	@revision:
*
*****************************************************************/

#include"osx_app.m"
#include"graphics.c"

#if MG_COMPILE_BACKEND_METAL
	#include"mtl_surface.m"
	#include"mtl_canvas.m"
#endif

#if MG_COMPILE_BACKEND_GLES
	#include"gl_loader.c"
	#include"egl_surface.c"
#endif

/*
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
	#include"osx_gles_surface.m"
#pragma clang diagnostic pop
*/
//#include"osx_surface_client.m"
