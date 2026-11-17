//
// Created by Giuseppe Coviello on 11/17/26.
//
// @brief Static byte-arena allocator
//
// Implements a zero-heap-allocation bump allocator operating within a fixed
// static .bss buffer. Guarantees 4-byte alignment for all allocated chunks
// to prevent alignment faults on RISC-V targets.
//

#ifndef PRG32_BASIC_ARENA_H
#define PRG32_BASIC_ARENA_H

#include <stdlib.h>

//
// @brief Initializes or resets the arena allocation offset to zero.
//
// Must be called at the start of every script compilation/loading phase to
// instantly invalidate and reclaim all previously allocated memory.
//
void arena_reset(void);

//
// @brief Allocates a block of memory of the specified size from the static arena.
//
// Automatically enforces 4-byte alignment on the requested size. Returns NULL
// if the 16 KB arena capacity is exceeded.
//
// @param size Number of bytes to allocate.
// @return Pointer to the allocated memory chunk, or NULL if out of memory.
//
void *arena_alloc(size_t size);

#endif //PRG32_BASIC_ARENA_H
