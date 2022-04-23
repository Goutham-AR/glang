#pragma once

#include "common.hh"

#include <string>
#include <iostream>

#include "Vm.hh"

// more work needs to be done on the REPL
// like handling multi-line inputs
inline void repl() {
    std::string line;

    while (true) {
        fmt::print(">>> ");

        if (!std::getline(std::cin, line)) {
            fmt::print("\n");
            break;
        }

        interpret(line);
    }
}