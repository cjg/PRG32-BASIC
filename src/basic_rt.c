//
// Created by Giuseppe Coviello on 11/18/26.
//

#include "basic_rt.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "arena.h"
#include "strpool.h"
#include "parser.h"
#include "logging.h"

char basic_script[BASIC_SCRIPT_SIZE];

// global interpreter execution state residing strictly in .bss
static ast_node_t *program_root = NULL;
static ast_node_t *pc = NULL;
static bool in_draw_phase = false;
static bool has_error = false;
static uint32_t error_line = 0;
static symbol_table_t symbol_table;

//
// @brief Evaluates an expression node recursively with safety checks.
//
static bool eval_expression(expr_node_t *expr, runtime_val_t *out_val) {
    if (!expr) return false;

    switch (expr->type) {
        case EXPR_INT_LITERAL:
            out_val->type = VAL_INT;
            out_val->data.int_val = expr->data.int_val;
            return true;

        case EXPR_STR_LITERAL:
            out_val->type = VAL_STR;
            out_val->data.str_val = expr->data.str_val;
            return true;

        case EXPR_VARIABLE:
            *out_val = symbol_table.symbols[expr->data.var_slot].val;
            return true;

        default:
            return false;
    }
}

//
// @brief Stackless AST advance logic using parent pointers.
//
// Avoids C call-stack overflow on deep loops and allows seamless step-by-step
// iteration between PRG32 update() and draw() callbacks.
//
static ast_node_t *advance_pc(ast_node_t *cur) {
  if (!cur) return NULL;

  // if current node has a direct sequential next statement, return it
  if (cur->next) {
    return cur->next;
  }

  // otherwise, walk upward through parent containers
  ast_node_t *p = cur->parent;
  DPRINTF("Parent of %p (%u) is %p", cur, cur->line_number, p);
  while (p) {
    // TODO: handle loops and branches
    p = p->parent;
  }

  // program completed
  return NULL;
}

void basic_init() {
  // initialize memory arena and symbol table
  arena_reset();
  symbol_table_init(&symbol_table);
  strpool_init(&symbol_table);

  // parse script
  program_root = parser_parse(basic_script, strlen(basic_script), &symbol_table);
  if (!program_root) {
    // TODO: improve handling of syntax error
    has_error = true;
  } else {
    pc = program_root;
  }
  DPRINTF("INITIALIZED -- Syntax error? %s Program Root: %p", has_error ? "true" : "false", program_root);
}

void basic_update() {
  if (has_error || !pc) {
    DPRINTF("Script completed");
    // error or script completed
    return;
  }
  uint32_t steps = 0;
  // TODO: consider putting a timeout for yielding and avoid the watchdog trigger
  while (pc && steps < BASIC_MAX_STEPS) {
    steps++;
    if (pc->type == NODE_PRINT) {
      in_draw_phase = true;
      // current instruction is a graphic statements, yielding to draw
      return;
    }

    // execute the logic instruction
    switch (pc->type) {
      case NODE_ASSIGN: {
        runtime_val_t val;
        if (!eval_expression(pc->data.assign.expr, &val)) {
          has_error = true;
          error_line = pc->line_number;
        }
        symbol_table.symbols[pc->data.assign.var_slot].val = val;
      }
        break;
      default:
        // unexpected node type
        has_error = true;
        error_line = pc->line_number;
        return;
    }
    pc = advance_pc(pc);
  }
}

static char print_buffer[120] = {0};

void basic_draw() {
  static bool error_printed = false;
  if (has_error) {
    // FIXME: render error properly
    if (!error_printed) {
      // fprintf(stderr, "RUNTIME ERROR AT LINE %d\n", error_line);
      error_printed = true;
    }
    return;
  }

  if (!in_draw_phase || !pc) {
    return;
  }

  uint32_t steps = 0;
  while (pc && steps < BASIC_MAX_STEPS && in_draw_phase) {
    steps++;
    switch (pc->type) {
      case NODE_PRINT: {
        runtime_val_t val;
        if (!eval_expression(pc->data.print_stmt.expr, &val)) {
          has_error = true;
          error_line = pc->line_number;
          return;
        }
        // TODO: improve print
        uint16_t len = val.data.str_val.len;
        if (len >= sizeof(print_buffer)) {
          len = sizeof(print_buffer) - 1;
        }
        memcpy(print_buffer, val.data.str_val.ptr, len);
        print_buffer[len] = '\0';
        printf("%s\n", print_buffer);
      }
        break;
      case NODE_ASSIGN:
        // switch to logic phase
        in_draw_phase = false;
        return;
    }
    pc = advance_pc(pc);
  }
}