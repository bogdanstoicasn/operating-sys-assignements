#include <time.h>
#include <internal/syscall.h>
#include <errno.h>

// used https://stackoverflow.com/questions/7684359/how-to-use-nanosleep-in-c-what-are-tim-tv-sec-and-tim-tv-nsec
int nanosleep(const struct timespec *req, struct timespec *rem)
{
	long int ret = syscall(__NR_nanosleep, req, rem);

	if (ret < 0) {
		errno = -ret;
		return -1;
	}
	return ret;
}
