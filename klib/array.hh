#pragma once
#include "int.hh"
#include "option.hh"
template<typename T, usize S>
class Array {
  public:
    struct iterator {
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
    usize len() const { return S; }
    Option<T> get(usize idx) const {
        if (idx < S) {
            return Option<T>(_arr[idx]); 
        } else {
            return Option<T>();
        }
    }
    T& operator[](usize idx) const { return _arr[idx]; }
    iterator begin() { return iterator(&_arr[0]); }
    iterator end() { return iterator(&_arr[S]); }
    T _arr[S];
};
