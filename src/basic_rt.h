//
// Created by Giuseppe Coviello on 11/17/26.
//
// @brief Core runtime structures, static memory buffers, and public API
// for the PRG32 BASIC interpreter.
//

#ifndef PRG32_BASIC_BASIC_RT_H
#define PRG32_BASIC_BASIC_RT_H

#define BASIC_ARENA_SIZE     (16 * 1024)  // 16 KB static AST arena
#define BASIC_STR_BANK_SIZE  1024         // 1 KB per bank
#define BASIC_MAX_ROOTS      8            // temporary expression root stack size
#define BASIC_MAX_VARS       64           // max variable symbol slots

#include <stdbool.h>
#include <stdint.h>

//
// @brief Type of runtime variable, just int or string are supported by now
//
typedef enum {
  VAL_INT = 0,
  VAL_STR
} val_type_t;

//
// @brief Zero-copy string descriptor pointing directly into source text
// or managed by the ping-pong string poo
//
typedef struct {
  const char* ptr; // pointer to string character data
  uint16_t len;    // length of the string
  bool is_literal; // true if ptr references read-only source asset
} string_val_t;

//
// @brief Value container for variable storage.
//
typedef struct {
  val_type_t type;
  union {
    int32_t int_val;
    string_val_t str_val;
  } data;
} runtime_val_t;

#endif //PRG32_BASIC_BASIC_RT_H
