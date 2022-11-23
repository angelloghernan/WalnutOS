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
    constexpr str(char const (&string)[S]) : string(string), size(S - 1) {}
    constexpr str(char const* string, usize size) : string(string), size(size - 1) {}
    constexpr str(char const* string) : string(string), size(strlen(string)) {}

    auto constexpr len() const -> usize { return size; }



    auto constexpr begin() const -> iterator<char const> { return iterator<char const>(string); }
    auto constexpr end() const -> iterator<char const> { return iterator<char const>(string + size); }

    auto constexpr operator[](usize const idx) const -> char const& { return string[idx]; }

    auto constexpr get(usize const idx) const -> Option<char> {
        if (idx < size) {
            return Option<char>(string[idx]);
        } else {
            return Option<char>();
        }
    }

  private:
    char const* string;
    usize const size;
    static auto constexpr strlen(char const* string) -> usize {
        usize s = 0;
        for (auto i = 0; string[i] != '\0'; ++i, ++s) {}
        return s;
    }
};
