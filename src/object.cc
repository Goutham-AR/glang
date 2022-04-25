#include "object.hh"
#include "memory.hh"

#define ALLOCATE_OBJ(type, objType) \
    (type*)allocateObject(sizeof(type), objType)

static Obj* allocateObject(size_t size, ObjType type) {
    Obj* object = (Obj*)reallocate(nullptr, 0, size);
    object->type = type;
    return object;
}

static ObjString* allocateString(char* chars, int length) {
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
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
    switch (OBJ_TYPE(value)) {
    case OBJ_STRING:
        return fmt::format("{}", AS_CSTRING(value));
        break;
    }
}

ObjString* takeString(char* chars, int length) {
    return allocateString(chars, length);
}
