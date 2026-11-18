//
// Created by Giuseppe Coviello on 11/17/26.
//


#include <stdio.h>

#include "arena.h"
#include "strpool.h"
#include "parser.h"

static void read_whole_file(char **content, size_t *length, const char *filename);

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s filename\n", argv[0]);
    exit(1);
  }
  symbol_table_t symbol_table;
  symbol_table_init(&symbol_table);
  arena_reset();
  strpool_init(&symbol_table);
  char *source = NULL;
  size_t length = 0;
  read_whole_file(&source, &length, argv[1]);
  ast_node_t *root = parser_parse(source, length, &symbol_table);
  if (root == NULL) {
    fprintf(stderr, "Could not parse file\n");
    exit(1);
  }
  while (root != NULL) {
    printf("Type: %d Line: %d\n", root->type, root->line_number);
    root = root->next;
  }
}

static void read_whole_file(char **content, size_t *length, const char *filename) {
  FILE *f = fopen(filename, "r");
  if (!f) {
    fprintf(stderr, "Could not open file %s\n", filename);
    exit(1);
  }
  fseek(f, 0, SEEK_END);
  *length = (size_t) ftell(f);
  fseek(f, 0, SEEK_SET);
  *content = (char *) malloc(*length);
  if (!*content) {
    fprintf(stderr, "Could not allocate memory for file content\n");
    exit(1);
  }
  fread(*content, *length, 1, f);
}