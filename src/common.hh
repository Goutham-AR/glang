#pragma once

#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "log.hh"

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using size = size_t;

// #define DEBUG_PRINT_BYTECODE
// #define TRACE_VM_EXECUTION

enum Result {
    Result_Ok,
    Result_Compile_Error,
    Result_Runtime_Error
};

#if defined(WIN32)
#define DEBUG_BREAK() __debugbreak()
#elif defined(__linux__)
#define DEBUG_BREAK() __asm__ __volatile__("int3")
#elif defined(__MACH__)
#define DEBUG_BREAK()
#endif
