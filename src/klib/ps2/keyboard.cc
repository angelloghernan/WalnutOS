#include "keyboard.hh"
#include "../result.hh"
#include "../option.hh"
#include "../array.hh"
#include "ps2.hh"

using namespace ps2;

static constexpr Array<char const, 256> key_table {
    '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0', '\0',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', '\0', 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', '\0', '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', '\0', '\0', '\0', ' ', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
};

static constexpr Array<char const, 256> shift_table {
    '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0', '\0',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', '\0', 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', '\0', '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', '\0', '\0', '\0', ' ', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
};

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

auto Ps2Keyboard::response_to_char(KeyboardResponse response) -> char {
    auto idx = static_cast<u8>(response);
    return key_table[idx];
}

auto Ps2Keyboard::response_to_shifted_char(KeyboardResponse response) -> char {
    auto idx = static_cast<u8>(response);
    return shift_table[idx];
}
