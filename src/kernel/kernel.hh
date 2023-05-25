#pragma once

#include "alloc.hh"

void setup_pagedir();
extern alloc::BuddyAllocator simple_allocator;

namespace kernel {
    auto constexpr SEGMENT_SIZE = 0x10000;
    auto constexpr KERNEL_START = 0x100000;
    auto constexpr KERNEL_CS_SEG_START = 0x10;
};
