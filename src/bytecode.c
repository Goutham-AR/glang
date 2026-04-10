#include "bytecode.h"
#include "memory.h"

void bytecode_init(ByteCode* bc) {
    bc->count    = 0;
    bc->capacity = 0;
    bc->code     = NULL;
    bc->lines    = NULL;
    value_array_init(&bc->constants);
}

void bytecode_free(ByteCode* bc) {
    FREE_ARRAY(uint8_t, bc->code,  bc->capacity);
    FREE_ARRAY(int,     bc->lines, bc->capacity);
    value_array_free(&bc->constants);
    bytecode_init(bc);
}

void bytecode_write_byte(ByteCode* bc, u8 byte, int line) {
    if (bc->capacity < bc->count + 1) {
        int old      = bc->capacity;
        bc->capacity = GROW_CAPACITY(old);
        bc->code     = GROW_ARRAY(uint8_t, bc->code,  old, bc->capacity);
        bc->lines    = GROW_ARRAY(int,     bc->lines, old, bc->capacity);
    }
    bc->code[bc->count]  = byte;
    bc->lines[bc->count] = line;
    bc->count++;
}

void bytecode_write_opcode(ByteCode* bc, OpCode code, int line) {
    bytecode_write_byte(bc, to_u8(code), line);
}

int bytecode_add_constant(ByteCode* bc, Value value) {
    return value_array_write(&bc->constants, value);
}
