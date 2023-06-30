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

template<typename T>
struct const_iterator {
 public:
    using pointer    = T const*;
    using reference  = T const&;

    constexpr const_iterator(pointer ptr) : ptr(ptr) {}

    auto constexpr operator*() const -> reference { return *ptr; }
    auto constexpr operator->() const -> pointer { return ptr; }
    auto constexpr operator++() -> const_iterator& { ++ptr; return *this; }
    auto constexpr operator++(int) -> const_iterator { iterator tmp = *this; ++(*this); return tmp; }
    auto constexpr friend operator==(const_iterator const& a, const_iterator const& b) -> bool { return a.ptr == b.ptr; };
    auto constexpr friend operator!=(const_iterator const& a, const_iterator const& b) -> bool { return a.ptr != b.ptr; };

  private:
    pointer ptr;
};
