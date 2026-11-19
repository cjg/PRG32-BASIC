//
// Created by Giuseppe Coviello on 11/18/26.
//

#include <string.h>
#include <stdio.h>

void *memcpy(void *dest, const void *src, size_t n) {
    size_t i = 0;
    char *d = dest;
    const char *s = src;
    while (i < n) {
        *d++ = *s++;
        i++;
    }
    return dest;
}

void *memset(void *dest, int c, size_t n) {
    size_t i = 0;
    char *d = dest;
    while (i < n) {
        *d++ = c;
        i++;
    }
}

size_t strlen(const char *s) {
    size_t i = 0;
    while (s[i] != '\0') {
        i++;
    }
    return i;
}

int printf(const char *format, ...) {
    return 0;
}

int strncasecmp(const char *s1, const char *s2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        unsigned char c1 = (unsigned char)s1[i];
        unsigned char c2 = (unsigned char)s2[i];

        if (c1 >= 'a' && c1 <= 'z') {
            c1 = (unsigned char)(c1 - 'a' + 'A');
        }
        if (c2 >= 'a' && c2 <= 'z') {
            c2 = (unsigned char)(c2 - 'a' + 'A');
        }

        if (c1 != c2) {
            return (int)c1 - (int)c2;
        }

        if (c1 == '\0') {
            return 0;
        }
    }

    return 0;
}

int atoi(const char *str) {
    int result = 0;
    int sign = 1;

    // skip leading whitespace (spaces, tabs, newlines)
    while (*str == ' ' || (*str >= '\t' && *str <= '\r')) {
        str++;
    }

    // parse optional sign
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    // accumulate digits until first non-digit or '\0'
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    return sign * result;
}
