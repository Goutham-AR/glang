#pragma once

#include "common.hh"
#include "object.hh"
#include "memory.hh"

#include <optional>

constexpr auto TABLE_MAX_LOAD = 0.75;

struct Entry {
    ObjString* key;
    Value value;
};

class HashTable {
public:
    HashTable() = default;
    ~HashTable() {
        memory::reallocate(array_, capacity_, 0);
    }

    HashTable& operator=(HashTable rhs) {
        for (int i = 0; i < rhs.capacity_; ++i) {
            auto entry = &rhs.array_[i];

            if (entry->key != nullptr) {
                rhs.set(entry->key, entry->value);
            }
        }
    }

    bool set(ObjString* key, Value value) {
        if (count_ + 1 > capacity_ * TABLE_MAX_LOAD) {
            auto capacity = growCapacity(capacity_);
            adjustCapacity(capacity);
        }

        auto entry = findEntry(key);

        bool isNewKey = (entry->key == nullptr);
        if (isNewKey && value::isNil(entry->value)) ++count_;

        entry->key = key;
        entry->value = value;

        return isNewKey;
    }

    std::optional<Value> get(ObjString* key) {
        if (count_ == 0) return {};

        auto entry = findEntry(key);
        if (entry->key == nullptr) return {};

        return entry->value;
    }

    bool deleteEntry(ObjString* key) {
        if (count_ == 0) return false;

        auto entry = findEntry(key);
        if (entry->key == nullptr) return false;

        entry->key = nullptr;
        entry->value = value::boolValue(true);

        return true;
    }

    Entry* findEntry(ObjString* key) {
        auto index = key->hash;
        Entry* tombstone = nullptr;
        while (true) {
            auto entry = &array_[index];
            if (entry->key == nullptr) {
                if (value::isNil(entry->value)) {
                    return tombstone != nullptr ? tombstone : entry;
                } else {
                    if (tombstone == nullptr) tombstone = entry;
                }
            } else if (entry->key == key) {
                return entry;
            }

            index = (index + 1) % capacity_;
        }
    }

private:
    size_t growCapacity(size_t oldCapacity) {
        auto newCapacity = (oldCapacity == 0 ? 8 : 2 * oldCapacity);
        memory::reallocate(array_, oldCapacity, newCapacity);
        return newCapacity;
    }

    void adjustCapacity(size_t capacity) {
        auto entries = memory::allocate<Entry>(capacity);
        for (int i = 0; i < capacity; ++i) {
            entries[i] = {nullptr, ValNil};
        }

        Entry* dest = nullptr;
        count_ = 0;
        for (int i = 0; i < capacity_; ++i) {
            auto entry = &array_[i];
            if (entry->key == nullptr) continue;

            auto index = entry->key->hash;
            while (true) {
                auto e = &entries[index];
                if (e->key == entry->key || e->key == nullptr) {
                    dest = e;
                }

                index = (index + 1) % capacity;
            }
            dest->key = entry->key;
            dest->value = entry->value;
            count_++;
        }

        memory::free(array_, capacity_);

        array_ = entries;
        capacity_ = capacity;
    }

private:
    Entry* array_{};
    size_t count_{};
    size_t capacity_{};
};