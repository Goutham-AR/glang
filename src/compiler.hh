#pragma once

#include "common.hh"
#include "Scanner.hh"

class ByteCode;

bool compile(const std::string& code, ByteCode& byteCode);
