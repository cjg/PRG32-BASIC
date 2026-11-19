//
// Created by Giuseppe Coviello on 11/17/26.
//

#ifndef PRG32_BASIC_LOGGING_H
#define PRG32_BASIC_LOGGING_H

#include <stdio.h>

#define NDEBUG

#ifndef NDEBUG
    #define DPRINTF(fmt, ...) \
        fprintf(stderr, "[DEBUG] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
    #define DPRINTF(fmt, ...) do {} while (0)
#endif

#endif //PRG32_BASIC_LOGGING_H
