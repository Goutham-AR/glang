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

namespace value {
inline bool asBool(Value value) { return value.as.boolean; }
inline double asNumber(Value value) { return value.as.number; }
inline Obj* asObj(Value value) { return value.as.obj; }

inline bool isBool(Value value) { return value.type == ValBool; }
inline bool isNil(Value value) { return value.type == ValNil; }
inline bool isNumber(Value value) { return value.type == ValNumber; }
inline bool isObj(Value value) { return value.type == ValObj; }

inline Value boolValue(bool value) { return Value{.type = ValBool, .as{.boolean = value}}; }
inline Value nilValue() { return Value{.type = ValNil, .as{.number = 0}}; }
inline Value numberValue(double value) { return Value{.type = ValNumber, .as{.number = value}}; }

template <typename T>
inline Value objValue(T* object) { return Value{.type = ValObj, .as{.obj = (Obj*)object}}; }

}
