#ifndef SOLV_INSTRUCTIONS_H
#define SOLV_INSTRUCTIONS_H

#include "common.h"

typedef enum {
    OP_RETURN,
    OP_CONSTANT,
    OP_NEGATE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_NOT,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_PRINT,
    OP_POP,
    OP_DEFINE_GLOBAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_SET_LOCAL,
    OP_GET_LOCAL,
    OP_JMP_IF_FALSE,
    OP_JMP,
    OP_LOOP
} OpCode;

static inline u8     to_u8(OpCode code) { return (u8)code;    }
static inline OpCode to_op(u8 byte)     { return (OpCode)byte; }

#endif /* SOLV_INSTRUCTIONS_H */
