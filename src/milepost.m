/************************************************************//**
*
*	@file: milepost.m
*	@author: Martin Fouilleul
*	@date: 13/02/2021
*	@revision:
*
*****************************************************************/

#ifdef _WIN32

#include"win32_app.c"

#else

#include"osx_app.m"
#include"metal_surface.m"
#include"metal_painter.m"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
	#include"osx_gles_surface.m"
#pragma clang diagnostic pop

#include"osx_surface_client.m"

#endif
