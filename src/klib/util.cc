#include "klib/util.hh"

void* memset(void* ptr, int ch, size_t count) {
    auto const ch_ptr = reinterpret_cast<char*>(ptr);
    auto const ch_value = static_cast<unsigned char>(ch);

    for (size_t i = 0; i < count; ++i) {
        *ch_ptr = ch_value;
    }

    return ptr;
}

