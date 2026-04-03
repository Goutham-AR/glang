# Glang: C++ to Plain C Migration Plan

## Table of Contents

1. [Executive Summary](#1-executive-summary)
2. [Current Architecture Analysis](#2-current-architecture-analysis)
3. [C++ Features Inventory & C Replacements](#3-c-features-inventory--c-replacements)
4. [File-by-File Migration Strategy](#4-file-by-file-migration-strategy)
5. [Data Structure Transformations](#5-data-structure-transformations)
6. [Build System Migration](#6-build-system-migration)
7. [Dependency Replacements](#7-dependency-replacements)
8. [Phased Migration Approach](#8-phased-migration-approach)
9. [Testing Strategy](#9-testing-strategy)
10. [Risk Assessment & Mitigations](#10-risk-assessment--mitigations)

---

## 1. Executive Summary

This document defines a plan to migrate the **Glang** programming language interpreter from **C++17** to **plain C (C11)**. Glang is a dynamically-typed, bytecode-compiled, stack-based interpreter (~2,074 LOC across 28 source files) that follows a classic **Scanner → Parser → ByteCode → VM** pipeline.

The codebase already leans heavily toward a C-style design — it uses manual memory management (`malloc`/`realloc`/`free`), C-style casts, raw pointers, and struct-based data layouts. The primary C++ features to replace are:

- `std::vector` (dynamic arrays)
- `std::string` / `std::string_view` (string handling)
- `std::variant` (tagged union for `Value`)
- `std::unordered_map` (hash table)
- `std::optional` (nullable return values)
- `std::function` (function pointers for parse rules)
- `enum class` (scoped enumerations)
- Classes with constructors/destructors
- Namespaces
- Templates
- `fmt` library (formatted output)

**Estimated effort**: 3–5 days for an experienced C developer.

---

## 2. Current Architecture Analysis

### 2.1 Pipeline Overview

```
Source Code (.gln)
       │
       ▼
┌──────────────┐
│   Scanner    │  Tokenizes source into Token stream
│ Scanner.hh/cc│  Uses: std::string, std::string_view
└──────┬───────┘
       │
       ▼
┌──────────────┐
│   Parser     │  Pratt precedence parser + code generation
│ Parser.hh/cc │  Uses: std::function, std::string_view, references
└──────┬───────┘
       │
       ▼
┌──────────────┐
│  ByteCode    │  Stores opcodes + constant pool
│ByteCode.hh/cc│  Uses: std::vector<u8>, std::vector<Value>
└──────┬───────┘
       │
       ▼
┌──────────────┐
│   GlangVm   │  Stack-based bytecode interpreter
│  Vm.hh/cc   │  Uses: std::vector (via ByteCode), templates
└──────────────┘
```

### 2.2 Source File Inventory (28 files)

| File | Lines | Role | C++ Features Used |
|------|-------|------|-------------------|
| `common.hh` | 28 | Type aliases, debug macros, `Result` enum | `enum class`, `<cstdint>` |
| `log.hh` | 3 | Includes `fmt/format.h` | `fmt` library |
| `instructions.hh` | 35 | `OpCode` enum + conversion helpers | `enum class`, `static_cast` |
| `Value.hh` | 42 | `Value` struct with tagged union | `std::variant`, `std::string`, templates, `static` factory methods, `[[nodiscard]]` |
| `Value.cc` | 34 | `toString()`, `equal()` | `fmt::format`, `std::string` |
| `object.hh` | 77 | Object system (`Obj`, `ObjString`, `ObjFactory`) | Classes, templates, `inline`, namespaces, `std::string` |
| `object.cc` | 49 | Object allocation, string interning | `std::memcpy`, C-style casts |
| `Scanner.hh` | 98 | Lexer class + `Token` struct | `std::string`, `std::string_view`, class with private members |
| `Scanner.cc` | 247 | Lexer implementation | `std::string`, `memcmp` |
| `Parser.hh` | 122 | Parser class + `ParseRule` table | `std::function`, `std::string_view`, references, `enum class` |
| `Parser.cc` | 584 | Full parser + code generator | `std::string_view`, references, designated initializers |
| `ByteCode.hh` | 43 | Bytecode container class | `std::vector`, `[[nodiscard]]`, friend classes |
| `ByteCode.cc` | 35 | Bytecode read/write operations | `std::vector`, `assert` |
| `Vm.hh` | 54 | VM class | Templates (`runtimeError`), class |
| `Vm.cc` | 282 | VM execution loop | `std::memcpy`, `fmt::print`, macros |
| `compiler.hh` | 21 | `Compiler` struct, `compile()` function | `extern`, `std::string` |
| `compiler.cc` | 35 | Compilation entry point | `std::string` |
| `HashTable.hh` | 35 | Hash table wrapper | `std::unordered_map`, `std::optional`, struct with `operator()` |
| `HashTable.cc` | 50 | Hash table operations | `std::optional`, range-for, structured bindings |
| `memory.hh` | 28 | Memory allocation wrappers | Templates, namespace, `inline` |
| `memory.cc` | 2 | (Empty, just includes header) | — |
| `debug.hh` | 10 | Disassembly function declarations | Namespace |
| `debug.cc` | 102 | Bytecode disassembler | `fmt::print`, `std::string_view` |
| `repl.hh` | 24 | REPL (inline) | `std::string`, `std::cin`, `std::getline` |
| `utils.hh` | 25 | File I/O utility | `std::ifstream`, `std::string`, namespace |
| `main.cc` | 29 | Entry point | `std::exit`, `fmt::print` |

### 2.3 External Dependencies

| Dependency | Location | Purpose | C Replacement |
|-----------|----------|---------|---------------|
| **fmt** | `external/fmt/` | Formatted printing (`fmt::print`, `fmt::format`) | `printf`, `snprintf`, `fprintf` |

---

## 3. C++ Features Inventory & C Replacements

### 3.1 Standard Library Types

| C++ Feature | Used In | C Replacement |
|-------------|---------|---------------|
| `std::vector<uint8_t>` | `ByteCode` (code_, lineNumbers_) | Custom dynamic array struct (`DynArray_u8`) with `realloc` |
| `std::vector<Value>` | `ByteCode` (constants_) | Custom dynamic array struct (`DynArray_Value`) with `realloc` |
| `std::vector<int>` | `ByteCode` (lineNumbers_) | Custom dynamic array struct (`DynArray_int`) with `realloc` |
| `std::string` | `Scanner`, `Value`, `repl`, `utils`, `compiler` | `char*` with manual management, or `const char*` for immutable strings |
| `std::string_view` | `Token`, `Parser` | `const char*` + `int length` pair (already the pattern used internally) |
| `std::variant<bool, double, Obj*>` | `Value.as` | C tagged union: `union { bool boolean; double number; Obj* obj; }` |
| `std::unordered_map` | `HashTable` | Custom open-addressing hash table (array of `Entry` structs) |
| `std::optional<Value>` | `HashTable::get()` | Return `bool` success + output parameter, or sentinel `Value` |
| `std::function<void(Parser*, bool)>` | `ParseRule.prefix/infix` | Plain C function pointer: `typedef void (*ParseFn)(Parser*, bool)` |
| `std::ifstream` | `utils::readTextFile` | `fopen`/`fread`/`fclose` |
| `std::cin`/`std::getline` | `repl` | `fgets(buf, size, stdin)` |

### 3.2 Language Features

| C++ Feature | Used In | C Replacement |
|-------------|---------|---------------|
| `enum class` | `Result`, `OpCode`, `Precedence` | Plain `enum` (or `typedef enum { ... } Name;`) |
| `class` (with private) | `Scanner`, `Parser`, `ByteCode`, `GlangVm`, `HashTable`, `ObjFactory` | `struct` + functions taking `StructName*` as first parameter |
| Constructors / Destructors | All classes | `xxx_init()` / `xxx_free()` functions |
| Method calls (`obj.method()`) | Everywhere | `module_function(&obj, ...)` |
| References (`&`) | Function parameters | Pointers (`*`) |
| `static` factory methods | `Value::createBool()`, etc. | Standalone functions or macros: `value_create_bool()` |
| Templates | `memory::allocate<T>`, `Value::createObj<T>`, `GlangVm::runtimeError` | Macros or `void*` with explicit size, `va_list` for variadic |
| `[[nodiscard]]` | `ByteCode`, `Parser` | Remove (no equivalent in C11; optionally use compiler-specific `__attribute__((warn_unused_result))`) |
| `#pragma once` | All headers | Standard include guards: `#ifndef FILE_H` / `#define FILE_H` / `#endif` |
| Namespaces (`memory::`, `debug::`, `object::`, `utils::`) | Multiple files | Prefix-based naming: `memory_allocate()`, `debug_disassemble()`, `object_is_string()`, `utils_read_file()` |
| `friend` class | `ByteCode` | Direct struct field access (all fields public in C) |
| Operator overloading | `Precedence operator+` | Standalone function: `precedence_next(Precedence p)` |
| Designated initializers | Token creation | C11 designated initializers (`.field = value` syntax — already valid C11) |
| `inline` functions | `memory.hh`, `object.hh`, `repl.hh`, `utils.hh` | `static inline` in headers, or move to `.c` files |
| `auto` keyword | Throughout | Explicit types |

### 3.3 fmt Library Replacement Map

| C++ (`fmt`) | C Replacement |
|-------------|---------------|
| `fmt::print("{}", x)` | `printf("%s", x)` / appropriate format specifier |
| `fmt::print("{}\n", msg)` | `printf("%s\n", msg)` |
| `fmt::print("[line {}] Error", line)` | `printf("[line %d] Error", line)` |
| `fmt::print("{:03} {:04} ", line, offset)` | `printf("%03d %04d ", line, offset)` |
| `fmt::format("{}", value)` | `snprintf(buf, size, "%g", value)` |
| `fmt::print(" at '{}'", name)` | `printf(" at '%.*s'", length, start)` (for string_view) |

---

## 4. File-by-File Migration Strategy

Each `.hh` file becomes a `.h` file, and each `.cc` file becomes a `.c` file. Below is the transformation plan for every file.

### 4.1 `common.hh` → `common.h`

**Changes:**
- Replace `#pragma once` with include guards
- Replace `using u8 = std::uint8_t;` with `typedef uint8_t u8;` (and similarly for `u16`, `u32`, `size`)
- Replace `enum class Result { ... }` with `typedef enum { RESULT_OK, RESULT_COMPILE_ERROR, RESULT_RUNTIME_ERROR } Result;`
- Replace `#include <cstdint>` with `#include <stdint.h>`, `<cstddef>` → `<stddef.h>`, etc.
- Remove `#include "log.hh"` (no longer needed — `printf` is from `<stdio.h>`)

```c
#ifndef GLANG_COMMON_H
#define GLANG_COMMON_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef size_t   gsize;

/* #define DEBUG_PRINT_BYTECODE */
/* #define TRACE_VM_EXECUTION   */

typedef enum {
    RESULT_OK,
    RESULT_COMPILE_ERROR,
    RESULT_RUNTIME_ERROR
} Result;

#if defined(WIN32)
#define DEBUG_BREAK() __debugbreak()
#elif defined(__linux__)
#define DEBUG_BREAK() __asm__ __volatile__("int3")
#endif

#endif /* GLANG_COMMON_H */
```

### 4.2 `log.hh` → **Removed**

The `log.hh` file simply includes `<fmt/format.h>`. In C, all formatting is done via `printf`/`snprintf` from `<stdio.h>` (already included by `common.h`). This file is deleted.

### 4.3 `instructions.hh` → `instructions.h`

**Changes:**
- Replace `enum class OpCode : std::uint8_t` with `typedef enum { ... } OpCode;`
- Replace `static_cast<>` in `toU8`/`toOp` with C casts
- Rename `OpCode::Return` → `OP_RETURN`, etc. (C convention for enum constants)

```c
#ifndef GLANG_INSTRUCTIONS_H
#define GLANG_INSTRUCTIONS_H

#include "common.h"

typedef enum {
    OP_RETURN,
    OP_CONSTANT,
    OP_NEGATE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_NOT,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_PRINT,
    OP_POP,
    OP_DEFINE_GLOBAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_SET_LOCAL,
    OP_GET_LOCAL,
    OP_JMP_IF_FALSE,
    OP_JMP,
    OP_LOOP
} OpCode;

static inline u8 opcode_to_u8(OpCode code) { return (u8)code; }
static inline OpCode u8_to_opcode(u8 byte) { return (OpCode)byte; }

#endif /* GLANG_INSTRUCTIONS_H */
```

### 4.4 `memory.hh` / `memory.cc` → `memory.h` / `memory.c`

**Changes:**
- Remove namespace `memory`; prefix functions with `memory_`
- Replace template `allocate<T>` with a macro: `#define ALLOCATE(type, count) ((type*)memory_reallocate(NULL, 0, sizeof(type) * (count)))`
- Replace `inline` namespace functions with `static inline` or move to `.c`

```c
#ifndef GLANG_MEMORY_H
#define GLANG_MEMORY_H

#include "common.h"

void* memory_reallocate(void* ptr, gsize old_size, gsize new_size);

#define ALLOCATE(type, count) \
    ((type*)memory_reallocate(NULL, 0, sizeof(type) * (count)))

#define FREE(type, ptr) \
    memory_reallocate(ptr, sizeof(type), 0)

#define FREE_ARRAY(type, ptr, count) \
    memory_reallocate(ptr, sizeof(type) * (count), 0)

#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(type, ptr, old_count, new_count) \
    ((type*)memory_reallocate(ptr, sizeof(type) * (old_count), sizeof(type) * (new_count)))

#endif /* GLANG_MEMORY_H */
```

### 4.5 `Value.hh` / `Value.cc` → `value.h` / `value.c`

**Changes:**
- Replace `std::variant<bool, double, Obj*>` with a plain C `union`
- Replace `enum ValueType` (already not `enum class` — minimal change)
- Replace `static` factory methods with macros or functions
- Replace `std::string toString()` with `void value_print(Value value)` and `int value_to_string(Value value, char* buf, int buf_size)`
- Replace `std::get<T>()` calls with direct union access

```c
#ifndef GLANG_VALUE_H
#define GLANG_VALUE_H

#include "common.h"

typedef struct Obj Obj;
typedef struct ObjString ObjString;

typedef enum {
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    VAL_OBJ
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool boolean;
        double number;
        Obj* obj;
    } as;
} Value;

/* Value constructors */
#define BOOL_VAL(value)   ((Value){ VAL_BOOL,   { .boolean = (value) } })
#define NIL_VAL           ((Value){ VAL_NIL,    { .number = 0 } })
#define NUMBER_VAL(value) ((Value){ VAL_NUMBER, { .number = (value) } })
#define OBJ_VAL(object)   ((Value){ VAL_OBJ,    { .obj = (Obj*)(object) } })

/* Value accessors */
#define AS_BOOL(value)   ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_OBJ(value)    ((value).as.obj)

/* Value type checks */
#define IS_BOOL(value)   ((value).type == VAL_BOOL)
#define IS_NIL(value)    ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_OBJ(value)    ((value).type == VAL_OBJ)

void value_print(Value value);
bool values_equal(Value a, Value b);

/* Dynamic array of Values */
typedef struct {
    int capacity;
    int count;
    Value* values;
} ValueArray;

void value_array_init(ValueArray* array);
void value_array_write(ValueArray* array, Value value);
void value_array_free(ValueArray* array);

#endif /* GLANG_VALUE_H */
```

### 4.6 `object.hh` / `object.cc` → `object.h` / `object.c`

**Changes:**
- Remove `namespace object { }` — prefix functions: `object_is_string()`, `object_as_string()`, etc.
- Remove `class ObjFactory` — replace with standalone functions: `obj_copy_string()`, `obj_take_string()`
- Remove templates (`allocateObj<T>`) — use a macro `ALLOCATE_OBJ(type, objectType)`
- Remove `std::string` from `object::toString` — use `printf` directly or return `const char*`
- The `Obj` / `ObjString` structs are already very C-like

```c
#ifndef GLANG_OBJECT_H
#define GLANG_OBJECT_H

#include "common.h"
#include "value.h"

typedef enum {
    OBJ_STRING
} ObjType;

struct Obj {
    ObjType type;
};

struct ObjString {
    Obj obj;
    int length;
    char* chars;
    u32 hash;
};

/* Type checking and casting macros */
#define OBJ_TYPE(value)     (AS_OBJ(value)->type)
#define IS_STRING(value)    is_obj_type(value, OBJ_STRING)
#define AS_STRING(value)    ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)   (((ObjString*)AS_OBJ(value))->chars)

static inline bool is_obj_type(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

u32 hash_string(const char* key, int length);
void object_print(Value value);

ObjString* obj_copy_string(const char* chars, int length);
ObjString* obj_take_string(char* chars, int length);

void obj_free_objects(void); /* cleanup at shutdown */

#endif /* GLANG_OBJECT_H */
```

### 4.7 `HashTable.hh` / `HashTable.cc` → `table.h` / `table.c`

**Changes:**
- Replace `std::unordered_map<ObjString*, Value, Hasher>` with a hand-written open-addressing hash table (array of `Entry` structs with linear probing or FNV-1a rehashing)
- Replace `std::optional<Value>` return in `get()` with `bool table_get(Table* table, ObjString* key, Value* out_value)`
- Remove `Hasher` struct (`operator()` overload)
- Remove destructor — use `table_free()`
- Remove range-for and structured bindings — use index-based iteration

```c
#ifndef GLANG_TABLE_H
#define GLANG_TABLE_H

#include "common.h"
#include "value.h"

typedef struct {
    ObjString* key;
    Value value;
} Entry;

typedef struct {
    int count;
    int capacity;
    Entry* entries;
} Table;

void table_init(Table* table);
void table_free(Table* table);
bool table_set(Table* table, ObjString* key, Value value);
bool table_get(Table* table, ObjString* key, Value* value);
bool table_delete(Table* table, ObjString* key);
ObjString* table_find_string(Table* table, const char* chars,
                              int length, u32 hash);

#endif /* GLANG_TABLE_H */
```

**Implementation notes:**
- Use open-addressing with linear probing
- Use `NULL` key as empty sentinel and a special tombstone marker for deletions
- Grow when load factor exceeds 0.75
- This is the most significant implementation change in the migration

### 4.8 `Scanner.hh` / `Scanner.cc` → `scanner.h` / `scanner.c`

**Changes:**
- Replace `class Scanner` with `typedef struct { ... } Scanner;`
- Replace `std::string source_` with `const char* source_` (caller owns the string)
- Replace `std::string_view` in `Token` with `const char* start; int length;`
- Replace constructor with `scanner_init(Scanner*, const char*)`
- Replace method calls with `scanner_scan_token(Scanner*)`, etc.
- All private methods become `static` functions in `scanner.c`

```c
/* Token struct */
typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;
} Token;

/* Scanner struct */
typedef struct {
    const char* start;
    const char* current;
    int line;
} Scanner;

void scanner_init(Scanner* scanner, const char* source);
Token scanner_scan_token(Scanner* scanner);
```

### 4.9 `ByteCode.hh` / `ByteCode.cc` → `chunk.h` / `chunk.c`

**Changes:**
- Replace `std::vector<uint8_t> code_` with a custom dynamic array `{ u8* code; int count; int capacity; }`
- Replace `std::vector<Value> constants_` with `ValueArray constants;`
- Replace `std::vector<int> lineNumbers_` with `{ int* lines; int count; int capacity; }`
- Remove `friend` declarations — all fields are public in C structs
- Replace constructor/destructor with `chunk_init()` / `chunk_free()`

```c
#ifndef GLANG_CHUNK_H
#define GLANG_CHUNK_H

#include "common.h"
#include "value.h"

typedef struct {
    int count;
    int capacity;
    u8* code;
    int* lines;
    ValueArray constants;
} Chunk;

void chunk_init(Chunk* chunk);
void chunk_free(Chunk* chunk);
void chunk_write(Chunk* chunk, u8 byte, int line);
int  chunk_add_constant(Chunk* chunk, Value value);

#endif /* GLANG_CHUNK_H */
```

### 4.10 `Parser.hh` / `Parser.cc` → `compiler.h` / `compiler.c`

**Changes:**
- Merge `Parser` and `compiler` into a single compilation unit (as the parser is the compiler in this single-pass architecture)
- Replace `std::function<void(Parser*, bool)>` (`ParseFn`) with a plain C function pointer `typedef void (*ParseFn)(bool canAssign);`
- Replace `enum class Precedence` with `typedef enum { ... } Precedence;`
- Replace `class Parser` with a file-static global `Parser parser;` struct (since there's only ever one parser active)
- Replace method calls with `static` functions inside `compiler.c`
- Replace `std::string_view` parameters with `const char*` + length
- Replace references (`Scanner&`, `ByteCode&`) with pointers

```c
/* In compiler.h */
#ifndef GLANG_COMPILER_H
#define GLANG_COMPILER_H

#include "common.h"
#include "chunk.h"

bool compile(const char* source, Chunk* chunk);

#endif /* GLANG_COMPILER_H */
```

The parse rules table, `ParseFn`, `Precedence`, `Local`, `Compiler` struct, and all parse/emit functions become **static** within `compiler.c`.

### 4.11 `Vm.hh` / `Vm.cc` → `vm.h` / `vm.c`

**Changes:**
- Replace `class GlangVm` with `typedef struct { ... } VM;`
- Replace template `runtimeError<T...>()` with a variadic C function using `va_list` or a macro wrapping `vfprintf`
- Replace `ByteCode code_` member with `Chunk* chunk` pointer
- Replace `std::vector` access on `code_.code_.data()` with `chunk->code`
- Replace constructor with `vm_init(VM*)` and cleanup with `vm_free(VM*)`

```c
#ifndef GLANG_VM_H
#define GLANG_VM_H

#include "common.h"
#include "chunk.h"
#include "table.h"

#define STACK_MAX 256

typedef struct {
    Chunk* chunk;
    u8* ip;
    Value stack[STACK_MAX];
    Value* stack_top;
    Table globals;
    Table strings;
    Obj* objects; /* linked list for GC */
} VM;

void vm_init(VM* vm);
void vm_free(VM* vm);
Result vm_interpret(VM* vm, const char* source);

void vm_push(VM* vm, Value value);
Value vm_pop(VM* vm);

#endif /* GLANG_VM_H */
```

### 4.12 `debug.hh` / `debug.cc` → `debug.h` / `debug.c`

**Changes:**
- Remove `namespace debug { }` — prefix functions: `debug_disassemble_chunk()`, `debug_disassemble_instruction()`
- Replace `fmt::print` calls with `printf`
- Replace `std::string_view` parameter with `const char*`

```c
#ifndef GLANG_DEBUG_H
#define GLANG_DEBUG_H

#include "chunk.h"

void debug_disassemble_chunk(Chunk* chunk, const char* name);
int  debug_disassemble_instruction(Chunk* chunk, int offset);

#endif /* GLANG_DEBUG_H */
```

### 4.13 `repl.hh` → `repl.h` / `repl.c`

**Changes:**
- Replace `std::getline(std::cin, line)` with `fgets(line, sizeof(line), stdin)`
- Replace `std::string` with a fixed-size `char` buffer
- Replace `fmt::print` with `printf`
- Move from header-only to `.h` + `.c`

```c
/* repl.c */
#include <stdio.h>
#include "vm.h"

void repl(VM* vm) {
    char line[1024];
    for (;;) {
        printf(">>> ");
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }
        vm_interpret(vm, line);
    }
}
```

### 4.14 `utils.hh` → `utils.h` / `utils.c`

**Changes:**
- Replace `std::ifstream` with `fopen` / `fseek` / `ftell` / `fread` / `fclose`
- Replace `std::string` return with `char*` (caller must free)
- Remove namespace — prefix: `utils_read_file()`

```c
/* utils.c */
#include "common.h"

char* utils_read_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }
    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(file_size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }
    size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
    if (bytes_read < file_size) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }
    buffer[bytes_read] = '\0';

    fclose(file);
    return buffer;
}
```

### 4.15 `main.cc` → `main.c`

**Changes:**
- Replace `fmt::print` with `printf`/`fprintf`
- Replace `std::exit` with `exit`
- Pass VM instance through to `repl()` and `runFile()`

```c
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "vm.h"
#include "utils.h"
#include "repl.h"

static void run_file(VM* vm, const char* path) {
    char* source = utils_read_file(path);
    Result result = vm_interpret(vm, source);
    free(source);

    if (result == RESULT_COMPILE_ERROR) exit(65);
    if (result == RESULT_RUNTIME_ERROR) exit(70);
}

int main(int argc, char** argv) {
    VM vm;
    vm_init(&vm);

    if (argc == 1) {
        repl(&vm);
    } else if (argc == 2) {
        run_file(&vm, argv[1]);
    } else {
        fprintf(stderr, "Usage: glang [path]\n");
        vm_free(&vm);
        return 64;
    }

    vm_free(&vm);
    return 0;
}
```

---

## 5. Data Structure Transformations

### 5.1 Dynamic Array (replaces `std::vector`)

A reusable dynamic array pattern is needed for three types: `u8`, `int`, and `Value`. Since C lacks templates, use macros or repeat the pattern for each type.

**Recommended approach:** Define the dynamic array directly in the `Chunk` struct and in `ValueArray`, using `GROW_ARRAY` / `GROW_CAPACITY` macros from `memory.h`.

```c
/* Growth pattern (used in chunk_write, value_array_write, etc.) */
if (array->count + 1 > array->capacity) {
    int old_capacity = array->capacity;
    array->capacity = GROW_CAPACITY(old_capacity);
    array->data = GROW_ARRAY(Type, array->data, old_capacity, array->capacity);
}
array->data[array->count] = item;
array->count++;
```

### 5.2 Value Tagged Union (replaces `std::variant`)

```
C++:  std::variant<bool, double, Obj*> as;
      std::get<double>(as)

C:    union { bool boolean; double number; Obj* obj; } as;
      value.as.number
```

Constructor macros provide type safety through compound literals:
```c
#define NUMBER_VAL(value) ((Value){ VAL_NUMBER, { .number = (value) } })
```

### 5.3 Hash Table (replaces `std::unordered_map`)

This is the most complex transformation. Replace `std::unordered_map<ObjString*, Value, Hasher>` with a hand-written open-addressing hash table:

```c
typedef struct {
    ObjString* key;  /* NULL = empty slot */
    Value value;
} Entry;

typedef struct {
    int count;
    int capacity;
    Entry* entries;
} Table;
```

**Algorithm:** Open addressing with linear probing.
- Tombstones: On delete, set key to `NULL` and value to `BOOL_VAL(true)` as a sentinel.
- Load factor: Grow at 75% capacity.
- Lookup: Hash → index, probe linearly, check key pointer equality (interned strings allow `==`).

### 5.4 String Interning

The existing string interning mechanism uses `HashTable::findString()`. In C, this becomes `table_find_string()` using the new hash table, searching by content hash and `memcmp`. This remains structurally identical.

### 5.5 Parse Rules Table (replaces `std::function`)

```
C++:  using ParseFn = std::function<void(Parser*, bool)>;
      {&Parser::grouping, nullptr, Precedence::None}

C:    typedef void (*ParseFn)(bool canAssign);
      {grouping, NULL, PREC_NONE}
```

Since the parser is file-static in `compiler.c`, parse functions access parser state through a file-scope global variable rather than `this` pointer.

---

## 6. Build System Migration

### 6.1 CMakeLists.txt Changes

```cmake
cmake_minimum_required(VERSION 3.20)

project(
    glang
    DESCRIPTION "A Programming Language"
    LANGUAGES C
    VERSION 0.1.0
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

# prevent in-source build
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source build detected!")
endif()

# No more external/ subdirectory (fmt removed)
add_subdirectory(src)
```

### 6.2 src/CMakeLists.txt Changes

```cmake
set(SRC_FILES
    main.c
    chunk.c
    value.c
    vm.c
    compiler.c
    scanner.c
    object.c
    memory.c
    table.c
    debug.c
    utils.c
    repl.c
)

set(HEADER_FILES
    common.h
    chunk.h
    value.h
    vm.h
    compiler.h
    scanner.h
    object.h
    memory.h
    table.h
    debug.h
    utils.h
    repl.h
    instructions.h
)

add_executable(glang ${SRC_FILES} ${HEADER_FILES})

# No more fmt dependency
```

### 6.3 Removal of External Dependencies

- Delete `external/fmt/` directory entirely
- Delete `external/CMakeLists.txt`
- Remove `add_subdirectory(external)` from root `CMakeLists.txt`
- Remove `target_link_libraries(glang PRIVATE fmt)` from `src/CMakeLists.txt`

---

## 7. Dependency Replacements

| Current (C++) | Replacement (C) | Notes |
|--------------|-----------------|-------|
| `fmt` library (`fmt::print`, `fmt::format`) | `printf`, `fprintf`, `snprintf` from `<stdio.h>` | Format specifiers change: `{}` → `%d`, `%g`, `%s`, etc. |
| `<string>` (`std::string`) | `char*` + `malloc`/`free` or `char[]` buffers | Must handle memory manually |
| `<string_view>` | `const char*` + `int length` | Token already uses pointer+length internally |
| `<variant>` | `union` inside `Value` struct | Direct member access, no `std::get<>()` |
| `<optional>` | Bool return + output parameter | `bool table_get(Table*, ObjString*, Value*)` |
| `<functional>` (`std::function`) | Function pointers: `void (*fn)(bool)` | Much lower overhead |
| `<unordered_map>` | Hand-written hash table | Open addressing, ~100-150 LOC |
| `<vector>` | Dynamic arrays with `realloc` | Macro-based growth pattern |
| `<fstream>` | `fopen`/`fread`/`fclose` | Standard C file I/O |
| `<iostream>` (`std::cin`) | `fgets(buf, size, stdin)` | For REPL input |

---

## 8. Phased Migration Approach

### Phase 1: Foundation Layer (Day 1)
**Goal:** Establish base types and memory management in C.

| Step | Task | Files |
|------|------|-------|
| 1.1 | Create `common.h` with type definitions and includes | `common.h` |
| 1.2 | Create `memory.h`/`memory.c` with allocation macros and functions | `memory.h`, `memory.c` |
| 1.3 | Create `value.h`/`value.c` with `Value` tagged union and `ValueArray` | `value.h`, `value.c` |
| 1.4 | Create `instructions.h` with `OpCode` enum | `instructions.h` |
| 1.5 | Verify: compile foundation layer independently | — |

### Phase 2: Data Structures (Day 1–2)
**Goal:** Implement core data structures.

| Step | Task | Files |
|------|------|-------|
| 2.1 | Create `chunk.h`/`chunk.c` with dynamic bytecode container | `chunk.h`, `chunk.c` |
| 2.2 | Create `table.h`/`table.c` with open-addressing hash table | `table.h`, `table.c` |
| 2.3 | Create `object.h`/`object.c` with `Obj`, `ObjString`, string interning | `object.h`, `object.c` |
| 2.4 | Create `debug.h`/`debug.c` with disassembler | `debug.h`, `debug.c` |
| 2.5 | Verify: compile all data structures, test hash table and chunk operations | — |

### Phase 3: Frontend — Scanner (Day 2)
**Goal:** Port the lexer.

| Step | Task | Files |
|------|------|-------|
| 3.1 | Create `scanner.h`/`scanner.c` with `Token` struct and scanner functions | `scanner.h`, `scanner.c` |
| 3.2 | Verify: tokenize a sample `.gln` file and validate output | — |

### Phase 4: Frontend — Compiler/Parser (Day 2–3)
**Goal:** Port the parser and code generator.

| Step | Task | Files |
|------|------|-------|
| 4.1 | Create `compiler.h`/`compiler.c` merging `Parser` + `compiler` | `compiler.h`, `compiler.c` |
| 4.2 | Implement `ParseFn` function pointer table | `compiler.c` |
| 4.3 | Implement all parse functions (expression, statement, etc.) | `compiler.c` |
| 4.4 | Verify: compile a `.gln` file to bytecode and disassemble | — |

### Phase 5: Backend — Virtual Machine (Day 3–4)
**Goal:** Port the VM.

| Step | Task | Files |
|------|------|-------|
| 5.1 | Create `vm.h`/`vm.c` with VM struct and execution loop | `vm.h`, `vm.c` |
| 5.2 | Implement `vm_interpret()` with full opcode dispatch | `vm.c` |
| 5.3 | Implement string concatenation and runtime error reporting | `vm.c` |
| 5.4 | Verify: execute `testFiles/test.gln` and compare output | — |

### Phase 6: Integration & Utilities (Day 4)
**Goal:** Complete the executable.

| Step | Task | Files |
|------|------|-------|
| 6.1 | Create `utils.h`/`utils.c` for file I/O | `utils.h`, `utils.c` |
| 6.2 | Create `repl.h`/`repl.c` for REPL mode | `repl.h`, `repl.c` |
| 6.3 | Create `main.c` with entry point | `main.c` |
| 6.4 | Verify: run REPL and file execution modes | — |

### Phase 7: Build System & Cleanup (Day 4–5)
**Goal:** Finalize the migration.

| Step | Task | Files |
|------|------|-------|
| 7.1 | Update `CMakeLists.txt` (root and src) | `CMakeLists.txt`, `src/CMakeLists.txt` |
| 7.2 | Remove `external/fmt/` directory | — |
| 7.3 | Remove all `.hh` and `.cc` files | — |
| 7.4 | Update `.clang-format` for C style | `.clang-format` |
| 7.5 | Update `README.md` for new build instructions | `README.md` |
| 7.6 | Full regression test against `testFiles/test.gln` | — |

---

## 9. Testing Strategy

### 9.1 Verification Approach

Since there are no automated tests in the repository, verification is done by comparing runtime behavior before and after migration.

| Test | Method | Expected |
|------|--------|----------|
| **Build** | `cmake .. && make` succeeds without errors | Clean build, no warnings with `-Wall -Wextra` |
| **Nested loop test** | Run `./bin/glang testFiles/test.gln` | Same output as C++ version (stars pattern) |
| **REPL basic** | Enter `print 1 + 2;` in REPL | Output: `3` |
| **Variables** | `def x = 10; print x;` | Output: `10` |
| **Strings** | `def s = "hello"; print s;` | Output: `hello` |
| **String concat** | `print "hello " + "world";` | Output: `hello world` |
| **Control flow** | `if (true) { print 1; } else { print 2; }` | Output: `1` |
| **While loop** | `def i = 0; while (i < 3) { print i; i = i + 1; }` | Output: `0`, `1`, `2` |
| **Boolean logic** | `print !false;` | Output: `True` |
| **Comparison** | `print 5 > 3;` | Output: `True` |
| **Error: undefined var** | `print x;` | Runtime error message |
| **Error: type mismatch** | `print 1 + "a";` | Runtime error message |

### 9.2 Recommended Post-Migration Test Suite

After migration, add a `tests/` directory with shell-based regression tests:

```bash
#!/bin/bash
# tests/run_tests.sh

GLANG=./bin/glang
PASS=0
FAIL=0

run_test() {
    local name="$1"
    local input="$2"
    local expected="$3"
    local actual=$(echo "$input" | $GLANG 2>&1)

    if [ "$actual" = "$expected" ]; then
        echo "PASS: $name"
        ((PASS++))
    else
        echo "FAIL: $name"
        echo "  expected: $expected"
        echo "  actual:   $actual"
        ((FAIL++))
    fi
}

run_test "arithmetic"    "print 1 + 2;"        "3"
run_test "string"        'print "hello";'       "hello"
run_test "boolean"       "print !false;"        "True"
run_test "nil"           "print nil;"           "Nil"
run_test "comparison"    "print 5 > 3;"         "True"

echo "Results: $PASS passed, $FAIL failed"
```

---

## 10. Risk Assessment & Mitigations

### 10.1 Risks

| Risk | Severity | Likelihood | Mitigation |
|------|----------|------------|------------|
| **Memory leaks** (no RAII) | High | High | Use Valgrind/ASan; add explicit `_free()` calls for every `_init()` |
| **Hash table bugs** (replacing `std::unordered_map`) | High | Medium | Port from a well-tested reference implementation (e.g., crafting interpreters book) |
| **String handling errors** (buffer overflows) | High | Medium | Use `snprintf` consistently; validate lengths; test edge cases |
| **Build system issues** | Low | Low | Incremental CMake changes; test at each phase |
| **Behavior differences** | Medium | Low | Run all test cases and compare output character-by-character |
| **Missing `\0` terminators** | Medium | Medium | Audit every string allocation; use ASan |

### 10.2 Tools for Verification

| Tool | Purpose |
|------|---------|
| `gcc -Wall -Wextra -Werror -pedantic` | Catch all warnings during compilation |
| `valgrind --leak-check=full` | Detect memory leaks |
| `-fsanitize=address` | Detect buffer overflows and use-after-free |
| `-fsanitize=undefined` | Detect undefined behavior |
| `diff` | Compare output of C++ vs C version |

### 10.3 Rollback Strategy

Maintain the C++ codebase on a separate branch (`main` or `cpp-original`) until the C migration is fully validated. The migration should be done on a dedicated branch (e.g., `c-migration`), and only merged once all tests pass.

---

## Appendix: Complete File Mapping

| C++ Source | C Source | Notes |
|-----------|----------|-------|
| `common.hh` | `common.h` | Type aliases, includes, `Result` enum |
| `log.hh` | *(deleted)* | Replaced by `<stdio.h>` |
| `instructions.hh` | `instructions.h` | `OpCode` enum with `OP_` prefix |
| `memory.hh` + `memory.cc` | `memory.h` + `memory.c` | Allocation macros and functions |
| `Value.hh` + `Value.cc` | `value.h` + `value.c` | Tagged union + `ValueArray` |
| `object.hh` + `object.cc` | `object.h` + `object.c` | `Obj`, `ObjString`, string interning |
| `HashTable.hh` + `HashTable.cc` | `table.h` + `table.c` | Open-addressing hash table |
| `ByteCode.hh` + `ByteCode.cc` | `chunk.h` + `chunk.c` | Bytecode container |
| `Scanner.hh` + `Scanner.cc` | `scanner.h` + `scanner.c` | Lexer |
| `Parser.hh` + `Parser.cc` + `compiler.hh` + `compiler.cc` | `compiler.h` + `compiler.c` | Merged parser + compiler |
| `Vm.hh` + `Vm.cc` | `vm.h` + `vm.c` | Virtual machine |
| `debug.hh` + `debug.cc` | `debug.h` + `debug.c` | Disassembler |
| `repl.hh` | `repl.h` + `repl.c` | REPL loop |
| `utils.hh` | `utils.h` + `utils.c` | File I/O |
| `main.cc` | `main.c` | Entry point |
| **Total: 28 C++ files** | **Total: 25 C files** | 3 files merged/removed |
