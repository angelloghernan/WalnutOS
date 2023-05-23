#pragma once
#include "int.hh"

namespace ports {
    void outb(u16 port, u8 val);
    void outw(u16 port, u16 val);
    void outl(u16 port, u32 val);
    auto inb(u16 port) -> u8;
    auto inw(u16 port) -> u16;
    auto inl(u16 port) -> u32;
    void insw(u16 port, uptr buffer, u32 count);
    void io_wait();
}
