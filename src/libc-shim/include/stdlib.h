#ifndef _STDLIB_H
#define _STDLIB_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define abort(...) OC_ABORT(__VA_ARGS__)

int abs(int);

void* malloc(size_t);
void* realloc(void*, size_t);
void* calloc(size_t count, size_t size);
void free(void*);

#ifdef __cplusplus
}
#endif

#endif
