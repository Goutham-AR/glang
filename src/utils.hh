#pragma once

#include "common.hh"

#include <string>
#include <fstream>

namespace utils {
inline std::string readTextFile(const char* filename) {
    std::ifstream file{filename, std::ios::ate | std::ios::in};

    if (!file.is_open()) {
        fmt::print("Failed to open file: {}", filename);
        std::exit(74);
    }
    auto size = file.tellg();
    std::string content(size, 0);

    file.seekg(0, std::ios::beg);

    file.read(content.data(), size);
    // fmt::print("content is {}", content);
    return content;
}
}