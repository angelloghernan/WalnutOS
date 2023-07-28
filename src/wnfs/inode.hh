#pragma once
#include "klib/int.hh"
#include "klib/array.hh"

namespace wnfs {
    class INode {
      public:
        u32 size_lower_32;
        u32 reserved;
        u32 creation_time;
        u32 last_modified_time;
        wlib::Array<u32, 9> direct_blocks;
        u32 indirect_block;
        u32 double_indirect_block;
        u32 triple_indirect_block;
      private:
    };
};
