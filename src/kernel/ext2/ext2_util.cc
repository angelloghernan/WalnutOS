#include "klib/strings.hh"
#include "kernel/ext2/ext2.hh"
#include "kernel/ext2/blocks.hh"
#include "kernel/ext2/inodes.hh"
#include "kernel/ext2/directory_entry.hh"

using namespace wlib;

namespace kernel::ext2 {
    void find_directory(u32 inode_num, str const dir_name) {
        // TODO: instead of loading fresh each time, look inside a cache to see if
        // we already have the inode/directory entries loaded
        
        // TODO
    }

    void list_directory(u32 inode) {
        
    }
}; // namespace kernel::ext2
