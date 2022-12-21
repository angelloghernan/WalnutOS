#pragma once
#include "int.hh"
#include "array.hh"
#include "result.hh"
#include "option.hh"

/// A generic circular buffer with a given size. The maximum size of the circular buffer
/// is 2^16 - 1 -- we do not expect to exceed this number of elements for most usecases.
/// This buffer allocates space for one extra element, since it wastes one slot. 
/// This may not be suitable for large elements.
/// TODO: Make another circular buffer class that uses a bool instead.
/// Far in the future: make another circular buffer class that is concurrent and lock-free 
/// once we use multiple CPU cores.
template<typename T, u16 S>
class CircularBuffer {
  public:
    /// Push an element into this buffer. Returns whether it was successful.
    auto constexpr push(T element) -> Result<Null, Null> {
        if (full()) {
            return Result<Null, Null>::Err({});
        }

        m_buffer[m_write_end] = element;
        m_write_end = (m_write_end + 1) % S;
        return Result<Null, Null>::Ok({});
    }
    /// Pop an element from this buffer. Returns None if empty.
    auto constexpr pop() -> Option<T> {
        if (empty()) {
            return {};
        }
        auto element = m_buffer[m_read_end];
        m_read_end = (m_read_end + 1) % S;
        return {element};
    }
    /// Push an element into this buffer without checking if it will be successful.
    /// May result in undefined behavior if buffer is full when called.
    void constexpr push_unchecked(T element) {
        m_buffer[m_write_end] = element; 
        m_write_end = (m_write_end + 1) % S;
    }
    /// Pop an element from this buffer without checking if it's empty.
    /// May result in undefined behavior if empty.
    auto constexpr pop_unchecked() -> T { 
        auto element = m_buffer[m_read_end];
        m_read_end = (m_read_end + 1) % S;
        return element;
    };
    /// Check if this buffer is empty.
    auto constexpr empty() -> bool { return m_read_end == m_write_end; }
    /// Check if this buffer is full.
    auto constexpr full() -> bool { return m_read_end == m_write_end + 1; }
    /// Return the capacity of this circular buffer.
    auto constexpr capacity() -> u16 { return S; }
    /// Return how many elements are in the buffer currently.
    auto constexpr len() -> u16 { 
        if (m_read_end <= m_write_end) {
            return m_write_end - m_read_end;
        } else {
            return m_write_end + (S - m_read_end);
        }
    }
  private:
    Array<T, S + 1> m_buffer;
    u16 m_read_end = 0;
    u16 m_write_end = 0;
};
