/************************************************************//**
*
*	@file: milepost.m
*	@author: Martin Fouilleul
*	@date: 13/02/2021
*	@revision:
*
*****************************************************************/

#include"platform/osx_path.m"
#include"app/osx_app.m"
#include"graphics/graphics_common.c"
#include"graphics/graphics_surface.c"

#if OC_COMPILE_METAL
	#include"graphics/mtl_surface.m"
#endif

#if OC_COMPILE_CANVAS
	#include"graphics/mtl_renderer.m"
#endif

#if OC_COMPILE_GLES
	#include"graphics/gl_loader.c"
	#include"graphics/egl_surface.c"
#endif
