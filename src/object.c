#include "object.h"
#include "table.h"
#include "memory.h"
#include <stdio.h>
#include <string.h>

/* Module-level string intern table — zero-initialised (valid empty state) */
static Table g_strings;

static Obj* allocate_object(size_t size, ObjType type) {
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;
    return object;
}

static ObjString* allocate_string(char* chars, int length, u32 hash) {
    ObjString* string = (ObjString*)allocate_object(sizeof(ObjString), OBJ_STRING);
    string->length    = length;
    string->chars     = chars;
    string->hash      = hash;
    table_set(&g_strings, string, NIL_VAL);
    return string;
}

u32 hash_string(const char* key, int length) {
    u32 hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (u8)key[i];
        hash *= 16777619u;
    }
    return hash;
}

ObjString* copy_string(const char* chars, int length) {
    u32        hash     = hash_string(chars, length);
    ObjString* interned = table_find_string(&g_strings, chars, length, hash);
    if (interned != NULL) return interned;

    char* heap = ALLOCATE(char, length + 1);
    memcpy(heap, chars, (size_t)length);
    heap[length] = '\0';
    return allocate_string(heap, length, hash);
}

ObjString* take_string(char* chars, int length) {
    u32        hash     = hash_string(chars, length);
    ObjString* interned = table_find_string(&g_strings, chars, length, hash);
    if (interned != NULL) {
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }
    return allocate_string(chars, length, hash);
}

void obj_print(Value value) {
    switch (OBJ_TYPE(value)) {
    case OBJ_STRING:
        printf("%s", AS_CSTRING(value));
        break;
    }
}
