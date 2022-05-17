#pragma once

#include "common.hh"
#include "Scanner.hh"

class ByteCode;

struct Local {
    Token name;
    int depth;
};

struct Compiler {
    Local locals[UINT8_MAX + 1];
    int localCount{};
    int scopeDepth{};
};

extern Compiler* g_current;
bool compile(const std::string& code, ByteCode& byteCode);
