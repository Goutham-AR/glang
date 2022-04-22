#include "ByteCode.hh"

void ByteCode::writeByte(u8 byte, int lineNumber) {
    code_.push_back(byte);
    lineNumbers_.push_back(lineNumber);
}

void ByteCode::writeConstantInstr(Value constant, int lineNumber) {
    writeOpCode(OpCode::Constant, lineNumber);
    auto offset = writeValue(constant);
    writeByte(offset, lineNumber);
}
u8 ByteCode::getByte(int offset) const {
    return code_[offset];
}

void ByteCode::writeOpCode(OpCode code, int lineNumber) {
    writeByte(toU8(code), lineNumber);
}

OpCode ByteCode::getOpCode(int offset) const {
    assert(offset >= 0);
    return toOp(getByte(offset));
}

size ByteCode::writeValue(Value value) {
    constants_.push_back(value);
    return constants_.size() - 1;
}

Value ByteCode::getConstant(int offset) const {
    assert(offset >= 0);
    auto constantOffset = getByte(offset);
    return constants_[constantOffset];
}

int ByteCode::getLineNumber(int offset) const {
    return lineNumbers_[offset];
}