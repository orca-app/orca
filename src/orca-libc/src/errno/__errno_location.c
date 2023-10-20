#include <errno.h>

static int __errno_val = 0;
int *__errno_location(void)
{
	return &__errno_val;
}

weak_alias(__errno_location, ___errno_location);
