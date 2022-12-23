#pragma once
#include "../int.hh"
#include "../result.hh"
#include "../array.hh"
#include "../circular_buffer.hh"
namespace ps2 {
    enum class ScanCodeSet : u8;
    enum class KeyboardCommand : u8;
    enum class KeyboardResponse : u8;
    enum class Key : u8;

    class Ps2Keyboard {
    public:
        Ps2Keyboard(Ps2Keyboard const&) = delete;
        Ps2Keyboard& operator=(Ps2Keyboard const&) = delete;

        auto get() -> Ps2Keyboard& {
            static Ps2Keyboard kb;
            return kb;
        }

        auto constexpr pop_keycode() -> Option<Key>;
        auto get_scan_code_set() -> Result<ScanCodeSet, Null>;
        auto set_scan_code_set(ScanCodeSet set) -> Result<Null, Null>;
        auto constexpr enqueue_command(KeyboardCommand cmd) -> Result<Null, Null>;

    private:
        Ps2Keyboard();
        auto read_ack() -> Result<u8, KeyboardResponse>;
        auto constexpr u8_to_response(u8 val) -> KeyboardResponse;
        CircularBuffer<Key, 4090> m_key_buffer;
        CircularBuffer<KeyboardCommand, 4090> m_cmd_queue;
        auto constexpr static SCANCODE_SERVICES = 0xF0_u8;
    };
    
    enum class ScanCodeSet : u8 {
        Set1,
        Set2,
        Set3,
    };
    
    /// All general keyboard commands without the get/set scancode commands.
    enum class KeyboardCommand : u8 {
        SetLEDs             = 0xED,
        Echo                = 0xEE, 
        IdentifyKeyboard    = 0xF2,
        TypematicRate       = 0xF3,
        EnableScanning      = 0xF4,
        DisableScanning     = 0xF5,
        SetDefault          = 0xF6,
        ResendLastByte      = 0xFE,
        ResetAndSelfTest    = 0xFF,
    };

    enum class Key : u8 {
        Unknown,
    };

    enum class KeyboardResponse : u8 {
        Echo                = 0xEE,
        Resend              = 0xFE,
        CommandAcknowledged = 0xFA,
        InternalError       = 0x0,
        HardwareError       = 0x01,
        SelfTestPassed      = 0xAA,
        SelfTestFailed      = 0xFC,
    };
};
