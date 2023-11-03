#include <stdlib.h>
#include"platform/platform.h"

_Noreturn void ORCA_IMPORT(oc_bridge_exit)(int);


_Noreturn void _Exit(int ec)
{
    oc_bridge_exit(ec);
}
