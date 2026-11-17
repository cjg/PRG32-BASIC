//
// Created by Giuseppe Coviello on 11/17/26.
//
// @brief Ping-pong string manager and garbage collector
//
// Manages two 1 KB static string banks. Provides dynamic string allocation,
// zero-copy substring slicing, and garbage collection anchored by variable
// slots and temporary expression root stacks.
//

#ifndef PRG32_BASIC_STRPOOL_H
#define PRG32_BASIC_STRPOOL_H

#include "basic_rt.h"

#include <stdint.h>

//
// @brief Initializes the string pool subsystem, resetting active banks and roots.
//
void strpool_init(void);

//
// @brief Allocates space for a dynamic string in the active string bank.
//
// Automatically triggers garbage collection (str_gc) if the requested length
// exceeds remaining space in the active bank.
//
// @param src Pointer to source character data to copy.
// @param len Length of the string in bytes.
// @return Pointer to the newly allocated string buffer, or NULL if allocation fails.
//
const char *strpool_alloc(const char *src, uint16_t len);

//
// @brief Pushes a string value descriptor onto the temporary expression root stack.
//
// Prevents intermediate concatenation operands from being garbage collected
// if a string allocation triggers a GC sweep mid-expression evaluation.
//
// @param str Pointer to the string descriptor to protect.
//
void strpool_push_root(string_val_t *str);

//
// @brief Pops a specified number of items off the temporary expression root stack.
//
// @param count Number of root entries to pop.
//
void strpool_pop_roots(uint8_t count);

//
// @brief Triggers a garbage collection sweep between the ping-pong string banks.
//
// Copies all live non-literal strings referenced by the 64 variable slots and
// active temporary roots into the inactive bank, flips banks, and resets offsets.
//
void strpool_gc(void);

#endif //PRG32_BASIC_STRPOOL_H
