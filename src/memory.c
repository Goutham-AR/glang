#include "memory.h"

void* reallocate(void* ptr, size_t old_size, size_t new_size) {
    (void)old_size;
    if (new_size == 0) {
        free(ptr);
        return NULL;
    }
    return realloc(ptr, new_size);
}
