#include "Vm.hh"
#include "debug.hh"

#define BINARY_OP(op)              \
    do {                           \
        auto op2 = popFromStack(); \
        auto op1 = popFromStack(); \
        pushToStack(op1 op op2);   \
    } while (false)

GlangVm::GlangVm(const ByteCode& code)
    : code_{code},
      iPtr_{nullptr},
      stack_{},
      stackTop_{stack_} {
}

void GlangVm::init(const ByteCode& code) {
    code_ = code;
    iPtr_ = nullptr;
    stackTop_ = stack_;
}

Result GlangVm::interpret() {
    iPtr_ = code_.code_.data();
    return run();
}
Result GlangVm::interpret(const ByteCode& code) {
    code_ = code;
    return interpret();
}

Result GlangVm::run() {
    while (true) {

#ifdef TRACE_VM_EXECUTION
        fmt::print("Stack: [");
        for (auto slot = stack_; slot < stackTop_; ++slot) {
            fmt::print(" {} ", valueToString(*slot));
        }
        fmt::print("]\n");
        debug::disassembleInstruction(code_, static_cast<int>(iPtr_ - code_.code_.data()));
#endif

        // fetch instruction
        auto instruction = readInstr();

        // decode and execute
        switch (instruction) {

        case OpCode::Return: {
            printValue(popFromStack());
            fmt::print("\n");
            return Result::Ok;
        }

        case OpCode::Constant: {
            auto constant = readConstant();
            pushToStack(constant);
            break;
        }

        case OpCode::Negate: {
            pushToStack(-popFromStack());
            break;
        }

        case OpCode::Add: {
            BINARY_OP(+);
            break;
        }
        case OpCode::Subtract: {
            BINARY_OP(-);
            break;
        }

        case OpCode::Multiply: {
            BINARY_OP(*);
            break;
        }

        case OpCode::Divide: {
            BINARY_OP(/);
            break;
        }
        }
    }
}
u8 GlangVm::readByte() {
    auto byte = *iPtr_;
    ++iPtr_;
    return byte;
}
OpCode GlangVm::readInstr() {
    return toOp(readByte());
}

Value GlangVm::readConstant() {
    auto operand = *iPtr_;
    ++iPtr_;
    return code_.getConstantAtOffset(operand);
}

void GlangVm::pushToStack(Value value) {
    *stackTop_ = value;
    ++stackTop_;
}
Value GlangVm::popFromStack() {
    --stackTop_;
    return *stackTop_;
}

#undef BINARY_OP