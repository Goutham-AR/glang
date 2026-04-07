#pragma once

#include "common.hh"

#include <stdio.h>
#include <stdlib.h>

#include "Vm.hh"

// more work needs to be done on the REPL
// like handling multi-line inputs
inline void repl() {
    size_t capacity = 32;
    char* line = (char*)malloc(capacity);

    while (true) {
        printf(">>> ");

        if (getline(&line, &capacity, stdin) == -1) {
            printf("\n");
            break;
        }

        interpret(line);
    }
}
