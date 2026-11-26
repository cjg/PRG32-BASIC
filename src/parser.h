//
// Created by Giuseppe Coviello on 11/17/26.
//
// @brief Pratt and recursive-descent parser for PRG32 BASIC.
//
// Parses token streams into hierarchical AST node structures allocated within
// the static memory arena. Performs post-parse validation of control flow links.
//

#ifndef PRG32_BASIC_PARSER_H
#define PRG32_BASIC_PARSER_H

#include "basic_rt.h"
#include "lexer.h"
#include "symbol_table.h"

#include <stdlib.h>

//
// @brief AST node definitions for statements and expression
//
typedef struct ast_node ast_node_t;

//
// @brief Expression AST node structure
//
typedef struct expr_node {
  enum {
    EXPR_INT_LITERAL,
    EXPR_STR_LITERAL,
    EXPR_VARIABLE,
    EXPR_BINARY,
    EXPR_UNARY,
    EXPR_CALL
} type;
  uint32_t line_number;
  union {
    int32_t int_val;
    string_val_t str_val;
    uint8_t var_slot;
    struct {
      int op;
      struct expr_node *left;
      struct expr_node *right;
    } binary;
    struct {
      int op;
      struct expr_node *expr;
    } unary;
    struct {
      int func_id;
      struct expr_node *args[4];
      uint8_t arg_count;
    } call;
  } data;
} expr_node_t;

/* Statement AST node structures extending base ast_node_t */
struct ast_node {
  node_type_t type;
  ast_node_t *next;
  ast_node_t *parent;
  uint32_t line_number;

  union {
    struct {
      uint8_t var_slot;
      expr_node_t *expr;
    } assign;
    struct {
      expr_node_t *expr;
    } print_stmt;
    struct {
      expr_node_t *cond;
      ast_node_t *body;
    } while_stmt;
  } data;
};

typedef struct {
  uint32_t line;
  const char *message;
  token_type_t token_type;
} parser_error_t;


typedef struct {
  bool has_error;
  ast_node_t *node;
  parser_error_t error;
} parser_result_t;

//
// @brief Parses a token stream into an AST root node linked list.
//
// @param result Pointer to a parser_result_t struct, containing AST on success or error message
// @param src Pointer to raw source buffer.
// @param len Byte length of source buffer.
// @param symbol_table Table of registered variables.
// @return Pointer to the root AST node, or NULL if a syntax/validation error occurs.
//
void parser_parse(parser_result_t *result, const char *src, size_t len, symbol_table_t *symbol_table);

#endif //PRG32_BASIC_PARSER_H
