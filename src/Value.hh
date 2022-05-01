#pragma once

#include "common.hh"

#include <string>
#include <variant>

struct Obj;
struct ObjString;

enum ValueType {
    ValBool,
    ValNil,
    ValNumber,
    ValObj
};

struct Value {

    double asNumber() { return std::get<double>(as); }
    bool asBool() { return std::get<bool>(as); }
    Obj* asObj() { return std::get<Obj*>(as); }

    [[nodiscard]] bool isBool() const { return type == ValBool; }
    [[nodiscard]] bool isNil() const { return type == ValNil; }
    [[nodiscard]] bool isNumber() const { return type == ValNumber; }
    [[nodiscard]] bool isObj() const { return type == ValObj; }

    static Value createBool(bool value) { return Value{.type = ValBool, .as{value}}; }
    static Value createNil() { return Value{.type = ValNil, .as{0.0}}; }
    static Value createNumber(double value) { return Value{.type = ValNumber, .as{value}}; }

    template <typename T>
    static Value createObj(T* object) { return Value{.type = ValObj, .as{(Obj*)object}}; }

    std::string toString();
    void print() { fmt::print(toString()); }
    static bool equal(Value a, Value b);

    ValueType type;
    std::variant<bool, double, Obj*> as;
};
