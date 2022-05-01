#pragma once

#include "common.hh"

#include <cstdio>

namespace memory {

inline void* reallocate(void* ptr, size oldSize, size newSize) {
    if (newSize == 0) {
        std::free(ptr);
        return nullptr;
    }

    auto result = std::realloc(ptr, newSize);
    return result;
}

template <typename T>
inline T* allocate(size_t count) {
    return static_cast<T*>(reallocate(nullptr, 0, sizeof(T) * count));
}

inline void free(void* ptr, size_t capacity) {
    reallocate(ptr, capacity, 0);
}

}