//
// Created by Giuseppe Coviello on 11/17/26.
//
// @brief Manage lookup symbol table

#ifndef PRG32_BASIC_SYMBOL_TABLE_H
#define PRG32_BASIC_SYMBOL_TABLE_H

#include "basic_rt.h"
#include "types.h"

#include <stdlib.h>

// Symbol lookup mapping
typedef struct {
  char name[BASIC_MAX_VAR_NAME_LEN];
  uint8_t slot;
  bool active;
  runtime_val_t val;
} symbol_entry_t;

typedef struct {
  symbol_entry_t symbols[BASIC_MAX_VARS];
  uint8_t symbol_count;
} symbol_table_t;

void symbol_table_init(symbol_table_t *table);

//
// @brief Resolves or creates a variable symbol mapping.
//
// @param symbol_table The symbol table
// @param name The name of the variable
// @param line The length of the name of the variable (must be <= BASIC_MAX_VAR_NAME_LEN)
// @returns a number > 0 or 0 for error
//
uint8_t symbol_table_get_or_create(symbol_table_t *symbol_table, const char *name, size_t len);

#endif //PRG32_BASIC_SYMBOL_TABLE_H
