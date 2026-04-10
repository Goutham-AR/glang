#include "vm.h"
#include "compiler.h"
#include "debug.h"
#include "object.h"
#include "memory.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static VM vm;

static void vm_reset_stack(void) { vm.stack_top = vm.stack; }

static void runtime_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
    size_t instruction = (size_t)(vm.ip - vm.bytecode->code - 1);
    int    line        = vm.bytecode->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
}

static void   vm_push(Value value) { *vm.stack_top++ = value; }
static Value  vm_pop(void)         { return *--vm.stack_top;  }
static Value  vm_peek(int distance){ return vm.stack_top[-1 - distance]; }

static bool is_falsey(Value value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate(void) {
    ObjString* b      = AS_STRING(vm_pop());
    ObjString* a      = AS_STRING(vm_pop());
    int        length = a->length + b->length;
    char*      chars  = ALLOCATE(char, length + 1);
    memcpy(chars,             a->chars, (size_t)a->length);
    memcpy(chars + a->length, b->chars, (size_t)b->length);
    chars[length] = '\0';
    vm_push(OBJ_VAL(take_string(chars, length)));
}

#define READ_BYTE()     (*vm.ip++)
#define READ_SHORT()    (vm.ip += 2, (u16)((vm.ip[-2] << 8) | vm.ip[-1]))
#define READ_CONSTANT() (vm.bytecode->constants.values[READ_BYTE()])
#define READ_STRING()   AS_STRING(READ_CONSTANT())

#define BINARY_OP(value_macro, op)                                      \
    do {                                                                \
        if (!IS_NUMBER(vm_peek(0)) || !IS_NUMBER(vm_peek(1))) {         \
            runtime_error("Operands must be numbers");                  \
            return RESULT_RUNTIME_ERROR;                                \
        }                                                               \
        double b = AS_NUMBER(vm_pop());                                 \
        double a = AS_NUMBER(vm_pop());                                 \
        vm_push(value_macro(a op b));                                   \
    } while (false)

static Result vm_run(void) {
#ifdef TRACE_VM_EXECUTION
    printf("==== Tracing execution ====\n");
#endif
    while (true) {
#ifdef TRACE_VM_EXECUTION
        printf("Stack: [");
        for (Value* slot = vm.stack; slot < vm.stack_top; slot++) {
            printf(" "); value_print(*slot); printf(" ");
        }
        printf("]\n");
        debug_disassemble_instruction(vm.bytecode, (int)(vm.ip - vm.bytecode->code));
#endif
        u8 instruction = READ_BYTE();
        switch ((OpCode)instruction) {
        case OP_RETURN: return RESULT_OK;

        case OP_CONSTANT: vm_push(READ_CONSTANT()); break;

        case OP_NEGATE:
            if (!IS_NUMBER(vm_peek(0))) {
                runtime_error("Operand must be a number");
                return RESULT_RUNTIME_ERROR;
            }
            vm_push(NUMBER_VAL(-AS_NUMBER(vm_pop())));
            break;

        case OP_ADD:
            if      (IS_STRING(vm_peek(0)) && IS_STRING(vm_peek(1))) { concatenate(); }
            else if (IS_NUMBER(vm_peek(0)) && IS_NUMBER(vm_peek(1))) {
                double b = AS_NUMBER(vm_pop());
                double a = AS_NUMBER(vm_pop());
                vm_push(NUMBER_VAL(a + b));
            } else {
                runtime_error("Operands must be two numbers or two strings");
                return RESULT_RUNTIME_ERROR;
            }
            break;

        case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
        case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
        case OP_DIVIDE:   BINARY_OP(NUMBER_VAL, /); break;

        case OP_NIL:   vm_push(NIL_VAL);          break;
        case OP_TRUE:  vm_push(BOOL_VAL(true));   break;
        case OP_FALSE: vm_push(BOOL_VAL(false));  break;

        case OP_NOT: vm_push(BOOL_VAL(is_falsey(vm_pop()))); break;

        case OP_EQUAL: {
            Value b = vm_pop(), a = vm_pop();
            vm_push(BOOL_VAL(values_equal(a, b)));
            break;
        }

        case OP_GREATER: BINARY_OP(BOOL_VAL, >); break;
        case OP_LESS:    BINARY_OP(BOOL_VAL, <); break;

        case OP_PRINT:
            value_print(vm_pop());
            printf("\n");
            break;

        case OP_POP: vm_pop(); break;

        case OP_DEFINE_GLOBAL: {
            ObjString* name = READ_STRING();
            table_set(&vm.globals, name, vm_peek(0));
            vm_pop();
            break;
        }

        case OP_GET_GLOBAL: {
            ObjString* name = READ_STRING();
            Value      val;
            if (!table_get(&vm.globals, name, &val)) {
                runtime_error("Undefined variable '%s'.", name->chars);
                return RESULT_RUNTIME_ERROR;
            }
            vm_push(val);
            break;
        }

        case OP_SET_GLOBAL: {
            ObjString* name = READ_STRING();
            if (table_set(&vm.globals, name, vm_peek(0))) {
                /* table_set returned true → new key → variable was not declared */
                table_delete(&vm.globals, name);
                runtime_error("Undefined variable '%s'.", name->chars);
                return RESULT_RUNTIME_ERROR;
            }
            break;
        }

        case OP_GET_LOCAL: { u8 slot = READ_BYTE(); vm_push(vm.stack[slot]); break; }
        case OP_SET_LOCAL: { u8 slot = READ_BYTE(); vm.stack[slot] = vm_peek(0); break; }

        case OP_JMP_IF_FALSE: {
            u16 offset = READ_SHORT();
            if (is_falsey(vm_peek(0))) vm.ip += offset;
            break;
        }
        case OP_JMP: { u16 offset = READ_SHORT(); vm.ip += offset; break; }
        case OP_LOOP:{ u16 offset = READ_SHORT(); vm.ip -= offset; break; }
        }
    }
}

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP

Result interpret(const char* source) {
    ByteCode bytecode;
    bytecode_init(&bytecode);

    if (!compile(source, &bytecode)) {
        bytecode_free(&bytecode);
        return RESULT_COMPILE_ERROR;
    }

    vm.bytecode = &bytecode;
    vm.ip       = bytecode.code;
    vm_reset_stack();
    table_init(&vm.globals);

    Result result = vm_run();

    table_free(&vm.globals);
    bytecode_free(&bytecode);
    return result;
}
