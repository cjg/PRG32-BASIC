//
// Created by Giuseppe Coviello on 11/17/26.
//

#include "arena.h"

#include "basic_rt.h"

// static `BASIC_ARENA_SIZE` AST allocation arena residing strictly in .bss
static uint8_t ast_arena[BASIC_ARENA_SIZE];

// current bump allocation offset from the start of ast_arena
static size_t arena_offset = 0;

void arena_reset(void) {
  // since the interpreter parse the scripts upon loading, resetting the offset pointer frees all
  arena_offset = 0;
}

void *arena_alloc(const size_t size)
{
  // enforce 4-byte alignment padding on the requested byte size
  // since RISC-V can trigger hardware exceptions on unaligned 32-bit integer accesses
  const size_t aligned_size = (size + 3) & ~3;

  if (arena_offset + aligned_size > BASIC_ARENA_SIZE) {
    // out of arena memory: the parser/loader will catch this and abort
    return NULL;
  }

  // retrieve pointer to current offset position and advance bump pointer
  void *ptr = &ast_arena[arena_offset];
  arena_offset += aligned_size;

  return ptr;
}