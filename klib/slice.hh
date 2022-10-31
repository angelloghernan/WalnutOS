#pragma once
#include "int.hh"
#include "array.hh"

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

    constexpr usize len() { return _size; }

    constexpr bool operator==(Slice const& other) {
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

    constexpr iterator<T> begin() { return iterator<T>(&_values[0]); }
    constexpr iterator<T> end() { return iterator<T>(&_values[_size]); }


  private:
    T* _values;
    usize _size;
};
