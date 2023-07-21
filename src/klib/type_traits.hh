#pragma once

namespace wlib::type_traits {
    template<bool B, typename T = void>
    struct enable_if {};

    template<typename T>
    struct enable_if<true, T> { typedef T type; };

    template<bool B, typename T>
    using enable_if_t = typename enable_if<B, T>::type;

    template<bool B, typename T, typename F>
    struct conditional { using type = T; };

    template<typename T, typename F>
    struct conditional<false, T, F> { using type = F; };

    template<bool B, typename T, typename F>
    using conditional_t = typename conditional<B,T,F>::type;

    template<typename T>
    struct remove_reference { typedef T type; };

    template<typename T>
    struct remove_reference<T&> { typedef T type; };

    template<typename T>
    struct remove_reference<T&&> { typedef T type; };

    template<typename T>
    using remove_reference_t = typename remove_reference<T>::type;

    template<typename T>
    auto declval() -> T;

    template<typename T>
    inline constexpr bool is_destructible = requires { declval<T>().~T(); };

    template<typename T, typename ...Args>
    inline constexpr bool is_trivially_constructible = __is_trivially_constructible(T, Args...);

    #if defined(__clang__)
        template<typename T>
        inline constexpr bool is_trivially_destructible = false; // clangd doesn't like to cooperate
    #else
        template<typename T, typename ...Args>
        inline constexpr bool is_trivially_destructible = __has_trivial_destructor(T) && is_destructible<T>; 
    #endif
};
