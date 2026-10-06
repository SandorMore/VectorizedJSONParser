#pragma once

#include <immintrin.h>
#include <filesystem>
#include <fstream>
#include <optional>
#include <cstddef>
#include <cstdint>
#include <cassert>

int read_file(const char* file_name)
{
    std::fstream fs(file_name, std::ios::binary);
}