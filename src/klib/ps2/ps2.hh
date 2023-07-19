#pragma once
#include "../int.hh"
#include "../ports.hh"
#include "../result.hh"
#include "../console.hh"


namespace wlib::ps2 {
    /// A driver for the PS/2 controller. Used for communicating with keyboard and mouse.
    /// TODO: This PS/2 driver isn't very robust. Have to potentially set up ACPI and do a lot of
    /// drudgework in order to make this fully sound on real hardware.
    class Ps2Controller {
      public:

        // Perform the PS/2 controller self test, returning whether it was successful.
        auto static self_test() -> Result<Null, Null>;

        // Enable the first PS/2 port.
        void static enable_first();

        // Read a byte from the PS/2 data port, blocking until data is ready or it has failed three times.
        auto static blocking_read() -> Result<u8, Null>;

        // Read a byte from the PS/2 data port.
        inline auto static read_byte() -> u8 {
            return ports::inb(DATA_PORT);
        }

        // Write a byte to the PS/2 data port.
        inline void static write_byte(u8 value) {
            ports::outb(DATA_PORT, value);
        }

        inline auto static polling_write(u8 value) -> Result<Null, Null> {
            auto counter = 0_i8;
            while (counter < 3 && ports::inb(CMD_STATUS_REGISTER) & 0b10) {
                ++counter;
            }

            if (counter == 3) {
                return Result<Null, Null>::Err({});
            }

            ports::outb(DATA_PORT, value);

            return Result<Null, Null>::Ok({});
        }

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
}; // namespace wlib::ps2
