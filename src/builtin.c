//
// Created by Giuseppe Coviello on 11/26/26.
//

#include "builtin.h"

#include "console.h"

#include <string.h>

#ifdef PRG32_BASIC_DEBUG
#include <stdlib.h>
#include <time.h>
#else
#include <prg32.h>
#endif

#define BUILTIN_OK() (builtin_result_t) { .has_error = false, .error = NULL }
#define BUILTIN_ERR(msg) ((builtin_result_t) { .has_error = false, .error = (msg) })

extern int atoi(const char *);

static builtin_result_t execute_int(runtime_val_t *out_val, runtime_val_t *args) {
    out_val->type = VAL_INT;
    if (args[0].type == VAL_INT) {
        out_val->data.int_val = args[0].data.int_val;
    } else if (args[0].type == VAL_REAL) {
        out_val->data.int_val = qnum_to_int(args[0].data.real_val);
    } else if (args[0].type == VAL_STR) {
        out_val->data.int_val = atoi(args[0].data.str_val.ptr);
    } else {
        return BUILTIN_ERR("INVALID ARGUMENT TYPE FOR INT");
    }
    return BUILTIN_OK();
}

static builtin_result_t execute_ticks(runtime_val_t *out_val, runtime_val_t *args) {
    // FIXME: VAL_INT is too small
    out_val->type = VAL_INT;
#ifdef PRG32_BASIC_DEBUG
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    out_val->data.int_val = (int32_t) ((uint64_t) ts.tv_sec * 1000ULL + (uint64_t) ts.tv_nsec / 1000000ULL);
#else
    out_val->data.int_val = prg32_ticks_ms();
#endif
    return BUILTIN_OK();
}

static builtin_result_t execute_rnd(runtime_val_t *out_val, runtime_val_t *args) {
    out_val->type = VAL_REAL;

#ifdef PRG32_BASIC_DEBUG
    srand(time(NULL));
    int r = rand();
    out_val->data.real_val = float_to_qnum((double) r / RAND_MAX);
#else
#define MAX_RANDOM (UINT32_MAX / 2 - 1)
    int32_t r = prg32_random(0, MAX_RANDOM);
    out_val->data.real_val = qnum_div(int_to_qnum(r), int_to_qnum(MAX_RANDOM));
#undef MAX_RANDOM
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
    {.name = "INT", .num_args = 1, .execute = execute_int},
    {.name = "RND", .num_args = 0, .execute = execute_rnd},
    {.name = "SQR", .num_args = 1, .execute = execute_sqrt},
    {.name = "TICKS", .num_args = 0, .execute = execute_ticks},
};

bool builtin_lookup_function(builtin_spec_t *spec, const char *name, uint16_t len) {
    for (unsigned i = 0; i < sizeof(builtin_functions) / sizeof(builtin_functions[0]); i++) {
        if (strlen(builtin_functions[i].name) == len && strncasecmp(name, builtin_functions[i].name, len) == 0) {
            spec->num_args = builtin_functions[i].num_args;
            spec->execute = builtin_functions[i].execute;
            return true;
        }
    }
    return false;
}
