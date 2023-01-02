#pragma once
#include "../klib/option.hh"
#include "../klib/result.hh"

namespace alloc {
    auto init_kalloc() -> Result<Null, Null>;
    auto kalloc(usize size) -> uptr;
    void kfree(uptr ptr);
};
