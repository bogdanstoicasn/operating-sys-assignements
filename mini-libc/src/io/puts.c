#include <sys/stat.h>
#include <internal/syscall.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "stdio.h"
#include "string.h"

#define LEN strlen(str) + 1

int puts(const char *str) {
    char buff[LEN]; //solved issue
    strcpy(buff, str);
    strcat(buff, "\n");
    return write(1, buff, strlen(buff));
}
