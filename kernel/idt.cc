#include "../klib/idt.hh"
#include "../klib/console.hh"
#include "../klib/assert.hh"
#include "../klib/console.hh"
#include "../klib/pic.hh"
#include "../klib/ps2/ps2.hh"
#include "../klib/ps2/keyboard.hh"
#include "../klib/ports.hh"

void end_of_interrupt(u8 vector_code) {
    if (vector_code - 0x20 >= 8) {
        ports::outb(0xA0, 0x20);
    }
    ports::outb(0x20, 0x20);
}

extern "C" void exception_handler(regstate& regs) {
    end_of_interrupt(regs.vector_code);
}

extern "C" void keyboard_handler(regstate& regs) {
    using ps2::KeyboardResponse;
    using enum KeyboardResponse;

    auto scan_code = static_cast<KeyboardResponse>(Ps2Controller::read_byte());
    switch (scan_code) {
        case SelfTestPassed:
            terminal.print_line("Self test passed");
            break;
        case CommandAcknowledged:
            terminal.print_line("ACK");
            break;
        case ADown:
            terminal.print_line("A down");
            break;
        case AUp:
            terminal.print_line("A up");
            break;
        case QDown:
            // Hack: shut down QEMU. Not portable outside of QEMU.
            ports::outw(0x604, 0x2000);
            break;
        default:
            terminal.print_line("Some other key pressed");
            break;
    }
    end_of_interrupt(0x21);
}
