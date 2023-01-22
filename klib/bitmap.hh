#pragma once
#include "array.hh"

template<usize S>
class Bitmap {
  public:
    struct iterator;

    class reference {
      public:
        constexpr void operator=(bool const b) {
            // Trick: untoggle bit, then OR with result of b to be branchless
            *m_ref = (*m_ref & (~(1 << m_idx))) | (u8(b) << m_idx); 
        }
        constexpr void flip() {
            *m_ref ^= (1 << m_idx); 
        }
        constexpr bool operator==(bool const b) const {
            // Take the [m_idx]'th bit of the underlying bool, then shift to compare
            return ((*m_ref & (1 << m_idx)) >> m_idx) == b; 
        }
        constexpr bool operator==(reference const& b) const {
            return m_ref == b.m_ref && m_idx == b.m_idx; 
        }
        constexpr bool operator!() const {
            return *this == false; 
        }
        constexpr bool operator!=(bool const b) const {
            return ((*m_ref & (1 << m_idx)) >> m_idx) != b; 
        }
        constexpr bool operator!=(reference const& b) const {
            return m_ref != b.m_ref || m_idx != b.m_idx; 
        }
        constexpr operator bool() const {
            return *this == true;
        }

      private:
        u8* m_ref;
        u8 m_idx;

        constexpr reference(u8& ref, u8 idx) : m_ref(&ref), m_idx(idx) {};
        friend class Bitmap;
        friend class iterator;
    };

    struct iterator {
      public:
        using pointer    = reference*;
        using ref        = reference&;

        constexpr iterator(reference const& bit_ref) : m_bit_ref(bit_ref) {}

        auto constexpr operator*() const -> bool { return m_bit_ref == true; }
        auto constexpr operator->() const -> pointer { return &m_bit_ref; }
        auto constexpr operator++() -> iterator& {
            ++m_bit_ref.m_idx;
            m_bit_ref.m_ref += m_bit_ref.m_idx == 8;
            m_bit_ref.m_idx %= 8;
            return *this; 
        }
        auto constexpr operator++(int) -> iterator { iterator tmp = *this; ++(*this); return tmp; }
        auto constexpr friend operator==(iterator const& a, iterator const& b) -> bool { 
            return a.m_bit_ref == b.m_bit_ref; 
        };
        auto constexpr friend operator!=(iterator const& a, iterator const& b) -> bool { 
            return a.m_bit_ref != b.m_bit_ref;
        };

      private:
        reference m_bit_ref;
    };

    Bitmap() {};

    // Return the *length* in terms of bits.
    [[nodiscard]] auto static constexpr len() -> usize { 
        return S;
    }

    // Return the *size* in terms of bytes.
    [[nodiscard]] auto static constexpr size() -> usize { 
        auto constexpr remainder = S % 8;
        if constexpr (remainder > 0) {
            return (S + (8 - remainder)) / 8;
        } else {
            return S / 8;
        }
    }

    void constexpr set_all()  {
        for (auto& byte : m_map) {
            byte |= 0xFF;
        }
    }

    void constexpr clear_all() {
        for (auto& byte : m_map) {
            byte &= 0x00;
        }
    }

    [[nodiscard]] auto constexpr last() -> reference {
        return reference { m_map[size() - 1], u8((S - 1) % 8) };
    }

    [[nodiscard]] constexpr reference operator[](usize const idx) { 
        return reference { m_map[idx / 8], u8(idx % 8) }; 
    };

    [[nodiscard]] constexpr bool operator[](usize const idx) const { 
        return m_map[(idx / 8)] & (idx % 8);
    };

    [[nodiscard]] auto constexpr begin() -> iterator { 
        return iterator( reference { m_map[0], 0_u8 } ); 
    }

    [[nodiscard]] auto constexpr end() -> iterator { 
        return iterator( reference { m_map[size()], u8(S % 8) } ); 
    }

  private:
    // Array of size S / 8 bits (one byte per 8 bits), rounded up to the nearest 8 bits
    Array<u8, size()> m_map; 
};
