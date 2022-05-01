#include "HashTable.hh"
#include "object.hh"

HashTable::~HashTable() {
    memory::reallocate(array_, capacity_, 0);
}

HashTable& HashTable::operator=(HashTable rhs) {
    for (int i = 0; i < rhs.capacity_; ++i) {
        auto entry = &rhs.array_[i];

        if (entry->key != nullptr) {
            rhs.set(entry->key, entry->value);
        }
    }
}

bool HashTable::set(ObjString* key, Value value) {
    if (count_ + 1 > capacity_ * TABLE_MAX_LOAD) {
        auto capacity = growCapacity(capacity_);
        adjustCapacity(capacity);
    }

    auto entry = findEntry(key);

    bool isNewKey = (entry->key == nullptr);
    if (isNewKey && entry->value.isNil()) ++count_;

    entry->key = key;
    entry->value = value;

    return isNewKey;
}

std::optional<Value> HashTable::get(ObjString* key) {
    if (count_ == 0) return {};

    auto entry = findEntry(key);
    if (entry->key == nullptr) return {};

    return entry->value;
}

bool HashTable::deleteEntry(ObjString* key) {
    if (count_ == 0) return false;

    auto entry = findEntry(key);
    if (entry->key == nullptr) return false;

    entry->key = nullptr;
    entry->value = Value::createBool(true);

    return true;
}

Entry* HashTable::findEntry(ObjString* key) {
    auto index = key->hash % capacity_;
    Entry* tombstone = nullptr;
    while (true) {
        auto entry = &array_[index];
        if (entry->key == nullptr) {
            if (entry->value.isNil()) {
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

size_t HashTable::growCapacity(size_t oldCapacity) {
    auto newCapacity = (oldCapacity == 0 ? 8 : 2 * oldCapacity);
    memory::reallocate(array_, oldCapacity, newCapacity);
    return newCapacity;
}

void HashTable::adjustCapacity(size_t capacity) {
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
ObjString* HashTable::findString(const char* chars, int length, u32 hash) {
    if (count_ == 0) return nullptr;

    auto index = hash % capacity_;

    while (true) {
        Entry* entry = &array_[index];

        if (entry->key == nullptr) {
            if (entry->value.isNil()) return nullptr;
        } else if (entry->key->length == length && entry->key->hash == hash && std::memcmp(entry->key->chars, chars, length) == 0) {
            return entry->key;
        }

        index = (index + 1) % capacity_;
    }
}
