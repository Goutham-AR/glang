#include "object.hh"
#include "memory.hh"

HashTable ObjFactory::strings_;

Obj* ObjFactory::allocateObject(size_t size, ObjType type) {
    Obj* object = (Obj*)memory::reallocate(nullptr, 0, size);
    object->type = type;
    return object;
}

ObjString* ObjFactory::allocateString(char* chars, int length, u32 hash) {
    auto string = allocateObj<ObjString>(OBJ_STRING);

    string->length = length;
    string->chars = chars;
    string->hash = hash;

    strings_.set(string, Value::createNil());

    return string;
}

ObjString* ObjFactory::copyString(const char* chars, int length) {

    u32 hash = hashString(chars, length);
    ObjString* interned = strings_.findString(chars, length, hash);
    if (interned != nullptr) {
        return interned;
    }

    char* heapChars = memory::allocate<char>(length + 1);
    std::memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    return allocateString(heapChars, length, hash);
}

ObjString* ObjFactory::takeString(char* chars, int length) {
    u32 hash = hashString(chars, length);
    ObjString* interned = strings_.findString(chars, length, hash);

    if (interned != nullptr) {
        memory::free(chars, length + 1);
        return interned;
    }

    return allocateString(chars, length, hash);
}
