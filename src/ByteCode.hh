#pragma once

#include "common.hh"
#include "instructions.hh"
#include "Value.hh"

// std
#include <vector>

class ByteCode {
public:
    ByteCode() = default;
    ~ByteCode() = default;

    void writeOpCode(OpCode code, int lineNumber);
    void writeConstantInstr(Value constant, int lineNumber);
    //  this offset refers to the offset into 'code_' not 'constants_'
    [[nodiscard]] Value getConstant(int offset) const;
    [[nodiscard]] int getLineNumber(int offset) const;
    [[nodiscard]] size codeSize() const { return code_.size(); }
    [[nodiscard]] OpCode getOpCode(int offset) const;

private:
    void writeByte(u8 byte, int lineNumber);
    size writeValue(Value value);

    [[nodiscard]] u8 getByte(int offset) const;

private:
    std::vector<std::uint8_t> code_;
    std::vector<Value> constants_;
    std::vector<int> lineNumbers_;
};