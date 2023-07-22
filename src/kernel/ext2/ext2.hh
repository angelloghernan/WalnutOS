#pragma once

namespace kernel::ext2 {
    // The location of the descriptor table when this OS is the one
    // that formatted the hard drive for ext2.
    auto static constexpr WNOS_GROUP_DT_LOCATION = 2048;
};
