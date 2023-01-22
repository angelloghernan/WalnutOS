#pragma once
#include "int.hh"
#include "concepts.hh"

auto static constexpr None = false;
auto static constexpr Some = true;

template<typename O>
concept Optionable = requires(O optionable){
    { optionable.is_sentinel() } -> concepts::is_type<bool>;
    { O::sentinel() } -> concepts::is_type<O>;
};

template <typename T>
class Option {
  public:
    constexpr Option(T const& value) : _val(value), _present(true) {}

    constexpr Option() : _present(false) {}

    auto constexpr none() const -> bool { return !_present; }
    auto constexpr some() const -> bool { return _present; }
    void constexpr make_none() { _present = false; }

    auto constexpr unwrap() -> T& {
        return _val;
    }

    auto constexpr unwrap() const -> T const& {
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


template <Optionable O>
class Option<O> {
  public:
    constexpr Option(O const& value) : _val(value) {}
    constexpr Option(O&& value) : _val(value) {}
    constexpr Option(O& value) : _val(value) {}

    constexpr Option() : _val(O::sentinel()) {}

    auto constexpr none() const -> bool { return _val.is_sentinel(); }
    auto constexpr some() const -> bool { return !_val.is_sentinel(); }
    auto constexpr matches() const -> bool { return _val.is_sentinel() ? Some : None; }

    auto constexpr unwrap() -> O& { return _val; }
    auto constexpr unwrap() const -> O const& { return _val; }

    void constexpr assign(O const& value) {
        _val = &value;
    }

    void constexpr make_none() { _val = O::sentinel(); }

    auto constexpr operator!() const -> bool { return none(); }
    void constexpr operator=(O& value) { _val = &value; }
    void constexpr operator=(O const& value) { _val = &value; }
    bool constexpr operator==(Option<O> const& value) { return _val == value._val; }
  private:
    O _val;
};

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
