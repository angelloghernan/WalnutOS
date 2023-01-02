#pragma once
#include "array.hh"

template<typename T, usize S>
class Bitmap {
  public:
    
  private:
    // An array of size S / 8 bits (one byte per 8 bits), rounded up to the nearest 8 bits
    Array<T, (S + (sizeof(bool) - S % sizeof(bool))) / sizeof(bool)> m_map; 
};
