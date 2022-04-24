#include "debug.hh"

#include "ByteCode.hh"

namespace debug {

static int simpleInstr(const char* name, int offset) {
    fmt::print("{}\n", name);
    return offset + 1;
}

static int constantInstruction(const ByteCode& code, int offset) {
    auto constantOffset = toU8(code.getOpCode(offset + 1));
    auto constant = code.getConstantAtOffset(constantOffset);
    fmt::print("Constant {} [{}]\n", constantOffset, valueToString(constant));

    return offset + 2;
}

void disassembleByteCode(const ByteCode& code) {
    fmt::print("== disassembly ==\n");

    for (int offset = 0; offset < code.codeSize();) {
        offset = disassembleInstruction(code, offset);
    }
}
int disassembleInstruction(const ByteCode& code, int offset) {
    fmt::print("{:03} {:04} ", code.getLineNumber(offset), offset);

    auto instruction = code.getOpCode(offset);

    switch (instruction) {
    case OpCode::Return:
        return simpleInstr("Return", offset);
    case OpCode::Constant:
        return constantInstruction(code, offset);
    case OpCode::Negate:
        return simpleInstr("Negate", offset);
    case OpCode::Add:
        return simpleInstr("Add", offset);
    case OpCode::Subtract:
        return simpleInstr("Subtract", offset);
    case OpCode::Multiply:
        return simpleInstr("Multiply", offset);
    case OpCode::Divide:
        return simpleInstr("Divide", offset);
    case OpCode::Nil:
        return simpleInstr("Nil", offset);
    case OpCode::False:
        return simpleInstr("False", offset);
    case OpCode::True:
        return simpleInstr("True", offset);
    case OpCode::Not:
        return simpleInstr("Not", offset);
    case OpCode::Equal:
        return simpleInstr("Equal", offset);
    case OpCode::Greater:
        return simpleInstr("Greater", offset);
    case OpCode::Less:
        return simpleInstr("Less", offset);

    default:
        fmt::print("unknown opcode\n");
        return offset + 1;
    }

    assert(false && "This Code path should not be taken");
}
}