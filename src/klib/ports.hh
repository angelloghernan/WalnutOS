#pragma once
#include "klib/int.hh"

namespace wlib::ports {
    inline void outb(u16 port, u8 val) {
        asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
    }

    inline void outw(u16 port, u16 val) {
        asm volatile ( "outw %0, %1" : : "a"(val), "Nd"(port) );
    }

    inline void outl(u16 port, u32 val) {
        asm volatile ( "outl %0, %1" : : "a"(val), "Nd"(port));
    }

    inline auto inb(u16 port) -> u8 {
        u8 ret;
        asm volatile ( "inb %1, %0 " : "=a" (ret) : "Nd" (port));
        return ret;
    }

    inline auto inw(u16 port) -> u16 {
        u16 ret;
        asm volatile ( "inw %1, %0 " : "=a" (ret) : "Nd" (port));
        return ret;
    }

    inline auto inl(u16 port) -> u32 {
        u32 ret;
        asm volatile ( "inl %1, %0 " : "=a" (ret) : "Nd" (port));
        return ret;
    }

    inline void insw(u16 port, uptr buffer, u32 count) {
        asm volatile (
            "rep insw"
            : "+D"(buffer), "+c"(count)
            : "d"(port)
            : "memory"
        );
    }

    inline void io_wait() {
        outb(0x80, 0);
    }
}; // namespace wlib
