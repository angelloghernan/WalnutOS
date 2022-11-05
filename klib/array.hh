#pragma once
#include "int.hh"
#include "option.hh"
#include "iterator.hh"
// A generic, compile-time sized array.
// Do not access _arr -- this is left public so we can use brace initialization.
template<typename T, usize S>
class Array {
  public:
    [[nodiscard]] constexpr usize len() const { return S; }
    [[nodiscard]] constexpr T* data() { return &_arr[0]; }
    [[nodiscard]] constexpr T const* data() const { return &_arr[0]; }
    [[nodiscard]] constexpr T& operator[](usize idx) { return _arr[idx]; }
    [[nodiscard]] constexpr T const& operator[](usize idx) const { return _arr[idx]; }

    [[nodiscard]] constexpr iterator<T> begin() { return iterator<T>(&_arr[0]); }
    [[nodiscard]] constexpr iterator<T> end() { return iterator<T>(&_arr[S]); }

    [[nodiscard]] constexpr Option<T> get(usize idx) const {
        if (idx < S) {
            return Option<T>(_arr[idx]); 
        } else {
            return Option<T>();
        }
    }

    T _arr[S];
};
