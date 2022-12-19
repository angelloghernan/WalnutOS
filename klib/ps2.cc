#include "../klib/ps2.hh"
#include "../klib/result.hh"
#include "../klib/console.hh"

using namespace ports;

auto Ps2Controller::enable_first() -> u8 {
    outb(CMD_STATUS_REGISTER, ENABLE_FIRST_PORT);
    outb(CMD_STATUS_REGISTER, 0x60);
    while ((inb(CMD_STATUS_REGISTER) & 0b10) != 0) {
        io_wait();
    }
    outb(CMD_STATUS_REGISTER, 0b00000101);
    // auto check = read_byte();
    return 0;
}

auto Ps2Controller::self_test() -> Result<Null, Null> {
    outb(CMD_STATUS_REGISTER, 0xAA);

    auto response = read_byte();

    if (response == 0x55) {
        return Result<Null, Null>::Ok({});
    } else {
        return Result<Null, Null>::Err({});
    }
}

auto Ps2Controller::read_byte() -> u8 {
    while ((inb(CMD_STATUS_REGISTER) & 0b1) != 1) {
        io_wait();
    }

    return inb(DATA_PORT);
}

