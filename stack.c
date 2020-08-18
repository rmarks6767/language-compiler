#include "stack.h"
#include <stdlib.h>
#include <stdio.h>

stack_t *make_stack(void) {
    stack_t *stack = (stack_t *)malloc(sizeof(stack_t));
    stack -> top = NULL;
    return stack;
}

void push(stack_t *stack, void *data) {
    stack_node_t *new_node = (stack_node_t *) malloc(sizeof(stack_node_t));
    
    new_node -> data = data;      
    
    if (stack -> top != NULL) {
        new_node -> next = stack -> top;
    } else {
        new_node -> next = NULL;
    }
    stack -> top = new_node;
}

void *top(stack_t *stack) {
    if (stack -> top)  {
        return stack -> top -> data;
    }
    // This error of a null stack will be handled in the parse function
    return NULL;
}

void pop(stack_t *stack) {
    if (stack -> top)  {
        stack_node_t *top_node = stack -> top;
        stack -> top = top_node -> next;

        free(top_node -> data);
        free(top_node);
        return;
    }
    fprintf(stderr, "The stack is NULL\n");
    exit(EXIT_FAILURE);
}

int empty_stack(stack_t *stack) {
    if (stack -> top) {
        return 0;
    } else {
        return 1;
    }
}

void free_stack(stack_t *stack) {
    while(stack -> top) {
        stack_node_t *top_node = stack -> top;
        stack -> top = top_node -> next;
        
        free(top_node -> data);
        free(top_node);
    }
    free(stack);
}
