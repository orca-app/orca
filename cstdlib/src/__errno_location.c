#include <errno.h>

int errno;

int *__errno_location(void)
{
    // NOTE(orca): We might need a better solution if we eventually support wasm threads.
    return &errno;
}
