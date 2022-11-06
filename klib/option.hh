#pragma once
#include "int.hh"

template <typename T>
class Option {
  public:
    constexpr Option(T value) : val(value), present(true) {}

    constexpr Option() : present(false) {}

    auto constexpr none() const -> bool { return !present; }
    auto constexpr some() const -> bool { return present; }

    auto constexpr unwrap() const -> T {
        if (present) {
            return val;
        } else {
            return T();
        }
    }

    auto constexpr operator!() const -> bool { return !present; }

    void constexpr assign(T const value) {
        val = value;
        present = true;
    }

  private:
    bool present;
    T val;
};
