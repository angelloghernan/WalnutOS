#pragma once
#include "klib/array.hh"
#include "kernel/alloc.hh"

namespace wlib::ahci {
    class CacheEntry {
      public:
        [[nodiscard]] auto constexpr operator[](usize idx) -> u8& { return _buffer[idx]; };
        [[nodiscard]] auto constexpr operator[](usize idx) const -> u8 const& { return _buffer[idx]; };
      private:
        Array<u8, 512> _buffer;
    }; 
};
