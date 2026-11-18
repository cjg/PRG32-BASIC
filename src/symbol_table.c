//
// Created by Giuseppe Coviello on 11/17/26.
//

#include "symbol_table.h"

#include <string.h>

void symbol_table_init(symbol_table_t *table) {
  table->symbol_count = 0;
  for (uint8_t i = 0; i < table->symbol_count; i++) {
    memset(table->symbols[i].name, 0, sizeof(table->symbols[i].name));
    table->symbols[i].slot = 0;
    table->symbols[i].active = false;
  }
}

uint8_t symbol_table_get_or_create(symbol_table_t *symbol_table, const char *name, size_t len) {
  if (len > BASIC_MAX_VAR_NAME_LEN) {
    // TODO: handle error better
    return 0;
  }

  for (uint8_t i = 0; i < symbol_table->symbol_count; i++) {
    if (strncasecmp(symbol_table->symbols[i].name, name, len) == 0 && symbol_table->symbols[i].name[len] == '\0') {
      // found variable already registered
      return symbol_table->symbols[i].slot;
    }
  }

  if (symbol_table->symbol_count < BASIC_MAX_VARS) {
    uint8_t slot = symbol_table->symbol_count++;
    memcpy(symbol_table->symbols[slot].name, name, len);
    symbol_table->symbols[slot].name[len] = '\0';
    symbol_table->symbols[slot].slot = slot;
    symbol_table->symbols[slot].active = true;

    // initialize variable type based on trailing $
    if (name[len - 1] == '$') {
      symbol_table->symbols[slot].val.type = VAL_STR;
      symbol_table->symbols[slot].val.data.str_val = (string_val_t){ .ptr = "", .len = 0, .is_literal = true };
    } else {
      symbol_table->symbols[slot].val.type = VAL_INT;
      symbol_table->symbols[slot].val.data.int_val = 0;
    }
    return slot;
  }

  return 0;
}