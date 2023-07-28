#pragma once
#include "klib/array.hh"

namespace wnfs {
    class alignas(256) TagNode {
      public:
        wlib::Array<char, 64> tag_name;
        u32 file_count;
        wlib::Array<u32, 46> file_ids;
        u32 extended_sector;
      private:
    };

    class alignas(256) ExtendedTagNode {
      public:
        u32 file_count;
        u32 extended_sector;
        wlib::Array<u32, 62> data;
    };
};
