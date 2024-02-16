#ifndef _STRUCT_TIMESPEC
#define _STRUCT_TIMESPEC 1

#include <internal/types.h>
# define __STD_TYPE		__extension__ typedef
#if __WORDSIZE == 64
typedef signed long int __int64_t;
typedef unsigned long int __uint64_t;
#else
__extension__ typedef signed long long int __int64_t;
__extension__ typedef unsigned long long int __uint64_t;
#endif
# define __SQUAD_TYPE		__int64_t
# define __SYSCALL_SLONG_TYPE	__SQUAD_TYPE
#define __TIME_T_TYPE		__SYSCALL_SLONG_TYPE
__STD_TYPE __TIME_T_TYPE __time_t;

struct timespec
{
#ifdef __USE_TIME_BITS64
  __time64_t tv_sec;		/* Seconds.  */
#else
  __time_t tv_sec;		/* Seconds.  */
#endif
#if __WORDSIZE == 64 \
  || (defined __SYSCALL_WORDSIZE && __SYSCALL_WORDSIZE == 64) \
  || (__TIMESIZE == 32 && !defined __USE_TIME_BITS64)
  __syscall_slong_t tv_nsec;	/* Nanoseconds.  */
#else
# if __BYTE_ORDER == __BIG_ENDIAN
  int: 32;           /* Padding.  */
  long int tv_nsec;  /* Nanoseconds.  */
# else
  long int tv_nsec;  /* Nanoseconds.  */
  int: 32;           /* Padding.  */
# endif
#endif
};
int nanosleep(const struct timespec *req, struct timespec *rem);
unsigned sleep(unsigned seconds);
#endif
