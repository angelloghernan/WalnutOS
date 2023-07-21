#pragma once
#include "slice.hh"
#include "option.hh"
#include "type_traits.hh"

namespace wlib::util {
    template<typename T>
    inline auto max(T const& lhs, T const& rhs) -> T {
        return lhs > rhs ? lhs : rhs;
    };

    template<typename T>
    inline auto max(Slice<T> const& slice) -> Option<T> {
        if (slice.empty()) {
            return Option<T> {};
        }

        auto maximum = slice[0];

        for (auto i = 1; i < slice.len(); ++i) {
            maximum = slice[i] > maximum ? slice[i] : maximum;
        }

        return maximum;
    }

    template<typename T>
    inline auto min(T const& lhs, T const& rhs) -> T {
        return lhs < rhs ? lhs : rhs;
    }

    template<typename T>
    inline auto min(Slice<T> const& slice) -> Option<T> {
        if (slice.empty()) {
            return Option<T> {};
        }

        auto minimum = slice[0];

        for (auto i = 1; i < slice.len(); ++i) {
            minimum = slice[i] < minimum ? slice[i] : minimum;
        }

        return minimum;
    }

    template<typename T>
    inline void memset(uptr ptr, T value, usize count) {
        auto const t_ptr = reinterpret_cast<T*>(ptr);
        for (usize i = 0; i < count; ++i) {
            *t_ptr = value;
        }
    }

    template<typename T>
    inline auto move(T&& arg) -> type_traits::remove_reference_t<T> {
        return static_cast<type_traits::remove_reference_t<T>>(arg);
    }
    
    inline auto kernel_to_physical_addr(uptr kernel_addr) -> uptr {
        // This does nothing right now, but may save time if we make a higher-half kernel
        return kernel_addr;
    }

    inline auto physical_addr_to_kernel(uptr physical_addr) -> uptr {
        return physical_addr;
    }
}; // namespace wlib::util

// This is outside of any namespace on purpose. "memset" is used by the compiler.
// Prefer to use util::memset as it can take advantage of greater-sized integers.
void* memset(void* ptr, int ch, size_t count);
