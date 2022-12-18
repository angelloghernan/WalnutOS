#pragma once
#include "../klib/int.hh"
#include "../klib/ports.hh"
#include "../klib/result.hh"

/// A driver for the PS/2 controller. Used for communicating with keyboard and mouse.
/// TODO: This PS/2 driver isn't very robust. Have to potentially set up ACPI and do a lot of
/// drudgework in order to make this fully sound on real hardware.
class Ps2Controller {
  public:
    auto static self_test() -> Result<Null, Null>;
    auto static enable_first() -> u8;

  private:
    // The data port is used for reading data from PS/2 devices or writing to them.
    static auto constexpr DATA_PORT            = 0x60_u8;
    // Used for reading status or writing command to PS/2 controller.
    static auto constexpr CMD_STATUS_REGISTER  = 0x64_u8;

    static auto constexpr ENABLE_FIRST_PORT    = 0xAE_u8;
    static auto constexpr DISABLE_FIRST_PORT   = 0xAD_u8;

    auto static read_byte() -> u8;
    void static write_byte(u8 port, u8 value);
};
