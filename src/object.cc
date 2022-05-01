#include "object.hh"
#include "memory.hh"

static u32 hashString(const char* key, int length) {
    u32 hash = 2166136261u;

    for (int i = 0; i < length; ++i) {
        hash ^= key[i];
        hash *= 16777619;
    }

    return hash;
}

static Obj* allocateObject(size_t size, ObjType type) {
    Obj* object = memory::reallocate<Obj>(nullptr, 0, size);
    object->type = type;
    return object;
}

template <typename T>
inline T* allocateObj(ObjType type) {
    return (T*)allocateObject(sizeof(T), type);
}

static ObjString* allocateString(char* chars, int length, u32 hash) {
    auto string = allocateObj<ObjString>(OBJ_STRING);

    string->length = length;
    string->chars = chars;
    string->hash = hash;

    return string;
}

ObjString* copyString(const char* chars, int length) {

    u32 hash = hashString(chars, length);
    char* heapChars = memory::allocate<char>(length + 1);
    std::memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    return allocateString(heapChars, length, hash);
}

std::string objectToString(Value value) {
    switch (object::objType(value)) {
    case OBJ_STRING:
        return fmt::format("{}", object::asCString(value));
        break;
    }
}

ObjString* takeString(char* chars, int length) {
    u32 hash = hashString(chars, length);
    return allocateString(chars, length, hash);
}
