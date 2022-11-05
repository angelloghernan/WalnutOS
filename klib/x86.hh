#pragma once
#include "int.hh"

// Credit to OSdev wiki for these snippets:
// https://wiki.osdev.org/Inline_Assembly/Examples

namespace x86 {
    inline void cpuid(int code, uint32_t* a, uint32_t* d) {
        asm volatile ( "cpuid" : "=a"(*a), "=d"(*d) : "0"(code) : "ebx", "ecx" );
    }

    inline void wrmsr(u32 msr_id, u64 msr_value) {
        asm volatile ( "wrmsr" : : "c" (msr_id), "A" (msr_value) );
    }  

    inline auto rdmsr(u32 msr_id) -> u64 {
        u64 msr_value;
        asm volatile ( "rdmsr" : "=A" (msr_value) : "c" (msr_id) );
        return msr_value;
    }
}
