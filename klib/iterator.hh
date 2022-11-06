#pragma once
template<typename T>
struct iterator {
  public:
    using pointer    = T*;
    using reference  = T&;

    constexpr iterator(pointer ptr) : ptr(ptr) {}

    auto constexpr operator*() const -> reference { return *ptr; }
    auto constexpr operator->() const -> pointer { return ptr; }
    auto constexpr operator++() -> iterator& { ++ptr; return *this; }
    auto constexpr operator++(int) -> iterator { iterator tmp = *this; ++(*this); return tmp; }
    auto constexpr friend operator==(iterator const& a, iterator const& b) -> bool { return a.ptr == b.ptr; };
    auto constexpr friend operator!=(iterator const& a, iterator const& b) -> bool { return a.ptr != b.ptr; };

  private:
    pointer ptr;
};
