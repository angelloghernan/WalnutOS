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
        constexpr void flip() { *m_ref ^= (1 << m_idx); }
        constexpr bool operator==(bool const b) const { return ((*m_ref & (1 << m_idx)) >> m_idx) == b; }
        constexpr bool operator==(reference const& b) const { return m_ref == b.m_ref && m_idx == b.m_idx; }
        constexpr bool operator!=(bool const b) const { return ((*m_ref & (1 << m_idx)) >> m_idx) != b; }
        constexpr bool operator!=(reference const& b) const { return m_ref != b.m_ref || m_idx != b.m_idx; }
        constexpr bool operator!() const { return *this == true; }

      private:
        bool* m_ref;
        u8 m_idx;

        reference(bool& ref, u8 idx) : m_ref(&ref), m_idx(idx) {};
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
            m_bit_ref.m_ref += m_bit_ref.m_idx == sizeof(bool);
            m_bit_ref.m_idx %= sizeof(bool);
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

    [[nodiscard]] auto static constexpr len() -> usize { 
        return (S + (sizeof(bool) - S % sizeof(bool))) / sizeof(bool);
    }

    [[nodiscard]] auto constexpr last() -> reference {
        return reference { m_map[(S - 1) / sizeof(bool)], (S - 1) % sizeof(bool) };
    }

    [[nodiscard]] constexpr reference operator[](usize const idx) { 
        return reference { m_map[idx / sizeof(bool)], idx % sizeof(bool) }; 
    };

    [[nodiscard]] constexpr reference const operator[](usize const idx) const { 
        return reference { m_map[idx / sizeof(bool)], idx % sizeof(bool) }; 
    };

    [[nodiscard]] auto constexpr begin() -> iterator { 
        return iterator( reference { m_map[0], 0 } ); 
    }

    [[nodiscard]] auto constexpr end() -> iterator { 
        return iterator( reference { m_map[S / sizeof(bool)], S % sizeof(bool) } ); 
    }

  private:
    // Array of size S / 8 bits (one byte per 8 bits), rounded up to the nearest 8 bits
    Array<bool, len()> m_map; 
};
