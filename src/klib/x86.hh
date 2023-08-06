#pragma once
#include "klib/int.hh"

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

    [[nodiscard]] inline auto read_cr2() -> uptr {
        uptr cr2;
        asm volatile("movl %%cr2, %0" : "=r" (cr2));
        return cr2;
    }

    

    [[nodiscard]] inline auto lzcnt_16(u16 num) -> u16 {
        u16 result;
        asm volatile ("lzcntw %1, %0" : "=r" (result) : "r" (num));
        return result;
    }

    [[nodiscard]] inline auto lzcnt_32(u32 num) -> u32 {
        u32 result;
        asm volatile ("lzcntl %1, %0" : "=r" (result) : "r" (num));
        return result;
    }

    [[nodiscard]] inline auto lzcnt_64(u64 num) -> u64 {
        u64 result;
        asm volatile ("lzcntq %1, %0" : "=r" (result) : "r" (num));
        return result;
    }

    [[nodiscard]] inline auto tzcnt_16(u16 num) -> u16 {
        u16 result;
        asm volatile ("tzcntw %1, %0" : "=r" (result) : "r" (num));
        return result;
    }

    [[nodiscard]] inline auto tzcnt_32(u32 num) -> u32 {
        u32 result;
        asm volatile ("tzcntl %1, %0" : "=r" (result) : "r" (num));
        return result;
    }

    [[nodiscard]] inline auto tzcnt_64(u64 num) -> u64 {
        u64 result;
        asm volatile ("tzcntq %1, %0" : "=r" (result) : "r" (num));
        return result;
    }
}; // namespace wlib
