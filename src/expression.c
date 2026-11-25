//
// Created by Giuseppe Coviello on 11/24/26.
//

#include "expression.h"

#include "lexer.h"
#include "logging.h"
#include "strpool.h"

#include <assert.h>
#include <string.h>

//
// Evaluate a binary expression
//
static runtime_result_t eval_binary(expr_node_t *expr, runtime_val_t *out_val, symbol_entry_t symbols[BASIC_MAX_VARS]) {
    runtime_val_t left, right;
    CHECK_RUNTIME_ERR(eval_expression(expr->data.binary.left, &left, symbols));

    // push temporary root for string evaluation safety during GC
    if (left.type == VAL_STR) {
        strpool_push_root(&left.data.str_val);
    }

    runtime_result_t result = eval_expression(expr->data.binary.right, &right, symbols);
    if (result.has_error) {
        if (left.type == VAL_STR) {
            // how I wish we had defer()
            strpool_pop_roots(1);
        }
        return result;
    }

    // string concatenation
    if (expr->data.binary.op == TOKEN_PLUS && left.type == VAL_STR && right.type == VAL_STR) {
        uint16_t total_len = left.data.str_val.len + right.data.str_val.len;
        const char *p = strpool_alloc(NULL, total_len);
        if (!p) {
            if (left.type == VAL_STR) strpool_pop_roots(1);
            return RUNTIME_ERR(expr, "OUT OF MEMORY");
        }
        /* Safe concatenation into pool buffer */
        char *dest = (char *) p;
        memcpy(dest, left.data.str_val.ptr, left.data.str_val.len);
        memcpy(dest + left.data.str_val.len, right.data.str_val.ptr, right.data.str_val.len);

        if (left.type == VAL_STR) strpool_pop_roots(1);

        out_val->type = VAL_STR;
        out_val->data.str_val = (string_val_t){.ptr = p, .len = total_len, .is_literal = false};
        return RUNTIME_OK();
    }

    if (left.type == VAL_STR) strpool_pop_roots(1);

    // standard integer binary operations
    if (left.type == VAL_INT && right.type == VAL_INT) {
        int32_t l = left.data.int_val;
        int32_t r = right.data.int_val;
        out_val->type = VAL_INT;
        switch (expr->data.binary.op) {
            case TOKEN_PLUS: out_val->data.int_val = l + r;
                break;
            case TOKEN_MINUS: out_val->data.int_val = l - r;
                break;
            case TOKEN_ASTERISK: out_val->data.int_val = l * r;
                break;
            case TOKEN_SLASH: if (r == 0) return RUNTIME_ERR(expr, "DIVISION BY 0");
                out_val->data.int_val = l / r;
                break;
            case TOKEN_MOD: if (r == 0) return RUNTIME_ERR(expr, "DIVISION BY 0");
                out_val->data.int_val = l % r;
                break;
            case TOKEN_NEQ: out_val->data.int_val = (l != r) ? 1 : 0;
                break;
            case TOKEN_LT: out_val->data.int_val = (l < r) ? 1 : 0;
                break;
            case TOKEN_GT: out_val->data.int_val = (l > r) ? 1 : 0;
                break;
            case TOKEN_LTE: out_val->data.int_val = (l <= r) ? 1 : 0;
                break;
            case TOKEN_GTE: out_val->data.int_val = (l >= r) ? 1 : 0;
                break;
            default: return RUNTIME_ERR(expr, "UNEXPECTED BINARY EXPRESSION");
        }
        return RUNTIME_OK();
    }
    return RUNTIME_ERR(expr, "UNEXPECTED EXPRESSION");
}

runtime_result_t eval_expression(expr_node_t *expr, runtime_val_t *out_val, symbol_entry_t symbols[BASIC_MAX_VARS]) {
    assert(expr != NULL);

    DEBUG("Evaluating expression of type: %d", expr->type);

    switch (expr->type) {
        case EXPR_INT_LITERAL:
            out_val->type = VAL_INT;
            out_val->data.int_val = expr->data.int_val;
            return RUNTIME_OK();

        case EXPR_STR_LITERAL:
            out_val->type = VAL_STR;
            out_val->data.str_val = expr->data.str_val;
            return RUNTIME_OK();

        case EXPR_VARIABLE:
            *out_val = symbols[expr->data.var_slot].val;
            return RUNTIME_OK();

        case EXPR_BINARY:
            return eval_binary(expr, out_val, symbols);

        default:
            return (runtime_result_t){true, expr->line_number, "UNEXPECTED EXPRESSION"};
    }
}
