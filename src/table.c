#include "table.h"
#include "object.h"
#include "memory.h"
#include <string.h>

#define TABLE_MAX_LOAD 0.75

void table_init(Table* table) {
    table->count    = 0;
    table->capacity = 0;
    table->entries  = NULL;
}

void table_free(Table* table) {
    FREE_ARRAY(Entry, table->entries, table->capacity);
    table_init(table);
}

/*
 * Finds the entry slot for `key` in `entries`.
 * Returns: a pointer to the slot containing the key,
 *          the first tombstone found (for reuse), or
 *          the first empty slot.
 * Capacity must be a power-of-2 and > 0.
 */
static Entry* find_entry(Entry* entries, int capacity, ObjString* key) {
    uint32_t index     = key->hash & (uint32_t)(capacity - 1);
    Entry*   tombstone = NULL;

    while (true) {
        Entry* entry = &entries[index];

        if (entry->key == NULL) {
            if (IS_NIL(entry->value)) {
                /* empty slot — return tombstone for reuse if we passed one */
                return tombstone != NULL ? tombstone : entry;
            } else {
                /* tombstone (key=NULL, value non-NIL) */
                if (tombstone == NULL) tombstone = entry;
            }
        } else if (entry->key == key) {
            return entry;
        }

        index = (index + 1) & (uint32_t)(capacity - 1);
    }
}

static void adjust_capacity(Table* table, int new_capacity) {
    Entry* entries = ALLOCATE(Entry, new_capacity);
    for (int i = 0; i < new_capacity; i++) {
        entries[i].key   = NULL;
        entries[i].value = NIL_VAL;
    }

    /* Rehash all live entries; tombstones are dropped */
    table->count = 0;
    for (int i = 0; i < table->capacity; i++) {
        Entry* src = &table->entries[i];
        if (src->key == NULL) continue;
        Entry* dest  = find_entry(entries, new_capacity, src->key);
        dest->key    = src->key;
        dest->value  = src->value;
        table->count++;
    }

    FREE_ARRAY(Entry, table->entries, table->capacity);
    table->entries  = entries;
    table->capacity = new_capacity;
}

bool table_set(Table* table, ObjString* key, Value value) {
    if ((double)(table->count + 1) > (double)table->capacity * TABLE_MAX_LOAD) {
        int new_cap = GROW_CAPACITY(table->capacity);
        adjust_capacity(table, new_cap);
    }

    Entry* entry  = find_entry(table->entries, table->capacity, key);
    bool   is_new = (entry->key == NULL);
    /* Only count a fresh empty slot — not a tombstone being reused */
    if (is_new && IS_NIL(entry->value)) table->count++;

    entry->key   = key;
    entry->value = value;
    return is_new;
}

bool table_get(Table* table, ObjString* key, Value* out_value) {
    if (table->count == 0) return false;

    Entry* entry = find_entry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    *out_value = entry->value;
    return true;
}

bool table_delete(Table* table, ObjString* key) {
    if (table->count == 0) return false;

    Entry* entry = find_entry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    /* Replace with tombstone: key=NULL, value=non-NIL sentinel */
    entry->key   = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}

ObjString* table_find_string(Table* table, const char* chars, int length, u32 hash) {
    if (table->capacity == 0) return NULL;

    uint32_t index = hash & (uint32_t)(table->capacity - 1);
    while (true) {
        Entry* entry = &table->entries[index];

        if (entry->key == NULL) {
            if (IS_NIL(entry->value)) return NULL; /* genuine empty slot */
            /* tombstone — keep probing */
        } else if (entry->key->length == length &&
                   entry->key->hash   == hash   &&
                   memcmp(entry->key->chars, chars, (size_t)length) == 0) {
            return entry->key;
        }

        index = (index + 1) & (uint32_t)(table->capacity - 1);
    }
}
