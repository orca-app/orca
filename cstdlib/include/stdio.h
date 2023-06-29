#ifndef _STDIO_H
#define _STDIO_H

#ifdef __cplusplus
extern "C" {
#endif

struct _IO_FILE { char __x; };
typedef struct _IO_FILE FILE;

extern FILE *const stdin;
extern FILE *const stdout;
extern FILE *const stderr;

#define stdin  (stdin)
#define stdout (stdout)
#define stderr (stderr)

int fprintf(FILE *__restrict, const char *__restrict, ...);

#ifdef __cplusplus
}
#endif

#endif
