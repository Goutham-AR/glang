#include "log.hh"
#include "ByteCode.hh"
#include "debug.hh"
#include "Vm.hh"
#include "repl.hh"
#include "utils.hh"

#include <stdio.h>

void runFile(const char* filename) {
    const char* sourceCodeContent = read_text_file(filename);
    std::string sourceCode{sourceCodeContent};
    auto result = interpret(sourceCode);

    if (result == Result::CompileError) exit(65);
    if (result == Result::RuntimeError) exit(70);
}

int main(int argc, char** argv) {

    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        printf("Usage: glang [path]");
        return 64;
    }

    return 0;
}
