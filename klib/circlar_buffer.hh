#pragma once
#include "int.hh"
#include "array.hh"
#include "result.hh"
#include "option.hh"

template<typename T, u16 S>
/// A generic circular buffer with a given size. The maximum size of the circular buffer
/// is 2^16 - 1 -- we do not expect to exceed this number of elements for most usecases.
/// This buffer allocates one extra element, since it wastes one slot. This may not be
/// suitable for large elements.
/// TODO: Make another circular buffer class that uses a bool instead.
/// Far in the future: make another circular buffer class that is lock-free, once we use
/// multiple CPU cores.
class CircularBuffer {
  public:
    /// Push an element into this buffer. Returns whether it was successful.
    auto constexpr push() -> Result<Null, Null>;
    /// Pop an element from this buffer. Returns None if empty.
    auto constexpr pop() -> Option<T>;
    /// Push an element into this buffer without checking if it will be successful.
    /// Will result in undefined behavior if buffer is full when called.
    auto constexpr push_unchecked();
    /// Pop an element from this buffer without checking if it's empty.
    /// Will result in undefined behavior if empty.
    auto constexpr pop_unchecked();
    /// Check if this buffer is empty.
    auto constexpr empty() -> bool { return m_read_end == m_write_end; }
    /// Check if this buffer is full.
    auto constexpr full() -> bool { return m_read_end == m_write_end + 1; }
    /// Return how many elements are in the buffer currently.
    auto constexpr len() -> u16 { return m_write_end - m_read_end; }
    /// Return the capacity of this circular buffer.
    auto constexpr capacity() -> u16 { return S; }
  private:
    Array<T, S + 1> m_buffer;
    u16 m_read_end = 1;
    u16 m_write_end = 0;
};
