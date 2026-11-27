//
// Created by Giuseppe Coviello on 11/27/26.
//

#ifndef PRG32_BASIC_FP_H
#define PRG32_BASIC_FP_H

#include "qnum.h"

#include <stdint.h>

#define FP_MAX_DECIMALS 9

//
// Parse a fixed-point literal in a string to a qnum
//
qnum_t fp_parse_qnum(const char *str);

#endif //PRG32_BASIC_FP_H
