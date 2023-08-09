/************************************************************//**
*
*	@file: milepost.h
*	@author: Martin Fouilleul
*	@date: 13/02/2021
*	@revision:
*
*****************************************************************/
#ifndef __MILEPOST_H_
#define __MILEPOST_H_

//----------------------------------------------------------------
// utility layer
//----------------------------------------------------------------
#include"platform/platform.h"
#include"util/typedefs.h"
#include"util/macro_helpers.h"
#include"platform/platform_log.h"
#include"util/lists.h"
#include"util/memory.h"
#include"util/strings.h"
#include"util/utf8.h"
#include"util/hash.h"

//----------------------------------------------------------------
// platform layer
//----------------------------------------------------------------
#include"platform/platform_clock.h"
#include"platform/platform_path.h"
#include"platform/platform_io.h"
#include"platform/platform_thread.h"
/*
#include"platform_rng.h"
#include"platform_socket.h"
#include"platform_thread.h"
*/

//----------------------------------------------------------------
// application/graphics layer
//----------------------------------------------------------------
#include"mp_app.h"
#include"graphics.h"
#include"input_state.h"
#include"ui.h"

#ifdef MG_INCLUDE_GL_API
	#include"gl_api.h"
#endif

//#include"ui.h"

#endif //__MILEPOST_H_
