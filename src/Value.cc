#include "Value.hh"
#include "object.hh"

void printValue(Value value) {
    fmt::print(valueToString(value));
}

bool valuesEqual(Value a, Value b) {
    if (a.type != b.type) return false;

    switch (a.type) {
    case ValBool:
        return value::asBool(a) == value::asBool(b);
    case ValNil:
        return true;
    case ValNumber:
        return value::asNumber(a) == value::asNumber(b);
    case ValObj: {
        ObjString* aString = object::asString(a);
        ObjString* bString = object::asString(b);
        return aString->length == bString->length && std::memcmp(aString->chars, bString->chars, aString->length) == 0;
    }

    default:
        return false;
    }
}

std::string valueToString(Value value) {

    switch (value.type) {
    case ValNumber:
        return fmt::format("{}", value::asNumber(value));
    case ValBool:
        return fmt::format("{}", value::asBool(value) ? "True" : "False");
    case ValNil:
        return fmt::format("Nil");
    case ValObj:
        return objectToString(value);
        break;
    }

    // return fmt::format("{}", value);
}