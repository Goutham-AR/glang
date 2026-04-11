#ifndef SOLV_BYTECODE_H
#define SOLV_BYTECODE_H

#include "common.h"
#include "instructions.h"
#include "value.h"

typedef struct {
    int        count;
    int        capacity;
    uint8_t*   code;
    int*       lines;
    ValueArray constants;
} ByteCode;

void bytecode_init(ByteCode* bc);
void bytecode_free(ByteCode* bc);
void bytecode_write_byte(ByteCode* bc, u8 byte, int line);
void bytecode_write_opcode(ByteCode* bc, OpCode code, int line);
int  bytecode_add_constant(ByteCode* bc, Value value);

#endif /* SOLV_BYTECODE_H */
