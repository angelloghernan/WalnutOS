#pragma once
#include "klib/int.hh"
#include "klib/array.hh"
#include "klib/result.hh"
#include "klib/strings.hh"

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
        u8 name_len;
        wlib::Array<u8, 63> name;

        // Set the name of the file. 
        // Returns err if the name is too long or invalid.
        inline auto set_name(wlib::str const filename) 
                    -> wlib::Result<wlib::Null, wlib::Null> {
            if (filename.len() > name.size()) {
                return wlib::Result<wlib::Null, wlib::Null>::Err({});
            }

            name_len = u8(filename.len() & 0xFF);

            for (auto i = 0; i < filename.len(); ++i) {
                name[i] = filename[i];
            }

            return wlib::Result<wlib::Null, wlib::Null>::Ok({});
        }

      private:
    };
};
