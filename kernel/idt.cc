#include "../klib/idt.hh"
#include "../klib/console.hh"
#include "../klib/assert.hh"
#include "../klib/console.hh"
#include "../klib/pic.hh"
#include "../klib/ps2/ps2.hh"
#include "../klib/strings.hh"
#include "../klib/ps2/keyboard.hh"
#include "../klib/ports.hh"

using namespace ps2;

Ps2Keyboard keyboard;

void end_of_interrupt(u8 vector_code) {
    if (vector_code - 0x20 >= 8) {
        ports::outb(0xA0, 0x20);
    }
    ports::outb(0x20, 0x20);
}

extern "C" void exception_handler(regstate& regs) {
    end_of_interrupt(regs.vector_code);
}

extern "C" void keyboard_handler() {
    using ps2::KeyboardResponse;
    using ps2::Ps2Keyboard;
    using enum KeyboardResponse;

    auto const scan_code = static_cast<KeyboardResponse>(Ps2Controller::read_byte());
    switch (scan_code) {
        case PS2SelfTestPassed: {
            break;
        }
        case CommandAcknowledged: {
            keyboard.pop_command();
            break;
        }
        case F1Down: {
            // Hack: shut down QEMU. Not portable outside of QEMU.
            ports::outw(0x604, 0x2000);
            break;
        }
        default: {
            keyboard.push_response(scan_code);
            break;
        }
    }

    auto const next = keyboard.next_command();
    if (next.some()) {
        terminal.print_line(str("Something"));
        auto result = Ps2Controller::polling_write(static_cast<u8>(next.unwrap()));
        warn_if(result.is_err(), "Error reading response from kb");
    }
    ports::outb(0x20, 0x20);
}
