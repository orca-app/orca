#include <stdlib.h>
#include"platform/platform.h"

_Noreturn void ORCA_IMPORT(oc_hostcall_exit)(int);

_Noreturn void _Exit(int ec)
{
    oc_hostcall_exit(ec);
}
