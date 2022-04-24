#include "Value.hh"

void printValue(Value value) {
    fmt::print(valueToString(value));
}

std::string valueToString(Value value) {

    switch (value.type) {
    case ValNumber:
        return fmt::format("{}", AS_NUMBER(value));
    case ValBool:
        return fmt::format("{}", AS_BOOL(value) ? "True" : "False");
    case ValNil:
        return fmt::format("Nil");
    }

    // return fmt::format("{}", value);
}