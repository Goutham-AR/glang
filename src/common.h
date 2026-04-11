#ifndef SOLV_COMMON_H
#define SOLV_COMMON_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;

/* #define DEBUG_PRINT_BYTECODE */
/* #define TRACE_VM_EXECUTION   */

typedef enum {
    RESULT_OK,
    RESULT_COMPILE_ERROR,
    RESULT_RUNTIME_ERROR
} Result;

/* DEBUG_BREAK is intentionally left empty (matches original behaviour) */
#define DEBUG_BREAK()

#endif /* SOLV_COMMON_H */
