#include "HashTable.hh"
#include "object.hh"

std::size_t Hasher::operator()(ObjString* key) const noexcept {
    return key->hash;
}

HashTable::~HashTable() = default;
bool HashTable::set(ObjString* key, Value value) {

    bool isNewKey = true;

    auto result = map_.insert_or_assign(key, value);
    isNewKey = result.second;

    return isNewKey;
}

std::optional<Value> HashTable::get(ObjString* key) {

    auto iter = map_.find(key);
    if (iter == map_.end()) return {};
    return iter->second;
}

bool HashTable::deleteEntry(ObjString* key) {

    auto iter = map_.find(key);
    if (iter != map_.end()) {
        map_.erase(iter);
        return true;
    }

    return false;
}

ObjString* HashTable::findString(const char* chars, int length, u32 hash) {

    for (auto& [key, value] : map_) {
        if (key->length == length && key->hash == hash && std::memcmp(key->chars, chars, length) == 0) {
            return key;
        }
    }

    return nullptr;
}
