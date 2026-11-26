//
// Created by Giuseppe Coviello on 11/17/26.
//

#include "parser.h"

#include <assert.h>

#include "arena.h"
#include "lexer.h"
#include "logging.h"

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

//
// @brief Advances parser lookahead tokens.
//
static void parser_advance(parser_ctx_t *parser) {
    parser->current = parser->peek_token;
    parser->peek_token = lexer_next(&parser->lexer);
    DEBUG("Current lexer token type: %d", parser->current.type);
}

//
// @brief Parses a primary expression literals, variables, parentheses, built-ins
//
static expr_node_t *parse_primary(parser_ctx_t *parser) {
    expr_node_t *node = (expr_node_t *) arena_alloc(sizeof(expr_node_t));
    if (!node) return NULL;

    if (parser->current.type == TOKEN_INT_LITERAL) {
        node->type = EXPR_INT_LITERAL;
        node->data.int_val = (int32_t) atoi(parser->current.start);
        parser_advance(parser);
        return node;
    }

    if (parser->current.type == TOKEN_STR_LITERAL) {
        node->type = EXPR_STR_LITERAL;
        node->data.str_val = (string_val_t){
            .ptr = parser->current.start,
            .len = (uint16_t) parser->current.length,
            .is_literal = true
        };
        parser_advance(parser);
        return node;
    }

    if (parser->current.type == TOKEN_IDENTIFIER) {
        if (parser->peek_token.type == TOKEN_LPAREN) {
            // function call
            token_t fn_ident = parser->current;
            parser_advance(parser);

            return parse_call(parser, fn_ident);
        }
        node->type = EXPR_VARIABLE;
        node->data.var_slot = symbol_table_get_or_create(parser->symbol_table,
                                                         parser->current.start,
                                                         parser->current.length);
        parser_advance(parser);
        return node;
    }

    parser->has_error = true;
    return NULL;
}

/**
 * @brief Pratt parsing expression precedence router
 */
static expr_node_t *parse_expression(parser_ctx_t *parser) {
    DEBUG("parse_expression()");

    /* Simplified recursive descent expression parser for binary ops */
    expr_node_t *left = parse_primary(parser);

    while (parser->current.type == TOKEN_PLUS || parser->current.type == TOKEN_MINUS ||
           parser->current.type == TOKEN_ASTERISK || parser->current.type == TOKEN_SLASH ||
           parser->current.type == TOKEN_MOD ||
           parser->current.type == TOKEN_NEQ || parser->current.type == TOKEN_LT ||
           parser->current.type == TOKEN_GT || parser->current.type == TOKEN_LTE ||
           parser->current.type == TOKEN_GTE) {
        token_t op_token = parser->current;
        parser_advance(parser);

        expr_node_t *right = parse_primary(parser);
        expr_node_t *bin_node = (expr_node_t *) arena_alloc(sizeof(expr_node_t));
        bin_node->type = EXPR_BINARY;
        bin_node->data.binary.op = op_token.type;
        bin_node->data.binary.left = left;
        bin_node->data.binary.right = right;
        left = bin_node;
    }

    return left;
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

static expr_node_t *parse_call(parser_ctx_t *parser, token_t ident)
{
    // Consume '('
    parser_advance(parser);

    expr_node_t *call = (expr_node_t *) arena_alloc(sizeof(expr_node_t));
    // TODO: return out of memory error properly
    if (!call) return NULL;

    call->type = EXPR_CALL;
    call->data.call.fn_name = ident.start;
    call->data.call.fn_name_len = ident.length;
    call->data.call.arg_count = 0;

    if (parser->current.type != TOKEN_RPAREN) {
        do {
            expr_node_t *arg = parse_expression(parser);
            if (!arg || call->data.call.arg_count >= BASIC_MAX_CALL_ARGS) {
                // FIXME: handle error properly
                return NULL;
            }
            call->data.call.args[call->data.call.arg_count++] = arg;
        } while (parser_match(parser, TOKEN_COMMA));
    }

    if (!parser_match(parser, TOKEN_RPAREN)) {
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
            return;
        }
    }

    // PRINT statement
    if (parser->current.type == TOKEN_KW_PRINT) {
        result->node->type = NODE_PRINT;
        parser_advance(parser);
        result->node->data.print_stmt.expr = parse_expression(parser);
        return;
    }

    /* WHILE Loop */
    if (parser->current.type == TOKEN_KW_WHILE) {
        DEBUG("WHILE loop");
        result->node->type = NODE_WHILE;
        parser_advance(parser);
        result->node->data.while_stmt.cond = parse_expression(parser);
        DEBUG("Parsed loop condition %p", result->node->data.while_stmt.cond);

        ast_node_t head_dummy = {0};
        ast_node_t *curr = &head_dummy;
        bool end_found = false;
        while (parser->current.type != TOKEN_KW_END && parser->current.type != TOKEN_EOF) {
            DEBUG("Parsing WHILE body, current type: %d", parser->current.type);
            parser_result_t res;
            res.node = (ast_node_t *) arena_alloc(sizeof(ast_node_t));
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
            DEBUG("Parsed statement in WHILE body");
            curr->next = res.node;
            if (curr->next) curr = curr->next;
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
        result->node = (ast_node_t *) arena_alloc(sizeof(ast_node_t));
        if (!result->node) {
            result->has_error = true;
            result->error.line = parser->current.line;
            result->error.message = "no memory available";
            return;
        }
        result->node->parent = parent;
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
        parser_result_t statement_result;
        parse_statement(&statement_result, &parser, curr);
        if (statement_result.has_error) {
            DEBUG("syntax error?!");
            result->has_error = true;
            result->error = statement_result.error;
            return;
        }
        assert(statement_result.node != NULL);
        DEBUG("Parsed statement type %d\n", statement_result.node->type);
        curr->next = statement_result.node;
        curr = statement_result.node;
        statement_result.node->next = NULL;
    }

    // TODO: validate AST

    head_dummy.next->parent = NULL;
    result->has_error = false;
    result->node = head_dummy.next;
}
