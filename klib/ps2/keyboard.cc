#include "keyboard.hh"
#include "../result.hh"
#include "../option.hh"
#include "ps2.hh"

using namespace ps2;

auto constexpr Ps2Keyboard::enqueue_command(KeyboardCommand const cmd) -> Result<Null, Null> {
    return Result<Null, Null>::Err({});
}

auto Ps2Keyboard::get_scan_code_set() -> Result<ScanCodeSet, Null> {
    Ps2Controller::write_byte(SCANCODE_SERVICES);
    // 0 -- get scan code set
}

auto Ps2Keyboard::read_ack() -> Result<u8, KeyboardResponse> {
    using enum KeyboardResponse;
    using Result = Result<u8, KeyboardResponse>;

    auto const check = Ps2Controller::blocking_read();

    if (check.is_err()) {
        return Result::Err(HardwareError);
    }

    auto const response = check.as_ok();

    if (response != static_cast<u8>(CommandAcknowledged)) {
        return Result::Err(u8_to_response(response));
    }

    auto const data = Ps2Controller::read_byte();

    return Result::Ok(data);
}

auto Ps2Keyboard::set_scan_code_set(ScanCodeSet const set) -> Result<Null, Null> {
    Ps2Controller::write_byte(static_cast<u8>(set));
}

auto constexpr Ps2Keyboard::pop_keycode() -> Option<Key> {
    return m_key_buffer.pop();
}

auto constexpr Ps2Keyboard::u8_to_response(u8 val) -> KeyboardResponse {
    using enum KeyboardResponse;

    switch (val) {
        case static_cast<u8>(Echo): {
            return Echo;
            break;
        }

        case static_cast<u8>(Resend): {
            return Resend;
            break;
        }

        case static_cast<u8>(CommandAcknowledged): {
            return CommandAcknowledged;
            break;
        }

        case 0x0: 
        case 0xFF: {
            return InternalError;
            break;
        }

        case static_cast<u8>(SelfTestPassed): {
            return SelfTestPassed;
            break;
        }

        case 0xFC:
        case 0xFD: {
            return SelfTestFailed;
            break;
        }

        default: {
            return HardwareError;
            break;
        }
    }
}
