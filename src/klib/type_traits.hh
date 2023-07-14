#pragma once

namespace type_traits {
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
}; // namespace type_traits
