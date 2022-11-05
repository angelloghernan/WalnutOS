#pragma once
#include "int.hh"
#include "ports.hh"

class Pic {
public:
    Pic(Pic const&) = delete;
    void operator=(Pic const&) = delete;

    void end_of_interrupt(u8 irq);

    auto static get() -> Pic& {
        static Pic pic;
        return pic;
    }

private:
    Pic() {}
    static u16 constexpr const PIC1_COMMAND = 0x20; // Master PIC
    static u16 constexpr const PIC2_COMMAND = 0xA0; // Slave PIC
    static u16 constexpr const PIC1_DATA    = 0x21;
    static u16 constexpr const PIC2_DATA    = 0xA1;
    static u8  constexpr const PIC_EOI      = 0x20;
};
