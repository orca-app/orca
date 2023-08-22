#include "platform.h"

#ifdef __cplusplus
extern "C"
{
#endif

	oc_host_platform oc_get_host_platform()
	{
		return OC_HOST_PLATFORM_WINDOWS;
	}

#ifdef __cplusplus
} // extern "C"
#endif