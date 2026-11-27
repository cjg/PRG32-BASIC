//
// Created by Giuseppe Coviello on 11/27/26.
//

#include "fp.h"

#include "logging.h"

#include <stdlib.h>

/* Converts decimal fraction frac_num / frac_den (where frac_den = 10^k, k <= 9)
   to 32-bit binary fraction bits / 2^32.
*/
static uint32_t dec_frac_to_q32(uint64_t frac_num, uint64_t frac_den) {
    if (frac_den <= 1ULL || frac_num == 0ULL) {
        return 0U;
    }
    // Round to nearest: (num * 2^32 + den / 2) / den
    uint64_t num_shifted = frac_num << 32;
    uint64_t half = frac_den >> 1;
    if (num_shifted <= UINT64_MAX - half) {
        num_shifted += half;
    }
    return (uint32_t)(num_shifted / frac_den);
}

qnum_t fp_parse_qnum(const char *str) {
    qnum_t qnum;
    qnum.integer = 0;
    qnum.fraction = 0;

    int32_t sign = 1;

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
        qnum.integer = qnum.integer * 10 + (*str - '0');
        str++;
    }

    if (*str == '.') {
        str++;
        // skip decimal dot
        uint16_t digits = 0;
#define MAX_DEC_DIGITS 9

        uint32_t frac_den = 1;
        uint16_t leading_zeroes = 0;
        // count leading zeroes
        while (*str == '0') {
            frac_den *= 10;
            str++;
            leading_zeroes++;
            digits++;
        }

        // collect remaining digits
        uint32_t frac_num = 0;
        while (*str >= '0' && *str <= '9') {
            if (digits < MAX_DEC_DIGITS) {
                frac_den *= 10;
                frac_num = frac_num * 10 + (*str - '0');
                digits++;
            }
            str++;
        }

        qnum_t f;
        f.integer = 0;
        f.fraction = dec_frac_to_q32(frac_num, frac_den);
        DEBUG("Leading zeroes: %u frac_num: %u frac_den: %u -> %lf", leading_zeroes, frac_num, frac_den, qnum_to_double(f));

        if (frac_num > 0) {
            qnum.fraction = dec_frac_to_q32(frac_num, frac_den);
        }
    }

    if (sign < 0) {
        qnum.integer = -(qnum.integer + 1);
        // maybe i could do better than 1-another qnum, but it is good enough for now
        qnum_t r = qnum_sub((qnum_t){1, 0}, qnum);
        qnum.fraction = r.fraction;
    }
    return qnum;
}