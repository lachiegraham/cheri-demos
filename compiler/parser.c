/*
 * parser.c
 * Copyright (C) 2016 alex <alex@alex>
 *
 * Distributed under terms of the MIT license.
 */

#include "tt.h"

static int parse_index = 0;
static lex *curtok;
static int indent_level = 0;

ast_t *parse_expression();
ast_t *parse_primary();

void print_expression(expressions*, int);
expressions *gather_function_params();

static int get_precedence(lex *current) {
    if (!current)
        return -2;
    TOKEN token = current->token;
    if (token == OR || token == AND) 
        return 2;
    if (token == ASSIGN)
        return 5;
    if (token == LT || token == GT || token == EQUAL || token == LTE || token == GTE || token == NEQ)
        return 10;
    if (token == ADD || token == MINUS)
        return 20;
    if (token == MUL || token == DIV)
        return 40;
    return -1;
}

static void get_next_token() {
    if (parse_index == lex_index) {
        curtok = NULL;
        return;
    }
    curtok = &lex_list[parse_index++];
}

static void unget_token() {
    if (parse_index > 0)
        curtok = &lex_list[--parse_index];
}

static int get_indent_level() {
    int indent = 0;
    if (!curtok) return 0;
    while (curtok->token == INDENT) {
        indent++;
        get_next_token();
    }
    return indent;
}

expressions *gather_expression() {
    lex *current = curtok;
    expressions *exps = (expressions*)malloc(sizeof(expressions));
    init_expressions(exps);
    expression *old;
    int previous_indent_level = indent_level;
    while (current && indent_level >= previous_indent_level) { // MAY SUPPORT == IM NOT SURE
        ast_t *new = parse_expression();
        
        if (new->type != WHILEAST && new->type != IFAST) {
            new->semicolon = 1;
        }
        expression *e = (expression*)malloc(sizeof(expression));
        e->ast = new;
        INIT_HLIST_NODE(&e->node);
        if (hlist_empty((const expressions*)exps)) {
            hlist_add_head(&e->node, exps);
        } else {
            hlist_add_after(&old->node, &e->node);
        }
        old = e;
        // update indentation level
        previous_indent_level = indent_level;
        indent_level = get_indent_level();

        current = curtok;
    }
    return exps;
}

ast_t *parse_while() {
    while_ast_t *res;
    get_next_token(); // skip while
    lex *current = curtok;
    if (current->token != LPARAN) {
        ERRORF(current_file, current->line, "expected LEFT PARENTHSIS (, got %s", current->value);
    }
    ast_t *cond = parse_primary();
    new_while_ast(res, cond, current->line);
    current = curtok;
    // if(current->token != LBRACE) { // INDENT
    //     ERRORF(current_file, current->line, "expected LEFT BRACE {, got %s", current->value);
    // }
    if (current->token != INDENT) {
        ERRORF(current_file, current->line, "expected INDENT after WHILE, got %s", current->value);
    }
    // consume and count indentations
    indent_level = get_indent_level();
    //get_next_token(); // skip {
    res->body = gather_expression(); // parse while body
    
    current = curtok;
    // if(current->token != RBRACE) {
    //     ERRORF(current_file, current->line, "expected RIGHT BRACE }, got %s", current->value);
    // }
    // get_next_token(); // skip }
    return (ast_t*)res;
}

ast_t *parse_if() {
    if_ast_t *res;
    get_next_token(); // skip if
    lex *current = curtok;
    if (current->token != LPARAN) {
        ERRORF(current_file, current->line, "expected LEFT PARENTHSIS (, got %s", current->value);
    }// printf("currently looking at %s, local indent level %d, global indent level %d\n", current->value, previous_indent_level, indent_level);


    ast_t *cond = parse_primary();
    new_if_ast(res, cond, current->line);

    current = curtok;
    // if (current->token != LBRACE) {
    //     ERRORF(current_file, current->line, "expected LEFT BRACE, {, got %s", current->value);
    // }
    // get_next_token(); // skip {
    if (current->token != INDENT) {
        ERRORF(current_file, current->line, "expected INDENT after IF, got %s", current->value);
    }
    // consume and count indentations
    indent_level = get_indent_level();

    res->then = gather_expression();
    // get_next_token(); // skip }

    current = curtok;
    if (current && current->token == ELSE) {
        get_next_token(); // skip else
        current = curtok;
        // if (!current || current->token != LBRACE) {
        //     ERRORF(current_file, current->line, "expected LEFT BRACE {, got %s", current->value);
        // }
        // get_next_token(); // skip {
        if (current->token != INDENT) {
            ERRORF(current_file, current->line, "expected INDENT after ELSE, got %s", current->value);
        }
        // consume and count indentations
        indent_level = get_indent_level();

        res->els = gather_expression();
        // get_next_token(); // skip }
    } else {
        res->els = NULL;
    }
    return (ast_t*)res;
}

ast_t *parse_identifier() {
    lex *current = curtok; 
    get_next_token(); // skip identifier
    lex *next = curtok;
    if (next && next->token == LPARAN) {
        // function call;
        call_ast_t *call;
        new_call_ast(call, current->line);
        call->name = current->value;
        current->value = NULL;
        get_next_token(); // consume LPARAN
        call->args = gather_function_params(); // consumes RPARAN

        call->should_return = 1;
        // unset the should_return flag for any nested calls
        ast_t *ast;
        expression *nodes;
        hlist_node_t *iter;
        hlist_for_each(call->args, iter) { // loop through nested calls
        nodes = hlist_entry(iter, expression, node); 
            ast = nodes->ast;
            if (ast->type == CALLAST) {
                ((call_ast_t *)ast)->should_return = 0;
            } else if (ast->type == NUMBERAST) {
                ((number_ast_t *)ast)->should_return = 0;
            }
        }
        // if we have a print statement, unset return flag
        // TODO this should be for all void type function calls but we will need a lookup table for that
        if (strcmp(call->name,"print") == 0 || strcmp(call->name,"println") == 0 || strcmp(call->name,"printInt") == 0) {
            call->should_return = 0;
        }
        return (ast_t*)call;
    } else {
        if (strcmp(current->value, "import") == 0 || strcmp(current->value, "require") == 0) {
            get_next_token(); // skip import/require keyword
            
            if (!next) {
                ERRORF(current_file, current->line, "expected argument for IMPORT");
            }
            if (next->token != IDENTIFIER) {
                ERRORF(current_file, current->line, "expected argument for IMPORT, got %s", next->value);
            }
            // Handle next word, then check if theres a dot. If there is, handle that whole part as one string.
            include_ast_t *import;
            new_include_ast(import, current->line);
            import->is_import_statement = strcmp(current->value, "import") == 0; // set flag
            import->value = next->value;

            // CHECK FOR dot
            char *arg = next->value;
            if (curtok->token != DOT) {
                return (ast_t*)import;
            }
            get_next_token(); // skip dot
            if (!curtok) {
                ERRORF(current_file, current->line, "bad formatting for IMPORT statement");
            }
            if (curtok->token != IDENTIFIER) {
                ERRORF(current_file, current->line, "bad argument for IMPORT statment, got %s", curtok->value);
            }
            // concatenate whole string together and return
            strcat(arg, ".");
            strcat(arg, curtok->value);
            import->value = arg;
            get_next_token();
            current->value = NULL;
            return (ast_t *)import;

        } else {
            if (next && next->token == DOT) {
                get_next_token(); // skip dot
                return parse_identifier();
            }
            // symbol reference;
            variable_ast_t *variable;
            new_variable_ast(variable, current->value, current->line);
            current->value = NULL;
            return (ast_t*)variable;
        }
    }
}

ast_t *parse_boolean() {
    boolean_ast_t *res;
    if (curtok->token == TRUE) {
        new_boolean_ast(res, 1, curtok->line);
    } else {
        new_boolean_ast(res, 0, curtok->line);
    }
    return (ast_t*)res;
}

ast_t *parse_number() {
    lex *current = curtok;
    long sum = 0;
    int length = strlen(current->value);
    for (int i = 0; i < length; i++) {
        sum = 10 * sum + current->value[i] - '0';
    }
    number_ast_t *number;
    new_number_ast(number, sum, current->line);
    number->should_return = 1;
    return (ast_t*)number;
}

// parse (exp)
ast_t *parse_paran() {
    get_next_token(); // consume (
    ast_t *inside = parse_expression();
    if (curtok->token != RPARAN) {
        ERRORF(current_file, curtok->line, "expected right parenthesis ), got %s", curtok->value);
    }
    //get_next_token(); // consume )
    return inside;
}

expressions* gather_function_params() {
    lex *current = curtok;
    expressions *exps = (expressions*)malloc(sizeof(expressions));
    init_expressions(exps);
    expression *old;
    while (current && current->token != RPARAN) {
        ast_t *new = parse_expression();
        expression *e = (expression*)malloc(sizeof(expression));
        
        // handle parameters having declared types
        current = curtok;
        if (current->token == COLON) {
            // handle function parameter type
            get_next_token(); // skip :
            
            char* type = curtok->value;
            for (int i = 0; i < strlen(type); i++) {
                type[i] = tolower(type[i]);
            }
            e->param_type = type;
            get_next_token(); // skip type
        } else {
            e->param_type = NULL;
        }
        e->ast = new;
        INIT_HLIST_NODE(&e->node);
        if (hlist_empty((const expressions*)exps)) {
            hlist_add_head(&e->node, exps);
        } else {
            hlist_add_after(&old->node, &e->node);
        }
        old = e;
        current = curtok;
        if (!current) {
            ERRORF(current_file, -1, "unexpected NIL");
        }
        
        if (current->token != RPARAN && current->token != COMMA) {
            ERRORF(current_file, current->line, "COMMA or RIGHT PARAN is expected, got %s", current->value);
        }
        if (current->token == COMMA) {
            get_next_token();
            current = curtok;
        }
    }
    get_next_token(); // skip )
    return exps;
}

ast_t *parse_character() {
    character_ast_t *s;
    new_character_ast(s, curtok->value[0], curtok->line);
    free(curtok->value);
    get_next_token();
    if(curtok->token != SINGLEQUOTE) {
        ERRORF(current_file, curtok->line, "require single quote ', got %s", curtok->value);
    }
    get_next_token(); // skip SINGLEQUOTE
    return (ast_t*)s;
}

ast_t *parse_string() {
    string_ast_t *s;
    new_string_ast(s, curtok->value, curtok->line);
    curtok->value = NULL;
    get_next_token();
    if(curtok->token != DOUBLEQUOTE) {
        ERRORF(current_file, curtok->line, "require double quote \" got %s", curtok->value);
    }
    get_next_token(); // skip DOUBLEQUOTE
    return (ast_t*)s;
}

ast_t *parse_function() {
    lex *current;
    get_next_token(); // skip define 
    function_ast_t *function;
    new_function_ast(function, curtok->value,curtok->line);
    curtok->value = NULL;
    get_next_token(); // skip name
    current = curtok;
    if (current->token != LPARAN) {
        ERRORF(current_file, curtok->line, "expected left parenthesis (, got %s", current->value);
    }
    get_next_token(); // skip (
    function->params = gather_function_params();

    // Handle :type
    current = curtok;
    if(current->token != COLON) {
        ERRORF(current_file, current->line, "expected colon :, got %s", current->value);
    }
    get_next_token(); //skip :
    char *ret_type = curtok->value;
    for (int i = 0; i < strlen(ret_type); i++) {
        ret_type[i] = tolower(ret_type[i]);
    }
    function->return_type = ret_type;

    get_next_token();
    current = curtok;
    // if(current->token != LBRACE) {
    //     ERRORF(current_file, current->line, "expected left brace {, got %s", current->value);
    // }
    // get_next_token(); // skip {
    if (current->token != INDENT) {
        ERRORF(current_file, current->line, "expected INDENT after ELSE, got %s", current->value);
    }
    // consume and count indentations
    indent_level = get_indent_level();

    function->body = gather_expression();
    function->env = NULL;
    // get_next_token(); // skip }
    return (ast_t*)function;
}

// var id:type
ast_t *parse_var() {
    get_next_token(); // skip var

    char *name = curtok->value; // copy identifier name
    get_next_token(); // skip id

    if (curtok->token != COLON) {
        ERRORF(current_file, curtok->line, "Expected colon :, got %s", curtok->value);
    }
    get_next_token(); // skip :

    char *var_type = curtok->value;
    for (int i = 0; i < strlen(var_type); i++) {
        var_type[i] = tolower(var_type[i]);
    }

    decl_ast_t *var;
    new_decl_ast(var, name, var_type,curtok->line);
    return (ast_t*)var;
}

// val id:type
ast_t *parse_val() {
    get_next_token(); // skip val

    char *name = curtok->value; 
    get_next_token(); // skip id

    if (curtok->token != COLON) {
        ERRORF(current_file, curtok->line, "Expected colon :, got %s", curtok->value);
    }
    get_next_token(); // skip :

    char *val_type = curtok->value;
    for (int i = 0; i < strlen(val_type); i++) {
        val_type[i] = tolower(val_type[i]);
    }

    const_ast_t *constant;
    new_const_ast(constant, name, val_type,curtok->line);
    return (ast_t*)constant;
}

ast_t *parse_primary() {
    if (!curtok) {
        return NULL;
    }
    ast_t *res;
    switch(curtok->token) {
        case INDENT:
            printf("found indent, line %d\n", curtok->line);
            indent_level = get_indent_level();
            return parse_primary();
        case DEFINE:
            LOG("%s\n", "parsing define...");
            res = parse_function();
            return res;
        case VAR:
            LOG("%s\n", "parsing var...");
            res = parse_var();
            get_next_token();
            return res;
        case VAL:
            LOG("%s\n", "parsing val...");
            res = parse_val();
            get_next_token();
            return res;
        case IDENTIFIER:
            LOG("%s\n", "parsing identifier...");
            res = parse_identifier();
            //get_next_token(); // skip identifier;
            return res;
        case NUMBER:
            LOG("%s\n", "parsing number...");
            res = parse_number();
            get_next_token(); // skip number;
            return res;
        case LPARAN:
            LOG("%s\n", "parsing (");
            res = parse_paran();
            get_next_token(); // skip right paran )
            return res;
        case IF:
            LOG("%s\n", "parsing if");
            res = parse_if();
            //get_next_token(); the } token is handled already
            return res;
        case WHILE:
            LOG("%s\n", "parsing while");
            res = parse_while();
            //get_next_token(); // skip right bracket }
            return res;
        case TRUE:
        case FALSE:
            LOG("%s\n", "parsing boolean");
            res = parse_boolean();
            get_next_token();
            return res;
        case SEMICOLON:
            LOG("%s\n", "parsing ;");
            get_next_token(); // skip SEMICOLON
            return parse_primary();
        case DOUBLEQUOTE:
            LOG("%s\n", "parsing \"");
            get_next_token(); // skip double quote
            res = parse_string();
            return res;
        case SINGLEQUOTE:
            LOG("%s\n", "parsing '");
            get_next_token(); // skip single quote
            res = parse_character();
            return res;
        case DOT:
            get_next_token(); // just skip
            printf("after dot: %s\n", curtok->value);
            return parse_identifier();
        default:
            ERRORF(current_file, curtok->line, "parse_primary: unexpected token (%d)", curtok->token);
    }
    return NULL;
}

ast_t *parse_binary_ops(int precedence, ast_t *lhs) {
    lex *cur;
    while(1) {
        int cur_prec = get_precedence(curtok);
        if (cur_prec < precedence) {
            return lhs;
        }
        string_ast_t *ops;
        new_string_ast(ops, curtok->value, curtok->line);
        cur = curtok;
        get_next_token(); // eat binop

        ast_t *rhs = parse_primary();
        if (!rhs) {
            return lhs;
        }

        //get_next_token();
        int next_prec = get_precedence(curtok);
        if (cur_prec < next_prec) {
            rhs = parse_binary_ops(cur_prec+1, rhs);
            if (!rhs) {
                // should parse error 2;
                ERRORF(current_file, -1, "parse error 2");
                return lhs;
            }
        }
        if (rhs->type == NUMBERAST) {
            ((number_ast_t *)rhs)->should_return = 0;
        } else if (rhs->type == CALLAST) {
            ((call_ast_t *)rhs)->should_return = 0;
        }

        binary_ast_t *n;
        new_binary_ast(n, (ast_t*)ops, lhs, rhs, cur->line);
        lhs = (ast_t *)n;
    }
}

ast_t *parse_expression() {
    ast_t *lhs = parse_primary();
    if (!lhs) {
        return NULL;
    }
    ast_t *res = parse_binary_ops(0, lhs);
    return res;
}

void pretty_format(int depth) {
    return;
    for(int i = 0; i < depth; i++) {
        printf(" ");
    }
}

void print_expression(expressions* exps, int depth) {
    if (!exps) {
        printf(" NIL ");
    }
    expression *nodes;
    ast_t *ast;
    hlist_node_t *iter;
    hlist_for_each(exps, iter) {
        nodes = hlist_entry(iter, expression, node);
        ast = nodes->ast;
        print_ast(ast, depth);
    }
}

void print_ast(ast_t *t, int depth) {
    if (!t) {
        printf("ast is NULL;\n");
        return;
    }

    //PRINTF_ENUM(t->type);
    switch (t->type) {
        case BOOLEANAST:
            pretty_format(depth);
            printf("%s ", ((boolean_ast_t*)t)->value ? "TRUE" : "FALSE");
            break;
        case NUMBERAST:
            pretty_format(depth);
            printf("%ld ", ((number_ast_t*)t)->value);
            break;
        case STRINGAST:
            pretty_format(depth);
            printf("%s ", ((string_ast_t*)t)->value);
            break;
        case BINARYAST:
            //binary_ast_t *temp = (binary_ast_t*)t;
            pretty_format(depth);
            printf("(");
            print_ast(((binary_ast_t*)t)->op, depth+1);
            print_ast(((binary_ast_t*)t)->lhs, depth+1);
            print_ast(((binary_ast_t*)t)->rhs, depth+1);
            pretty_format(depth);
            printf(") ");
            break;
        case VARIABLEAST:
            pretty_format(depth);
            printf("%s ", ((variable_ast_t*)t)->value);
            break;
        case CHARACTERAST:
            pretty_format(depth);
            printf("%c ", ((character_ast_t*)t)->value);
            break;
        case IFAST:
            pretty_format(depth);
            printf("if ");
            print_ast(((if_ast_t*)t)->condition, depth+1);
            printf("then ");
            print_expression(((if_ast_t*)t)->then, depth+1);
            printf("else ");
            print_expression(((if_ast_t*)t)->els, depth+1);
            break;
        case WHILEAST:
            pretty_format(depth);
            printf("while ");
            print_ast(((while_ast_t*)t)->condition, depth+1);
            printf("body ");
            print_expression(((while_ast_t*)t)->body, depth+1);
            break;
        case FUNCTIONAST:
            pretty_format(depth);
            printf("define ");
            printf("%s ", ((function_ast_t*)t)->name);
            printf("params ");
            print_expression(((function_ast_t*)t)->params, depth+1);
            printf("body ");
            print_expression(((function_ast_t*)t)->body, depth+1);
            break;
        case CALLAST:
            pretty_format(depth);
            printf("call ");
            printf("%s ", ((call_ast_t*)t)->name);
            printf("args ");
            print_expression(((call_ast_t*)t)->args, depth+1);
            break;
        case CONSTAST:
            pretty_format(depth);
            printf("const ");
            printf("%s %s ", ((const_ast_t*)t)->valuetype, ((const_ast_t*)t)->value);
            break;
        case DECLAST:
            pretty_format(depth);
            printf("var ");
            printf("%s %s ", ((const_ast_t*)t)->valuetype, ((const_ast_t*)t)->value);
            break;
        default:
            ERRORF(current_file, t->line, "no such ast type (%d)", t->type);
    }
}

expressions *parser() {
    // init curtok
    parse_index = 0;
    get_next_token();

    // parse
    expressions *ast = gather_expression();
    return ast;

    /*
    expression *exp;
    hlist_node_t *iter;
    hlist_for_each(ast, iter) {
        exp = hlist_entry(iter, expression, node);
        print_ast(exp->ast, 0);
    }
    */
    //print_expression(ast, 0);
    //printf("\n");
    /*
    ast_t *ast;
    while(1) {
        ast = parse_expression();
        if (!ast) 
            break;

        lex *current = curtok;
        // handle the end of expression
        if (current && current->token == SEMICOLON) {
            get_next_token();
        }
        print_ast(ast, 0);
        printf("\n");
    }
    return NULL;
    */
}
