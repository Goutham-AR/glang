#pragma once

#include "common.hh"
#include "ByteCode.hh"
#include "HashTable.hh"

#define STACK_MAX 256

Result interpret(const std::string& code);

class GlangVm {
public:
    explicit GlangVm(const ByteCode& code);
    GlangVm() = default;
    ~GlangVm() = default;

    void init(const ByteCode& code);

    Result interpret();
    Result interpret(const ByteCode& code);

private:
    Result run();
    u8 readByte();
    u16 readShort();

    OpCode readInstr();
    Value readConstant();

    void pushToStack(Value value);
    Value popFromStack();
    Value peekStack(int distance);
    void printStack();

    void concatenate();

    template <typename... T>
    void runtimeError(std::string_view msg, T&&... args) {
        size_t instruction = iPtr_ - code_.code_.data() - 1;
        auto line = code_.lineNumbers_[instruction];
        fmt::print(msg, std::forward<T>(args)...);
        fmt::print("\n");
        fmt::print("[line {}] in script\n", line);
    }

private:
    ByteCode code_;
    u8* iPtr_{};

    Value stack_[STACK_MAX]{};
    Value* stackTop_{}; // points to where the next element is to be pushed

    HashTable globals_;
};
