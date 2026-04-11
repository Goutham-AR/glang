#ifndef SOLV_MEMORY_H
#define SOLV_MEMORY_H

#include "common.h"
#include <stdlib.h>

void* reallocate(void* ptr, size_t old_size, size_t new_size);

#define GROW_CAPACITY(cap) ((cap) < 8 ? 8 : (cap) * 2)

#define GROW_ARRAY(type, ptr, old_count, new_count) \
    ((type*)reallocate(ptr, sizeof(type) * (size_t)(old_count), sizeof(type) * (size_t)(new_count)))

#define FREE_ARRAY(type, ptr, old_count) \
    reallocate(ptr, sizeof(type) * (size_t)(old_count), 0)

#define ALLOCATE(type, count) \
    ((type*)reallocate(NULL, 0, sizeof(type) * (size_t)(count)))

#endif /* SOLV_MEMORY_H */
