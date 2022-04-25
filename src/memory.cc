#include "memory.hh"

void* reallocate(void* ptr, size oldSize, size newSize) {
    if (newSize == 0) {
        std::free(ptr);
        return nullptr;
    }

    auto result = std::realloc(ptr, newSize);
    return result;
}
