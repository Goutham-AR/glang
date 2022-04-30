#include "object.hh"
#include "memory.hh"

// #define ALLOCATE_OBJ(type, objType) \
//     (type*)allocateObject(sizeof(type), objType)

static Obj* allocateObject(size_t size, ObjType type) {
    Obj* object = (Obj*)reallocate(nullptr, 0, size);
    object->type = type;
    return object;
}

template <typename T>
inline T* allocateObj(ObjType type) {
    return (T*)allocateObject(sizeof(T), type);
}

static ObjString* allocateString(char* chars, int length) {
    auto string = allocateObj<ObjString>(OBJ_STRING);

    string->length = length;
    string->chars = chars;

    return string;
}

ObjString* copyString(const char* chars, int length) {
    char* heapChars = ALLOCATE(char, length + 1);
    std::memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    return allocateString(heapChars, length);
}

std::string objectToString(Value value) {
    switch (object::objType(value)) {
    case OBJ_STRING:
        return fmt::format("{}", object::asCString(value));
        break;
    }
}

ObjString* takeString(char* chars, int length) {
    return allocateString(chars, length);
}
