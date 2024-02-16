// SPDX-License-Identifier: BSD-3-Clause

#include <fcntl.h>
#include <internal/syscall.h>
#include <internal/types.h>
#include <stdarg.h>
#include <errno.h>

int open(const char *filename, int flags, ...)
{
	/* TODO: Implement open system call. */
	mode_t mod = 0;
	if (flags & O_CREAT) {
		va_list argument;
		va_start(argument, flags);
		mod = va_arg(argument, mode_t);
		va_end(argument);// end of varible arguments
	}
	int ret = syscall(__NR_open, filename, flags, mod);

	if (ret < 0) {
		errno = -ret;
		return -1;
	}
	return ret;
}
