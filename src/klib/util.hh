#pragma once
#include "slice.hh"
#include "option.hh"

namespace util {
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
};
