#include "keyboard.hh"

using namespace ps2;

auto Ps2Keyboard::enqueue(KeyboardCommand cmd) -> Result<Null, Null> {
    return m_buffer.push(cmd);
}

auto Ps2Keyboard::get_set() -> ScanCodeSet {
    
}
