#pragma once

#include "common.hh"

#include <cstdio>

void* reallocate(void* ptr, size oldSize, size newSize);

#define ALLOCATE(type, count) \
    (type*)reallocate(nullptr, 0, sizeof(type) * (count))