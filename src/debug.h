#ifndef GLANG_DEBUG_H
#define GLANG_DEBUG_H

#include "bytecode.h"

void debug_disassemble_bytecode(ByteCode* bc);
int  debug_disassemble_instruction(ByteCode* bc, int offset);

#endif /* GLANG_DEBUG_H */
