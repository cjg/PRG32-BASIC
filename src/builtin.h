//
// Created by Giuseppe Coviello on 11/26/26.
//

#ifndef PRG32_BASIC_BUILTIN_H
#define PRG32_BASIC_BUILTIN_H

#include "types.h"

#include <stdint.h>

typedef enum {
    TICKS_FN,

    UNKNOWN_FN,
} builtin_fn_t;

typedef struct {
    bool has_error;
    const char *error;
} builtin_result_t;

typedef builtin_result_t (*builtin_function_handler_t)(runtime_val_t *out_val);

typedef struct {
    builtin_fn_t id;
    const char *name;
    uint8_t num_args;
    builtin_function_handler_t execute;
} builtin_spec_t;

//
// Return builtin function from name
//
builtin_spec_t builtin_lookup_function(const char *name, uint16_t len);

#endif //PRG32_BASIC_BUILTIN_H
