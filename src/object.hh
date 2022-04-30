#pragma once

#include "common.hh"
#include "Value.hh"

enum ObjType {
    OBJ_STRING
};

struct Obj {
    ObjType type;
};

struct ObjString {
    Obj obj;
    int length;
    char* chars;
};

static inline bool isObjType(Value value, ObjType type) {
    return value::isObj(value) && value::asObj(value)->type == type;
}

namespace object {
inline ObjType objType(Value value) { return value::asObj(value)->type; }
inline bool isString(Value value) { return isObjType(value, OBJ_STRING); }
inline ObjString* asString(Value value) { return (ObjString*)value::asObj(value); }
inline char* asCString(Value value) { return ((ObjString*)value::asObj(value))->chars; }

}

ObjString* copyString(const char* chars, int length);
ObjString* takeString(char* chars, int length);
std::string objectToString(Value value);
