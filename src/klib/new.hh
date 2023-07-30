#pragma once
#include "klib/int.hh"

// Placement new/delete operators (placement delete shouldn't be used, call the destructor)
inline void* operator new(usize, void* ptr) noexcept { return ptr; }
inline void* operator new[](usize, void* ptr) noexcept { return ptr; }

inline void operator delete(void*, void*) noexcept {}
inline void operator delete[](void*, void*) noexcept {}
