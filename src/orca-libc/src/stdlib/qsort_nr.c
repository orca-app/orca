#define _BSD_SOURCE
#include <stdlib.h>

typedef int (*cmpfun)(const void *, const void *);
typedef int (*cmpfun_r)(const void *v1, const void *v2, void *cmp);

static int wrapper_cmp(const void *v1, const void *v2, void *cmp)
{
	return ((cmpfun)cmp)(v1, v2);
}

void __qsort_r(void *base, size_t nel, size_t width, cmpfun_r cmp, void *arg);

void qsort(void *base, size_t nel, size_t width, cmpfun cmp)
{
	__qsort_r(base, nel, width, wrapper_cmp, cmp);
}
