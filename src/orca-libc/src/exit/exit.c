#include <stdlib.h>
#include <stdint.h>

hidden void __funcs_on_exit(void);

static void dummy()
{
}

/* atexit.c and __stdio_exit.c override these. the latter is linked
 * as a consequence of linking either __toread.c or __towrite.c. */
weak_alias(dummy, __funcs_on_exit);
weak_alias(dummy, __stdio_exit);

// Split out the cleanup functions so that we can call them without calling
// _Exit if we don't need to. This allows _start to just return if main
// returns 0.
void __wasm_call_dtors(void)
{
	__funcs_on_exit();
	__stdio_exit();
}

_Noreturn void exit(int code)
{
	__wasm_call_dtors();
	_Exit(code);
}
