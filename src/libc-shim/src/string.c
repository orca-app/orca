/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <string.h>

void* memset(void* b, int c, size_t n)
{
    return (__builtin_memset(b, c, n));
}

void* memcpy(void* __restrict dst, const void* __restrict src, size_t n)
{
    return (__builtin_memcpy(dst, src, n));
}

void* memmove(void* dst, const void* src, size_t n)
{
    return (__builtin_memmove(dst, src, n));
}

int memcmp(const void* s1, const void* s2, size_t n)
{
    return (__builtin_memcmp(s1, s2, n));
}

#define STB_SPRINTF_IMPLEMENTATION
#include "stb/stb_sprintf.h"

size_t strlen(const char* s)
{
    size_t len = 0;
    while(s[len] != '\0')
    {
        len++;
    }
    return (len);
}

int strcmp(const char* s1, const char* s2)
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
    return (res);
}

int strncmp(const char* s1, const char* s2, size_t n)
{
    n--;
    if(n == 0)
    {
        return(0);
    }
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
    return (res);
}

char* strcpy(char* __restrict s1, const char* __restrict s2)
{
    size_t i = 0;
    while(s2[i] != '\0')
    {
        s1[i] = s2[i];
        i++;
    }
    s1[i] = '\0';
    return (s1);
}

char* strncpy(char* __restrict s1, const char* __restrict s2, size_t len)
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
    return (s1);
}
