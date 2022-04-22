#include "Value.hh"

void printValue(Value value) {
    fmt::print(valueToString(value));
}

std::string valueToString(Value value) {
    return fmt::format("{}", value);
}