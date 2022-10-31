#pragma once
template<typename T>
struct iterator {
  public:
    using value_type = T;
    using pointer    = T*;
    using reference  = T&;

    iterator(pointer ptr) : ptr(ptr) {}

    reference operator*() const { return *ptr; }
    pointer operator->() const { return ptr; }
    iterator& operator++() { ++ptr; return *this; }
    iterator operator++(int) { iterator tmp = *this; ++(*this); return tmp; }
    friend bool operator==(iterator const& a, iterator const& b) { return a.ptr == b.ptr; };
    friend bool operator!=(iterator const& a, iterator const& b) { return a.ptr != b.ptr; };

  private:
    pointer ptr;
};
