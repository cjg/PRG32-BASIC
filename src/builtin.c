//
// Created by Giuseppe Coviello on 11/26/26.
//

#include "builtin.h"

#include "console.h"

#include <string.h>

#ifdef PRG32_BASIC_DEBUG
#include <time.h>
#else
#include <prg32.h>
#endif

#define BUILTIN_OK() (builtin_result_t) { .has_error = false, .error = NULL }
#define BUILTIN_ERR(msg) ((builtin_result_t) { .has_error = false, .error = (msg) })

static builtin_result_t execute_ticks(runtime_val_t *out_val, runtime_val_t *args) {
    // FIXME: VAL_INT is too small
    out_val->type = VAL_INT;
#ifdef PRG32_BASIC_DEBUG
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    out_val->data.int_val = (int32_t)((uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL);
#else
    out_val->data.int_val = prg32_ticks_ms();
#endif
    return BUILTIN_OK();
}

static builtin_result_t execute_sqrt(runtime_val_t *out_val, runtime_val_t *args) {
    // if the argument is an INT we convert it to REAL
    if (args[0].type == VAL_INT) {
        args[0].type = VAL_REAL;
        args[0].data.real_val = int_to_qnum(args[0].data.int_val);
    }

    if (args[0].type != VAL_REAL) {
        return BUILTIN_ERR("INVALID ARGUMENT TYPE FOR SQRT");
    }

    out_val->type = VAL_REAL;
    out_val->data.real_val = qnum_sqrt(args[0].data.real_val);
    return BUILTIN_OK();
}

static const builtin_spec_t builtin_functions[] = {
    {.id = TICKS_FN, .name = "TICKS", .num_args = 0, .execute = execute_ticks},
    {.id = SQRT_FN, .name = "SQRT", .num_args = 1, .execute = execute_sqrt},
};

builtin_spec_t builtin_lookup_function(const char *name, uint16_t len) {
    for (unsigned i = 0; i < sizeof(builtin_functions) / sizeof(builtin_functions[0]); i++) {
        if (strlen(builtin_functions[i].name) == len && strncasecmp(name, builtin_functions[i].name, len) == 0) {
            return (builtin_spec_t ){.num_args = builtin_functions[i].num_args, .execute = builtin_functions[i].execute};
        }
    }
    return (builtin_spec_t ){.id = UNKNOWN_FN};
}