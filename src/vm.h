#ifndef GLANG_VM_H
#define GLANG_VM_H

#include "common.h"
#include "bytecode.h"
#include "table.h"

#define STACK_MAX 256

typedef struct {
    ByteCode* bytecode;
    u8*       ip;
    Value     stack[STACK_MAX];
    Value*    stack_top;
    Table     globals;
} VM;

Result interpret(const char* source);

#endif /* GLANG_VM_H */
