/************************************************************//**
*
*	@file: orca_log.c
*	@author: Martin Fouilleul
*	@date: 17/04/2023
*
*****************************************************************/
#include"orca_log.h"
#include"platform_varg.h"

#define STB_SPRINTF_IMPLEMENTATION
#include"ext/stb_sprintf.h"

extern void orca_log(size_t len, const char* ptr);

//TODO: later, move this to orca_strings in milepost
size_t strlen(const char *s)
{
	size_t len = 0;
	while(s[len] != '\0')
	{
		len++;
	}
	return(len);
}

void log_print(const char* fmt, ...)
{
	char buf[4096];

	va_list ap;
	va_start(ap, fmt);
	stbsp_vsnprintf(buf, 4096, fmt, ap);
	va_end(ap);

	orca_log(strlen(buf), buf);
}
