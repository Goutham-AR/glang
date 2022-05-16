#pragma once
#include "common.hh"

enum class OpCode : std::uint8_t {
    //           format
    Return,   // one byte
    Constant, // two bytes: Constant, offset into the constant pool
    Negate,
    Add,
    Subtract,
    Multiply,
    Divide,
    Nil,
    True,
    False,
    Not,
    Equal,
    Greater,
    Less,
    //
    Print,
    Pop,
    DefineGlobal,
    GetGlobal,
    SetGlobal
};

inline std::uint8_t toU8(OpCode code) { return static_cast<std::uint8_t>(code); }
inline OpCode toOp(std::uint8_t byte) { return static_cast<OpCode>(byte); }
