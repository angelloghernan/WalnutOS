#pragma once
#include "../int.hh"
#include "../result.hh"
#include "../array.hh"
#include "../circular_buffer.hh"
namespace ps2 {
    enum class ScanCodeSet;
    enum class KeyboardCommand;
    enum class Key;

    class Ps2Keyboard {
    public:
        Ps2Keyboard(Ps2Keyboard const&) = delete;
        Ps2Keyboard& operator=(Ps2Keyboard const&) = delete;

        auto get() -> Ps2Keyboard& {
            static Ps2Keyboard kb;
            return kb;
        }

        auto read_keycode() -> Result<Key, Null>;
        auto get_set() -> ScanCodeSet;
        auto enqueue(KeyboardCommand cmd) -> Result<Null, Null>;

    private:
        Ps2Keyboard();

        // Size is 4095 since circular buffer allocates 4096 (wastes a slot)
        CircularBuffer<KeyboardCommand, 4095> m_buffer;
    };
    
    enum class ScanCodeSet {
        Set1,
        Set2,
        Set3,
    };
    
    enum class KeyboardCommand {
        
    };

    enum class Key {
        Unknown,
    };
};
