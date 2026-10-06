#pragma once
#include <cstdint>
#include <cstring>

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
  #define QL_X86 1
  #if defined(_MSC_VER)
    #include <intrin.h>
  #else
    #include <cpuid.h>
  #endif
#elif defined(__aarch64__) || defined(_M_ARM64)
  #define QL_ARM64 1
  #if defined(__linux__)
    #include <sys/auxv.h>
    #include <asm/hwcap.h>
  #elif defined(_WIN32)
    #include <windows.h>
  #endif
#endif

namespace ql::cpu {

enum class Vendor { Unknown, Intel, AMD, ARM };

struct Features {
    Vendor vendor = Vendor::Unknown;
    bool sse42 = false, popcnt = false, pclmul = false;
    bool avx = false, avx2 = false, bmi1 = false, bmi2 = false;
    bool avx512f = false, avx512bw = false, avx512vl = false;
    bool avx512vbmi = false, avx512vbmi2 = false;
    bool neon = false;
};

#ifdef QL_X86
struct CpuidResult { uint32_t eax, ebx, ecx, edx; };

inline CpuidResult cpuid(uint32_t leaf, uint32_t subleaf = 0) {
    CpuidResult r{};
#if defined(_MSC_VER)
    int regs[4];
    __cpuidex(regs, int(leaf), int(subleaf));
    r = { uint32_t(regs[0]), uint32_t(regs[1]), uint32_t(regs[2]), uint32_t(regs[3]) };
#else
    __cpuid_count(leaf, subleaf, r.eax, r.ebx, r.ecx, r.edx);
#endif
    return r;
}

inline uint64_t xgetbv0() {
#if defined(_MSC_VER)
    return _xgetbv(0);
#else
    uint32_t eax, edx;
    __asm__ volatile("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));
    return (uint64_t(edx) << 32) | eax;
#endif
}

inline Features detect() {
    Features f;
    auto l0 = cpuid(0);
    const uint32_t max_leaf = l0.eax;

    char vs[13];
    std::memcpy(vs + 0, &l0.ebx, 4);
    std::memcpy(vs + 4, &l0.edx, 4);
    std::memcpy(vs + 8, &l0.ecx, 4);
    vs[12] = 0;
    if (!std::strcmp(vs, "GenuineIntel")) f.vendor = Vendor::Intel;
    else if (!std::strcmp(vs, "AuthenticAMD")) f.vendor = Vendor::AMD;

    if (max_leaf < 1) return f;
    auto l1 = cpuid(1);
    f.sse42  = l1.ecx & (1u << 20);
    f.popcnt = l1.ecx & (1u << 23);
    f.pclmul = l1.ecx & (1u << 1);

    // AVX-hez: OSXSAVE + az OS tényleg menti az YMM állapotot
    const bool osxsave = l1.ecx & (1u << 27);
    uint64_t xcr0 = osxsave ? xgetbv0() : 0;
    const bool ymm_ok = (xcr0 & 0x6) == 0x6;           // XMM + YMM
    const bool zmm_ok = ymm_ok && (xcr0 & 0xE0) == 0xE0; // opmask + ZMM_Hi256 + Hi16_ZMM

    f.avx = ymm_ok && (l1.ecx & (1u << 28));

    if (max_leaf >= 7) {
        auto l7 = cpuid(7, 0);
        f.bmi1 = l7.ebx & (1u << 3);
        f.bmi2 = l7.ebx & (1u << 8);
        f.avx2 = f.avx && (l7.ebx & (1u << 5));

        if (zmm_ok) {
            f.avx512f  = l7.ebx & (1u << 16);
            f.avx512bw = l7.ebx & (1u << 30);
            f.avx512vl = l7.ebx & (1u << 31);
            f.avx512vbmi  = l7.ecx & (1u << 1);
            f.avx512vbmi2 = l7.ecx & (1u << 6);
        }
    }
    return f;
}
#endif // QL_X86

#ifdef QL_ARM64
inline Features detect() {
    Features f;
    f.vendor = Vendor::ARM;
    f.neon = true; // AArch64-en az Advanced SIMD kötelező
    return f;
}
#endif

inline const Features& features() {
    static const Features f = detect(); // egyszer fut le
    return f;
}

} // namespace ql::cpu