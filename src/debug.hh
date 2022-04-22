#pragma once

#include "common.hh"

class ByteCode;

namespace debug {
void disassembleByteCode(const ByteCode& code);
int disassembleInstruction(const ByteCode& code, int offset);
}