//
// Created by Giuseppe Coviello on 11/17/26.
//


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "basic_rt.h"

static void read_whole_file(char **content, size_t *length, const char *filename);

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s filename\n", argv[0]);
    exit(1);
  }
  char *source = NULL;
  size_t length = 0;
  read_whole_file(&source, &length, argv[1]);
  printf("length: %lu\n", length);
  memcpy(basic_script, source, length);
printf("%p\n", basic_script);
  basic_init();
  for (;;) {
    basic_update();
    basic_draw();
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