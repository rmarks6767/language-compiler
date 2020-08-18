/// @author: River Marks
/// @class: CS241 MOPS

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "interp.h"
#include "symtab.h"

int main(int argc, char **argv) {
    char *filename;
    if (argc == 2){
        filename = argv[1];
    } 
    else if (argc == 1){
        filename = NULL;
    }
    else {
        fprintf(stderr, "Usage: interp [sym-table]\n");
        exit(EXIT_FAILURE);
    }

    if (filename) {
        build_table(filename);
    }
    dump_table();

    printf("Enter postfix expressions (CTRL-D to exit):\n");
    
    int cur_pos = 0;
    char c, exp[MAX_LINE];

    for (int i = 0; i <= MAX_LINE; i++) {
        exp[i] = '\0';
    }

    printf("> ");
    while((c = getchar()) != EOF && cur_pos < MAX_LINE) {
        if (c == '\n'){
            rep(exp);
            for (int i = 0; i <= cur_pos; i++) {
                exp[i] = '\0';
            }
            printf("> ");
            cur_pos = 0;
        }
        else {
            exp[cur_pos] = c;
            cur_pos++;
        }
    }
    printf("\n");
    dump_table();
    free_table();
    return 0;
}
