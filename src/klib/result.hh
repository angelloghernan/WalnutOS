#pragma once
#include "klib/new.hh"
#include "klib/int.hh"
#include "klib/util_move.hh"
#include "klib/type_traits.hh"
#include "klib/concepts.hh"

namespace wlib {
    struct Null { private: u8 nan[0]; };

    template<typename T, typename E>
    class Result {
      public:

        auto constexpr static Err(E const& err) -> Result<T, E> {
            Result result;
            result.m_is_success = false;
            result.m_data.err = err;
            return util::move(result);
        }

        auto constexpr static Err(E&& err) -> Result<T, E> requires concepts::is_different_type<T, E> {
            return Result(false, util::move(err));
        }

        auto constexpr static Err(T&& err) -> Result<T, E> requires concepts::is_same_type<T, E> {
            return Result(false, util::move(err));
        }

        auto constexpr static Ok(T const& data) -> Result<T, E> {
            Result result;
            result.m_is_success = true;
            result.m_data.data = data;
            return util::move(result);
        }

        auto constexpr static Ok(T&& data) -> Result<T, E> {
            return Result(true, util::move(data));
        }
        
        template<typename ...Args>
        auto static constexpr OkInPlace(Args... args) -> Result<T, E>{
            Result result;
            auto* ptr = static_cast<void*>(&result.m_data.data);

            new (ptr) T(type_traits::forward<Args>(args)...);
            result.m_is_success = true;
            return result;
        }

        auto constexpr static Ok() -> Result<T, E> {
            return Result(true);
        }

        auto constexpr static Err() -> Result<T, E> {
            return Result(false);
        }

        template<typename ...Args>
        auto static constexpr ErrInPlace(Args... args) -> Result<T, E>{
            Result result;
            auto* ptr = static_cast<void*>(&result.m_data.err);

            new (ptr) E(type_traits::forward(args)...);
            result.m_is_success = true;
            return result;
        }

        auto constexpr as_err() -> E& {
            return m_data.err;
        }

        auto constexpr as_err() const -> E const& {
            return m_data.err;
        }

        auto constexpr as_ok() -> T& {
            return m_data.data;
        }

        auto constexpr as_ok() const -> T const& {
            return m_data.data;
        }

        auto constexpr matches() const -> bool {
            return m_is_success;
        }

        auto constexpr is_err() const -> bool {
            return !m_is_success;
        }

        auto constexpr is_ok() const -> bool {
            return m_is_success;
        }

        void constexpr into_err() {
            m_is_success = false;
        }

        void constexpr into_ok() {
            m_is_success = true;
        }

        template<typename F>
        auto constexpr ok_or_else(F lambda) && -> T {
            if (is_ok()) {
                return m_data.data;
            } else {
                return lambda();
            }
        } 
        
        template<typename F>
        auto constexpr ok_or_else(F lambda) & -> T& {
            if (is_ok()) {
                return m_data.data;
            } else {
                return lambda();
            }
        }

        #if defined(__clang__)
            constexpr ~Result() {
                if (m_is_success) {
                    m_data.data.~T();
                } else {
                    m_data.err.~E();
                }
            }
        #else
            constexpr ~Result() requires (!type_traits::is_trivially_destructible<T>) {
                if (m_is_success) {
                    m_data.data.~T();
                } else {
                    m_data.err.~E();
                }
            }

            constexpr ~Result() = default;
        #endif

      private:
        constexpr Result() {}
        constexpr Result(bool success) : m_is_success(success) {}
        constexpr Result(bool success, T&& obj) : m_data(util::move(obj)), m_is_success(success) {}

        constexpr Result(bool success, E&& obj) requires concepts::is_different_type<T, E>
            : m_data(util::move(obj)), m_is_success(success) {}

        union data_internal {
            T data;
            E err;
            constexpr ~data_internal() {};
            constexpr data_internal() {};
            constexpr data_internal(T&& d) : data(util::move(d)) {};

            constexpr data_internal(E&& e) requires concepts::is_different_type<T, E>
                : err(util::move(e)) {};
        } m_data;
        bool m_is_success;
    };
}; // namespace wlib
