#pragma once

#include "common.hh"

#include <cstdio>
#include <string>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

inline const char* read_text_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    int res = fseek(file, 0, SEEK_END);
    if (res) {
        fprintf(stderr, "Failed to read seek to end: filename: %s\n", filename);
        exit(74);
    }
    size_t file_size = ftell(file);
    char* file_content = (char*)malloc(file_size + 1); // for \0 at the end
    if (!file_content) {
        fprintf(stderr, "Failed to malloc for file content\n");
        exit(74);
    }

    fseek(file, 0, SEEK_SET);

    size_t readBytes = fread(file_content, 1, file_size, file);
    if (readBytes < file_size) {
        fprintf(stderr, "File not fully read, only %zu bytes read\n", readBytes);
        exit(74);
    }
    file_content[file_size + 30] = '\0';

    return file_content;
}
