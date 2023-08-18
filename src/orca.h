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
#include"util/macros.h"
#include"util/debug.h"
#include"util/lists.h"
#include"util/memory.h"
#include"util/strings.h"
#include"util/utf8.h"
#include"util/hash.h"
#include"util/algebra.h"

#include"platform/platform.h"
#include"platform/platform_clock.h"
#include"platform/platform_path.h"
#include"platform/platform_io.h"

#if !defined(OC_PLATFORM_ORCA) || !(OC_PLATFORM_ORCA)
	#include"platform/platform_thread.h"
#endif

#include"app/app.h"

//----------------------------------------------------------------
// graphics
//----------------------------------------------------------------
#include"graphics/graphics.h"

#if OC_PLATFORM_ORCA
	//TODO: maybe make this conditional
	#include"graphics/orca_gl31.h"

	oc_surface oc_surface_canvas();
	oc_surface oc_surface_gles();

#else
	#ifdef OC_INCLUDE_GL_API
		#include"graphics/gl_api.h"
	#endif
#endif
//----------------------------------------------------------------
// UI
//----------------------------------------------------------------
#include"ui/input_state.h"
#include"ui/ui.h"

#endif //__ORCA_H_
