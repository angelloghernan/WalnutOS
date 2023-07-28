#pragma once
#include "klib/int.hh"
#include "klib/option.hh"
#include "klib/iterator.hh"
#include "klib/slice.hh"

namespace wlib {
    // str: A primitive type wrapping around string literals, allowing for easier iteration and slicing.
    // When passing size, it is the size of the entire string, *including a null character*.
    struct str {
      public:
        template<usize S>
        constexpr str(char const (&string)[S]) : m_string(string), m_size(S - 1) {}
        constexpr str(char const* string, usize size) : m_string(string), m_size(size - 1) {}

        auto constexpr len() const -> usize { return m_size; }

        auto constexpr begin() const -> iterator<char const> { return iterator<char const>(m_string); }
        auto constexpr end() const -> iterator<char const> { return iterator<char const>(m_string + m_size); }

        auto constexpr operator[](usize const idx) const -> char const& { return m_string[idx]; }

        auto constexpr operator==(str const& other) const -> bool {
            if (other.m_size != m_size) {
                return false;
            }
            
            for (usize i = 0; i < m_size; ++i) {
                if (other[i] != m_string[i]) {
                    return false;
                }
            }

            return true;
        }

        auto constexpr operator==(Slice<char> const& other) const -> bool {
            if (other.len() != m_size) {
                return false;
            }
            
            for (usize i = 0; i < m_size; ++i) {
                if (other[i] != m_string[i]) {
                    return false;
                }
            }

            return true;
        }

        auto constexpr get(usize const idx) const -> Option<char> {
            if (idx < m_size) [[likely]] {
                return m_string[idx];
            } else {
                return {};
            }
        }

        auto constexpr as_slice() const -> Slice<char const> {
            return Slice<char const>(m_string, m_size);
        }

      private:
        char const* m_string;
        usize m_size;
    };
};
