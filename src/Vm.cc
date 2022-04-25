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
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

#define BINARY_OP(valueType, op)                                    \
    do {                                                            \
        if (!IS_NUMBER(peekStack(0)) || !IS_NUMBER(peekStack(1))) { \
            runtimeError("Operands must be numbers");               \
            return Result::RuntimeError;                            \
        }                                                           \
        double op2 = AS_NUMBER(popFromStack());                     \
        double op1 = AS_NUMBER(popFromStack());                     \
        pushToStack(valueType(op1 op op2));                         \
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
            if (!IS_NUMBER(peekStack(0))) {
                runtimeError("Operand must be a number");
                return Result::RuntimeError;
            }

            pushToStack(NUMBER_VAL(-AS_NUMBER(popFromStack())));
            break;
        }

        case OpCode::Add: {
            if (IS_STRING(peekStack(0)) && IS_STRING(peekStack(1))) {
                concatenate();
            } else if (IS_NUMBER(peekStack(0)) && IS_NUMBER(peekStack(1))) {
                double b = AS_NUMBER(popFromStack());
                double a = AS_NUMBER(popFromStack());
                pushToStack(NUMBER_VAL(a + b));
            } else {
                runtimeError("Operands must be two numbers or two strings");
                return Result::RuntimeError;
            }
            break;
        }
        case OpCode::Subtract: {
            BINARY_OP(NUMBER_VAL, -);
            break;
        }

        case OpCode::Multiply: {
            BINARY_OP(NUMBER_VAL, *);
            break;
        }

        case OpCode::Divide: {
            BINARY_OP(NUMBER_VAL, /);
            break;
        }

        case OpCode::Nil:
            pushToStack(NIL_VAL());
            break;
        case OpCode::True:
            pushToStack(BOOL_VAL(true));
            break;
        case OpCode::False:
            pushToStack(BOOL_VAL(false));
            break;
        case OpCode::Not:
            pushToStack(BOOL_VAL(isFalsey(popFromStack())));
            break;

        case OpCode::Equal: {
            Value b = popFromStack();
            Value a = popFromStack();
            pushToStack(BOOL_VAL(valuesEqual(a, b)));
            break;
        }

        case OpCode::Greater:
            BINARY_OP(BOOL_VAL, >);
            break;
        case OpCode::Less:
            BINARY_OP(BOOL_VAL, <);
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
    ObjString* b = AS_STRING(popFromStack());
    ObjString* a = AS_STRING(popFromStack());

    auto length = a->length + b->length;
    char* chars = ALLOCATE(char, length + 1);
    std::memcpy(chars, a->chars, a->length);
    std::memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString* result = takeString(chars, length);
    pushToStack(OBJ_VAL(result));
}

#undef BINARY_OP
