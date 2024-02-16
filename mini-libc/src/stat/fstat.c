// SPDX-License-Identifier: BSD-3-Clause

#include <sys/stat.h>
#include <internal/syscall.h>
#include <errno.h>

// used musl as reference
int fstat(int fd, struct stat *st)
{
    // Invoke the system call to get the file status
    int ret = syscall(__NR_fstat, fd, st);

    // Check for errors returned by the syscall
    if (ret < 0) {
        errno = -ret;
        return -1;
    }

    return 0; // On success, return 0
}
