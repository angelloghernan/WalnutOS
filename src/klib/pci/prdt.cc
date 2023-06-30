#include "prdt.hh"
#include "../../kernel/kernel.hh"

using namespace pci;

auto PRDT::create(usize entry_count) -> Result<PRDT, Null> {
    usize bytes = usize(entry_count) * sizeof(PRD_Entry);
    auto location = simple_allocator.kalloc(bytes);

    PRDT prdt;
    prdt.prdt_location = location.unwrap_as<PRD_Entry*>();
    prdt.count = entry_count;

    return Result<PRDT, Null>::Ok(prdt);
}

auto PRDT::add_entry(Pair<uptr, usize> const& entry) -> Result<Null, Null> {
    
}

auto PRDT::add_entries(Slice<Pair<uptr, usize>> const& entries) -> Result<Null, u16> {
    auto entry_count = 0_u16;

    for (auto const& entry : entries) {
        if (add_entry(entry).is_err()) {
            return Result<Null, u16>::Err(entry_count);
        } else {
            ++entry_count;
        }
    }

    return Result<Null, u16>::Ok({});
}
