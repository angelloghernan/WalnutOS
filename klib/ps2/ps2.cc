#include "ps2.hh"
#include "../result.hh"
#include "../console.hh"
#include "../idt.hh"

using namespace ports;

void Ps2Controller::enable_first() {
    outb(CMD_STATUS_REGISTER, ENABLE_FIRST_PORT);
    outb(CMD_STATUS_REGISTER, 0x60);
    while ((inb(CMD_STATUS_REGISTER) & 0b10) != 0) {
        io_wait();
    }
    outb(CMD_STATUS_REGISTER, 0b00000101);
}

auto Ps2Controller::self_test() -> Result<Null, Null> {
    Idt::disable_interrupts();

    outb(CMD_STATUS_REGISTER, 0xAA);

    auto response = blocking_read();

    Idt::enable_interrupts();

    if (response == 0x55) {
        return Result<Null, Null>::Ok({});
    } else {
        return Result<Null, Null>::Err({});
    }
}

auto Ps2Controller::blocking_read() -> u8 {
    while ((inb(CMD_STATUS_REGISTER) & 0b1) != 1) {
        io_wait();
    }

    return inb(DATA_PORT);
}
