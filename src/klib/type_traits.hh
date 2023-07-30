#pragma once

namespace wlib::type_traits {
    template<typename T>
    struct remove_reference { typedef T type; };

    template<typename T>
    struct remove_reference<T&> { typedef T type; };

    template<typename T>
    struct remove_reference<T&&> { typedef T type; };

    template<typename T>
    using remove_reference_t = typename remove_reference<T>::type;

    template<typename T, T val>
    struct integral_constant
    {
        static constexpr T value = val;
        using value_type = T;
        using type = integral_constant;       
        constexpr operator value_type() const noexcept { return value; }
        constexpr value_type operator()() const noexcept { return value; }
    };

    using true_type = integral_constant<bool, true>;

    using false_type = integral_constant<bool, false>;

    template<bool B, typename T = void>
    struct enable_if {};

    template<typename T>
    struct enable_if<true, T> { typedef T type; };

    template<bool B, typename T = void>
    using enable_if_t = typename enable_if<B, T>::type;

    template<bool B, typename T, typename F>
    struct conditional { using type = T; };

    template<typename T, typename F>
    struct conditional<false, T, F> { using type = F; };

    template<bool B, typename T, typename F>
    using conditional_t = typename conditional<B,T,F>::type;

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

    template<typename T, typename U>
    struct is_same : false_type {};

    template<typename T>
    struct is_same<T, T> : true_type {};
    
    template<typename T, typename U>
    struct is_different : true_type {};

    template<typename T>
    struct is_different<T, T> : true_type {};

    template<typename T>
    struct is_lvalue_reference : false_type {};

    template<typename T>
    struct is_lvalue_reference<T&> : true_type {};

    template<typename T>
    using is_lvalue_reference_t = typename is_lvalue_reference<T>::type;


    template<typename T>
    inline T&& forward(remove_reference_t<T>& t) {
        return static_cast<T&&>(t);
    }

    template<typename T>
    inline T&& forward(remove_reference_t<T>&& t) {
        static_assert(!is_lvalue_reference<T>::type, 
                      "Cannot forward an rvalue as an lvalue");
        return static_cast<T&&>(t);
    }
};
