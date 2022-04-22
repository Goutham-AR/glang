#include "debug.hh"

#include "ByteCode.hh"

namespace debug {

static int simpleInstr(const char* name, int offset) {
    fmt::print("{}\n", name);
    return offset + 1;
}

static int constantInstruction(const ByteCode& code, int offset) {
    auto constantOffset = toU8(code.getOpCode(offset + 1));
    auto constant = code.getConstant(offset + 1);
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
    default:
        fmt::print("unknown opcode\n");
        return offset + 1;
    }

    assert(false && "This Code path should not be taken");
}
}