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
	#include<math.h>
#else

#define M_PI 3.14159265358979323846

double fabs(double x);
double sqrt(double sqrt);
double cos(double x);
double sin(double x);

#endif

#endif //__PLATFORM_MATH_H_
