#ifndef GLANG_OBJECT_H
#define GLANG_OBJECT_H

#include "common.h"
#include "value.h"
#include "memory.h"

typedef enum {
    OBJ_STRING
} ObjType;

struct Obj {
    ObjType type;
};

/* Full definition of ObjString — forward-declared in table.h */
struct ObjString {
    struct Obj obj;
    int        length;
    char*      chars;
    u32        hash;
};

/* Compatible typedef redeclaration with the one in table.h (C11 §6.7.3) */
typedef struct ObjString ObjString;

#define OBJ_TYPE(value)   (AS_OBJ(value)->type)
#define IS_STRING(value)  is_obj_type(value, OBJ_STRING)
#define AS_STRING(value)  ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)

static inline bool is_obj_type(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

void       obj_print(Value value);
u32        hash_string(const char* key, int length);
ObjString* copy_string(const char* chars, int length);
ObjString* take_string(char* chars, int length);

#endif /* GLANG_OBJECT_H */
