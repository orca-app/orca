/************************************************************//**
*
*	@file: orca.h
*	@author: Martin Fouilleul
*	@date: 13/02/2021
*	@revision:
*
*****************************************************************/
#ifndef __ORCA_H_
#define __ORCA_H_

#include"util/typedefs.h"
#include"util/macro_helpers.h"
#include"util/debug.h"
#include"util/lists.h"
#include"util/memory.h"
#include"util/strings.h"
#include"util/utf8.h"
#include"util/hash.h"

#include"platform/platform.h"
#include"platform/platform_clock.h"
#include"platform/platform_path.h"
#include"platform/platform_io.h"

#if !defined(PLATFORM_ORCA) || !(PLATFORM_ORCA)
	#include"platform/platform_thread.h"
#endif

#include"app/mp_app.h"

//----------------------------------------------------------------
// graphics
//----------------------------------------------------------------
#include"graphics/graphics.h"

#if PLATFORM_ORCA
	//TODO: maybe make this conditional
	#include"graphics/orca_gl31.h"

	mg_surface mg_surface_canvas();
	mg_surface mg_surface_gles();

#else
	#ifdef MG_INCLUDE_GL_API
		#include"graphics/gl_api.h"
	#endif
#endif
//----------------------------------------------------------------
// UI
//----------------------------------------------------------------
#include"ui/input_state.h"
#include"ui/ui.h"

#endif //__ORCA_H_
