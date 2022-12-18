#pragma once
#include "../klib/int.hh"

static auto constexpr Ok = true;
static auto constexpr Err = false;

struct Null { private: u8 nan[0]; };

template<typename T, typename E>
class Result {
  public:

    auto constexpr static Err(E& err) -> Result<T, E> {
        auto result = Result();
        result.m_is_success = false;
        result.m_data.err = err;
        return result;
    }

    auto constexpr static Err(E&& err) -> Result<T, E> {
        auto result = Result();
        result.m_is_success = false;
        result.m_data.err = err;
        return result;
    }

    auto constexpr static Ok(T& data) -> Result<T, E> {
        auto result = Result();
        result.m_is_success = true;
        result.m_data.err = data;
        return result;
    }

    auto constexpr static Ok(T&& data) -> Result<T, E> {
        auto result = Result();
        result.m_is_success = true;
        result.m_data.err = data;
        return result;
    }

    auto constexpr as_err() -> E& {
        return m_data.err;
    }

    auto constexpr as_ok() -> T& {
        return m_data.data;
    }

    auto constexpr match() -> bool {
        return m_is_success;
    }
    
  private:
    Result() {}

    bool m_is_success;
    union {
        T data;
        E err;
    } m_data;
};
