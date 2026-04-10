#ifndef GLANG_VALUE_H
#define GLANG_VALUE_H

#include "common.h"
#include "memory.h"

/* Forward declaration — breaks circular dependency with object.h */
struct Obj;
typedef struct Obj Obj;

typedef enum {
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    VAL_OBJ
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool   as_bool;
        double as_number;
        Obj*   as_obj;
    } as;
} Value;

/* Value constructors */
#define BOOL_VAL(v)   ((Value){ VAL_BOOL,   { .as_bool   = (v)         } })
#define NIL_VAL       ((Value){ VAL_NIL,    { .as_number = 0.0         } })
#define NUMBER_VAL(v) ((Value){ VAL_NUMBER, { .as_number = (v)         } })
#define OBJ_VAL(obj)  ((Value){ VAL_OBJ,   { .as_obj    = (Obj*)(obj)  } })

/* Value accessors */
#define AS_BOOL(v)    ((v).as.as_bool)
#define AS_NUMBER(v)  ((v).as.as_number)
#define AS_OBJ(v)     ((v).as.as_obj)

/* Type checks */
#define IS_BOOL(v)    ((v).type == VAL_BOOL)
#define IS_NIL(v)     ((v).type == VAL_NIL)
#define IS_NUMBER(v)  ((v).type == VAL_NUMBER)
#define IS_OBJ(v)     ((v).type == VAL_OBJ)

/* Dynamic array of Values (replaces std::vector<Value>) */
typedef struct {
    int    capacity;
    int    count;
    Value* values;
} ValueArray;

void value_array_init(ValueArray* array);
int  value_array_write(ValueArray* array, Value value);
void value_array_free(ValueArray* array);

void value_print(Value value);
bool values_equal(Value a, Value b);

#endif /* GLANG_VALUE_H */
