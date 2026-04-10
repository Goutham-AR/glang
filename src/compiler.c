#include "compiler.h"
#include "parser.h"

#ifdef DEBUG_PRINT_BYTECODE
#include "debug.h"
#endif

Compiler* g_current = NULL;

bool compile(const char* source, ByteCode* bytecode) {
    Scanner scanner;
    scanner_init(&scanner, source);

    Parser parser;
    parser_init(&parser, &scanner, bytecode);

    Compiler compiler;
    compiler.local_count = 0;
    compiler.scope_depth = 0;
    g_current = &compiler;

    parser_advance(&parser);
    while (!parser_match(&parser, TokenEof))
        parser_declaration(&parser);

    parser_emit_return(&parser);

#ifdef DEBUG_PRINT_BYTECODE
    if (!parser_has_error(&parser))
        debug_disassemble_bytecode(bytecode);
#endif

    return !parser_has_error(&parser);
}
