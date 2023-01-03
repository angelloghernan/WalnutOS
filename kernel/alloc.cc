#include "alloc.hh"
#include "../klib/int.hh"
#include "../klib/intrusive_list.hh"
#include "../klib/array.hh"
#include "../klib/result.hh"
#include "../klib/bitmap.hh"

using namespace alloc;


auto BuddyAllocator::kalloc(usize size) -> uptr {
    return 0;
}

