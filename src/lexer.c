//
// Created by Giuseppe Coviello on 11/17/26.
//

#include "lexer.h"

#include "mini_ctype.h"

#include <stdbool.h>
#include <string.h>

//
// @brief Helper to check if character is alphanumeric or underscore for identifiers.
//
static bool is_ident_char(char c)
{
    return isalnum((unsigned char)c) || c == '_' || c == '$';
}

//
// @brief Matches keyword strings case-insensitively against an identifier token.
//
static token_type_t get_keyword_type(const char *str, size_t len)
{
    if (len == 5) {
        if (strncasecmp(str, "PRINT", 5) == 0) return TOKEN_KW_PRINT;
    }

    return TOKEN_IDENTIFIER;
}

void lexer_init(lexer_t *l, const char *src, size_t len) {
  l->src = src;
  l->len = len;
  l->pos = 0;
  l->line = 1;
}

token_t lexer_next(lexer_t *l) {
    while (l->pos < l->len) {
        char c = l->src[l->pos];

        // skip horizontal whitespace and carriage returns
        if (c == ' ' || c == '\r' || c == '\t') {
            l->pos++;
            continue;
        }

        // track line numbers on newlines
        if (c == '\n') {
            l->pos++;
            l->line++;
            return (token_t){ .type = TOKEN_NEWLINE, .start = &l->src[l->pos - 1], .length = 1, .line = l->line - 1 };
        }

        // skip single-quote comments until end of line
        if (c == '\'') {
            while (l->pos < l->len && l->src[l->pos] != '\n') {
                l->pos++;
            }
            continue;
        }

        const char *start_ptr = &l->src[l->pos];
        uint32_t current_line = l->line;

        // string literals enclosed in double quotes
        if (c == '"') {
            l->pos++; // skip opening quote
            const char *content_start = &l->src[l->pos];
            size_t str_len = 0;
            while (l->pos < l->len && l->src[l->pos] != '"' && l->src[l->pos] != '\n') {
                l->pos++;
                str_len++;
            }
            if (l->pos < l->len && l->src[l->pos] == '"') {
                l->pos++; // skip closing quote
            }
            return (token_t){
                .type = TOKEN_STR_LITERAL,
                .start = content_start,
                .length = str_len,
                .line = current_line
            };
        }

        // numeric integer literals
        if (isdigit((unsigned char)c)) {
            size_t len = 0;
            while (l->pos < l->len && isdigit((unsigned char)l->src[l->pos])) {
                l->pos++;
                len++;
            }
            return (token_t){
                .type = TOKEN_INT_LITERAL,
                .start = start_ptr,
                .length = len,
                .line = current_line
            };
        }

        /* Identifiers and Keywords */
        if (isalpha((unsigned char)c) || c == '_') {
            size_t len = 0;
            while (l->pos < l->len && is_ident_char(l->src[l->pos])) {
                l->pos++;
                len++;
            }

            token_type_t ttype = get_keyword_type(start_ptr, len);
            return (token_t){
                .type = ttype,
                .start = start_ptr,
                .length = len,
                .line = current_line
            };
        }

        /* Single-character operators & punctuation */
        l->pos++;
        switch (c) {
            case '=': return (token_t){ .type = TOKEN_ASSIGN, .start = start_ptr, .length = 1, .line = current_line };
            case '+': return (token_t){.type = TOKEN_PLUS, .start = start_ptr, .length = 1, .line = current_line };
            case '-': return (token_t){.type = TOKEN_MINUS, .start = start_ptr, .length = 1, .line = current_line };
            case '*': return (token_t){.type = TOKEN_ASTERISK, .start = start_ptr, .length = 1, .line = current_line };
            case '/': return (token_t){.type = TOKEN_SLASH, .start = start_ptr, .length = 1, .line = current_line };
            case '%': return (token_t){.type = TOKEN_MOD, .start = start_ptr, .length = 1, .line = current_line };
            default:  return (token_t){ .type = TOKEN_UNKNOWN, .start = start_ptr, .length = 1, .line = current_line };
        }
    }

    return (token_t){ .type = TOKEN_EOF, .start = &l->src[l->len], .length = 0, .line = l->line };
}