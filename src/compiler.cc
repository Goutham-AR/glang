#include "compiler.hh"
#include "ByteCode.hh"
#include "Parser.hh"

#ifdef DEBUG_PRINT_BYTECODE
#include "debug.hh"
#endif

Compiler* g_current = nullptr;

bool compile(const std::string& code, ByteCode& byteCode) {
    Scanner scanner{code};
    Parser parser{scanner, byteCode};
    Compiler compiler;
    compiler.localCount = 0;
    compiler.scopeDepth = 0;
    g_current = &compiler;
    parser.advance();

    // parser.expression();
    // parser.consume(TokenEof, "Expect end of expression");

    while (!parser.match(TokenEof)) {
        parser.declaration();
    }

    parser.emitReturn();
#ifdef DEBUG_PRINT_BYTECODE
    if (!parser.hasError()) {
        debug::disassembleByteCode(byteCode);
    }
#endif

    return !parser.hasError();
}