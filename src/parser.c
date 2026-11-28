//
// Created by Giuseppe Coviello on 11/17/26.
//

#include "parser.h"

#include <assert.h>

#include "arena.h"
#include "lexer.h"
#include "logging.h"
#include "fp.h"

#include <stdlib.h>

// parser context tracking lexer state and lookahead token
typedef struct {
    lexer_t lexer;
    token_t current;
    token_t peek_token;
    symbol_table_t *symbol_table;
    bool has_error;
} parser_ctx_t;

static expr_node_t *parse_call(parser_ctx_t *parser, token_t ident);
static expr_node_t *parse_expression(parser_ctx_t *parser);

typedef enum {
    PREC_NONE = 0,
    PREC_COMPARISON, /* =, <>, <, >, <=, >= */
    PREC_TERM,       /* +, - */
    PREC_FACTOR,     /* *, /, % */
    PREC_UNARY       /* unary -, + */
} precedence_t;

static precedence_t get_infix_precedence(token_type_t type) {
    switch (type) {
        case TOKEN_ASSIGN:
        case TOKEN_LT:
        case TOKEN_GT:
        case TOKEN_LTE:
        case TOKEN_GTE:
        case TOKEN_NEQ:
            return PREC_COMPARISON;
        case TOKEN_PLUS:
        case TOKEN_MINUS:
            return PREC_TERM;
        case TOKEN_ASTERISK:
        case TOKEN_SLASH:
        case TOKEN_MOD:
            return PREC_FACTOR;
        default:
            return PREC_NONE;
    }
}

//
// @brief Allocates and initializes an expression AST node from the static arena.
//
static expr_node_t *alloc_expr_node(parser_ctx_t *parser, int type, uint32_t line_number) {
    expr_node_t *node = (expr_node_t *) arena_alloc(sizeof(expr_node_t));
    if (!node) {
        parser->has_error = true;
        return NULL;
    }
    node->type = type;
    node->line_number = line_number;
    return node;
}

//
// @brief Advances parser lookahead tokens.
//
static void parser_advance(parser_ctx_t *parser) {
    parser->current = parser->peek_token;
    parser->peek_token = lexer_next(&parser->lexer);
    DEBUG("Current lexer token type: %d", parser->current.type);
}

//
// @brief Checks if current token matches expected type.
//
static bool parser_match(parser_ctx_t *parser, token_type_t type) {
    if (parser->current.type == type) {
        parser_advance(parser);
        return true;
    }
    return false;
}

static expr_node_t *parse_expression_bp(parser_ctx_t *parser, precedence_t min_prec);

//
// @brief Parses a prefix expression: literals, variables, parentheses, unary ops, calls
//
static expr_node_t *parse_prefix(parser_ctx_t *parser) {
    token_t tok = parser->current;
    DEBUG("parse_prefix() token type: %d", tok.type);

    if (tok.type == TOKEN_LPAREN) {
        parser_advance(parser);
        expr_node_t *inner = parse_expression_bp(parser, PREC_NONE);
        if (!inner) {
            return NULL;
        }
        if (!parser_match(parser, TOKEN_RPAREN)) {
            parser->has_error = true;
            return NULL;
        }
        return inner;
    }

    if (tok.type == TOKEN_MINUS || tok.type == TOKEN_PLUS) {
        DEBUG("parse_prefix() UNARY %d", tok.type);
        parser_advance(parser);
        expr_node_t *operand = parse_expression_bp(parser, PREC_UNARY);
        if (!operand) {
            return NULL;
        }
        expr_node_t *node = alloc_expr_node(parser, EXPR_UNARY, tok.line);
        if (!node) {
            return NULL;
        }
        node->data.unary.op = tok.type;
        node->data.unary.expr = operand;
        return node;
    }

    if (tok.type == TOKEN_INT_LITERAL) {
        expr_node_t *node = alloc_expr_node(parser, EXPR_INT_LITERAL, tok.line);
        if (!node) return NULL;
        node->data.int_val = (int32_t) atoi(tok.start);
        parser_advance(parser);
        return node;
    }

    if (tok.type == TOKEN_REAL_LITERAL) {
        expr_node_t *node = alloc_expr_node(parser, EXPR_REAL_LITERAL, tok.line);
        if (!node) return NULL;
        node->data.real_val = fp_parse_qnum(tok.start);
        parser_advance(parser);
        return node;
    }

    if (tok.type == TOKEN_STR_LITERAL) {
        expr_node_t *node = alloc_expr_node(parser, EXPR_STR_LITERAL, tok.line);
        if (!node) return NULL;
        node->data.str_val = (string_val_t){
            .ptr = tok.start,
            .len = (uint16_t) tok.length,
            .is_literal = true
        };
        parser_advance(parser);
        return node;
    }

    if (tok.type == TOKEN_IDENTIFIER) {
        if (parser->peek_token.type == TOKEN_LPAREN) {
            // function call
            parser_advance(parser);
            return parse_call(parser, tok);
        }
        expr_node_t *node = alloc_expr_node(parser, EXPR_VARIABLE, tok.line);
        if (!node) return NULL;
        node->data.var_slot = symbol_table_get_or_create(parser->symbol_table,
                                                         tok.start,
                                                         tok.length);
        parser_advance(parser);
        return node;
    }

    parser->has_error = true;
    return NULL;
}

/**
 * @brief Pratt parsing expression precedence router
 */
static expr_node_t *parse_expression_bp(parser_ctx_t *parser, precedence_t min_prec) {
    DEBUG("parse_expression_bp(min_prec=%d)", min_prec);

    expr_node_t *left = parse_prefix(parser);
    if (!left) {
        return NULL;
    }

    DEBUG("parse_expression_bp(): left->type = %d", left->type);

    for (;;) {
        precedence_t prec = get_infix_precedence(parser->current.type);
        if (prec <= min_prec) {
            break;
        }

        token_t op_token = parser->current;
        parser_advance(parser);

        expr_node_t *right = parse_expression_bp(parser, prec);
        if (!right) {
            return NULL;
        }

        expr_node_t *bin_node = alloc_expr_node(parser, EXPR_BINARY, op_token.line);
        if (!bin_node) {
            return NULL;
        }
        bin_node->data.binary.op = op_token.type;
        bin_node->data.binary.left = left;
        bin_node->data.binary.right = right;
        left = bin_node;
    }

    return left;
}

static expr_node_t *parse_expression(parser_ctx_t *parser) {
    return parse_expression_bp(parser, PREC_NONE);
}

static expr_node_t *parse_call(parser_ctx_t *parser, token_t ident)
{
    // Consume '('
    parser_advance(parser);

    expr_node_t *call = alloc_expr_node(parser, EXPR_CALL, ident.line);
    if (!call) return NULL;

    call->data.call.fn_name = ident.start;
    call->data.call.fn_name_len = (uint16_t) ident.length;
    call->data.call.arg_count = 0;

    if (parser->current.type != TOKEN_RPAREN) {
        do {
            expr_node_t *arg = parse_expression(parser);
            if (!arg || call->data.call.arg_count >= BASIC_MAX_CALL_ARGS) {
                parser->has_error = true;
                return NULL;
            }
            call->data.call.args[call->data.call.arg_count++] = arg;
        } while (parser_match(parser, TOKEN_COMMA));
    }

    if (!parser_match(parser, TOKEN_RPAREN)) {
        parser->has_error = true;
        return NULL;
    }

    return call;
}

//
// @brief Parses an individual statement node.
//
static void parse_statement(parser_result_t *result, parser_ctx_t *parser, ast_node_t *parent) {
    assert(result != NULL);

    DEBUG("Current token type: %d\n", parser->current.type);
    // skip newlines
    while (parser->current.type == TOKEN_NEWLINE) {
        DEBUG("Advancing NL");
        parser_advance(parser);
    }

    // reached end of file
    if (parser->current.type == TOKEN_EOF) {
        result->has_error = false;
        result->node = NULL;
        DEBUG("End of file");
        return;
    }

    DEBUG("Parsing statement at '%s'", parser->current.start);

    result->node = (ast_node_t *) arena_alloc(sizeof(ast_node_t));
    if (!result->node) {
        result->has_error = true;
        result->error.line = parser->current.line;
        result->error.message = "no memory available";
        return;
    }

    result->has_error = false;
    result->error = (parser_error_t){0};
    result->node->parent = parent;
    result->node->line_number = parser->current.line;
    result->node->next = NULL;

    // Assignment: var = expr
    if (parser->current.type == TOKEN_IDENTIFIER && parser->peek_token.type == TOKEN_ASSIGN) {
        token_t id_token = parser->current;
        parser_advance(parser);
        if (parser->current.type == TOKEN_ASSIGN) {
            parser_advance(parser);
            result->node->type = NODE_ASSIGN;
            result->node->data.assign.var_slot = symbol_table_get_or_create(parser->symbol_table,
                                                                            id_token.start,
                                                                            id_token.length);
            result->node->data.assign.expr = parse_expression(parser);
            if (result->node->data.assign.expr == NULL) {
                result->has_error = true;
                result->error.line = parser->current.line;
                result->error.message = "EXPECTED EXPRESSION";
            }
            return;
        }
    }

    // PRINT statement
    if (parser->current.type == TOKEN_KW_PRINT) {
        result->node->type = NODE_PRINT;
        parser_advance(parser);

        expr_node_t head = {0};
        expr_node_t *current = &head;
        do {
            expr_node_t *expr = parse_expression(parser);
            if (expr == NULL) {
                parser->has_error = true;
                result->has_error = true;
                result->error.line = parser->current.line;
                result->error.message = "EXPECTED EXPRESSION";
                return;
            }
            current->next = expr;
            expr->next = NULL;
            current = current->next;
        } while (parser_match(parser, TOKEN_COMMA));

        result->node->data.print_stmt.expr = head.next;
        return;
    }

    /* WHILE Loop */
    if (parser->current.type == TOKEN_KW_WHILE) {
        DEBUG("WHILE loop");
        result->node->type = NODE_WHILE;
        parser_advance(parser);
        result->node->data.while_stmt.cond = parse_expression(parser);
        if (result->node->data.while_stmt.cond == NULL) {
            result->has_error = true;
            result->error.line = parser->current.line;
            result->error.message = "EXPECTED EXPRESSION";
            return;
        }
        DEBUG("Parsed loop condition %p", result->node->data.while_stmt.cond);

        ast_node_t head_dummy = {0};
        ast_node_t *curr = &head_dummy;
        bool end_found = false;
        for (;;) {
            while (parser->current.type == TOKEN_NEWLINE) {
                parser_advance(parser);
            }
            if (parser->current.type == TOKEN_KW_END || parser->current.type == TOKEN_EOF) {
                break;
            }
            DEBUG("Parsing WHILE body, current type: %d", parser->current.type);
            parser_result_t res = {0};
            parse_statement(&res, parser, result->node);
            if (res.has_error) {
                if (res.error.token_type == TOKEN_KW_END) {
                    // that unexpected END belongs here
                    end_found = true;
                    break;
                }
                result->has_error = true;
                result->error = res.error;
                return;
            }
            if (res.node) {
                DEBUG("Parsed statement in WHILE body");
                curr->next = res.node;
                curr = curr->next;
            }
        }
        // go back to evaluate condition
        curr->next = result->node;
        result->node->data.while_stmt.body = head_dummy.next;

        uint32_t line = parser->current.line;
        if ((!end_found && !parser_match(parser, TOKEN_KW_END)) || !parser_match(parser, TOKEN_KW_WHILE)) {
            result->has_error = true;
            result->error.line = line;
            result->error.message = "EXPECTED 'END WHILE'";
        }
        return;
    }

    // function call: identifier followed by left parenthesis
    if (parser->current.type == TOKEN_IDENTIFIER && parser->peek_token.type == TOKEN_LPAREN) {
        token_t fn_ident = parser->current;
        parser_advance(parser);

        expr_node_t *call_expr = parse_call(parser, fn_ident);
        if (!call_expr) {
            result->has_error = true;
            result->error.line = fn_ident.line;
            result->error.message = "PARSING FUNCTION CALL";
            return;
        }
        result->node->type = NODE_CALL;
        result->node->data.call.call_expr = call_expr;
        return;
    }

    result->has_error = true;
    result->error.line = parser->current.line;
    result->error.message = "unexpected token";
    result->error.token_type = parser->current.type;
    // skip unknown tokens to prevent infinite loops
    DEBUG("Unexpected token %d '%s'", parser->current.type, parser->current.start);
    parser_advance(parser);
}

void parser_parse(parser_result_t *result, const char *src, size_t len, symbol_table_t *symbol_table) {
    parser_ctx_t parser;
    parser.symbol_table = symbol_table;
    lexer_init(&parser.lexer, src, len);
    parser.has_error = false;

    // calling advance twice to initialize current and next (peek_token)
    parser_advance(&parser);
    parser_advance(&parser);

    ast_node_t head_dummy = {0};
    ast_node_t *curr = &head_dummy;

    while (parser.current.type != TOKEN_EOF && !parser.has_error) {
        DEBUG("Parsing statement |%s|\n", parser.current.start);
        parser_result_t statement_result = {0};
        parse_statement(&statement_result, &parser, curr);
        if (statement_result.has_error) {
            DEBUG("syntax error?!");
            result->has_error = true;
            result->error = statement_result.error;
            return;
        }
        if (statement_result.node == NULL) {
            break;
        }
        DEBUG("Parsed statement type %d\n", statement_result.node->type);
        curr->next = statement_result.node;
        curr = statement_result.node;
        statement_result.node->next = NULL;
    }

    // TODO: validate AST

    if (head_dummy.next) {
        head_dummy.next->parent = NULL;
    }
    result->has_error = false;
    result->node = head_dummy.next;
}
