#pragma once
#include "klib/type_traits.hh"

namespace wlib::util {
    template<typename T>
    inline auto move(T&& arg) -> type_traits::remove_reference_t<T>&& {
        return static_cast<type_traits::remove_reference_t<T>&&>(arg);
    }
};
