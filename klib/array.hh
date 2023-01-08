#pragma once
#include "int.hh"
#include "option.hh"
#include "iterator.hh"
// A generic, compile-time sized array.
// Do not access _arr -- this is left public so we can use brace initialization.
template<typename T, usize S>
class Array { 
  public:
    // Return the length of the array.
    [[nodiscard]] auto constexpr len() const -> usize { return S; }

    [[nodiscard]] auto constexpr data()       -> T* { return &m_arr[0]; }
    [[nodiscard]] auto constexpr data() const -> T const* { return &m_arr[0]; }

    [[nodiscard]] auto constexpr operator[](usize idx)       -> T& { return m_arr[idx]; }
    [[nodiscard]] auto constexpr operator[](usize idx) const -> T const& { return m_arr[idx]; }

    [[nodiscard]] auto constexpr begin() const -> iterator<T> { return iterator<T>(&m_arr[0]); }
    [[nodiscard]] auto constexpr end() const   -> iterator<T> { return iterator<T>(&m_arr[S]); }

    [[nodiscard]] auto constexpr get(usize idx) const -> Option<T> {
        if (idx < S) {
            return {m_arr[idx]}; 
        } else {
            return {};
        }
    }

    [[nodiscard]] auto constexpr last() -> T& { return m_arr[S - 1]; }
    [[nodiscard]] auto constexpr last() const -> T const& { return m_arr[S - 1]; }

    T m_arr[S];
};

template<typename T, typename... Types>
Array(T, Types...) -> Array<T, sizeof...(Types) + 1>;
