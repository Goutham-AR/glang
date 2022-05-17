#include "debug.hh"

#include "ByteCode.hh"

namespace debug {

static int simpleInstr(const char* name, int offset) {
    fmt::print("{}\n", name);
    return offset + 1;
}

static int constantInstruction(std::string_view name, const ByteCode& code, int offset) {
    auto constantOffset = toU8(code.getOpCode(offset + 1));
    auto constant = code.getConstantAtOffset(constantOffset);
    fmt::print("{} {} [{}]\n", name, constantOffset, constant.toString());

    return offset + 2;
}

static int byteInstruction(std::string_view name, const ByteCode& code, int offset) {
    auto slot = toU8(code.getOpCode(offset + 1));
    fmt::print("{} {}\n", name, slot);
    return offset + 2;
}

static int jumpInstruction(std::string_view name, int sign, const ByteCode& code, int offset) {
    auto jump = (uint16_t)(toU8(code.getOpCode(offset + 1)) << 8);
    jump |= toU8(code.getOpCode(offset + 2));
    fmt::print("{} {} -> {}\n", name, offset, offset + 3 + sign * jump);
    return offset + 3;
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
        return constantInstruction("Constant", code, offset);
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
    case OpCode::Print:
        return simpleInstr("Print", offset);
    case OpCode::Pop:
        return simpleInstr("Pop", offset);
    case OpCode::DefineGlobal:
        return constantInstruction("DefineGlobal", code, offset);
    case OpCode::GetGlobal:
        return constantInstruction("GetGlobal", code, offset);
    case OpCode::SetGlobal:
        return constantInstruction("SetGlobal", code, offset);
    case OpCode::GetLocal:
        return byteInstruction("GetLocal", code, offset);
    case OpCode::SetLocal:
        return byteInstruction("SetLocal", code, offset);
    case OpCode::Jmp:
        return jumpInstruction("Jmp", 1, code, offset);
    case OpCode::JmpIfFalse:
        return jumpInstruction("JmpIfFalse", 1, code, offset);
    case OpCode::Loop:
        return jumpInstruction("Loop", -1, code, offset);

    default:
        fmt::print("unknown opcode\n");
        return offset + 1;
    }

    assert(false && "This Code path should not be taken");
}
}