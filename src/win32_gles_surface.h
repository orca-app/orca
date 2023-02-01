/************************************************************//**
*
*	@file: win32_gles_surface.h
*	@author: Martin Fouilleul
*	@date: 28/01/2023
*	@revision:
*
*****************************************************************/
#ifndef __WIN32_GLES_SURFACE_H_
#define __WIN32_GLES_SURFACE_H_

#include"graphics.h"
#include"mp_app.h"

mg_surface mg_gles_surface_create_for_window(mp_window window);

#endif // __WIN32_GLES_SURFACE_H_
