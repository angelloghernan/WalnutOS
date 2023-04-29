#pragma once

#include "alloc.hh"

void setup_pagedir();
extern alloc::BuddyAllocator simple_allocator;
