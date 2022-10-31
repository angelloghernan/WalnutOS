#pragma once
#include "int.hh"

template <typename T>
class Option {
  public:
    Option(T value) : val(value), present(true) {}

    Option() : present(false) {}

    bool none() const { return !present; }
    bool some() const { return present; }

    T unwrap() const {
        if (present) {
            return val;
        } else {
            return T();
        }
    }

    bool operator!() const { return !present; }

    void assign(T const value) {
        val = value;
        present = true;
    }

  private:
    bool present;
    T val;
};
