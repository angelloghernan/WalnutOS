#pragma once
#include "../int.hh"
#include "../ports.hh"
#include "../result.hh"

/// A driver for the PS/2 controller. Used for communicating with keyboard and mouse.
/// TODO: This PS/2 driver isn't very robust. Have to potentially set up ACPI and do a lot of
/// drudgework in order to make this fully sound on real hardware.
class Ps2Controller {
  public:

    // Perform the PS/2 controller self test, returning whether it was successful.
    auto static self_test() -> Result<Null, Null>;

    // Enable the first PS/2 port.
    void static enable_first();

    // Read a byte from the PS/2 data port, blocking until data is ready.
    auto static blocking_read() -> u8;

    // Read a byte from the PS/2 data port.
    inline auto static read_byte() -> u8 {
        return ports::inb(DATA_PORT);
    }

    // Write a byte to the PS/2 command port.
    void static write_byte(u8 port, u8 value);

  private:
    // The data port is used for reading data from PS/2 devices or writing to them.
    static auto constexpr DATA_PORT            = 0x60_u8;
    // Used for reading status or writing command to PS/2 controller.
    static auto constexpr CMD_STATUS_REGISTER  = 0x64_u8;

    static auto constexpr ENABLE_FIRST_PORT    = 0xAE_u8;
    static auto constexpr DISABLE_FIRST_PORT   = 0xAD_u8;

    static auto constexpr SELF_CHECK_SUCCESS   = 0x55_u8;

    static auto constexpr PERFORM_SELF_CHECK   = 0xAA_u8;
};
