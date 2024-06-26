#pragma once
#include "klib/int.hh"
#include "klib/array.hh"
#include "klib/concepts.hh"

namespace wlib {
    template<typename T>
    class Slice {
      public:
        constexpr Slice(T* values, usize size) : _values(values), _size(size) {}
        
        template<usize S>
        constexpr Slice(T* values) : _values(values), _size(S) {}

        template<usize S>
        constexpr Slice(Array<T, S>& array) : _values(array.data()), _size(S) {}

        template<usize S>
        constexpr Slice(Array<T, S> const& array) : _values(array.data()), _size(S) {}

        template<usize S>
        constexpr Slice(Array<T, S> const& array, usize off) : 
                        _values((T*)(uptr(array.data()) + off)), _size(S - off) {}
    
        template<usize S>
        constexpr Slice(Array<T, S> const& array, usize off, usize size) :
                        _values((T*)(uptr(array.data()) + off)), _size(size) {}

        auto constexpr len() const -> usize { return _size; }

        auto constexpr size() -> usize { return _size * sizeof(T); }

        auto constexpr begin() -> iterator<T> { return iterator<T>(&_values[0]); }
        auto constexpr end() -> iterator<T> { return iterator<T>(&_values[_size]); }

        auto constexpr begin() const -> const_iterator<T> { return const_iterator<T>(&_values[0]); }
        auto constexpr end() const -> const_iterator<T> { return const_iterator<T>(&_values[_size]); }

        auto constexpr operator==(Slice const& other) -> bool {
            if (other.len() != len()) {
                return false;
            }

            for (usize i = 0; i < len(); ++i) {
                if (_values[i] != other._values[i]) {
                    return false;
                }
            }

            return true;
        }
    
        template<typename T2>
        [[nodiscard]] auto constexpr into_slice() -> Slice<T2> requires(concepts::is_same_type<T, u8>) {
            return Slice<T2>((T2*)(_values), _size / sizeof(T2));
        }

        [[nodiscard]] auto constexpr empty() -> bool { return _size == 0; }
        
        [[nodiscard]] auto constexpr operator[](usize idx) -> T& { return _values[idx]; }
        [[nodiscard]] auto constexpr operator[](usize idx) const -> T const& { return _values[idx]; }

        [[nodiscard]] auto constexpr to_raw_ptr() -> T* { return _values; }
        [[nodiscard]] auto constexpr to_raw_ref() -> T& { return *_values; }

        [[nodiscard]] auto constexpr to_uptr() const -> uptr { return reinterpret_cast<uptr>(_values); }
        [[nodiscard]] auto constexpr to_raw_ptr() const -> T const* { return _values; }
        [[nodiscard]] auto constexpr to_raw_ref() const -> T const& { return *_values; }

        [[nodiscard]] auto constexpr to_raw_bytes() -> Slice<u8> { 
            return Slice<u8>((u8*)(_values), _size * sizeof(T));
        }

        [[nodiscard]] auto constexpr to_raw_bytes() const -> Slice<u8 const> { 
            return Slice<u8 const>((u8 const*)(_values), _size * sizeof(T));
        }

      private:
        T* _values;
        usize _size;
    };
};
