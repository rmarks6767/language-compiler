#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "stack.h"
#include "tree_node.h"
#include "symtab.h"

#define MAX_TOKEN 100

// These will be used for tracking any errors that occur in the program
parse_error_t parse_error = PARSE_NONE;
eval_error_t eval_error = EVAL_NONE;

// Used for keeping track of state when reading in the parse tree
typedef enum parse_tree_e {
    READING,
    READ,
    COMMENT,
    ERROR
} parse_tree_t;

/// Determines if the char is one of the acceptable operators
///
/// @param operator The symbol to be checked
/// @return Returns true if it is an operator
int isoperand(char operator){
    switch(operator){
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '?':
        case '=':
            return 1;
        default:
            return 0;
    }
}

/// Used to see if the string is all numbers
///
/// @param string The string to check if it is all numbers
/// @return Returns true if the entire string is a number
int isstringdigit(char *string){
    int isstringdigit = 1;
    for (int i = 0; i < (int)strlen(string); i++) {
        if (isdigit(string[i]) != 0){
            isstringdigit *= 1;
        }
        else {
            isstringdigit *= 0;
        }
    }
    return isstringdigit;
}

void rep(char *exp) {
    // First we build the parse tree
    tree_node_t *tree = make_parse_tree(exp);
    // Make sure no errors occured in the construction of the parse tree
    if (parse_error != PARSE_NONE){
        switch(parse_error){  
            case TOO_FEW_TOKENS:
                fprintf(stderr, "Not enough tokens in expression!\n");
                break;
            case TOO_MANY_TOKENS:
                fprintf(stderr, "Too many tokens in expression!\n");
                break;
            case INVALID_ASSIGNMENT: 
                fprintf(stderr, "Assign to left hand side not a variable!\n");
                break;
            case ILLEGAL_TOKEN:
                fprintf(stderr, "Illegal token is present!\n");
                break;
            default:
                fprintf(stderr, "Unkown error occured!\n");
                break;
        }
        // Cleanup everything then reset the parse error back to NONE
        if (tree != NULL){
            cleanup_tree(tree);
        }
        parse_error = PARSE_NONE;
        return;
    }
    
    // Now that there were no errors re evaluate the tree
    int value = eval_tree(tree);

    // Check if there were any errors in the eval
    if (eval_error != EVAL_NONE){
        switch(eval_error) {
            case DIVISION_BY_ZERO:
                fprintf(stderr, "Division by zero\n");
                break;
            case INVALID_MODULUS:
                fprintf(stderr, "Division by zero\n");
                break;
            case UNDEFINED_SYMBOL:
                fprintf(stderr, "Symbol is not in the table!\n");
                break;
            case UNKNOWN_OPERATION:
                fprintf(stderr, "Operation is unknown!\n");
                break;
            case UNKNOWN_EXP_TYPE:
                fprintf(stderr, "Unknown expression type\n");            
                break;        
            case MISSING_LVALUE:
                fprintf(stderr, "Missing a value on the left!\n");
                break;
            case INVALID_LVALUE:
                fprintf(stderr, "Invalid value on the left\n");
                break;
            case SYMTAB_FULL:
                fprintf(stderr, "Could not make symbol, symbol table is full!\n");
                break;
            default:
                fprintf(stderr, "A unknown error occured!\n");
                break;
        }
    } 
    else {
        // If no errors, we print
        print_infix(tree);
        printf(" = %d\n", value);
    }
    // Reset the error value and cleanup the tree
    eval_error = EVAL_NONE;
    cleanup_tree(tree);
}

tree_node_t *parse(stack_t *stack) {
    // We need to make a copy of the string, otherwise it will be freed
    char *token, *dst = (char *) calloc(MAX_TOKEN, sizeof(char));
    // Attempt to get the top of the stack
    char *stack_val = (char *)top(stack);

    // Check the stack_val for not NULL
    if (stack_val){
        // Copy the value to the token
        token = strcpy(dst, (char *)top(stack));
    }
    else {
        // If the stack is empty that means there wasn't enough tokens
        parse_error = TOO_FEW_TOKENS;
        free(dst);
        return NULL;
    }

    // Remove it from the stack
    pop(stack);

    // Check if it is a token by making sure it's not a digit or char
    if (isalpha(*token) == 0 && isdigit(*token) == 0){
        op_type_t op;
        tree_node_t *left, *right;

        // Check the operation based on the token
        if (!strcmp(token, ADD_OP_STR)){
            op = ADD_OP;
            right = parse(stack);
            left = parse(stack);
        } 
        else if (!strcmp(token, SUB_OP_STR)){
            op = SUB_OP;
            right = parse(stack);
            left = parse(stack);
        } 
        else if (!strcmp(token, MUL_OP_STR)){
            op = MUL_OP;
            right = parse(stack);
            left = parse(stack);
        } 
        else if (!strcmp(token, DIV_OP_STR)){
            op = DIV_OP;
            right = parse(stack);
            left = parse(stack);
        } 
        else if (!strcmp(token, MOD_OP_STR)){
            op = MOD_OP;
            right = parse(stack);
            left = parse(stack);
        } 
        else if (!strcmp(token, Q_OP_STR)){
            op = Q_OP;
            
            // Get the left and right off of the : operator
            tree_node_t *alt_right = parse(stack);
            tree_node_t *alt_left = parse(stack);
            
            char *operand = calloc(sizeof(char), 2);
            
            // Set the operand as the :
            operand[0] = ':';
            
            // Get both sides of the tree node
            right = make_interior(ALT_OP, operand, alt_left, alt_right);
            left = parse(stack);

            return make_interior(op, token, left, right);
        } 
        else if (!strcmp(token, ASSIGN_OP_STR)){
            op = ASSIGN_OP;
            right = parse(stack);
            left = parse(stack);

            // We can't set a number on the left equal to something
            if (isstringdigit(left -> token)){
                parse_error = INVALID_ASSIGNMENT;
                cleanup_tree(left);
                cleanup_tree(right);
                free(dst);
                return NULL;
            }
        }
        else {
            // There was an illegal token present
            parse_error = ILLEGAL_TOKEN;
        }

        return make_interior(op, token, left, right);

    } 
    else {
        exp_type_t op;
        if (isdigit(*token) != 0){
            op = INTEGER;
        } 
        else {
            op = SYMBOL;
        }
        return make_leaf(op, token);
    }
}

tree_node_t *make_parse_tree(char *expr) {
    // Make a stack to put the tokens in
    stack_t *stack = make_stack();

    int cur_index = 0;

    char *data = (char *)calloc(MAX_TOKEN, sizeof(char));

    parse_tree_t state = READING;

    // Iterate through the expression and switch states accordingly
    for (int i = 0; i < (int)(strlen(expr)); i++){
        switch(state) {
            case READING:
                // if the data has a value and the expression element is a space
                if (data[0] != '\0' && expr[i] == ' ') {
                    state = READ;
                }
                else if (expr[i] == '#'){
                    state = COMMENT;
                    break;
                }
                else if (isoperand(expr[i]) == 0 && isalpha(expr[i]) == 0 && isdigit(expr[i]) == 0) {
                    parse_error = ILLEGAL_TOKEN;
                    state = ERROR;
                    break;
                }
                else if (expr[i] != ' ') {
                    data[cur_index] = expr[i];
                    cur_index++;
                    break;
                } 
                else {
                    break;
                }
            case READ:
                if(isstringdigit(data) == 0 && isdigit(data[0]) != 0){
                    parse_error = ILLEGAL_TOKEN;
                    state = ERROR;
                } 
                push(stack, (void *) data);
                data = (char *)calloc(MAX_TOKEN, sizeof(char));
                cur_index = 0;
                if (expr[i] == '#'){
                    state = COMMENT;
                }
                else {
                    state = READING;
                }
                break;
            case COMMENT:
                if (data[0] != '\0') {
                    push(stack, (void *) data);
                    data = (char *)calloc(MAX_TOKEN, sizeof(char));
                }
                if (expr[i] == '\n'){
                    state = READING;
                }
                break;
            case ERROR:
                if (data[0] != '\0'){
                    push(stack, (void *) data);
                }
                else {
                    free(data);
                }
                free_stack(stack);
                return NULL;
                break;
        }
    }
    // Illegal token if the data is not a digit
    if(isstringdigit(data) == 0 && isdigit(data[0]) != 0){
        parse_error = ILLEGAL_TOKEN;
        return NULL;
    } 

    if (data[0] != '\0'){
        push(stack, (void *) data);
    }
    else {
        free(data);
    }
    // If we get here we can now parse the stack
    tree_node_t *tree = parse(stack);

    // if the stack isn't empty we will say that there were too many tokens
    if (empty_stack(stack)){
        free_stack(stack);
        return tree;
    }
    else {
        free_stack(stack);
        parse_error = TOO_MANY_TOKENS;
        cleanup_tree(tree);
        return NULL;
    }
}

int eval_tree(tree_node_t *node) {
    // Check to see if the type is a leaf or an interior
    if (node -> type == LEAF) {
        // We will return the value of the leaf if it exists
        leaf_node_t *leaf = (leaf_node_t *)node -> node;

        // We are missing a node to evaluate
        if (leaf == NULL){
            eval_error = MISSING_LVALUE;
            return -1;
        }

        switch(leaf -> exp_type){
            case INTEGER: {
                return atoi(node -> token);
            }
            case SYMBOL:{
                symbol_t *var_node = lookup_table(node -> token);
                if (var_node == NULL){
                    eval_error = UNDEFINED_SYMBOL;
                    return -1;
                }
                return var_node -> val;
            }
            default: {
                // This will be an unknown exp type
                eval_error = UNKNOWN_EXP_TYPE;
                return -1; 
            }
        }
    }
    else if (node -> type == INTERIOR){
        interior_node_t *interior = (interior_node_t *)node -> node;

        // Get all the parts of the interior node
        op_type_t op = interior -> op;

        tree_node_t *left = interior -> left;
        tree_node_t *right = interior -> right;

        int right_val = 0, left_val = 0;
        // Check the operator to see what to perform on the symbols
        switch(op){
            case ADD_OP:{
                return eval_tree(left) + eval_tree(right);
            }
            case SUB_OP:{
                return eval_tree(left) - eval_tree(right);
            }
            case MUL_OP:{
                return eval_tree(left) * eval_tree(right);
            }
            case DIV_OP:{
                // Must check for divide by zero
                right_val = eval_tree(right);
                left_val = eval_tree(left);

                if (right_val == 0){
                    eval_error = DIVISION_BY_ZERO;
                    return -1;
                }
                else {
                    return left_val / right_val;
                }
            }
            case MOD_OP:{
                // Must check for invalid mod
                right_val = eval_tree(right);
                left_val = eval_tree(left);

                if (right_val == 0){
                    eval_error = INVALID_MODULUS;
                    return -1;
                }
                else {
                    return left_val % right_val;
                }
            }
            case ASSIGN_OP:{
                // This will be an invalid Lvalue
                if (left -> type != LEAF){
                    eval_error = INVALID_LVALUE;             
                }
                char *left_val = calloc(strlen(left -> token) + 1, sizeof(char));

                strcpy(left_val, left -> token);

                int right_val = eval_tree(right);

                symbol_t *sym = lookup_table(left_val);

                if (sym){
                    sym -> val = right_val;
                    free(left_val);
                }
                else {
                    symbol_t *symbol = create_symbol(left_val, right_val);
                    // Could not allocate enough memory for this symbol
                    if (symbol == NULL){
                        eval_error = SYMTAB_FULL;
                    }
                }
                return right_val;
            }
            case Q_OP:{
                // If we get not 0 for the left side, then we will call the left node's eval
                if (eval_tree(left)){
                    return eval_tree(((interior_node_t *)right -> node) -> left);
                }
                else {
                    return eval_tree(((interior_node_t *)right -> node) -> right);
                }
            }
            case ALT_OP:
            case NO_OP:
            default:
                // Both of the above and anything else should are not allowed
                eval_error = UNKNOWN_OPERATION;
                return -1;                
        }
    }
    else {
        // If the LVALUE is not either leaf or interior
        eval_error = MISSING_LVALUE;
        return -1;
    }
}

void print_infix(tree_node_t *node) {
    if (node -> type == LEAF) {
        printf("%s", node -> token);
        return;
    }

    if (node -> type == INTERIOR){
        interior_node_t *interior = (interior_node_t  *)node -> node;
        printf("(");

        // Print the left
        print_infix(interior -> left);
        
        // Print the middle
        printf("%s", node -> token);
        
        // Print the right
        print_infix(interior -> right);
        printf(")");
    }  
}

void cleanup_tree(tree_node_t *node) {
    // Recursively cleanup all of the elements in the tree
    if (node){
        switch(node -> type){
            case INTERIOR: {
                interior_node_t *interior = (interior_node_t *)node -> node;
                cleanup_tree(interior -> right);
                cleanup_tree(interior -> left);
                free(interior);
                break;
            }
            case LEAF: {
                leaf_node_t *leaf = (leaf_node_t *)node -> node;
                free(leaf);
                break;
            }
        }   
        free(node -> token);
        free(node); 
    }
}