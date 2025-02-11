#include "tt.h"


    // FILE *fptr;
    // fptr = fopen("test.txt", "w");
    // int i = 0;
    //         fprintf(fptr,"-----%d-----\n",i);
    //     ++i;
    //     fprintf(fptr, "%u\n\n",e->ast->line);
    // fclose(fptr);
void print_stringast(ast_t *ast, FILE *fp) {
    char *string = ((string_ast_t*)ast)->value;
    if (strcmp(string, "<") == 0 || strcmp(string, "<=") == 0 || 
    strcmp(string, ">") == 0 || strcmp(string, ">=") == 0 || 
    strcmp(string, "=") == 0 || strcmp(string, "==") == 0 ||
    strcmp(string, "!=") == 0 || strcmp(string, "+") == 0 ||
    strcmp(string, "-") == 0 || strcmp(string, "/") == 0 ||
    strcmp(string, "*") == 0 || strcmp(string, "||") == 0 ||
    strcmp(string, "&&") == 0) {
        fprintf(fp, "%s ", string);
    } else {
        fprintf(fp, "\"%s\" ", string);
    }
}

void translate_ast(ast_t *ast, FILE *fp) {
    switch (ast->type) {
        case BOOLEANAST:
            fprintf(fp, "%s ", ((boolean_ast_t*)ast)->value ? "TRUE" : "FALSE");
            break;
        case NUMBERAST:
            fprintf(fp, "%ld ", ((number_ast_t*)ast)->value);
            break;
        case STRINGAST:
            print_stringast(ast, fp);
            break;
        case BINARYAST:
            //binary_ast_t *temp = (binary_ast_t*)t;
            fprintf(fp, "(");
            translate_ast(((binary_ast_t*)ast)->lhs, fp);
            translate_ast(((binary_ast_t*)ast)->op, fp);
            translate_ast(((binary_ast_t*)ast)->rhs, fp);
            fprintf(fp, ")");
            break;
        case VARIABLEAST:
            fprintf(fp, "%s ", ((variable_ast_t*)ast)->value);
            break;
        case DECLAST:
            fprintf(fp, "%s %s ", ((decl_ast_t*)ast)->valuetype, ((decl_ast_t*)ast)->value);
            break;
        case CONSTAST:
            fprintf(fp, "const %s %s ", ((const_ast_t*)ast)->valuetype, ((const_ast_t*)ast)->value);
            break;
        case CHARACTERAST:
            fprintf(fp, "%c ", ((character_ast_t*)ast)->value);
            break;
        case IFAST:
            fprintf(fp, "if ");
            translate_ast(((if_ast_t*)ast)->condition, fp);
            fprintf(fp, "{\n ");
            translate_primary(((if_ast_t*)ast)->then, fp);
            fprintf(fp, "\n} ");
            if (((if_ast_t*)ast)->els) {
                fprintf(fp, "else {\n");
                translate_primary(((if_ast_t*)ast)->els, fp);
                fprintf(fp, "\n} ");
            }
            fprintf(fp, "\n");
            break;
        case WHILEAST:
            fprintf(fp, "while ");
            translate_ast(((while_ast_t*)ast)->condition, fp);
            fprintf(fp, "{ \n");
            translate_primary(((while_ast_t*)ast)->body, fp);
            fprintf(fp, "\n}\n");
            break;
        case FUNCTIONAST:
            fprintf(fp, "define ");
            fprintf(fp, "%s ", ((function_ast_t*)ast)->name);
            fprintf(fp, "( ");
            translate_primary(((function_ast_t*)ast)->params, fp);
            fprintf(fp, ") {\n");
            translate_primary(((function_ast_t*)ast)->body, fp);
            fprintf(fp, "\n}\n");
            break;
        case CALLAST:
            fprintf(fp, "%s ", ((call_ast_t*)ast)->name);
            fprintf(fp, "( ");
            translate_primary(((call_ast_t*)ast)->args, fp);
            fprintf(fp, ") ");
            break;
        default:
            ERRORF(current_file, ast->line, "no such ast type (%d)", ast->type);
    }
}


// Loop through parser's output, write code based on ast type
void translate_primary(expressions *exp, FILE *fp) {
    ast_t *res = NULL;
    hlist_node_t *iter;
    expression *e;
    hlist_for_each(exp, iter) {
        e = hlist_entry(iter, expression, node);
        //print_ast(e->ast, 0);
        if (!e->ast) {
            continue;
        }
        translate_ast(e->ast, fp);
    }
}