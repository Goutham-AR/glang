#pragma once

#include "common.hh"

#include <cstdio>

namespace memory {
template <typename T>
inline T* reallocate(T* ptr, size oldSize, size newSize) {
    if (newSize == 0) {
        std::free(ptr);
        return nullptr;
    }

    auto result = std::realloc(ptr, newSize);
    return static_cast<T*>(result);
}

template <typename T>
inline T* allocate(size_t count) {
    return static_cast<T*>(reallocate<T>(nullptr, 0, sizeof(T) * count));
}

template <typename T>
inline void free(T* ptr, size_t capacity) {
    reallocate<T>(ptr, capacity, 0);
}

}