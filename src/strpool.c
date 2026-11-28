//
// Created by Giuseppe Coviello on 11/17/26.
//

#include "strpool.h"

#include "logging.h"

#include <string.h>

// dual 1 KB static string banks residing in .bss
static char str_banks[2][BASIC_STR_BANK_SIZE];

// active bank index (0 or 1)
static uint8_t active_bank = 0;

// current byte offset bump pointer within the active string bank
static uint16_t str_offset = 0;

// temporary expression root stack to protect intermediate pointers during GC
static string_val_t* temp_roots[BASIC_MAX_ROOTS];
static uint8_t temp_root_top = 0;
static symbol_table_t* _symbol_table;

void strpool_init(symbol_table_t *symbol_table) {
  active_bank = 0;
  str_offset = 0;
  temp_root_top = 0;
  _symbol_table = symbol_table;
  memset(str_banks, 0, sizeof(str_banks));
}

void strpool_push_root(string_val_t* str) {
  if (temp_root_top < BASIC_MAX_ROOTS) {
    temp_roots[temp_root_top++] = str;
  }
}

void strpool_pop_roots(uint8_t count) {
  if (count <= temp_root_top) {
    temp_root_top -= count;
  } else {
    temp_root_top = 0;
  }
}

static void relocate_string(string_val_t* sval, const uint8_t target_bank, uint16_t* target_offset) {
  if (sval && !sval->is_literal && sval->ptr) {
    // check if pointer lies within the current active bank
    const char* bank_start = str_banks[active_bank];
    const char* bank_end = bank_start + BASIC_STR_BANK_SIZE;
    if (sval->ptr >= bank_start && (sval)->ptr < bank_end) {
      if (*target_offset + (sval)->len <= BASIC_STR_BANK_SIZE) {
        char* dest = &str_banks[target_bank][*target_offset];
        memcpy(dest, sval->ptr, sval->len);
        sval->ptr = dest;
        *target_offset += sval->len;
      }
    }
  }
}

void strpool_gc(void) {
  DEBUG(">>> strpool_gc active bank %d offset %d", active_bank, str_offset);
  // When the active bank runs out of space, we must salvage all live strings
  // referenced by variables and active expression roots, copy them into the inactive bank,
  // and update their pointers accordingly before switching banks.

  const uint8_t target_bank = active_bank ^ 1;
  uint16_t target_offset = 0;

  // mark and sweep across all 64 variable slots
  for (int i = 0; i < BASIC_MAX_VARS; i++) {
    if (_symbol_table->symbols[i].active && _symbol_table->symbols[i].val.type == VAL_STR) {
      relocate_string(&_symbol_table->symbols[i].val.data.str_val, target_bank, &target_offset);
    }
  }

  // mark and sweep across active temporary expression root stack
  for (int i = 0; i < temp_root_top; i++) {
    if (temp_roots[i]) {
      relocate_string(temp_roots[i], target_bank, &target_offset);
    }
  }

  // switch active bank and update offset
  active_bank = target_bank;
  str_offset = target_offset;
  DEBUG("<<< strpool_gc active bank %d offset %d", active_bank, str_offset);
}

const char* strpool_alloc(const char* src, uint16_t len) {
  // if allocation exceeds bank capacity, trigger garbage collection
  if (str_offset + len > BASIC_STR_BANK_SIZE) {
    strpool_gc();
    // if still exceeding after GC, return NULL (out of memory error)
    if (str_offset + len > BASIC_STR_BANK_SIZE) {
      return NULL;
    }
  }

  char* dest = &str_banks[active_bank][str_offset];
  if (src != NULL) {
    memcpy(dest, src, len);
  }
  str_offset += len;

  return dest;
}
