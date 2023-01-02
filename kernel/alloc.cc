#include "alloc.hh"
#include "../klib/int.hh"
#include "../klib/intrusive_list.hh"
#include "../klib/array.hh"
#include "../klib/result.hh"
#include "../klib/bitmap.hh"

using namespace alloc;

// Kernel allocation... how to do it, how to do it
// Skip lists? Unlikely I could do it correctly if ticki couldn't
// Buddy allocaton? Think I could do it easily now that I know how to abstract well
// Plus I can always replace it later.

class Allocator {
  public:
    
  private:
    auto static constexpr NUM_BLOCKS = 32;
    // Array[i] = the ith block's address
    Array<uptr, 32> m_block_addrs;

    // Array[i] = the ith block's size
    Array<usize, 32> m_block_sizes;

    // An index 'pointing' to the next block in the array.
    Array<u8, 32> m_block_next;
};

auto init_kalloc() -> Result<Null, Null> {
    using Result = Result<Null, Null>;
    
    

    return Result::Ok({});
}

auto kalloc(usize size) -> uptr {
    
    return 0;
}
