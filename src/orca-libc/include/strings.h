#ifndef	_STRINGS_H
#define	_STRINGS_H

#ifdef __cplusplus
extern "C" {
#endif

#define __NEED_size_t
#include <bits/alltypes.h>

int bcmp (const void *, const void *, size_t);
void bcopy (const void *, void *, size_t);
void bzero (void *, size_t);
char *index (const char *, int);
char *rindex (const char *, int);

int ffs (int);
int ffsl (long);
int ffsll (long long);

int strcasecmp (const char *, const char *);
int strncasecmp (const char *, const char *, size_t);

#ifdef __cplusplus
}
#endif

#endif
