#include <sys/mman.h>
#include <errno.h>
#include <internal/syscall.h>

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    long int result = syscall(__NR_mmap, addr, length, prot, flags, fd, offset);

    if (result < 0) {
        errno = -result;
		return (void *) -1;
    }

    return (void *)result;
}

void *mremap(void *old_address, size_t old_size, size_t new_size, int flags)
{
	long int result = syscall(__NR_mremap, old_address, old_size, new_size, flags);

    if (result < 0) {
        errno = -result; // adjust the error
		return (void *) -1;
    }

    return (void *)result;
}

int munmap(void *addr, size_t length)
{
    int result = syscall(__NR_munmap, addr, length);

    if (result < 0) {
        errno = -result;
        return -1;
    }

    return result;
}
