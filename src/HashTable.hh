#pragma once

#include "common.hh"
#include "memory.hh"
#include "Value.hh"

#include <optional>
#include <unordered_map>

struct ObjString;

// custom hasher
struct Hasher {
    std::size_t operator()(ObjString* key) const noexcept;
};

struct Entry {
    ObjString* key;
    Value value;
};

class HashTable {
public:
    HashTable() = default;
    ~HashTable();
    HashTable& operator=(const HashTable& rhs) = default;

    bool set(ObjString* key, Value value);
    std::optional<Value> get(ObjString* key);
    bool deleteEntry(ObjString* key);
    ObjString* findString(const char* chars, int length, u32 hash);

private:
    std::unordered_map<ObjString*, Value, Hasher> map_;
};