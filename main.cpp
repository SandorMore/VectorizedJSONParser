#include <iostream>
#include "src/info.hpp"
#define DEBUG

int main(int argc, char** argv)
{
    const auto& cpu = ql::cpu::features();

#ifdef DEBUG
    std::cout << "AVX2: " << cpu.avx2
              << ", AVX-512F: " << cpu.avx512f
              << ", NEON: " << cpu.neon << '\n';
#endif

    if(cpu.avx2)
    {
        //parse_avx2(input);
    } 
    else 
    {
        //parse_scalar(input)
    }

    return 0;
}