#ifndef SOLV_COMPILER_H
#define SOLV_COMPILER_H

#include "common.h"
#include "scanner.h"
#include "bytecode.h"

#define LOCALS_MAX (UINT8_MAX + 1)

typedef struct {
    Token name;
    int   depth;
} Local;

typedef struct {
    Local locals[LOCALS_MAX];
    int   local_count;
    int   scope_depth;
} Compiler;

extern Compiler* g_current;

bool compile(const char* source, ByteCode* bytecode);

#endif /* SOLV_COMPILER_H */
