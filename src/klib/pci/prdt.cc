#include "prdt.hh"
#include "../../kernel/kernel.hh"
#include "../../kernel/alloc.hh"
#include "../ports.hh"
#include "../util.hh"

using namespace wlib;
using namespace pci;

auto PRDT::initialize(u16 const entry_count, 
                      u16 const bus_master_register, 
                      ChannelType const channel) -> Result<Null, Null> {
    usize bytes = usize(entry_count) * sizeof(PRD_Entry);
    auto location = simple_allocator.kalloc(bytes);

    if (location.none()) {
        return Result<Null, Null>::Err({});
    }

    this->prdt_location = location.unwrap_as<PRD_Entry*>();
    this->bus_master_register = bus_master_register + static_cast<u8>(channel);
    this->entry_count = entry_count;

    auto const port = this->bus_master_register + static_cast<u8>(BMROffset::Command);

    ports::outl(port, location.unwrap());

    return Result<Null, Null>::Ok({});
}

auto PRDT::set_up_dma(Slice<Pair<uptr, usize>> const& entries,
                     ChannelType const channel,
                     DMAOpMask const operation) -> Result<Null, u16> {
    auto num_to_add = util::min(usize(entry_count), entries.len());

    for (usize i = 0; i < entries.len(); ++i) {
        prdt_location[i].set_address(entries[i].first);
        prdt_location[i].set_size(entries[i].second);
    }

    prdt_location[num_to_add - 1].set_last_entry_flag();

    // Now actually begin the DMA...
    //
    // Calculate the correct port with BMR as base
    auto const port = bus_master_register + static_cast<u8>(channel) +
                          static_cast<u8>(BMROffset::Command);

    auto cmd = ports::inb(port);

    ports::outb(port, cmd & u8(StartMask::Stop));

    cmd |= u8(StartMask::Start);
    cmd |= u8(operation);

    ports::outb(port, cmd);

    if (num_to_add != entries.len()) {
        return Result<Null, u16>::Err(num_to_add);
    } else {
        return Result<Null, u16>::Ok({});
    }
}
