#include "inodes.hh"
#include "blocks.hh"
#include "group_descriptor.hh"
#include "../../klib/ahci/ahci.hh"
#include "../../klib/dynarray.hh"

using namespace wlib;

namespace kernel::ext2 {
    auto GroupDescriptorTable::load(uptr location, usize size) -> Option<GroupDescriptorTable> {
        auto cache = DynArray<GroupDescriptor>::initalize(size / sizeof(GroupDescriptor));
        if (cache.none()) {
            return Option<GroupDescriptorTable>::None();
        }


        auto slice = cache.unwrap().into_slice().to_raw_bytes();

        auto result = sata_disk0.unwrap().read(slice, 0);

        if (result.is_err()) {
            return Option<GroupDescriptorTable>::None();
        }
        
        return Option<GroupDescriptorTable>(util::move(GroupDescriptorTable(util::move(cache.unwrap()))));
    }

    auto format_disk(Superblock* superblock) -> Result<Null, ahci::IOError> {
        using Result = Result<Null, ahci::IOError>;

        if (sata_disk0.none()) {
            return Result::Err(ahci::IOError::DeviceError);
        }

        auto result = superblock->format_superblock();
        if (result.is_err()) {
            return result;
        }

        return Result::Ok({});
    }
}; // namespace kernel::ext2


