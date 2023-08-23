#pragma once
#include "klib/int.hh"
#include "klib/array.hh"
#include "klib/slice.hh"

namespace wlib {
    template<typename T, usize S>
    class StaticSlice {
      public:
        constexpr StaticSlice(T* values) : _values(values) {}

        constexpr StaticSlice(Array<T, S>& array) : _values(array.data()) {}

        constexpr StaticSlice(Array<T, S> const& array) : _values(array.data()) {}

        constexpr StaticSlice(Array<T, S> const& array, usize off) : 
                              _values((T*)(uptr(array.data()) + off)) {}

        [[nodiscard]] auto consteval len() -> usize { return S; }

        [[nodiscard]] auto consteval size() -> usize { return S * sizeof(T); }

        [[nodiscard]] auto constexpr begin() -> iterator<T> { return iterator<T>(&_values[0]); }
        [[nodiscard]] auto constexpr end() -> iterator<T> { return iterator<T>(&_values[S]); }

        [[nodiscard]] auto constexpr begin() const -> const_iterator<T> { return const_iterator<T>(&_values[0]); }
        [[nodiscard]] auto constexpr end() const -> const_iterator<T> { return const_iterator<T>(&_values[S]); }

        [[nodiscard]] auto constexpr operator==(StaticSlice<T, S>& other) -> bool {
            for (usize i = 0; i < len(); ++i) {
                if (_values[i] != other._values[i]) {
                    return false;
                }
            }

            return true;
        }

        [[nodiscard]] auto consteval empty() -> bool { return S == 0; }

        [[nodiscard]] auto constexpr operator[](usize idx) -> T& { return _values[idx]; }
        [[nodiscard]] auto constexpr operator[](usize idx) const -> T const& { return _values[idx]; }

        [[nodiscard]] auto constexpr to_raw_ptr() -> T* { return _values; }
        [[nodiscard]] auto constexpr to_raw_ref() -> T& { return *_values; }

        [[nodiscard]] auto constexpr to_uptr() const -> uptr { return reinterpret_cast<uptr>(_values); }
        [[nodiscard]] auto constexpr to_raw_ptr() const -> T const* { return _values; }
        [[nodiscard]] auto constexpr to_raw_ref() const -> T const& { return *_values; }

        [[nodiscard]] auto constexpr to_raw_bytes() -> StaticSlice<u8, S> { 
            return StaticSlice<u8, S>((u8*)(_values));
        }

        [[nodiscard]] auto constexpr to_raw_bytes() const -> StaticSlice<u8 const, S> { 
            return StaticSlice<u8 const, S>((u8 const*)(_values));
        }

        [[nodiscard]] auto constexpr to_slice() const -> Slice<T> {
            return Slice<T>(_values, S);
        }

      private:
        T* _values;
    };
};
