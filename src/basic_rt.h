//
// Created by Giuseppe Coviello on 11/17/26.
//
// @brief Core runtime structures, static memory buffers, and public API
// for the PRG32 BASIC interpreter.
//

#ifndef PRG32_BASIC_BASIC_RT_H
#define PRG32_BASIC_BASIC_RT_H

#define BASIC_SCRIPT_SIZE      (16 * 1024)  // 16 KB script size
#define BASIC_ARENA_SIZE       (16 * 1024)  // 16 KB static AST arena
#define BASIC_STR_BANK_SIZE    1024         // 1 KB per bank
#define BASIC_MAX_ROOTS        8            // temporary expression root stack size
#define BASIC_MAX_VARS         64           // max variable symbol slots
#define BASIC_MAX_VAR_NAME_LEN 64           // max length for variable names
#define BASIC_MAX_STEPS        2000         // max AST node steps per frame before yielding

extern char basic_script[BASIC_SCRIPT_SIZE];

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

//
// @brief Indicate if the execution of a statement or the evaluation of an expression is successful
//
typedef struct {
  bool has_error;
  uint32_t line_number;
  const char *error_msg;
} runtime_result_t;

#define RUNTIME_ERR(node, msg) (runtime_result_t) { .has_error = true, .line_number = (node)->line_number, .error_msg = (msg) }
#define RUNTIME_OK() (runtime_result_t) { .has_error = false, .error_msg = NULL }

#define CHECK_RUNTIME_ERR(statement) \
  do { \
    runtime_result_t __result; \
    __result = statement; \
    if (__result.has_error) { \
      return __result; \
    } \
  } while(0)

//
// @brief AST node type classifications
//
typedef enum {
  NODE_ASSIGN = 0,
  NODE_PRINT,
  NODE_WHILE
} node_type_t;

//
// @brief Initialize the basic runtime
//
void basic_init();

//
// @brief Executes script logic steps during PRG32's update() callback.
// Automatically yields and switches phase when a graphic statement or delay is hit.
//
void basic_update();

//
// @brief Executes graphics and neutral control flow steps during PRG32's draw() callback.
// Automatically yields back to update phase when a logic mutation statement is hit.
//
void basic_draw();


#endif //PRG32_BASIC_BASIC_RT_H
