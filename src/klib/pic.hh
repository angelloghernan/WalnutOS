#pragma once
#include "int.hh"
#include "ports.hh"

class Pic {
public:
    Pic(Pic const&) = delete;
    void operator=(Pic const&) = delete;

    void static end_of_interrupt(u8 irq);
    void static remap(u16 master_offset, u16 slave_offset);

    auto static get() -> Pic& {
        static Pic pic;
        return pic;
    }

private:
    Pic() {}
    static auto constexpr PIC1_COMMAND    = 0x20_u16; // Master PIC
    static auto constexpr PIC2_COMMAND    = 0xA0_u16; // Slave PIC
    static auto constexpr PIC1_DATA       = 0x21_u16;
    static auto constexpr PIC2_DATA       = 0xA1_u8;
    static auto constexpr PIC_EOI         = 0x20_u8;

    static auto constexpr ICW1_ICW4	      = 0x01_u8; // ICW4 (not) needed 
    static auto constexpr ICW1_SINGLE     = 0x02_u8; // Single (cascade) mode 
    static auto constexpr ICW1_INTERVAL4  = 0x04_u8; // Call address interval 4 (8) 
    static auto constexpr ICW1_LEVEL      = 0x08_u8; // Level triggered (edge) mode
    static auto constexpr ICW1_INIT       = 0x10_u8; // Initialization - required!
                 
    static auto constexpr ICW4_8086       = 0x01_u8; // 8086/88 (MCS-80/85) mode
    static auto constexpr ICW4_AUTO	      = 0x02_u8; // Auto (normal) EOI
    static auto constexpr ICW4_BUF_SLAVE  = 0x08_u8; // Buffered mode/slave
    static auto constexpr ICW4_BUF_MASTER = 0x0C_u8; // Buffered mode/master 
    static auto constexpr ICW4_SFNM       = 0x10_u8; // Special fully nested (not)
};
