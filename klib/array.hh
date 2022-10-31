#pragma once
#include "int.hh"
#include "option.hh"
#include "iterator.hh"
template<typename T, usize S>
class Array {
  public:

    constexpr Option<T> get(usize idx) const {
        if (idx < S) {
            return Option<T>(_arr[idx]); 
        } else {
            return Option<T>();
        }
    }
    
    constexpr usize len() const { return S; }
    constexpr T* data() { return &_arr[0]; }
    constexpr T const* data() const { return &_arr[0]; }
    constexpr T& operator[](usize idx) { return _arr[idx]; }
    constexpr T const& operator[](usize idx) const { return _arr[idx]; }

    constexpr iterator<T> begin() { return iterator<T>(&_arr[0]); }
    constexpr iterator<T> end() { return iterator<T>(&_arr[S]); }
    T _arr[S];
};
