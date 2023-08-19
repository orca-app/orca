#ifndef _STDLIB_H
#define _STDLIB_H

#ifdef __cplusplus
extern "C"
{
#endif

#define abort(...) OC_ABORT(__VA_ARGS__)

    int abs(int);

#ifdef __cplusplus
}
#endif

#endif
