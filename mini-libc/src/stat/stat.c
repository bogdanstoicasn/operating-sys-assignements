// SPDX-License-Identifier: BSD-3-Clause

#include <sys/stat.h>
#include <internal/syscall.h>
#include <fcntl.h>
#include <errno.h>

// used "musl" as reference
int stat(const char *restrict path, struct stat *restrict buf)
{
	/* TODO: Implement stat(). */
	int ret = syscall(__NR_stat, path, buf);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}
	return ret;
}
