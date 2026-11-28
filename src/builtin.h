//
// Created by Giuseppe Coviello on 11/26/26.
//

#ifndef PRG32_BASIC_BUILTIN_H
#define PRG32_BASIC_BUILTIN_H

#include "types.h"

#include <stdint.h>

typedef struct {
    bool has_error;
    const char *error;
} builtin_result_t;

typedef builtin_result_t (*builtin_function_handler_t)(runtime_val_t *out_val, runtime_val_t *args);

typedef struct {
    const char *name;
    uint8_t num_args;
    builtin_function_handler_t execute;
} builtin_spec_t;

//
// Return builtin function from name
//
bool builtin_lookup_function(builtin_spec_t *spec, const char *name, uint16_t len);

#endif //PRG32_BASIC_BUILTIN_H
