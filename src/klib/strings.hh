#pragma once
#include "klib/int.hh"
#include "klib/slice.hh"
#include "klib/array.hh"
#include "klib/option.hh"
#include "klib/result.hh"
#include "klib/iterator.hh"
#include "klib/type_traits.hh"

namespace wlib {
    // str: A primitive type wrapping around string literals, allowing for easier iteration and slicing.
    // When passing size, it is the size of the entire string, *including a null character*. Arrays are assumed
    // to not include a null-terminator.
    struct str {
      public:
        template<usize S>
        constexpr str(char const (&string)[S]) : _string(string), _size(S - 1) {}

        constexpr str(char const* string, usize size) : _string(string), _size(size - 1) {}
        
        template<usize S>
        constexpr str(Array<char, S> const& array) : _string(&array[0]), _size(S) {}

        template<usize S>
        constexpr str(Array<char, S> const& array, usize start, usize end) 
                  : _string(&array[start]), _size(end - start) {}

        auto constexpr len() const -> usize { return _size; }

        // Return a pointer to the first character in this string.
        // Unsafe: Not to be used on string literals. Use as_const_ptr instead.
        auto constexpr as_ptr() -> char* { return const_cast<char*>(&_string[0]); }

        auto constexpr as_const_ptr() const -> char const* { return &_string[0]; }

        auto constexpr begin() const -> iterator<char const> { return iterator<char const>(_string); }
        auto constexpr end() const -> iterator<char const> { return iterator<char const>(_string + _size); }

        auto constexpr operator[](usize const idx) const -> char const& { return _string[idx]; }

        auto constexpr into_u32() -> Result<u32, Null> {
            if (_size > 10 || _size == 0) {
                return Result<u32, Null>::ErrInPlace();
            }

            i32 place = i32(_size - 1);
            u32 mult = 1;
            u32 value = 0;
            u32 prev = value;

            for (; place >= 0; --place) {
                if (_string[place] > '9' || _string[place] < '0') {
                    return Result<u32, Null>::ErrInPlace();
                }
                value += u32(u32(_string[place] - '0') * mult);
                if (value < prev) {
                    return Result<u32, Null>::ErrInPlace();
                }
                prev = value;
                mult *= 10;
            }

            return Result<u32, Null>::OkInPlace(value);
        }

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

        class Split {
          public:
            constexpr Split(str& string, char ch) 
                      : _string(string.as_const_ptr()), _pos(0), _end(string._size), _delimiter(ch) {
                for (; _pos < _end && _string[_pos] == _delimiter; ++_pos) {}
            }

            [[nodiscard]] auto constexpr remainder() const -> str {
                return str(_string + _pos, _end - _pos + 1);
            }

            auto constexpr next() -> Option<str> {
                if (_pos >= _end) {
                    return Option<str>::None();
                }

                for (; _pos < _end && _string[_pos] == _delimiter; ++_pos) {}

                if (_pos == _end) {
                    return Option<str>::None();
                }
                
                usize start = _pos;
                usize end = _end;

                for (auto i = _pos; i < _end; ++i) {
                    if (_string[i] == _delimiter) {
                        end = i;
                        break;
                    }
                }

                _pos = end + 1;

                return Option<str>::Some(_string + start, end - start + 1);
            }
            
            auto constexpr done() const -> bool {
                return _pos >= _end;
            }

            char const* _string;
            usize _pos;
            usize _end;
            char _delimiter;
        };

        [[nodiscard]] auto constexpr split(char ch) -> Split {
            return Split(*this, ch);
        }

      private:
        char const* _string;
        usize _size;
    };
    
};
