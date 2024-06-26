#include "klib/ps2/ps2.hh"
#include "klib/result.hh"
#include "klib/console.hh"
#include "klib/idt.hh"

using namespace wlib;
using namespace ports;
using namespace ps2;

void Ps2Controller::enable_first() {
    outb(CMD_STATUS_REGISTER, ENABLE_FIRST_PORT);
    outb(CMD_STATUS_REGISTER, 0x60);
    while ((inb(CMD_STATUS_REGISTER) & 0b10) != 0) {
        io_wait();
    }
    outb(CMD_STATUS_REGISTER, 0b00000101);
}

auto Ps2Controller::self_test() -> Result<Null, Null> {
    using Result = Result<Null, Null>;
    Idt::disable_interrupts();

    outb(CMD_STATUS_REGISTER, PERFORM_SELF_CHECK);

    auto response = blocking_read();

    Idt::enable_interrupts();

    if (response.is_err() || response.as_ok() != SELF_CHECK_SUCCESS) {
        return Result::Err({});
    }

    return Result::Ok({});
}

auto Ps2Controller::blocking_read() -> Result<u8, Null> {
    auto count = 0_i8;
    while ((inb(CMD_STATUS_REGISTER) & 0b1) != 1 && count < 3) {
        io_wait();
        ++count;
    }

    if (count == 3) {
        return Result<u8, Null>::Err({});
    }

    return Result<u8, Null>::Ok(inb(DATA_PORT));
}
