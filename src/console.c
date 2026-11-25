//
// Created by Giuseppe Coviello on 11/24/26.
//

#include "console.h"

#include "logging.h"

#ifdef PRG32_BASIC_DEBUG
#include <stdio.h>
#else
#include <prg32.h>
#endif

static void console_putc(char c) {
#ifdef PRG32_BASIC_DEBUG
    printf("%c", c);
    fflush(stdout);
#else
    prg32_console_putc(c);
#endif
}

void console_print_string(const char *str) {
#ifdef PRG32_BASIC_DEBUG
    printf("%s\n", str);
    fflush(stdout);
#else
    prg32_console_write(str);
#endif
}

void console_print_int(int32_t value) {
    if (value == 0) {
        console_putc('0');
        return;
    }
    if (value < 0) {
        console_putc('-');
        value = -value;
    }

    char buffer[10] = {0};
    uint16_t len = 0;

    while (value > 0) {
        buffer[len++] = '0' + (value % 10);
        value /= 10;
    }

    while (len > 0) {
        console_putc(buffer[len - 1]);
        len--;
    }
}

void console_print_nl() {
    console_putc('\n');
#ifdef PRG32_BASIC_DEBUG
    fflush(stdout);
#endif
}