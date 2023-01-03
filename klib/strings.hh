#pragma once
#include "int.hh"
#include "option.hh"
#include "iterator.hh"

// str: A primitive type wrapping around string literals, allowing
// for easier iteration and slicing.
// When passing size, it is the size of the entire string, *including a null character*.
struct str {
  public:
    template<usize S>
    str(char const (&string)[S]) : m_string(string), m_size(S - 1) {}
    constexpr str(char const* string, usize size) : m_string(string), m_size(size - 1) {}

    auto constexpr len() const -> usize { return m_size; }

    auto constexpr begin() const -> iterator<char const> { return iterator<char const>(m_string); }
    auto constexpr end() const -> iterator<char const> { return iterator<char const>(m_string + m_size); }

    auto constexpr operator[](usize const idx) const -> char const& { return m_string[idx]; }

    auto constexpr get(usize const idx) const -> Option<char> {
        if (idx < m_size) {
            return Option<char>(m_string[idx]);
        } else {
            return Option<char>();
        }
    }


  private:
    char const* m_string;
    usize m_size;
};
