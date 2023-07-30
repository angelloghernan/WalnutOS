#pragma once
namespace wlib::concepts {
    template<typename T, typename U>
    inline constexpr bool is_same_type = false;

    template<typename T>
    inline constexpr bool is_same_type<T, T> = true;

    template<typename T, typename U>
    inline constexpr bool is_different_type = true;

    template <typename T>
    inline constexpr bool is_different_type<T, T> = false;

    namespace detail {
        template<typename T, typename U>
        concept _type = is_same_type<T, U>;
    } // namespace detail

    template<typename T, typename U>
    concept is_type = detail::_type<T, U> && detail::_type<U, T>;
}; // namespace wlib::concepts
