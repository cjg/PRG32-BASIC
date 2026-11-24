//
// Created by Giuseppe Coviello on 11/24/26.
//

#ifndef PRG32_BASIC_EXPRESSION_H
#define PRG32_BASIC_EXPRESSION_H

#include "basic_rt.h"
#include "parser.h"

//
// @brief Evaluates an expression node recursively with safety checks.
//
runtime_result_t eval_expression(expr_node_t *expr, runtime_val_t *out_val, symbol_entry_t symbols[BASIC_MAX_VARS]);

#endif //PRG32_BASIC_EXPRESSION_H
