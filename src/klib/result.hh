#pragma once
#include "../klib/int.hh"

static auto constexpr Ok = true;
static auto constexpr Err = false;

struct Null { private: u8 nan[0]; };

template<typename T, typename E>
class Result {
  public:

    auto constexpr static Err(E const& err) -> Result<T, E> {
        auto result = Result();
        result.m_is_success = false;
        result.m_data.err = err;
        return result;
    }

    auto constexpr static Err(E const&& err) -> Result<T, E> {
        auto result = Result();
        result.m_is_success = false;
        result.m_data.err = err;
        return result;
    }

    auto constexpr static Ok(T const& data) -> Result<T, E> {
        auto result = Result();
        result.m_is_success = true;
        result.m_data.data = data;
        return result;
    }

    auto constexpr static Ok(T const&& data) -> Result<T, E> {
        auto result = Result();
        result.m_is_success = true;
        result.m_data.data = data;
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

  private:
    constexpr Result() {}

    bool m_is_success;
    union data_internal {
        T data;
        E err;
        constexpr data_internal() {}
    } m_data;
};
