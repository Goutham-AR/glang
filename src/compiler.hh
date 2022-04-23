#pragma once

#include "common.hh"
#include "Scanner.hh"

inline void compile(const std::string& code) {
    Scanner scanner{code};

    int line = -1;

    while (true) {
        auto token = scanner.scanToken();
        if (token.line != line) {
            fmt::print("{:4} ", token.line);
            line = token.line;
        } else {
            fmt::print("   |");
        }
        fmt::print("{:2} '{}'\n", token.type, token.name);

        if (token.type == TokenEof) break;
    }
}
