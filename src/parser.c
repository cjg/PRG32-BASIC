//
// Created by Giuseppe Coviello on 11/17/26.
//

#include "parser.h"

#include "arena.h"
#include "lexer.h"
#include "logging.h"

// parser context tracking lexer state and lookahead token
typedef struct {
  lexer_t lexer;
  token_t current;
  token_t peek_token;
  symbol_table_t* symbol_table;
  bool has_error;
} parser_ctx_t;

//
// @brief Advances parser lookahead tokens.
//
static void parser_advance(parser_ctx_t* parser) {
  parser->current = parser->peek_token;
  parser->peek_token = lexer_next(&parser->lexer);
}

//
// @brief Parses a primary expression literals, variables, parentheses, built-ins
//
static expr_node_t* parse_primary(parser_ctx_t* parser) {
  expr_node_t* node = (expr_node_t *) arena_alloc(sizeof(expr_node_t));
  if (!node) return NULL;

  if (parser->current.type == TOKEN_INT_LITERAL) {
    node->type = EXPR_INT_LITERAL;
    node->data.int_val = (int32_t) strtol(parser->current.start, NULL, 10);
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
 * @brief Pratt parsing expression precedence router (placeholder simplified for skeleton).
 */
static expr_node_t* parse_expression(parser_ctx_t* parser) {
  /* Simplified recursive descent expression parser for binary ops */
  expr_node_t* left = parse_primary(parser);

  return left;
}

typedef struct {
  uint32_t line;
  const char *message;
} parser_error_t;

typedef struct {
  bool has_error;
  ast_node_t* node;
  parser_error_t error;
} parse_statement_result_t;

//
// @brief Parses an individual statement node.
//
static void parse_statement(parse_statement_result_t *result, parser_ctx_t* parser, ast_node_t* parent) {
  DPRINTF("Current token type: %d\n", parser->current.type);
  // skip newlines
  while (parser->current.type == TOKEN_NEWLINE) {
    DPRINTF("Advancing NL");
    parser_advance(parser);
  }

  // reached end of file
  if (parser->current.type == TOKEN_EOF) {
    result->has_error = false;
    DPRINTF("End of file");
    return;
  }

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
  if (parser->current.type == TOKEN_IDENTIFIER) {
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
    // this is a PRINT expected string after PRINT keyword
    if (parser->peek_token.type != TOKEN_IDENTIFIER && parser->peek_token.type != TOKEN_STR_LITERAL) {
      result->has_error = true;
      result->error.line = parser->current.line;
      result->error.message = "expected identifier or string literal after PRINT keyword";
      return;
    }
    result->node->type = NODE_PRINT;
    parser_advance(parser);
    result->node->data.print_stmt.expr = parse_expression(parser);
    return;
  }

  // skip unknown tokens to prevent infinite loops
  DPRINTF("Unexpected token %d '%s'", parser->current.type, parser->current.start);
  parser_advance(parser);
}

ast_node_t* parser_parse(const char* src, size_t len, symbol_table_t* symbol_table) {
  parser_ctx_t parser;
  parser.symbol_table = symbol_table;
  lexer_init(&parser.lexer, src, len);
  parser.has_error = false;

  // calling advance twice to initialize current and next (peek_token)
  parser_advance(&parser);
  parser_advance(&parser);

  ast_node_t head_dummy = {0};
  ast_node_t* curr = &head_dummy;

  while (parser.current.type != TOKEN_EOF && !parser.has_error) {
    DPRINTF("Parsing statement %s\n", parser.current.start);
    parse_statement_result_t result;
    parse_statement(&result, &parser, curr);
    if (result.has_error) {
      fprintf(stderr, "Parsing statement error at line %d: %s\n", result.error.line, result.error.message);
      break;
    }
    if (result.node) {
      DPRINTF("Parsed statement type %d\n", result.node->type);
      curr->next = result.node;
      curr = result.node;
      result.node->next = NULL;
    } else {
      DPRINTF("Error parsing statement\n");
    }
  }

  // TODO: validate AST

  return head_dummy.next;
}
