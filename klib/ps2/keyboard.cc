#include "keyboard.hh"
#include "../result.hh"
#include "../option.hh"
#include "ps2.hh"

using namespace ps2;

auto constexpr Ps2Keyboard::enqueue_command(KeyboardCommand const cmd) -> Result<Null, Null> {
    return m_cmd_queue.push(cmd);
}

auto Ps2Keyboard::get_scan_code_set() -> Result<ScanCodeSet, Null> {
    using enum ScanCodeSet;
    using Result = Result<ScanCodeSet, Null>;

    Ps2Controller::write_byte(SCANCODE_SERVICES);
    Ps2Controller::write_byte(0x0);

    auto maybe_response = read_ack();

    if (maybe_response.is_err() || maybe_response.as_ok() == 0  || maybe_response.as_ok() > 3) {
        return Result::Err({});
    }

    auto set = static_cast<ScanCodeSet>(maybe_response.as_ok());

    return Result::Ok(set);
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
        return Result::Err(static_cast<KeyboardResponse>(response));
    }

    auto const data = Ps2Controller::read_byte();

    return Result::Ok(data);
}

auto Ps2Keyboard::set_scan_code_set(ScanCodeSet const set) -> Result<Null, Null> {
    Ps2Controller::write_byte(static_cast<u8>(set));

    auto check = read_ack();
    if (check.is_err()) {
        return Result<Null, Null>::Err({});
    }

    return Result<Null, Null>::Ok({});
}

auto Ps2Keyboard::read_response() -> Option<KeyboardResponse> {
    auto response = Ps2Controller::blocking_read();

    if (response.is_err()) {
        return {};
    }

    return static_cast<KeyboardResponse>(response.as_ok());
}
