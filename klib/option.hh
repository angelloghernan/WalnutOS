#pragma once
#include "int.hh"

auto static constexpr None = false;
auto static constexpr Some = true;

template <typename T>
class Option {
  public:
    constexpr Option(T const& value) : _val(value), _present(true) {}

    constexpr Option() : _present(false) {}

    auto constexpr none() const -> bool { return !_present; }
    auto constexpr some() const -> bool { return _present; }
    void constexpr make_none() { _present = false; }

    auto constexpr unwrap() const -> T {
        return _val;
    }

    void constexpr assign(T const& value) {
        _val = value;
        _present = true;
    }

    auto constexpr operator!() const -> bool { return !_present; }
    void constexpr operator=(T const& value) { 
        _val = value; 
        _present = true; 
    }

    bool constexpr operator==(Option<T>& value) { return _val == value.val; }


  private:
    T _val;
    bool _present;
};

template <typename T>
Option(T) -> Option<T>;

template <typename T>
class Option<T&> {
  public:
    constexpr Option(T const& value) : _val(&value) {}
    constexpr Option(T& value) : _val(&value) {}

    constexpr Option() : _val(nullptr) {}

    auto constexpr none() const -> bool { return !_val; }
    auto constexpr some() const -> bool { return _val; }
    auto constexpr matches() const -> bool { return _val ? Some : None; }

    auto constexpr unwrap() -> T& { return *_val; }
    auto constexpr unwrap() const -> T const& { return *_val; }

    void constexpr assign(T const& value) {
        _val = &value;
    }

    void constexpr make_none() { _val = nullptr; }

    auto constexpr operator!() const -> bool { return !_val; }
    void constexpr operator=(T& value) { _val = &value; }
    void constexpr operator=(T const& value) { _val = &value; }
    bool constexpr operator==(Option<T&> const& value) { return _val == value._val; }


  private:
    T* _val;
};

template <typename T>
Option(T&) -> Option<T&>;

template<>
class Option<uptr> {
  public:
    constexpr Option(uptr const value) : _val(value) {}

    constexpr Option() : _val(0) {}

    auto constexpr none() const -> bool { return !_val; }
    auto constexpr some() const -> bool { return _val; }
    auto constexpr matches() const -> bool { return _val ? Some : None; }

    auto constexpr unwrap() -> uptr& { return _val; }
    auto constexpr unwrap() const -> uptr const& { return _val; }

    void constexpr assign(uptr const& value) { _val = value; }

    void constexpr make_none() { _val = 0; }

    auto constexpr operator!() const -> bool { return !_val; }
    void constexpr operator=(uptr const value) { _val = value; }
    bool constexpr operator==(Option<uptr> const& value) { return _val == value._val; }
    
  private:
    uptr _val;
};
