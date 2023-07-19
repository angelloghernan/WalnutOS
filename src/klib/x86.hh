#pragma once
#include "int.hh"

// Credit to OSdev wiki for these snippets:
// https://wiki.osdev.org/Inline_Assembly/Examples

namespace wlib::x86 {
    struct cpuid_t {
        u32 eax;
        u32 edx;
    };

    [[nodiscard]] inline auto cpuid(u32 const code) -> cpuid_t {
        cpuid_t result;
        asm volatile ( "cpuid" : "=a"(result.eax), "=d"(result.edx) : "0"(code) : "ebx", "ecx" );
        return result;
    }

    struct rdmsr_t {
        u32 eax;
        u32 edx;
    };

    [[nodiscard]] inline auto rdmsr(u32 const msr_id) -> rdmsr_t {
        rdmsr_t result;
        asm volatile ( "rdmsr" : "=a" (result.eax), "=d" (result.edx) : "c" (msr_id) );
        return result;
    }

    inline void wrmsr(u32 const msr_id, u32 const lo, u32 const hi) {
        asm volatile ( "wrmsr" : : "a" (lo), "d" (hi), "c" (msr_id));
    }

    inline void pause() {
        asm volatile("pause" : : : "memory");
    }

    inline auto read_cr2() -> uptr {
        uptr cr2;
        asm volatile("movl %%cr2, %0" : "=r" (cr2));
        return cr2;
    }
}; // namespace wlib
