#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cassert>

#include "log.hh"

using u8 = std::uint8_t;
using u32 = std::uint32_t;
using size = std::size_t;

#define DEBUG_PRINT_BYTECODE
#define TRACE_VM_EXECUTION

enum class Result {
    Ok,
    CompileError,
    RuntimeError
};

#if defined(WIN32)
#define DEBUG_BREAK() __debugbreak()
#elif defined(__linux__)
#define DEBUG_BREAK() __asm__ __volatile__("int3")
#endif