//
// Created by Giuseppe Coviello on 11/18/26.
//

#include "basic_rt.h"

#include <ctype.h>
#include <prg32.h>

#include <string.h>
#include <stdio.h>

#include "arena.h"
#include "strpool.h"
#include "parser.h"
#include "logging.h"
#include "mini_ctype.h"

char basic_script[BASIC_SCRIPT_SIZE] = {0};

// global interpreter execution state residing strictly in .bss
static ast_node_t *program_root = NULL;
static ast_node_t *pc = NULL;
static bool in_draw_phase = false;
static bool has_error = false;
static uint32_t error_line = 0;
static symbol_table_t symbol_table;
static bool interactive = true;

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
  int8_t fd = prg32_fs_open("HELLO.BAS", "w");
  prg32_fs_write(fd, "PRINT \"HELLO WORLD!\"", 21);
  prg32_fs_write(fd, "\n", 2);
  prg32_fs_close(fd);
  prg32_console_write("-- PRG32 BASIC --\n\n");
  interactive = true;
}

static bool start_prompt = true;
static bool reading_command = true;
static char command_key = 0;
#define MAX_COMMAND_LEN 20
static char command_line[MAX_COMMAND_LEN];
static uint8_t command_len = 0;
static const char *interactive_error = NULL;
static bool print_script = false;

static const char *read_script(const char *filename) {
  // skip spaces
  while (*filename != '\0' && *filename == ' ') {
    ++filename;
  }
  if (*filename == '\0') {
    return "FILE NOT PROVIDED";
  }
  int8_t fd = prg32_fs_open(filename, "r");
  if (fd < 0) {
    return "ERROR OPENING FILE";
  }
  int32_t bytes_read = prg32_fs_read(fd, basic_script, BASIC_SCRIPT_SIZE);
  prg32_fs_close(fd);
  if (bytes_read < 0) {
    return "ERROR READING FILE";
  }
  if (bytes_read == BASIC_SCRIPT_SIZE) {
    return "FILE TOO BIG";
  }

  basic_script[bytes_read] = 0;

  return NULL;
}

static void interactive_update() {
  interactive_error = NULL;
  if (reading_command) {
    uint16_t key = prg32_ps2kbd_read_key();
    if (key != 0) {
      if (isprint(key)) {
        command_key = (char) key;
        command_line[command_len] = command_key;
        command_len++;
        command_line[command_len] = '\0';
        if (command_len + 1 >= MAX_COMMAND_LEN) {
          interactive_error = "COMMAND TOO LONG";
          command_len = 0;
          start_prompt = true;
        }
      }
      if (key == 0xd) {
        // we need this to make sure the last character is echo-ed
        command_key = '\n';
        reading_command = false;
      }
    }
  } else {
    command_len = 0;
    // parsing command
    if (strncasecmp(command_line, "RUN ", 4) == 0) {
      // RUN <filename>
      interactive_error = read_script(command_line + 4);
      if (interactive_error != NULL) {
        reading_command = true;
        start_prompt = true;
        return;
      }

      arena_reset();
      symbol_table_init(&symbol_table);
      strpool_init(&symbol_table);

      // parse script
      program_root = parser_parse(basic_script, strlen(basic_script), &symbol_table);
      if (!program_root) {
        interactive_error = "SYNTAX ERROR";
        reading_command = true;
        start_prompt = true;
        return;
      } else {
        pc = program_root;
        interactive = false;
      }
    } else if (strncasecmp(command_line, "CAT ", 4) == 0) {
      // CAT <filename>
      interactive_error = read_script(command_line + 4);
      if (interactive_error != NULL) {
        reading_command = true;
        start_prompt = true;
        return;
      }
      print_script = true;
      reading_command = true;
      start_prompt = true;
    } else {
      interactive_error = "INVALID COMMAND";
      reading_command = true;
      start_prompt = true;
    }
  }
}

static void console_write_int(int32_t value) {
  if (value == 0) {
    prg32_console_putc('0');
    return;
  }
  if (value < 0) {
    prg32_console_putc('-');
    value = -value;
  }

  char buffer[10] = {0};
  uint16_t len = 0;

  while (value > 0) {
    buffer[len++] = '0' + (value % 10);
    value /= 10;
  }

  while (len > 0) {
    prg32_console_putc(buffer[len - 1]);
    len--;
  }
}

static void do_print_script() {
  prg32_console_putc('\n');
  uint16_t line_number = 1;
  const char *ptr = basic_script;
  while (*ptr != 0) {
    // print line_number
    if (line_number < 100) {
      prg32_console_write("  ");
    } else if (line_number < 10) {
      prg32_console_putc(' ');
    }
    console_write_int((int) line_number);
    line_number++;
    prg32_console_write("| ");
    while (*ptr != 0 && *ptr != '\n') {
      prg32_console_putc(*ptr);
      ptr++;
    }
    if (*ptr == '\n') {
      prg32_console_putc('\n');
      ptr++;
    }
  }
  prg32_console_putc('\n');
}

static void interactive_draw() {
  if (print_script) {
    do_print_script();
    print_script = false;
    return;
  }
  if (interactive_error) {
    prg32_console_write("\n!! ");
    prg32_console_write(interactive_error);
    prg32_console_write(" !!\n");
    interactive_error = NULL;
  }
  if (start_prompt) {
    prg32_console_write("\n> ");
    start_prompt = false;
  }
  if (command_key != 0) {
    prg32_console_putc(command_key);
    command_key = 0;
  }
}

static void script_update() {
  static bool completed_communicated = false;
  if (has_error || !pc) {
    if (!completed_communicated) {
      prg32_console_write("\n\n-- SCRIPT COMPLETED --\n\n");
      completed_communicated = true;
    }
    // error or script completed
    interactive_error = NULL;
    command_len = 0;
    interactive = true;
    start_prompt = true;
    reading_command = true;
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

static void script_draw() {
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
        prg32_console_write(print_buffer);
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

void basic_update() {
  if (interactive) {
    interactive_update();
  } else {
    script_update();
  }
}

void basic_draw() {
  if (interactive) {
    interactive_draw();
  } else {
    script_draw();
  }
}