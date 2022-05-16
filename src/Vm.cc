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
    return value.isNil() || (value.isBool() && !value.asBool());
}

#define BINARY_OP(valueType, op)                                    \
    do {                                                            \
        if (!peekStack(0).isNumber() || !peekStack(1).isNumber()) { \
            runtimeError("Operands must be numbers");               \
            return Result::RuntimeError;                            \
        }                                                           \
        double op2 = popFromStack().asNumber();                     \
        double op1 = popFromStack().asNumber();                     \
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
            return Result::Ok;
        }

        case OpCode::Constant: {
            auto constant = readConstant();
            pushToStack(constant);
            break;
        }

        case OpCode::Negate: {
            if (peekStack(0).isNumber()) {
                runtimeError("Operand must be a number");
                return Result::RuntimeError;
            }

            pushToStack(Value::createNumber(-(popFromStack().asNumber())));
            break;
        }

        case OpCode::Add: {
            if (object::isString(peekStack(0)) && object::isString(peekStack(1))) {
                concatenate();
            } else if (peekStack(0).isNumber() && peekStack(1).isNumber()) {
                double b = popFromStack().asNumber();
                double a = popFromStack().asNumber();
                pushToStack(Value::createNumber(a + b));
            } else {
                runtimeError("Operands must be two numbers or two strings");
                return Result::RuntimeError;
            }
            break;
        }
        case OpCode::Subtract: {
            BINARY_OP(Value::createNumber, -);
            break;
        }

        case OpCode::Multiply: {
            BINARY_OP(Value::createNumber, *);
            break;
        }

        case OpCode::Divide: {
            BINARY_OP(Value::createNumber, /);
            break;
        }

        case OpCode::Nil:
            pushToStack(Value::createNil());
            break;
        case OpCode::True:
            pushToStack(Value::createBool(true));
            break;
        case OpCode::False:
            pushToStack(Value::createBool(false));
            break;
        case OpCode::Not:
            pushToStack(Value::createBool(isFalsey(popFromStack())));
            break;

        case OpCode::Equal: {
            Value b = popFromStack();
            Value a = popFromStack();
            pushToStack(Value::createBool(Value::equal(a, b)));
            break;
        }

        case OpCode::Greater:
            BINARY_OP(Value::createBool, >);
            break;
        case OpCode::Less:
            BINARY_OP(Value::createBool, <);
            break;

        case OpCode::Print:
            popFromStack().print();
            fmt::print("\n");
            break;

        case OpCode::Pop:
            popFromStack();
            break;

        case OpCode::DefineGlobal: {
            ObjString* name = object::asString(readConstant());
            globals_.set(name, peekStack(0));
            popFromStack();
            break;
        }

        case OpCode::GetGlobal: {
            ObjString* name = object::asString(readConstant());

            auto val = globals_.get(name);
            if (!val.has_value()) {
                runtimeError("Undefined variable {}.", name->chars);
                return Result::RuntimeError;
            }

            pushToStack(val.value());
            break;
        }

        case OpCode::SetGlobal: {
            ObjString* name = object::asString(readConstant());
            if (globals_.set(name, peekStack(0))) {
                globals_.deleteEntry(name);
                runtimeError("Undefined Variable {}.", name->chars);
                return Result::RuntimeError;
            }
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
Value GlangVm::peekStack(int distance) {
    return stackTop_[-1 - distance];
}

void GlangVm::printStack() {
    fmt::print("Stack: [");
    for (auto slot = stack_; slot < stackTop_; ++slot) {
        fmt::print(" {} ", slot->toString());
    }
    fmt::print("]\n");
}

// void GlangVm::runtimeError(std::string_view msg) {

//     fmt::print("{}\n", msg);
//     size_t instruction = iPtr_ - code_.code_.data() - 1;
//     auto line = code_.lineNumbers_[instruction];
//     fmt::print("[line {}] in script\n", line);
// }

void GlangVm::concatenate() {
    ObjString* b = object::asString(popFromStack());
    ObjString* a = object::asString(popFromStack());

    auto length = a->length + b->length;
    auto chars = memory::allocate<char>(length + 1);
    std::memcpy(chars, a->chars, a->length);
    std::memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString* result = ObjFactory::takeString(chars, length);
    pushToStack(Value::createObj(result));
}

#undef BINARY_OP
