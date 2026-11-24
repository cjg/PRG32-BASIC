//
// Created by Giuseppe Coviello on 11/17/26.
//

#ifndef PRG32_BASIC_LOGGING_H
#define PRG32_BASIC_LOGGING_H


#ifdef PRG32_BASIC_DEBUG
#include <stdio.h>
#define INFO(msg) fprintf(stderr, "[INFO] %s:%d: %s", __FILE__, __LINE__, (msg))
#define DEBUG(fmt, ...) \
        fprintf(stderr, "[DEBUG] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#include <prg32.h>
#define INFO(msg) do {prg32_console_write("\n--" ); prg32_console_write((msg)); prg32_console_write(" --\n");}while(0)
#define DEBUG(fmt, ...) do {} while (0)
#endif

#endif //PRG32_BASIC_LOGGING_H
