#include "stdio_impl.h"

#undef stderr

static unsigned char buf[BUFSIZ+UNGET];
hidden FILE __stderr_FILE = {
	.buf = buf+UNGET,
	.buf_size = sizeof buf-UNGET,
	.orca_file =  0, // oc_file handle 0 is the nil handle,
	.flags = F_PERM | F_NOWR | F_NORD,
	.read = __file_read_err_shim,
	.write = __file_write_err_shim,
	.seek = __file_seek_err_shim,
	.close = __file_close_err_shim,
#if defined(__wasilibc_unmodified_upstream) || defined(_REENTRANT)
	.lock = -1,
#endif
};
FILE *const stderr = &__stderr_FILE;
FILE *volatile __stderr_used = &__stderr_FILE;
