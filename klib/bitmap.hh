#pragma once
#include "array.hh"
#include "iterator.hh"

template<usize S>
class Bitmap {
  public:
    class reference {
      public:
        constexpr void operator=(bool const b) { 
            // Trick: untoggle bit, then OR with result of b to be branchless
            m_ref = (m_ref & (~(1 << m_idx))) | (u8(b) << m_idx); 
        }
        constexpr void flip() { m_ref ^= (1 << m_idx); }
        constexpr bool operator==(bool const b) const { return ((m_ref & (1 << m_idx)) >> m_idx) == b; }

      private:
        bool& m_ref;
        u8 m_idx;

        reference(bool& ref, u8 idx) : m_ref(ref), m_idx(idx) {};
        friend class Bitmap;
    };

    Bitmap<S>();

    [[nodiscard]] constexpr reference& operator[](usize const idx) { 
        return reference { m_map[idx / sizeof(bool)], idx % sizeof(bool) }; 
    };

    [[nodiscard]] constexpr reference const& operator[](usize const idx) const { 
        return reference { m_map[idx / sizeof(bool)], idx % sizeof(bool) }; 
    };

  private:
    // An array of size S / 8 bits (one byte per 8 bits), rounded up to the nearest 8 bits
    Array<bool, (S + (sizeof(bool) - S % sizeof(bool))) / sizeof(bool)> m_map; 
};
