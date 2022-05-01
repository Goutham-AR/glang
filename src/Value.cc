#include "Value.hh"
#include "object.hh"

std::string Value::toString() {
    switch (type) {
    case ValNumber:
        return fmt::format("{}", asNumber());
    case ValBool:
        return fmt::format("{}", asBool() ? "True" : "False");
    case ValNil:
        return fmt::format("Nil");
    case ValObj:
        return object::toString(*this);
        break;
    }
}

bool Value::equal(Value a, Value b) {
    if (a.type != b.type) return false;

    switch (a.type) {
    case ValBool:
        return a.asBool() == b.asBool();
    case ValNil:
        return true;
    case ValNumber:
        return a.asNumber() == b.asNumber();
    case ValObj:
        return a.asObj() == b.asObj();

    default:
        return false;
    }
}