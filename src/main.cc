#include "log.hh"
#include "ByteCode.hh"
#include "debug.hh"
#include "Vm.hh"

int main(int argv, char** argc) {
    ByteCode code;

    code.writeConstantInstr(23.4, 2);
    code.writeConstantInstr(3, 12);
    code.writeOpCode(OpCode::Multiply, 13);
    code.writeOpCode(OpCode::Negate, 12);
    code.writeOpCode(OpCode::Return, 1);

    GlangVm vMachine{code};
    vMachine.interpret();

    // debug::disassembleByteCode(code);

    return 0;
}