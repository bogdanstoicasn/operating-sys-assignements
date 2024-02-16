// SPDX-License-Identifier: BSD-3-Clause

#include <string.h>

char *strcpy(char *destination, const char *source) {
    if (!destination || !source)
        return NULL;

    char *dst = destination;

    // copy the characters
    while (*source) {
        *destination = *source;
        ++destination;
        ++source;
    }

    // add a null terminator
    *destination = '\0';

    return dst;
}

char *strncpy(char *destination, const char *source, size_t len) {
    if (!destination)
        return NULL;

    char *dest = destination;

    // copy at most 'len' characters
    while (*source && len > 0) {
        *destination = *source;
        ++destination;
        ++source;
        --len;
    }

    // fill the remaining space
    while (len > 0) {
        *destination = '\0';
        ++destination;
        --len;
    }

    return dest;
}

char *strcat(char *destination, const char *source) {
    if (!destination || !source)
        return NULL;

    char *dst = destination;

    // move to the end
    while (*destination)
        ++destination;

    // append characters
    while (*source) {
        *destination = *source;
        ++destination;
        ++source;
    }

    // null-terminate
    *destination = '\0';

    return dst;
}

char *strncat(char *destination, const char *source, size_t len) {
    if (!destination || !source)
        return NULL;

    char *dst = destination;

    // move to the end
    while (*destination)
        ++destination;

    // append at most 'len' characters
    while (*source && len > 0) {
        *destination = *source;
        ++destination;
        ++source;
        --len;
    }

    // null-terminate
    *destination = '\0';

    return dst;
}

int strcmp(const char *str1, const char *str2) {
    if (!str1 || !str2)
        return -1;

    while (*str1 && (*str1 == *str2))
        ++str1, ++str2;

    return *(unsigned char *)str1 - *(unsigned char *)str2;
}

int strncmp(const char *str1, const char *str2, size_t len) {
    if (!str1 || !str2)
        return -1;

    while (len > 0 && *str1 && *str2 && *str1 == *str2)
        ++str1, ++str2, --len;

    if (len == 0)
		return 0;

    return *(unsigned char *)str1 - *(unsigned char *)str2;
}

size_t strlen(const char *str)
{
	size_t i = 0;

	for (; *str != '\0'; str++, i++)
		;

	return i;
}

char *strchr(const char *str, int c) {
    if (!str)
        return NULL;

    while (*str != '\0') {
        if (*str == c)
            return (char *)str; // found
        ++str;
    }

    if (c == '\0')
        return (char *)str;

    return NULL;
}

char *strrchr(const char *str, int c) {
    if (!str)
        return NULL;
    const char *last_oc = NULL;

    while (*str != '\0') {
        if (*str == c)
            last_oc = str;
        ++str;
    }

    if (c == '\0')
        return (char *)str;

    return (char *)last_oc;
}

char *strstr(const char *haystack, const char *needle) {
    if (!haystack || !needle)
        return NULL;

    if (*needle == '\0')
        return (char *)haystack;

    while (*haystack) {
        const char *h = haystack;
        const char *n = needle;

        while (*h && *n && *h == *n)
            ++h, ++n;

        //end of the needle
        if (*n == '\0')
            return (char *)haystack;

        ++haystack;
    }

    return NULL;
}


char *strrstr(const char *haystack, const char *needle) {
    if (!haystack || !needle)
        return NULL;

    size_t haystack_len = strlen(haystack);
    size_t needle_len = strlen(needle);

    if (needle_len == 0)
        return (char *)(haystack + haystack_len);

    if (needle_len > haystack_len)
        return NULL;

    for (const char *pointer = haystack + haystack_len - needle_len; pointer >= haystack; --pointer)
        if (strncmp(pointer, needle, needle_len) == 0)
            return (char *)pointer;
    return NULL;
}

void *memcpy(void *destination, const void *source, size_t num) {
    if (!destination || !source)
        return NULL;

    char *dst = (char *)destination;
    const char *src = (const char *)source;

    size_t i = 0;
	while (i < num) {
		dst[i] = src[i];
		++i;
	}

    return destination;
}

void *memmove(void *destination, const void *source, size_t num) {
    if (!destination|| !source)
        return NULL;

    char *dst = (char *)destination;
    const char *src = (const char *)source;

    // check if there's an overlap
    if (dst < src) {
        for (size_t i = 0; i < num; ++i)
            dst[i] = src[i];
    } else if (dst > src) {
        // copy in reverse
        for (size_t i = num - 1; i != (size_t)-1; --i)
            dst[i] = src[i];
    }

    return destination;
}

int memcmp(const void *ptr1, const void *ptr2, size_t num) {
    if (!ptr1|| !ptr2)
        return -1;

    const unsigned char *p1 = (const unsigned char *)ptr1;
    const unsigned char *p2 = (const unsigned char *)ptr2;

    size_t i = 0;
	while (i < num) {
    	if (p1[i] < p2[i])
        	return -1; // p1 is less than p2
    	else if (p1[i] > p2[i])
        	return 1;  // p1 is greater than p2
    	++i;
	}

    return 0;
	// ptr1 and ptr2 are equal
}

void *memset(void *source, int value, size_t num) {
    if (!source)
        return NULL;

    char *pointer = (char *)source;
    char byte_value = (char)value;

    size_t i = 0;
	while (i < num) {
		pointer[i] = byte_value;
		++i;
	}

    return source;
}
