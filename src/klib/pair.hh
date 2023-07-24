#pragma once
#include "klib/type_traits.hh"

namespace wlib {
    template<typename T1, typename T2>
    class Pair {
      private:
        using Type1 = type_traits::conditional_t<(sizeof(T1) > sizeof(T2)), T1, T2>;
        using Type2 = type_traits::conditional_t<(sizeof(T1) > sizeof(T2)), T2, T1>;
      public:
        Type1 first;
        Type2 second;
    };
};
