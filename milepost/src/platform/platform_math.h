/************************************************************//**
*
*	@file: platform_math.h
*	@author: Martin Fouilleul
*	@date: 26/04/2023
*
*****************************************************************/
#ifndef __PLATFORM_MATH_H_
#define __PLATFORM_MATH_H_

#include"platform.h"

#if !PLATFORM_ORCA
	#define _USE_MATH_DEFINES //NOTE: necessary for MSVC
	#include<math.h>
#else

#define M_PI 3.14159265358979323846

double ORCA_IMPORT(fabs)(double x);
double ORCA_IMPORT(sqrt)(double sqrt);
double ORCA_IMPORT(cos)(double x);
double ORCA_IMPORT(sin)(double x);

#endif

#endif //__PLATFORM_MATH_H_
