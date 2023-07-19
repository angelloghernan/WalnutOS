#pragma once

namespace wlib {
    // A version of Option that uses in-band data to determine whether it is valid.
    //
    // A Nullable type generally will not cause a fault or undefined behavior when merely unwrapped,
    // however, it is generally used to indicate when unwrapping an object and treating it as valid 
    // data would result in a violation of an invariant, which may then lead to undefined behavior.
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

        [[gnu::always_inline]] auto static constexpr Some(T const& value) -> Nullable<T, NullValue> { return Nullable(value); }
        [[gnu::always_inline]] auto static constexpr None() -> Nullable<T, NullValue> { return Nullable(NullValue); }
        
        template<typename TNew>
        [[gnu::always_inline]] auto constexpr unwrap_as() -> TNew { return (TNew)(_data); }

        [[gnu::always_inline]] void constexpr operator=(T&& data) { _data = data; }
        [[gnu::always_inline]] void constexpr operator=(T const& data) { _data = data; }

        [[gnu::always_inline]] bool constexpr operator==(Nullable<T, NullValue>&& value) { return _data == value._data; }
        [[gnu::always_inline]] bool constexpr operator==(Nullable<T, NullValue> const& value) { return _data == value._data; }
        
      private:
        T _data;
    };
}; // namespace wlib
