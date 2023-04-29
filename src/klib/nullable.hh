#pragma once

// A version of Option that uses in-band data to determine whether it is valid.
template<typename T, T NullValue>
class Nullable {
  public:
    constexpr Nullable(T&& data) : _data(data) {}
    constexpr Nullable(T const& data) : _data(data) {}
    constexpr Nullable(T& data) : _data(data) {}

    constexpr Nullable() : _data(NullValue) {}

    [[gnu::always_inline]] auto constexpr some() const -> bool { return _data != NullValue; }
    [[gnu::always_inline]] auto constexpr none() const -> bool { return _data == NullValue; }

    [[gnu::always_inline]] void constexpr become_none() { _data = NullValue; }

    [[gnu::always_inline]] auto constexpr unwrap() -> T& { return _data; }
    [[gnu::always_inline]] auto constexpr unwrap() const -> T const& { return _data; }
    
    template<typename TNew>
    [[gnu::always_inline]] auto constexpr unwrap_as() -> TNew { return (TNew)(_data); }

    [[gnu::always_inline]] void constexpr operator=(T&& data) { _data = data; }
    [[gnu::always_inline]] void constexpr operator=(T const& data) { _data = data; }

    [[gnu::always_inline]] bool constexpr operator==(Nullable<T, NullValue>&& value) { return _data == value._data; }
    [[gnu::always_inline]] bool constexpr operator==(Nullable<T, NullValue> const& value) { return _data == value._data; }
    
  private:
    T _data;
};
