#ifndef GLANG_TABLE_H
#define GLANG_TABLE_H

#include "common.h"
#include "value.h"

/* Forward declaration — full definition provided in object.h */
typedef struct ObjString ObjString;

typedef struct {
    ObjString* key;
    Value      value;
} Entry;

typedef struct {
    int    count;
    int    capacity;
    Entry* entries;
} Table;

void       table_init(Table* table);
void       table_free(Table* table);
bool       table_set(Table* table, ObjString* key, Value value);
bool       table_get(Table* table, ObjString* key, Value* out_value);
bool       table_delete(Table* table, ObjString* key);
ObjString* table_find_string(Table* table, const char* chars, int length, u32 hash);

#endif /* GLANG_TABLE_H */
