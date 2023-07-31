#pragma once
#include "klib/int.hh"
#include "klib/option.hh"
#include "klib/iterator.hh"
// A generic, compile-time sized array.
// Do not access _arr -- this is left public so we can use brace initialization.
namespace wlib {
    template<typename T, usize S>
    class Array { 
      public:
        [[nodiscard]] auto static constexpr filled(T const& element) -> Array {
            Array array;

            for (usize i = 0; i < S; ++i) {
                array.m_arr[i] = element;
            }

            return array;
        }

        void constexpr fill(T const& element) {
            for (usize i = 0; i < S; ++i) {
                m_arr[i] = element;
            }
        }

        // Return the number of elements in the array.
        [[nodiscard]] auto constexpr len() const -> usize { return S; }

        // Return the size in bytes.
        [[nodiscard]] auto constexpr size() const -> usize { return S * sizeof(T); }

        [[nodiscard]] auto constexpr data() -> T* { return &m_arr[0]; }
        [[nodiscard]] auto constexpr data() const -> T const* { return &m_arr[0]; }

        [[nodiscard]] auto constexpr operator[](usize idx) -> T& { return m_arr[idx]; }
        [[nodiscard]] auto constexpr operator[](usize idx) const -> T const& { return m_arr[idx]; }

        [[nodiscard]] auto constexpr operator[](usize idx) volatile -> volatile T& { return m_arr[idx]; }
        [[nodiscard]] auto constexpr operator[](usize idx) volatile const -> volatile T const& { return m_arr[idx]; }

        [[nodiscard]] auto constexpr begin() const -> const_iterator<T> { return const_iterator<T>(&m_arr[0]); }
        [[nodiscard]] auto constexpr end() const -> const_iterator<T> { return const_iterator<T>(&m_arr[S]); }

        [[nodiscard]] auto constexpr begin() -> iterator<T> { return iterator<T>(&m_arr[0]); }
        [[nodiscard]] auto constexpr end() -> iterator<T> { return iterator<T>(&m_arr[S]); }
        
        [[nodiscard]] auto constexpr first() -> T& { return m_arr[0]; }
        [[nodiscard]] auto constexpr first() const -> T const& { return m_arr[0]; }

        [[nodiscard]] auto constexpr last() -> T& { return m_arr[S - 1]; }
        [[nodiscard]] auto constexpr last() const -> T const& { return m_arr[S - 1]; }

        [[nodiscard]] auto constexpr get(usize idx) const -> Option<T> {
            if (idx < S) {
                return Option<T>::Some(m_arr[idx]);
            } else {
                return Option<T>::None();
            }
        }
        T m_arr[S];
    };

    template<typename T, typename... Types>
    Array(T, Types...) -> Array<T, sizeof...(Types) + 1>;

}; // namespace wlib
