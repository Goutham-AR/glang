#pragma once

#include "common.hh"
#include "Value.hh"
#include "memory.hh"
#include "HashTable.hh"

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
    u32 hash;
};

inline bool isObjType(Value value, ObjType type) {
    return value.isObj() && value.asObj()->type == type;
}

namespace object {
inline ObjType objType(Value value) { return value.asObj()->type; }
inline bool isString(Value value) { return isObjType(value, OBJ_STRING); }
inline ObjString* asString(Value value) { return (ObjString*)value.asObj(); }
inline char* asCString(Value value) {
    return ((ObjString*)value.asObj())->chars;
}

inline std::string toString(Value value) {
    switch (object::objType(value)) {
    case OBJ_STRING:
        return fmt::format("{}", object::asCString(value));
        break;
    }
}
}

inline u32 hashString(const char* key, int length) {
    u32 hash = 2166136261u;

    for (int i = 0; i < length; ++i) {
        hash ^= key[i];
        hash *= 16777619;
    }

    return hash;
}

class ObjFactory {
public:
    ObjFactory() = default;
    ~ObjFactory() = default;

    static ObjString* allocateString(char* chars, int length, u32 hash);
    static ObjString* copyString(const char* chars, int length);
    static ObjString* takeString(char* chars, int length);

    static HashTable& get() { return strings_; }

private:
    static Obj* allocateObject(size_t size, ObjType type);

    template <typename T>
    static inline T* allocateObj(ObjType type) {
        return (T*)allocateObject(sizeof(T), type);
    }

private:
    static HashTable strings_;
};
