#pragma once
#include "int.hh"
#include "option.hh"

struct str {
  public:
    struct iterator {
        using value_type = char;
        using pointer    = char const*;
        using reference  = char const&;

        iterator(pointer ptr) : ptr(ptr) {}

        reference operator*() const { return *ptr; }
        pointer operator->() { return ptr; }
        iterator& operator++() { ++ptr; return *this; }
        iterator operator++(int) { iterator tmp = *this; ++(*this); return tmp; }
        friend bool operator==(iterator const& a, iterator const& b) { return a.ptr == b.ptr; };
        friend bool operator!=(iterator const& a, iterator const& b) { return a.ptr != b.ptr; };

      private:
        char const* ptr;
    };
    
    template<usize S>
    str(char const (&string)[S]) : string(string), size(S - 1) {}

    usize len() const { return size; }

    Option<char> get(usize idx) const {
        if (idx < size) {
            return Option<char>(string[idx]);
        } else {
            return Option<char>();
        }
    }

    char operator[](usize idx) const {
        return string[idx];
    }

    iterator begin() const { return iterator(string); }
    iterator end() const { return iterator(string + size); }

  private:
        char const* string;
        usize const size;
};
