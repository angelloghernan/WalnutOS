#include "../klib/int.hh"
#include "../klib/pic.hh"

using namespace ports;

void Pic::end_of_interrupt(u8 irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }

    outb(PIC1_COMMAND, PIC_EOI);
}

void Pic::remap(u16 master_offset, u16 slave_offset) {
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); // Interrupt Command Word 1 -- Initialize
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC1_DATA, master_offset);
    io_wait();
    outb(PIC2_DATA, slave_offset);
    io_wait();
    outb(PIC1_DATA, 0x04);
    io_wait();
    outb(PIC2_DATA, 0x02);
    io_wait();

    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}
