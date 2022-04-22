#include "log.hh"
#include "ByteCode.hh"
#include "debug.hh"

int main(int argv, char** argc) {
    ByteCode code;

    code.writeOpCode(OpCode::Return, 1);
    code.writeConstantInstr(23.4, 2);
    code.writeConstantInstr(3, 12);

    debug::disassembleByteCode(code);

    return 0;
}