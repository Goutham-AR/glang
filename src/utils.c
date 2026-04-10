#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

const char* read_text_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        exit(74);
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "Failed to seek in file: %s\n", filename);
        fclose(file);
        exit(74);
    }

    long file_size = ftell(file);
    if (file_size < 0) {
        fprintf(stderr, "Failed to determine file size: %s\n", filename);
        fclose(file);
        exit(74);
    }

    char* content = (char*)malloc((size_t)file_size + 1);
    if (!content) {
        fprintf(stderr, "Failed to allocate memory for file content\n");
        fclose(file);
        exit(74);
    }

    fseek(file, 0, SEEK_SET);
    size_t read_bytes = fread(content, 1, (size_t)file_size, file);
    fclose(file);
    content[read_bytes] = '\0'; /* fixed: was [file_size + 30] in original */
    return content;
}
