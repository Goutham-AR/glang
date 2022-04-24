#pragma once

#include "common.hh"

#include <string>

enum ValueType {
    ValBool,
    ValNil,
    ValNumber
};

struct Value {
    ValueType type;
    union {
        bool boolean;
        double number;
    } as;
};

void printValue(Value value);
std::string valueToString(Value value);

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)

#define IS_BOOL(value) ((value).type == ValBool)
#define IS_NIL(value) ((value).type == ValNil)
#define IS_NUMBER(value) ((value).type == ValNumber)

#define BOOL_VAL(value) ((Value){ValBool, {.boolean = (value)}})
#define NIL_VAL() ((Value){ValNil, {.number = 0}})
#define NUMBER_VAL(value) ((Value){ValNumber, {.number = (value)}})