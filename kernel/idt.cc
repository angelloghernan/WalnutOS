#include "../klib/idt.hh"
#include "../klib/console.hh"
#include "../klib/assert.hh"
#include "../klib/console.hh"
#include "../klib/pic.hh"
#include "../klib/ps2/ps2.hh"
#include "../klib/ports.hh"

extern "C" void exception_handler(regstate& regs) {
    Pic::end_of_interrupt(regs.vector_code);
}

extern "C" void keyboard_handler(regstate& regs) {
    auto scan_code = Ps2Controller::read_byte();
    // 'q' key
    if (scan_code == 16) {
        // Hack: shut down QEMU. Not portable outside of QEMU.
        ports::outw(0x604, 0x2000);
    }
    terminal.print_line("Key pushed: ", scan_code);
    Pic::end_of_interrupt(0x21);
}
