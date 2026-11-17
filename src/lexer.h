//
// Created by Giuseppe Coviello on 11/17/26.
//

#ifndef PRG32_BASIC_LEXER_H
#define PRG32_BASIC_LEXER_H

#include <stdlib.h>

//
// @brief token classifications recognized by the lexer and parser
//
typedef enum {
  TOKEN_EOF = 0,
  TOKEN_NEWLINE,

  /* Literals & Identifiers */
  TOKEN_IDENTIFIER,    // variable name (e.g., score, px, a$)
  TOKEN_INT_LITERAL,   // numeric integer literal
  TOKEN_STR_LITERAL,   // quoted string literal

  /* Keywords */
  TOKEN_KW_PRINT,

  /* Operators & Delimiters */
  TOKEN_ASSIGN,        /* = */

  TOKEN_UNKNOWN
} token_type_t;

//
// @brief Token structure holding a view into the source buffer
//
typedef struct {
  token_type_t type;
  const char *start;   // pointer to start of token in source buffer
  size_t length;       // length of token
  uint32_t line;       // source line number for error diagnostics, starting from 1
} token_t;

//
// @brief Lexer state context.
//
typedef struct {
  const char *src;     // source text buffer
  size_t len;          // source text buffer length
  size_t pos;          // current scanning index
  uint32_t line;       // current line number, starting from 1
} lexer_t;

//
// @brief Initializes the lexer context with a source text buffer.
//
// @param l Pointer to lexer structure.
// @param src Pointer to source text.
// @param len Length of source text.
//
void lexer_init(lexer_t *l, const char *src, size_t len);

//
// @brief Scans and returns the next token from the source stream.
//
// @param l Pointer to lexer structure.
// @return Next parsed token_t structure.
//
token_t lexer_next(lexer_t *l);

#endif //PRG32_BASIC_LEXER_H
