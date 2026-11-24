#include <stdio.h>
#include <stdlib.h>

#include "basic_rt.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr,"Usage: %s script", argv[0]);
        exit(1);
    }
    FILE *f = fopen(argv[1], "r");
    if (f == NULL) {
        fprintf(stderr,"Error opening script file %s\n", argv[1]);
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    size_t file_size = ftell(f);
    if (file_size > BASIC_SCRIPT_SIZE) {
        fprintf(stderr,"Script file size is too big\n");
        exit(1);
    }
    fseek(f, 0, SEEK_SET);
    fread(basic_script, 1, file_size, f);
    fclose(f);
    basic_script[file_size] = '\0';
    basic_init();
    for (;;) {
        basic_update();
        basic_draw();
    }
}
