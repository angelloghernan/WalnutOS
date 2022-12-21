#include "circlar_buffer.hh"

template<typename T, u16 S>
auto constexpr CircularBuffer<T, S>::push() -> Result<Null, Null> {
    return Result<Null, Null>::Err({});
}

template<typename T, u16 S>
auto constexpr CircularBuffer<T, S>::pop() -> Option<T> {
    return Option<T>();
}
