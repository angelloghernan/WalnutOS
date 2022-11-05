#include "../klib/int.hh"
#include "../klib/pic.hh"


void Pic::end_of_interrupt(u8 irq) {
    if (irq >= 8) {
        ports::outb(PIC2_COMMAND, PIC_EOI);
    }

    ports::outb(PIC1_COMMAND, PIC_EOI);
}
