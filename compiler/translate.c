#include "tt.h"
int inside_function_def_flag = 0;
void translate_ast(ast_t*, FILE*);


// function that adds quotes around strings, but not if the string matches a binop
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
            fprintf(fp, "%s ", ((boolean_ast_t*)ast)->value ? "true" : "false");
            if (ast->semicolon == 1) {
                fprintf(fp,";\n");
            }
            break;
        case NUMBERAST:
        if (((number_ast_t*)ast)->should_return == 1) {
            fprintf(fp, "return ");
        }
            fprintf(fp, "%ld ", ((number_ast_t*)ast)->value);
            if (ast->semicolon == 1) {
                fprintf(fp,";\n");
            }
            break;
        case STRINGAST:
            print_stringast(ast, fp);
            if (ast->semicolon == 1) {
                fprintf(fp,";\n");
            }
            break;
        case BINARYAST:
            //binary_ast_t *temp = (binary_ast_t*)t;
            // TODO brackets around binary expressions are complicated and annoying and full of edge cases.
            // fprintf(fp, "(");
            translate_ast(((binary_ast_t*)ast)->lhs, fp);
            translate_ast(((binary_ast_t*)ast)->op, fp);
            translate_ast(((binary_ast_t*)ast)->rhs, fp);
            // fprintf(fp, ")");
            if (ast->semicolon == 1) {
                fprintf(fp,";\n");
            }
            break;
        case VARIABLEAST:
            fprintf(fp, "%s ", ((variable_ast_t*)ast)->value);
            if (ast->semicolon == 1) {
                fprintf(fp,";\n");
            }
            break;
        case DECLAST:
            if (strcmp(((decl_ast_t*)ast)->valuetype, "string") == 0) {
                ((decl_ast_t*)ast)->valuetype = "char *";
            }
            fprintf(fp, "%s %s ", ((decl_ast_t*)ast)->valuetype, ((decl_ast_t*)ast)->value);
            if (ast->semicolon == 1) {
                fprintf(fp,";\n");
            }
            break;
        case CONSTAST:
            if (strcmp(((const_ast_t*)ast)->valuetype, "string") == 0) {
                ((const_ast_t*)ast)->valuetype = "char *";
            }
            fprintf(fp, "const %s %s ", ((const_ast_t*)ast)->valuetype, ((const_ast_t*)ast)->value);
            if (ast->semicolon == 1) {
                fprintf(fp,";\n");
            }
            break;
        case CHARACTERAST:
            fprintf(fp, "%c ", ((character_ast_t*)ast)->value);
            if (ast->semicolon == 1) {
                fprintf(fp,";\n");
            }
            break;
        case IFAST:
            fprintf(fp, "if (");
            translate_ast(((if_ast_t*)ast)->condition, fp);
            fprintf(fp, ") {\n ");
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
            fprintf(fp, "while (");
            translate_ast(((while_ast_t*)ast)->condition, fp);
            fprintf(fp, "){ \n");
            translate_primary(((while_ast_t*)ast)->body, fp);
            fprintf(fp, "\n}\n");
            break;
        case FUNCTIONAST: // int foo(int bar) {}
            fprintf(fp, "%s ", ((function_ast_t*)ast)->return_type);
            fprintf(fp, "%s", ((function_ast_t*)ast)->name);
            fprintf(fp, "(");
            translate_primary(((function_ast_t*)ast)->params, fp);
            fprintf(fp, ") {\n");
            inside_function_def_flag = 1;
            translate_primary(((function_ast_t*)ast)->body, fp);
            fprintf(fp, "\n}\n");
            inside_function_def_flag = 0;
            break;
        case CALLAST:
            if (((call_ast_t*)ast)->should_return == 1 && inside_function_def_flag == 1) {
                fprintf(fp, "return ");
            }
            // handle the transation of all the inbuilt print functions
            if (strcmp(((call_ast_t*)ast)->name, "print") == 0) {
                fprintf(fp, "printf");
            } else if (strcmp(((call_ast_t*)ast)->name, "println") == 0) {
                // The amount of stuff that has to be done to print a '\n' is crazy
                fprintf(fp, "printf");
                if (hlist_empty(((call_ast_t*)ast)->args)) { // add string_ast holding '\n' to args
                    expression *e = (expression*)malloc(sizeof(expression));
                    string_ast_t *string_ast;
                    new_string_ast(string_ast, "\\n", -1);
                    e->ast = (ast_t*)string_ast;
                    INIT_HLIST_NODE(&e->node);
                    hlist_add_head(&e->node,((call_ast_t*)ast)->args);
                } else {
                    ERRORF(current_file, ast->line, "println should not have arguments");
                }
            } else if (strcmp(((call_ast_t*)ast)->name, "printInt") == 0) {
                fprintf(fp, "printf");
                // Add '%d' as an arg before the existing args
                if (hlist_empty(((call_ast_t*)ast)->args)) {
                    ERRORF(current_file, ast->line, "printInt needs arguments");
                } else {
                    expression *e = (expression*)malloc(sizeof(expression));
                    string_ast_t *string_ast;
                    new_string_ast(string_ast, "\%d", -1);
                    e->ast = (ast_t*)string_ast;
                    INIT_HLIST_NODE(&e->node);
                    hlist_add_head(&e->node,((call_ast_t*)ast)->args);
                    
                }
                
            } else {
                // DEFAULT, UNMODIFIED FUNCTION NAME
                fprintf(fp, "%s", ((call_ast_t*)ast)->name);
            }
            fprintf(fp, "(");
            //translate_primary(((call_ast_t*)ast)->args, fp);
            // would love to just do ^, but we need to add commas in between each arg
            hlist_node_t *iter;
            expression *e;
            hlist_for_each(((call_ast_t*)ast)->args, iter) {
                e = hlist_entry(iter, expression, node);
                if (!e->ast) {
                    continue;
                }
                translate_ast(e->ast, fp);
                // if we have a next, add a comma
                if (iter->next) {
                    fprintf(fp,", ");
                }
            }
            // end of args
            fprintf(fp, ")");
            if (ast->semicolon == 1) {
                fprintf(fp,";\n");
            }
            break;
        case INCLUDEAST:
            if (((include_ast_t*)ast)->is_import_statement == 0) {
                // REQUIRE
                // hardcode stdout
                if (strcmp(((include_ast_t*)ast)->value,"stdout") == 0) {
                    fprintf(fp, "#include <stdio.h>\n");
                    break;
                }
            }
            // IMPORT and also any REQUIREs that aren't hardcoded
            fprintf(fp, "#include \"%s\"\n",((include_ast_t*)ast)->value);
            break;
        default:
            ERRORF(current_file, ast->line, "no such ast type (%d)", ast->type);
    }
}


// Main func. Loop through parser's output, write code based on ast type
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
        // handle function params types printing
        if (e->param_type) { 
            fprintf(fp,"%s ", e->param_type);
        }
        translate_ast(e->ast, fp);
    }
}
