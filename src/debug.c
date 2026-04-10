#include "debug.h"
#include "value.h"
#include <stdio.h>

static int simple_instr(const char* name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}

static int constant_instr(const char* name, ByteCode* bc, int offset) {
    u8    idx      = bc->code[offset + 1];
    Value constant = bc->constants.values[idx];
    printf("%s %d [", name, idx);
    value_print(constant);
    printf("]\n");
    return offset + 2;
}

static int byte_instr(const char* name, ByteCode* bc, int offset) {
    printf("%s %d\n", name, bc->code[offset + 1]);
    return offset + 2;
}

static int jump_instr(const char* name, int sign, ByteCode* bc, int offset) {
    u16 jump = (u16)((bc->code[offset + 1] << 8) | bc->code[offset + 2]);
    printf("%s %d -> %d\n", name, offset, offset + 3 + sign * (int)jump);
    return offset + 3;
}

void debug_disassemble_bytecode(ByteCode* bc) {
    printf("== disassembly ==\n");
    for (int offset = 0; offset < bc->count; )
        offset = debug_disassemble_instruction(bc, offset);
}

int debug_disassemble_instruction(ByteCode* bc, int offset) {
    printf("%03d %04d ", bc->lines[offset], offset);
    OpCode instr = (OpCode)bc->code[offset];
    switch (instr) {
    case OP_RETURN:        return simple_instr("Return",        offset);
    case OP_CONSTANT:      return constant_instr("Constant",    bc, offset);
    case OP_NEGATE:        return simple_instr("Negate",        offset);
    case OP_ADD:           return simple_instr("Add",           offset);
    case OP_SUBTRACT:      return simple_instr("Subtract",      offset);
    case OP_MULTIPLY:      return simple_instr("Multiply",      offset);
    case OP_DIVIDE:        return simple_instr("Divide",        offset);
    case OP_NIL:           return simple_instr("Nil",           offset);
    case OP_TRUE:          return simple_instr("True",          offset);
    case OP_FALSE:         return simple_instr("False",         offset);
    case OP_NOT:           return simple_instr("Not",           offset);
    case OP_EQUAL:         return simple_instr("Equal",         offset);
    case OP_GREATER:       return simple_instr("Greater",       offset);
    case OP_LESS:          return simple_instr("Less",          offset);
    case OP_PRINT:         return simple_instr("Print",         offset);
    case OP_POP:           return simple_instr("Pop",           offset);
    case OP_DEFINE_GLOBAL: return constant_instr("DefineGlobal",bc, offset);
    case OP_GET_GLOBAL:    return constant_instr("GetGlobal",   bc, offset);
    case OP_SET_GLOBAL:    return constant_instr("SetGlobal",   bc, offset);
    case OP_GET_LOCAL:     return byte_instr("GetLocal",        bc, offset);
    case OP_SET_LOCAL:     return byte_instr("SetLocal",        bc, offset);
    case OP_JMP:           return jump_instr("Jmp",           1, bc, offset);
    case OP_JMP_IF_FALSE:  return jump_instr("JmpIfFalse",    1, bc, offset);
    case OP_LOOP:          return jump_instr("Loop",         -1, bc, offset);
    default:
        printf("unknown opcode %d\n", (int)instr);
        return offset + 1;
    }
}
