#pragma once
#include "int.hh"
#include "option.hh"
#include "iterator.hh"

// str: A primitive type wrapping around string literals, allowing
// for easier iteration and slicing.
// Do not construct a str with a custom const char array.
struct str {
  public:
    template<usize S>
    str(char const (&string)[S]) : string(string), size(S - 1) {}

    usize len() const { return size; }

    Option<char> get(usize const idx) const {
        if (idx < size) {
            return Option<char>(string[idx]);
        } else {
            return Option<char>();
        }
    }

    char const& operator[](usize const idx) const {
        return string[idx];
    }

    iterator<char const> begin() const { return iterator<char const>(string); }
    iterator<char const> end() const { return iterator<char const>(string + size); }

  private:
        char const* string;
        usize const size;
};
