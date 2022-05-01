#include "Vm.hh"
#include "debug.hh"
#include "compiler.hh"
#include "object.hh"
#include "memory.hh"

Result interpret(const std::string& code) {
    ByteCode byteCode;

    if (!compile(code, byteCode)) {
        return Result::CompileError;
    }

    GlangVm vMachine{byteCode};

    auto result = vMachine.interpret();

    // fmt::print("{}\n", code);
    return result;
}

static bool isFalsey(Value value) {
    return value::isNil(value) || (value::isBool(value) && !value::asBool(value));
}

#define BINARY_OP(valueType, op)                                                \
    do {                                                                        \
        if (!value::isNumber(peekStack(0)) || !value::isNumber(peekStack(1))) { \
            runtimeError("Operands must be numbers");                           \
            return Result::RuntimeError;                                        \
        }                                                                       \
        double op2 = value::asNumber(popFromStack());                           \
        double op1 = value::asNumber(popFromStack());                           \
        pushToStack(valueType(op1 op op2));                                     \
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
#ifdef TRACE_VM_EXECUTION
    fmt::print("==== Tracing execution ====\n");
#endif

    while (true) {
#ifdef TRACE_VM_EXECUTION
        printStack();
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
            if (!value::isNumber(peekStack(0))) {
                runtimeError("Operand must be a number");
                return Result::RuntimeError;
            }

            pushToStack(value::numberValue(-value::asNumber(popFromStack())));
            break;
        }

        case OpCode::Add: {
            if (object::isString(peekStack(0)) && object::isString(peekStack(1))) {
                concatenate();
            } else if (value::isNumber(peekStack(0)) && value::isNumber(peekStack(1))) {
                double b = value::asNumber(popFromStack());
                double a = value::asNumber(popFromStack());
                pushToStack(value::numberValue(a + b));
            } else {
                runtimeError("Operands must be two numbers or two strings");
                return Result::RuntimeError;
            }
            break;
        }
        case OpCode::Subtract: {
            BINARY_OP(value::numberValue, -);
            break;
        }

        case OpCode::Multiply: {
            BINARY_OP(value::numberValue, *);
            break;
        }

        case OpCode::Divide: {
            BINARY_OP(value::numberValue, /);
            break;
        }

        case OpCode::Nil:
            pushToStack(value::nilValue());
            break;
        case OpCode::True:
            pushToStack(value::boolValue(true));
            break;
        case OpCode::False:
            pushToStack(value::boolValue(false));
            break;
        case OpCode::Not:
            pushToStack(value::boolValue(isFalsey(popFromStack())));
            break;

        case OpCode::Equal: {
            Value b = popFromStack();
            Value a = popFromStack();
            pushToStack(value::boolValue(valuesEqual(a, b)));
            break;
        }

        case OpCode::Greater:
            BINARY_OP(value::boolValue, >);
            break;
        case OpCode::Less:
            BINARY_OP(value::boolValue, <);
            break;
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
Value GlangVm::peekStack(int distance) {
    return stackTop_[-1 - distance];
}

void GlangVm::printStack() {
    fmt::print("Stack: [");
    for (auto slot = stack_; slot < stackTop_; ++slot) {
        fmt::print(" {} ", valueToString(*slot));
    }
    fmt::print("]\n");
}

void GlangVm::runtimeError(std::string_view msg) {

    fmt::print("{}\n", msg);
    size_t instruction = iPtr_ - code_.code_.data() - 1;
    auto line = code_.lineNumbers_[instruction];
    fmt::print("[line {}] in script\n", line);
}

void GlangVm::concatenate() {
    ObjString* b = object::asString(popFromStack());
    ObjString* a = object::asString(popFromStack());

    auto length = a->length + b->length;
    auto chars = memory::allocate<char>(length + 1);
    std::memcpy(chars, a->chars, a->length);
    std::memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString* result = takeString(chars, length);
    pushToStack(value::objValue(result));
}

#undef BINARY_OP
