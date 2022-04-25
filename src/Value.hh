#pragma once

#include "common.hh"

#include <string>

struct Obj;
struct ObjString;

enum ValueType {
    ValBool,
    ValNil,
    ValNumber,
    ValObj
};

struct Value {
    ValueType type;
    union {
        bool boolean;
        double number;
        Obj* obj;
    } as;
};

void printValue(Value value);
std::string valueToString(Value value);
bool valuesEqual(Value a, Value b);

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_OBJ(value) ((value).as.obj)

#define IS_BOOL(value) ((value).type == ValBool)
#define IS_NIL(value) ((value).type == ValNil)
#define IS_NUMBER(value) ((value).type == ValNumber)
#define IS_OBJ(value) ((value).type == ValObj)

#define BOOL_VAL(value) ((Value){ValBool, {.boolean = (value)}})
#define NIL_VAL() ((Value){ValNil, {.number = 0}})
#define NUMBER_VAL(value) ((Value){ValNumber, {.number = (value)}})
#define OBJ_VAL(object) ((Value){ValObj, {.obj = (Obj*)(object)}})