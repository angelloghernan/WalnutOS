#pragma once
#include "klib/int.hh"
#include "klib/new.hh"
#include "klib/concepts.hh"
#include "klib/util_move.hh"
#include "klib/type_traits.hh"

namespace wlib {
    template <typename T>
    class Option {
      public:
        auto static constexpr None() -> Option {
            return Option(none_type {});
        }
        
        template<typename ...Args>
        auto static constexpr Some(Args... args) -> Option {
            return Option(type_traits::forward<Args>(args)...);
        }

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

        #if defined(__clang__)
            constexpr ~Option() {
                if (_present) {
                    _val.~T();
                }
            }
        #else
            constexpr ~Option() requires(!type_traits::is_trivially_destructible<T>) {
                if (_present) {
                    _val.~T();
                } 
            }

            constexpr ~Option() = default;
        #endif


      private:
        struct empty { u8 empty[0]; };
        struct none_type { u8 empty[0]; };
        constexpr Option(none_type) : _present(false) {}

        template<typename ...Args>
        constexpr Option(Args... args) : _present(true) {
            auto* ptr = static_cast<void*>(&_val);

            new (ptr) T(type_traits::forward<Args>(args)...);
        }

        union {
            T _val;
            empty emp;  
        };
        bool _present;
    };

    template <typename T>
    Option(T) -> Option<T>;

    template <typename T>
    class Option<T&> {
      public:
        auto static constexpr Some(T& value) -> Option {
            return Option(type_traits::forward<T&>(value));
        }

        auto static constexpr None() -> Option {
            return Option();
        }

        auto constexpr none() const -> bool { return !_val; }
        auto constexpr some() const -> bool { return _val; }
        auto constexpr matches() const -> bool { return _val ? true : false; }

        auto operator*() -> T& {
            return _val;
        }

        auto operator*() const -> T const& {
            return *_val;
        }

        auto operator->() -> T* {
            return _val;
        }

        // debating whether i should delete the indirection operator
        auto operator->() const -> T const* {
            return _val;
        }

        auto constexpr unwrap() -> T& { return *_val; }
        auto constexpr unwrap() const -> T const& { return *_val; }

        void constexpr assign(T* const value) {
            _val = value;
        }

        void constexpr make_none() { _val = nullptr; }

        auto constexpr operator!() const -> bool { return !_val; }
        void constexpr operator=(T& value) { _val = &value; }
        void constexpr operator=(T const& value) { _val = &value; }
        bool constexpr operator==(Option<T&> const& value) { return _val == value._val; }

        #if defined(__clang__)
            constexpr ~Option() {
                if (_val != nullptr) {
                    _val->~T();
                }
            }
        #else
            constexpr ~Option() requires(!type_traits::is_trivially_destructible<T>) {
                if (_val != nullptr) {
                    _val->~T();
                } 
            }

            constexpr ~Option() = default;
        #endif


      private:
        T* _val;

        constexpr Option(T& value) : _val(&value) {}
        constexpr Option() : _val(nullptr) {}
    };

    template <typename T>
    Option(T&) -> Option<T&>;
};
