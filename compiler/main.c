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

    // Opening in write mode clears the file. We can close immediately.
    FILE *fp2 = fopen("test2.txt", "w");
    if (!fp) {
        ERRORF(current_file,-1, "no such file test2.txt");
    }
    fclose(fp2);

    fp = fopen("test.txt", "w");
    if (!fp) {
        ERRORF("main.c",-1, "no such file %s", "test.txt");
    }
    fprintf(fp,"int main() {\n");
    translate_primary(exps, fp);
    fprintf(fp,"}");
    fclose(fp);
    

    // concatenate main method after function decls
    fp2 = fopen("test2.txt", "a");
    fp = fopen("test.txt", "r");
    char *buf = (char *)malloc(sizeof(char));
    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);
        fprintf(fp2, "%s", buf);
    }
    fclose(fp);
    fclose(fp2);

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

