#include "vm.h"
#include "repl.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

static void run_file(const char* filename) {
    const char* source = read_text_file(filename);
    Result      result = interpret(source);
    free((void*)source);
    if (result == RESULT_COMPILE_ERROR) exit(65);
    if (result == RESULT_RUNTIME_ERROR) exit(70);
}

int main(int argc, char** argv) {
    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        run_file(argv[1]);
    } else {
        printf("Usage: solv [path]");
        return 64;
    }
    return 0;
}
