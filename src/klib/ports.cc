#include "ports.hh"

namespace wlib::ports {
    void outb(u16 port, u8 val) {
        asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
    }

    void outw(u16 port, u16 val) {
        asm volatile ( "outw %0, %1" : : "a"(val), "Nd"(port) );
    }

    void outl(u16 port, u32 val) {
        asm volatile ( "outl %0, %1" : : "a"(val), "Nd"(port));
    }

    auto inb(u16 port) -> u8 {
        u8 ret;
        asm volatile ( "inb %1, %0 " : "=a" (ret) : "Nd" (port));
        return ret;
    }

    auto inw(u16 port) -> u16 {
        u16 ret;
        asm volatile ( "inw %1, %0 " : "=a" (ret) : "Nd" (port));
        return ret;
    }

    auto inl(u16 port) -> u32 {
        u32 ret;
        asm volatile ( "inl %1, %0 " : "=a" (ret) : "Nd" (port));
        return ret;
    }

    void insw(u16 port, uptr buffer, u32 count) {
        asm volatile (
            "rep insw"
            : "+D"(buffer), "+c"(count)
            : "d"(port)
            : "memory"
        );
    }

    void io_wait() {
        outb(0x80, 0);
    }
}
