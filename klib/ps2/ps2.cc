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

    outb(CMD_STATUS_REGISTER, PERFORM_SELF_CHECK);

    auto response = blocking_read();

    Idt::enable_interrupts();

    if (!response.is_success() || response.as_ok() != SELF_CHECK_SUCCESS) {
        return Result<Null, Null>::Err({});
    }

    return Result<Null, Null>::Ok({});
}

auto Ps2Controller::blocking_read() -> Result<u8, Null> {
    auto count = 0_i8;
    while ((inb(CMD_STATUS_REGISTER) & 0b1) != 1 && count < 3) {
        io_wait();
    }

    if (count == 3) {
        return Result<u8, Null>::Err({});
    }

    return Result<u8, Null>::Ok(inb(DATA_PORT));
}
