#include <features.h>
#include"platform/platform_debug.h"

#undef assert

#ifdef NDEBUG
#define	assert(x) (void)0
#else
#define assert(x) ASSERT(x)
#endif

#if __STDC_VERSION__ >= 201112L && !defined(__cplusplus)
#define static_assert _Static_assert
#endif
