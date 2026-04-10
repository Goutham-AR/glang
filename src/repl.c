#include "repl.h"
#include "vm.h"
#include <stdio.h>
#include <string.h>

#define REPL_LINE_MAX 1024

void repl(void) {
    char line[REPL_LINE_MAX];
    while (true) {
        printf(">>> ");
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }
        line[strcspn(line, "\n")] = '\0';
        interpret(line);
    }
}
