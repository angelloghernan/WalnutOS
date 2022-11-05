#pragma once
#include "int.hh"
#include "option.hh"
#include "iterator.hh"
// A generic, compile-time sized array.
// Do not access _arr -- this is left public so we can use brace initialization.
template<typename T, usize S>
class Array {
  public:
    [[nodiscard]] auto constexpr len() const -> usize { return S; }

    [[nodiscard]] auto constexpr data()       -> T* { return &_arr[0]; }
    [[nodiscard]] auto constexpr data() const -> T const* { return &_arr[0]; }

    [[nodiscard]] auto constexpr operator[](usize idx)       -> T& { return _arr[idx]; }
    [[nodiscard]] auto constexpr operator[](usize idx) const -> T const& { return _arr[idx]; }

    [[nodiscard]] auto constexpr begin() -> iterator<T> { return iterator<T>(&_arr[0]); }
    [[nodiscard]] auto constexpr end()   -> iterator<T> { return iterator<T>(&_arr[S]); }

    [[nodiscard]] constexpr Option<T> get(usize idx) const {
        if (idx < S) {
            return Option<T>(_arr[idx]); 
        } else {
            return Option<T>();
        }
    }

    T _arr[S];
};
