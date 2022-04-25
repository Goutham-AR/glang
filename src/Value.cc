#include "Value.hh"
#include "object.hh"

void printValue(Value value) {
    fmt::print(valueToString(value));
}

bool valuesEqual(Value a, Value b) {
    if (a.type != b.type) return false;

    switch (a.type) {
    case ValBool:
        return AS_BOOL(a) == AS_BOOL(b);
    case ValNil:
        return true;
    case ValNumber:
        return AS_NUMBER(a) == AS_NUMBER(b);
    case ValObj: {
        ObjString* aString = AS_STRING(a);
        ObjString* bString = AS_STRING(b);
        return aString->length == bString->length && std::memcmp(aString->chars, bString->chars, aString->length) == 0;
    }

    default:
        return false;
    }
}

std::string valueToString(Value value) {

    switch (value.type) {
    case ValNumber:
        return fmt::format("{}", AS_NUMBER(value));
    case ValBool:
        return fmt::format("{}", AS_BOOL(value) ? "True" : "False");
    case ValNil:
        return fmt::format("Nil");
    case ValObj:
        return objectToString(value);
        break;
    }

    // return fmt::format("{}", value);
}