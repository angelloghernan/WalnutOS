#pragma once
#include "../klib/int.hh"
#include "../klib/util_move.hh"

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

        auto constexpr static Err(E&& err) -> Result<T, E> {
            Result result;
            result.m_is_success = false;
            result.m_data.err = util::move(err);
            return util::move(result);
        }

        auto constexpr static Ok(T const& data) -> Result<T, E> {
            Result result;
            result.m_is_success = true;
            result.m_data.data = data;
            return util::move(result);
        }

        auto constexpr static Ok(T&& data) -> Result<T, E> {
            Result result;
            result.m_is_success = true;
            result.m_data.data = util::move(data);
            return util::move(result);
        }

        auto constexpr static Ok() -> Result<T, E> {
            return Result(true);
        }

        auto constexpr static Err() -> Result<T, E> {
            return Result(false);
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

        bool m_is_success;
        union data_internal {
            T data;
            E err;
            constexpr data_internal() {}
        } m_data;
    };
}; // namespace wlib
