#include "platform.h"

oc_host_platform ORCA_IMPORT(oc_get_host_platform_impl)();

#ifdef __cplusplus
extern "C"
{
#endif

	oc_host_platform oc_get_host_platform()
	{
		return oc_get_host_platform_impl();
	}

#ifdef __cplusplus
} // extern "C"
#endif