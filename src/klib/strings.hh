#pragma once
#include "klib/int.hh"
#include "klib/option.hh"
#include "klib/iterator.hh"
#include "klib/slice.hh"
#include "klib/type_traits.hh"

namespace wlib {
    // str: A primitive type wrapping around string literals, allowing for easier iteration and slicing.
    // When passing size, it is the size of the entire string, *including a null character*.
    struct str {
      public:
        template<usize S>
        constexpr str(char const (&string)[S]) : _string(string), _size(S - 1) {}
        constexpr str(char const* string, usize size) : _string(string), _size(size - 1) {}

        auto constexpr len() const -> usize { return _size; }

        auto constexpr begin() const -> iterator<char const> { return iterator<char const>(_string); }
        auto constexpr end() const -> iterator<char const> { return iterator<char const>(_string + _size); }

        auto constexpr operator[](usize const idx) const -> char const& { return _string[idx]; }

        auto constexpr operator==(str const& other) const -> bool {
            if (other._size != _size) {
                return false;
            }
            
            for (usize i = 0; i < _size; ++i) {
                if (other[i] != _string[i]) {
                    return false;
                }
            }

            return true;
        }

        auto constexpr operator==(Slice<char> const& other) const -> bool {
            if (other.len() != _size) {
                return false;
            }
            
            for (usize i = 0; i < _size; ++i) {
                if (other[i] != _string[i]) {
                    return false;
                }
            }

            return true;
        }

        auto constexpr get(usize const idx) const -> Option<char> {
            if (idx < _size) [[likely]] {
                return Option<char>::Some(_string[idx]);
            } else {
                return Option<char>::None();
            }
        }

        auto constexpr as_slice() const -> Slice<char const> {
            return Slice<char const>(_string, _size);
        }

      private:
        char const* _string;
        usize _size;
    };
};
