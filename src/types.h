//
// Created by Giuseppe Coviello on 11/27/26.
//
//
// Types are INTEGER, REAL, and STRING
//
// The REAL uses a software fixed-point arithmetic.
//

#ifndef PRG32_BASIC_TYPES_H
#define PRG32_BASIC_TYPES_H

#include <stdint.h>
#include "qnum.h"

//
// @brief Type of runtime variable, just int or string are supported by now
//
typedef enum {
    VAL_INT,
    VAL_REAL,
    VAL_STR
} val_type_t;

//
// @brief Zero-copy string descriptor pointing directly into source text
// or managed by the ping-pong string poo
//
typedef struct {
    const char *ptr; // pointer to string character data
    uint16_t len; // length of the string
    bool is_literal; // true if ptr references read-only source asset
} string_val_t;

//
// @brief Value container for variable storage.
//
typedef struct {
    val_type_t type;

    union {
        int32_t int_val;
        qnum_t real_val;
        string_val_t str_val;
    } data;
} runtime_val_t;


#endif //PRG32_BASIC_TYPES_H
