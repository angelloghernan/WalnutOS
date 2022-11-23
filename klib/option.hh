#pragma once
#include "int.hh"

auto constexpr None = false;
auto constexpr Some = true;

template <typename T>
class Option {
  public:
    constexpr Option(T const& value) : _val(value), _present(true) {}

    constexpr Option() : _present(false) {}

    auto constexpr none() const -> bool { return !_present; }
    auto constexpr some() const -> bool { return _present; }
    void constexpr make_none() { _present = false; }

    auto constexpr unwrap() const -> T {
        if (_present) {
            return _val;
        } else {
            return T();
        }
    }
    void constexpr assign(T const& value) {
        _val = value;
        _present = true;
    }


    auto constexpr operator!() const -> bool { return !_present; }


  private:
    T _val;
    bool _present;
};

template <typename T>
class Option<T&> {
  public:
    constexpr Option(T const& value) : _val(&value), _present(true) {}
    constexpr Option(T& value) : _val(&value), _present(true) {}

    constexpr Option() : _present(false) {}

    auto constexpr none() const -> bool { return !_present; }
    auto constexpr some() const -> bool { return _present; }
    auto constexpr match() const -> bool { return _present ? Some : None; }

    auto constexpr unwrap() const -> T& { return *_val; }

    void constexpr assign(T const& value) {
        _val = &value;
        _present = true;
    }

    void constexpr make_none() { _present = false; }

    auto constexpr operator!() const -> bool { return !_present; }


  private:
    T* _val;
    bool _present;
};

