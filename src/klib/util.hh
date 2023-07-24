#pragma once
#include "klib/slice.hh"
#include "klib/option.hh"
#include "klib/type_traits.hh"

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
    inline void memset(void* ptr, T value, usize count) {
        auto const t_ptr = reinterpret_cast<T*>(ptr);
        for (usize i = 0; i < count; ++i) {
            *t_ptr = value;
        }
    }

    template<typename T, typename U>
    [[nodiscard]] constexpr inline auto bit_cast(U const& u) -> T {
        #if (__has_builtin(__builtin_bit_cast))
            return __builtin_bit_cast(T, u);
        #else
            T result;
            auto const u_ptr = (char*)(&u);
            auto const t_ptr = (char*)(&result);
            for (auto i = 0; i < sizeof(U); ++i) {
                t_ptr[i] = u_ptr[i];
            } 
            return result;
        #endif
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
