//
// Created by Giuseppe Coviello on 11/24/26.
//

#include "console.h"

#include "fp.h"

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
    printf("%s", str);
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

void console_print_fp(qnum_t v) {
    // Compute 10^decimals
    uint64_t scale10 = 1ULL;
    for (int i = 0; i < FP_MAX_DECIMALS; i++) {
        scale10 *= 10ULL;
    }

    /* Convert two's-complement / floor-style (int_part + frac_part / 2^32)
       to sign + non-negative magnitude (whole + frac_bits / 2^32). */
    bool neg = false;
    uint64_t whole = 0;
    uint32_t frac_bits = 0;

    if (v.integer < 0) {
        neg = true;
        /* Magnitude of (int_part + frac_part / 2^32) is:
           (|int_part| - 1) + ((2^32 - frac_part) / 2^32), or |int_part| if frac_part == 0.
           Handle INT32_MIN safely via uint64_t. */
        uint64_t abs_hi = (uint64_t)(-(int64_t)v.integer);
        if (v.fraction == 0U) {
            whole = abs_hi;
            frac_bits = 0U;
        } else {
            whole = abs_hi - 1ULL;
            frac_bits = (uint32_t)(0x100000000ULL - (uint64_t)v.fraction);
        }
    } else {
        neg = false;
        whole = (uint64_t)(uint32_t)v.integer;
        frac_bits = v.fraction;
    }

    /* Scale fractional bits to 10^decimals with round-to-nearest */
    uint64_t dec_frac = 0;
    if (frac_bits > 0U) {
        uint64_t prod = (uint64_t)frac_bits * scale10;
        dec_frac = (prod + (1ULL << 31)) >> 32;
        if (dec_frac >= scale10) {
            dec_frac = 0ULL;
            whole += 1ULL;
        }
    }

    /* Avoid "-0.000000" when magnitude rounds to zero */
    if (whole == 0ULL && dec_frac == 0ULL) {
        neg = false;
    }

    if (neg) {
        console_putc('-');
    }
    console_print_int(whole);
    console_putc('.');
    while  (dec_frac < scale10 / 10) {
        dec_frac *= 10;
        console_putc('0');
    }
    console_print_int(dec_frac);
}

void console_print_nl() {
    console_putc('\n');
}