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

        auto static get() -> Ps2Keyboard& {
            static Ps2Keyboard kb;
            return kb;
        }

        auto read_response() -> Option<KeyboardResponse>;
        auto static get_scan_code_set() -> Result<ScanCodeSet, Null>;
        auto static set_scan_code_set(ScanCodeSet set) -> Result<Null, Null>;
        auto constexpr enqueue_command(KeyboardCommand cmd) -> Result<Null, Null>;
        auto constexpr pop_command() -> Option<KeyboardCommand>;

    private:
        Ps2Keyboard();
        auto static read_ack() -> Result<u8, KeyboardResponse>;
        auto static constexpr u8_to_response(u8 val) -> KeyboardResponse; 
        CircularBuffer<Key, 4090> m_key_buffer;
        CircularBuffer<KeyboardCommand, 4090> m_cmd_queue;
        auto constexpr static SCANCODE_SERVICES = 0xF0_u8;
    };
    
    enum class ScanCodeSet : u8 {
        Set1 = 1,
        Set2 = 2,
        Set3 = 3,
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

    // Keyboard response table for scan code set 1.
    enum class KeyboardResponse : u8 {
        // Non-key responses
        Echo                = 0xEE,
        Resend              = 0xFE,
        CommandAcknowledged = 0xFA,
        InternalError       = 0x00,
        HardwareError       = 0xD4, // 0xD4 since this corresponds to nothing
        SelfTestPassed      = 0xAA,
        SelfTestFailed      = 0xFC,
        // Key responses
        EscDown             = 0x01,
        OneDown             = 0x02,
        TwoDown             = 0x03,
        ThreeDown           = 0x04,
        FourDown            = 0x05,
        FiveDown            = 0x06,
        SixDown             = 0x07,
        SevenDown           = 0x08,
        EightDown           = 0x09,
        NineDown            = 0x0A,
        ZeroDown            = 0x0B,
        DashDown            = 0x0C,
        EqualsDown          = 0x0D,
        BackspaceDown       = 0x0E,
        TabDown             = 0x0F,
        QDown               = 0x10,
        WDown               = 0x11,
        EDown               = 0x12,
        RDown               = 0x13,
        TDown               = 0x14,
        YDown               = 0x15,
        UDown               = 0x16,
        IDown               = 0x17,
        ODown               = 0x18,
        PDown               = 0x19,
        LeftBracketDown     = 0x1A,
        RightBracketDown    = 0x1B,
        EnterDown           = 0x1C,
        LeftCtrlDown        = 0x1D,
        ADown               = 0x1E,
        SDown               = 0x1F,
        DDown               = 0x20,
        FDown               = 0x21,
        GDown               = 0x22,
        HDown               = 0x23,
        JDown               = 0x24,
        KDown               = 0x25,
        LDown               = 0x26,
        SemicolonDown       = 0x27,
        SingleQuoteDown     = 0x28,
        BackTickDown        = 0x29,
        LeftShiftDown       = 0x2A,
        BackSlashDown       = 0x2B,
        ZDown               = 0x2C,
        XDown               = 0x2D,
        CDown               = 0x2E,
        VDown               = 0x2F,
        BDown               = 0x30,
        NDown               = 0x31,
        MDown               = 0x32,
        CommaDown           = 0x33,
        PeriodDown          = 0x34,
        SlashDown           = 0x35,
        RightShiftDown      = 0x36,
        KeypadAsteriskDown  = 0x37,
        LeftAltDown         = 0x38,
        SpaceDown           = 0x39,
        CapsLockDown        = 0x3A,
        F1Down              = 0x3B,
        F2Down              = 0x3C,
        F3Down              = 0x3D,
        F4Down              = 0x3E,
        F5Down              = 0x3F,
        F6Down              = 0x40,
        F7Down              = 0x41,
        F8Down              = 0x42,
        F9Down              = 0x43,
        F10Down             = 0x44,
        NumberLockDown      = 0x45,
        ScrollLockDown      = 0x46,
        KeypadSevenDown     = 0x47,
        KeypadEightDown     = 0x48,
        KeypadNineDown      = 0x49,
        KeypadDashDown      = 0x4A,
        KeypadFourDown      = 0x4B,
        KeypadFiveDown      = 0x4C,
        KeypadSixDown       = 0x4D,
        KeypadPlusDown      = 0x4E,
        KeypadOneDown       = 0x4F,
        KeypadTwoDown       = 0x50,
        KeypadThreeDown     = 0x51,
        KeypadZeroDown      = 0x52,
        KeypadPeriodDown    = 0x53,
        // Gap of non-valid/reserved codes
        F11Down             = 0x57,
        F12Down             = 0x58,
        // Gap of non-valid/reserved codes
        EscapeUp            = 0x81,
        OneUp               = 0x82,
        TwoUp               = 0x83,
        ThreeUp             = 0x84,
        FourUp              = 0x85,
        FiveUp              = 0x86,
        SixUp               = 0x87,
        SevenUp             = 0x88,
        EightUp             = 0x89,
        NineUp              = 0x8A,
        ZeroUp              = 0x8B,
        DashUp              = 0x8C,
        EqualsUp            = 0x8D,
        BackspaceUp         = 0x8E,
        TabUp               = 0x8F,
        QUp                 = 0x90,
        WUp                 = 0x91,
        EUp                 = 0x92,
        RUp                 = 0x93,
        TUp                 = 0x94,
        YUp                 = 0x95,
        UUp                 = 0x96,
        IUp                 = 0x97,
        OUp                 = 0x98,
        PUp                 = 0x99,
        LeftBracketUp       = 0x9A,
        RightBracketUp      = 0x9B,
        EnterUp             = 0x9C,
        LeftCtrlUp          = 0x9D,
        AUp                 = 0x9E,
        SUp                 = 0x9F,
        DUp                 = 0xA0,
        FUp                 = 0xA1,
        GUp                 = 0xA2,
        HUp                 = 0xA3,
        JUp                 = 0xA4,
        KUp                 = 0xA5,
        LUp                 = 0xA6,
        SemicolonUp         = 0xA7,
        SingleQuoteUp       = 0xA8,
        BackTickUp          = 0xA9,
        LeftShiftUp         = 0xAA,
        BackSlashUp         = 0xAB,
        ZUp                 = 0xAC,
        XUp                 = 0xAD,
        CUp                 = 0xAE,
        VUp                 = 0xAF,
        BUp                 = 0xB0,
        NUp                 = 0xB1,
        MUp                 = 0xB2,
        CommaUp             = 0xB3,
        PeriodUp            = 0xB4,
        SlashUp             = 0xB5,
        RightShiftUp        = 0xB6,
        KeypadAsteriskUp    = 0xB7,
        LeftAltUp           = 0xB8,
        SpaceUp             = 0xB9,
        CapsLockUp          = 0xBA,
        F1Up                = 0xBB,
        F2Up                = 0xBC,
        F3Up                = 0xBD,
        F4Up                = 0xBE,
        F5Up                = 0xBF,
        F6Up                = 0xC0,
        F7Up                = 0xC1,
        F8Up                = 0xC2,
        F9Up                = 0xC3,
        F10Up               = 0xC4,
        NumberLockUp        = 0xC5,
        ScrollLockUp        = 0xC6,
        KeypadSevenUp       = 0xC7,
        KeypadEightUp       = 0xC8,
        KeypadNineUp        = 0xC9,
        KeypadDashUp        = 0xCA,
        KeypadFourUp        = 0xCB,
        KeypadFiveUp        = 0xCC,
        KeypadSixUp         = 0xCD,
        KeypadPlusUp        = 0xCE,
        KeypadOneUp         = 0xCF,
        KeypadTwoUp         = 0xD0,
        KeypadThreeUp       = 0xD1,
        KeypadZeroUp        = 0xD2,
        KeypadPeriodUp      = 0xD3,
        F11Up               = 0xD4,
        F12Up               = 0xD5,
        ExtendedKeyCode     = 0xE0,
    };
};
