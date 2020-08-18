#include "symtab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEN 100

typedef enum input_e {
    READING_NAME,
    READING_VALUE,
    MAKE_SYMBOL,
    COMMENT,
    ERROR
} input_t;

static symbol_t *table;

void build_table(char *filename) {
    // Read the file
    FILE *fp = fopen(filename, "r");
    
    // If it is null that means we could not read it
    if (fp == NULL) {
        perror("Could not open the file!\n");
        exit(EXIT_FAILURE);
    }

    // Now to build the sym table
    int cur_name = 0, cur_val = 0;
    char c, *name = (char *)calloc(MAX_LEN + 1, sizeof(char)); 
    char *val = (char *)calloc(MAX_LEN + 1, sizeof(char)); 

    input_t state = READING_NAME;

    while((c = fgetc(fp)) != EOF){
        switch(state) {
            case READING_NAME:
                if (name[0] != '\0' && (c == ' ' || c == '\t')) {
                    state = READING_VALUE;
                }
                else if (c == '#'){
                    state = COMMENT;
                    break;
                }
                else if (isalpha(c) != 0 || isdigit(c) != 0) {
                    name[cur_name] = c;
                    cur_name++;
                    break;
                } 
                else {
                    break;
                }
            case READING_VALUE:
                if ((c == '\n' || c == ' ' || c == '\t') && val[0] != '\0') {
                    state = MAKE_SYMBOL;
                }
                else if (c == '#'){
                    state = COMMENT;
                    break;
                }
                else if (isdigit(c) != 0) {
                    val[cur_val] = c;
                    cur_val++;
                    break;
                } 
                else {
                    break;
                }
            case MAKE_SYMBOL:
                create_symbol(name, atoi(val));
                free(val);
                cur_name = 0;
                cur_val = 0;
                name = (char *)calloc(MAX_LEN, sizeof(char) * (MAX_LEN + 1)); 
                val = (char *)calloc(MAX_LEN, sizeof(char) * (MAX_LEN + 1));
                state = READING_NAME; 
                break;
            case COMMENT:
                if (name[0] != '\0' && val[0] != '\0') {
                    create_symbol(name, atoi(val));
                    free(val);
                    cur_name = 0;
                    cur_val = 0;
                    name = (char *)calloc(MAX_LEN, sizeof(char) * (MAX_LEN + 1)); 
                    val = (char *)calloc(MAX_LEN, sizeof(char) * (MAX_LEN + 1)); 
                }
                if (c == '\n'){
                    free(name);
                    free(val);
                    name = (char *)calloc(MAX_LEN, sizeof(char) * (MAX_LEN + 1)); 
                    val = (char *)calloc(MAX_LEN, sizeof(char) * (MAX_LEN + 1));
                    cur_name = 0;
                    cur_val = 0;
                    state = READING_NAME;
                }
                break;
            case ERROR:
                printf("We shouldn't get here!");
                break;
        }               
    }
    if (name[0]){
        create_symbol(name, atoi(val));
    } 
    else {
        free(name);
    }
    free(val);
    fclose(fp);
}

void dump_table(void){
    if (table){
        symbol_t *current = table;
        printf("SYMBOL TABLE:\n");
        while(current) {
            printf("\tName: %s, Value: %d\n", current -> var_name, current -> val);
            current = current -> next;
        }
    }
}

symbol_t *lookup_table(char *variable) {
    symbol_t *current = table;

    while(current) {
        if (!strcmp(current -> var_name, variable)){
            return current;
        }
        current = current -> next;
    }
    return NULL;
}

symbol_t *create_symbol(char *name, int val){
    symbol_t *new_symbol = (symbol_t *)malloc(sizeof(symbol_t));

    // This means that we could not create a new symbol
    if (new_symbol == NULL){
        return NULL;
    }

    new_symbol -> var_name = name;
    new_symbol -> val = val;
    new_symbol -> next = NULL;

    if (table == NULL) {
        table = new_symbol;
    } 
    else {
        symbol_t *next = table;
        table = new_symbol;
        table -> next = next;
    }
    return new_symbol;
}

void free_table(void){
    symbol_t *current = table;

    while(current) {
        symbol_t *to_remove = current;

        current = current -> next;

        free(to_remove -> var_name);
        free(to_remove);
    }
}