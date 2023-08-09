#include"stb/stb_sprintf.h"

void* memset(void* b, int c, size_t n);
void* memcpy(void *restrict dst, const void *restrict src, size_t n);
void* memmove(void *dst, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char* strcpy(char *restrict s1, const char *restrict s2);

#define vsnprintf stbsp_vsnprintf
