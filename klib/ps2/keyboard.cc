#include "keyboard.hh"

using namespace ps2;

auto Ps2Keyboard::enqueue(KeyboardCommand cmd) -> Result<Null, Null> {
    return Result<Null, Null>::Err({});
}
