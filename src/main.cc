#include "log.hh"
#include "ByteCode.hh"
#include "debug.hh"
#include "Vm.hh"
#include "repl.hh"
#include "utils.hh"

void runFile(const char* filename) {
    auto sourceCode = utils::readTextFile(filename);
    sourceCode.push_back('\0');
    auto result = interpret(sourceCode);

    if (result == Result::CompileError) std::exit(65);
    if (result == Result::RuntimeError) std::exit(70);
}

int main(int argc, char** argv) {

    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        fmt::print("Usage: glang [path]");
        return 64;
    }

    return 0;
}