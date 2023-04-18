/************************************************************//**
*
*	@file: orca_strings.c
*	@author: Martin Fouilleul
*	@date: 18/04/2023
*
*****************************************************************/
#include"platform_strings.h"

#define STB_SPRINTF_IMPLEMENTATION
#include"ext/stb_sprintf.h"

size_t strlen(const char *s)
{
	size_t len = 0;
	while(s[len] != '\0')
	{
		len++;
	}
	return(len);
}

int strcmp(const char *s1, const char *s2)
{
	size_t i = 0;
	while(s1[i] != '\0' && s1[i] == s2[i])
	{
		i++;
	}
	int res = 0;
	if(s1[i] != s2[i])
	{
		if(s1[i] == '\0')
		{
			res = -1;
		}
		else if(s2[i] == '\0')
		{
			res = 1;
		}
		else
		{
			res = (unsigned char)s1[i] - (unsigned char)s2[i];
		}
	}
	return(res);
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	size_t i = 0;
	while(i < n && s1[i] != '\0' && s1[i] == s2[i])
	{
		i++;
	}
	int res = 0;
	if(s1[i] != s2[i])
	{
		if(s1[i] == '\0')
		{
			res = -1;
		}
		else if(s2[i] == '\0')
		{
			res = 1;
		}
		else
		{
			res = (unsigned char)s1[i] - (unsigned char)s2[i];
		}
	}
	return(res);
}

char* strcpy(char *restrict s1, const char *restrict s2)
{
	size_t i = 0;
	while(s2[i] != '\0')
	{
		s1[i] = s2[i];
		i++;
	}
	s1[i] = '\0';
	return(s1);
}

char* strncpy(char *restrict s1, const char *restrict s2, size_t len)
{
	size_t i = 0;
	while(i < len && s2[i] != '\0')
	{
		s1[i] = s2[i];
		i++;
	}
	while(i < len)
	{
		s1[i] = 0;
		i++;
	}
	return(s1);
}
