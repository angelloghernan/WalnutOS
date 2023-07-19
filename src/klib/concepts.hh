#pragma once
namespace wlib {
    namespace concepts {
        template<typename T, typename U>
        inline constexpr bool is_same_type = false;

        template<typename T>
        inline constexpr bool is_same_type<T, T> = true;

        namespace detail {
            template<typename T, typename U>
            concept _type = is_same_type<T, U>;
        } // namespace detail

        template<typename T, typename U>
        concept is_type = detail::_type<T, U> && detail::_type<U, T>;
    } // namespace concepts
};
