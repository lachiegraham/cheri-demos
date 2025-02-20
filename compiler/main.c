/*
 * test_lexer.c
 * Copyright (C) 2016 alex <alex@alex>
 * https://github.com/asxalex/TTtL
 *
 * Distributed under terms of the MIT license.
 */

#include "tt.h"

char *current_file = "";
void interactive_mode() {
    // TODO
}

void file_mode(const char *filename) {
    current_file = (char*)filename;
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        ERRORF(current_file,-1, "no such file %s", filename);
    }
    lexer(fp);
    //print_lexer_result();
    //printf("=================================\n");
    fclose(fp);
    expressions *exps = parser();
    //environment *env = init_env();
    fp = fopen("test.txt", "w");
    if (!fp) {
        ERRORF("main.c",-1, "no such file %s", "test.txt");
    }
    fprintf(fp, "#include <stdio.h>\n\n");
    translate_primary(exps, fp);
    fclose(fp);
    // ast_t *evald = eval_expressions(exps, &env);
    //translate_primary(evald, &env);
    //print_env(env);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        interactive_mode();
        fprintf(stderr, "usage: ./tt program_file.tt\n");
        exit(-1);
    } else {
        file_mode(argv[1]);
    }
    return 0;
}

