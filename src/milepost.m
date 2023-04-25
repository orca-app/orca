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

#if MG_COMPILE_METAL
	#include"mtl_surface.m"
#endif

#if MG_COMPILE_CANVAS
	#include"mtl_renderer.m"
#endif

#if MG_COMPILE_GLES
	#include"gl_loader.c"
	#include"egl_surface.c"
#endif
