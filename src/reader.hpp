#pragma once

#include <immintrin.h>
#include <filesystem>
#include <fstream>
#include <optional>
#include <cstddef>
#include <cstdint>
#include <cassert>

std::fstream& set_input_file(const char* input_file_name)
{
    return std::fstream(input_file_name);
}

