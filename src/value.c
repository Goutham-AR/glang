#include "value.h"
#include "object.h"
#include "memory.h"
#include <math.h>
#include <stdio.h>

void value_array_init(ValueArray* array) {
    array->capacity = 0;
    array->count    = 0;
    array->values   = NULL;
}

int value_array_write(ValueArray* array, Value value) {
    if (array->capacity < array->count + 1) {
        int old_cap     = array->capacity;
        array->capacity = GROW_CAPACITY(old_cap);
        array->values   = GROW_ARRAY(Value, array->values, old_cap, array->capacity);
    }
    array->values[array->count] = value;
    return array->count++;
}

void value_array_free(ValueArray* array) {
    FREE_ARRAY(Value, array->values, array->capacity);
    value_array_init(array);
}

void value_print(Value value) {
    switch (value.type) {
    case VAL_BOOL:   printf("%s", AS_BOOL(value) ? "True" : "False"); break;
    case VAL_NIL:    printf("Nil");                                    break;
    case VAL_NUMBER: {
            double d = AS_NUMBER(value);
            if (d == floor(d) && d >= -1e15 && d <= 1e15)
                printf("%.0f", d);
            else
                printf("%g", d);
            break;
        }
    case VAL_OBJ:    obj_print(value);                                 break;
    }
}

bool values_equal(Value a, Value b) {
    if (a.type != b.type) return false;
    switch (a.type) {
    case VAL_BOOL:   return AS_BOOL(a)   == AS_BOOL(b);
    case VAL_NIL:    return true;
    case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
    case VAL_OBJ:    return AS_OBJ(a)   == AS_OBJ(b);
    default:         return false;
    }
}
