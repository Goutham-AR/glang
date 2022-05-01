#pragma once

#include "common.hh"
#include "memory.hh"
#include "Value.hh"

#include <optional>

struct ObjString;

constexpr auto TABLE_MAX_LOAD = 0.75;

struct Entry {
    ObjString* key;
    Value value;
};

class HashTable {
public:
    HashTable() = default;
    ~HashTable();
    HashTable& operator=(HashTable rhs);

    bool set(ObjString* key, Value value);
    std::optional<Value> get(ObjString* key);
    bool deleteEntry(ObjString* key);
    Entry* findEntry(ObjString* key);
    ObjString* findString(const char* chars, int length, u32 hash);

private:
    size_t growCapacity(size_t oldCapacity);
    void adjustCapacity(size_t capacity);

private:
    Entry* array_{};
    size_t count_{};
    size_t capacity_{};
};