#include "klib/ahci/ahci.hh"
#include "klib/ports.hh"
#include "klib/pci/pci.hh"
#include "klib/x86.hh"
#include "klib/util.hh"
#include "klib/console.hh"
#include "klib/assert.hh"
#include "klib/pic.hh"
#include "klib/idt.hh"
#include "kernel/alloc.hh"
#include "kernel/kernel.hh"

using namespace wlib;
using namespace ahci;

void AHCIState::handle_interrupt() {
    // IMPORTANT: This *MUST* be lock-protected when multi-core is set up
    auto is_error = (_port_registers.interrupt_status &
                     u32(InterruptMasks::FatalErrorMask)) ||
                    (_port_registers.tfd & u32(RStatusMasks::Error));

    _port_registers.interrupt_status = ~0U;
    _drive_registers.interrupt_status = ~0U;

    auto acks = _slots_outstanding_mask & ~(_port_registers.ncq_active);

    for (auto slot = 0; acks != 0; ++slot, acks >>= 1) {
        if (acks & 1) {
            acknowledge(slot, 0);
        }
    }

    if (is_error) {
        terminal.print_line_color(console::Color::Red, console::Color::Black, 
                                  "Error when handling interrupt for AHCI");
        this->handle_error_interrupt();
    }
}

void AHCIState::handle_error_interrupt() {
    for (int slot = 0; slot < 32; ++slot) {
        if (_slots_outstanding_mask & (1U << slot)) {
            acknowledge(slot, u32(IOError::DeviceError));
        }
    }

    _port_registers.command_and_status =
        _port_registers.command_and_status & ~u32(PortCommandMasks::Start);
    while (_port_registers.command_and_status & u32(PortCommandMasks::CommandRunning)) {
        x86::pause();
    }
    _port_registers.serror = ~0U;
    _port_registers.command_and_status =
        _port_registers.command_and_status | u32(PortCommandMasks::Start);

    // TODO: Issue "READ LOG EXT" to find info on error?
}

auto AHCIState::find(pci::PCIState::bus_slot_addr addr, 
                     u32 slot) -> Option<AHCIState&> {

    using bus_slot_addr = pci::PCIState::bus_slot_addr;

    auto& pci = pci::PCIState::get();

    auto addr_opt = Option<bus_slot_addr>(addr);

    for (; addr_opt.some(); pci.next_addr(addr_opt)) {
        auto& addr = addr_opt.unwrap();
        auto subclass = pci.config_read_word(addr.bus, addr.slot, addr.func, 
                                             pci::Register::Subclass);
        if (subclass != 0x0106) {
            continue;
        }

        auto const phys_addr = pci.config_read_u32(addr.bus, addr.slot, addr.func,
                                                   pci::Register::GDBaseAddress5);

        
        if (phys_addr == 0) {
            continue;
        }

        auto result = kernel_pagedir.try_map(phys_addr, phys_addr, PTE_PWU);

        assert(result.is_ok(), "Couldn't map AHCI!");

        auto drive_regs 
            = reinterpret_cast<volatile registers*>(util::physical_addr_to_kernel(phys_addr));

        if (!(drive_regs->global_hba_control & u32(GHCMasks::AHCIEnable))) {
            drive_regs->global_hba_control = u32(GHCMasks::AHCIEnable);
        }

        for (; slot < 32; ++slot) {
            if ((drive_regs->port_mask & (1U << slot)) &&
                drive_regs->port_regs[slot].sstatus) {
                auto maybe_ahci_ptr = simple_allocator.kalloc(sizeof(AHCIState));

                assert(maybe_ahci_ptr.some(), 
                       "Could not allocate enough space for AHCI pointer");

                auto* ahci_ptr = reinterpret_cast<AHCIState*>(maybe_ahci_ptr.unwrap());
                ::new (ahci_ptr) AHCIState(addr.bus, addr.slot, addr.func, 
                                           slot, *drive_regs);
                return Option<AHCIState&>(*ahci_ptr);
            }
        }
    }
    
    return Option<AHCIState&>();
}

// Create a new object to keep track of AHCI-relevant state
// `bus` / `slot` / `func_number`: the relevant PCI bus/slot/function for the
// AHCI controller
// `sata_port`: the port for this device on the AHCI controller
// `dr`: the drive registers, as pointed to by BAR 5 of the AHCI controller
//
// Recommended to call `find(addr, port)` unless one is already searching for 
// PCI devices and wishes to avoid repeating work.
AHCIState::AHCIState(u8 const bus,
                     u8 const slot,
                     u8 const func_number,
                     u32 const sata_port, 
                     volatile registers& dr) 
    : _bus(bus), _slot(slot), _func(func_number), _sata_port(sata_port),
      _drive_registers(dr), _port_registers(dr.port_regs[sata_port]),
     _num_ncq_slots(1), _num_slots_available(1), _slots_outstanding_mask(0) {

    for (auto& slot : _slot_status) {
        slot = nullptr;
    }

    auto& pci_state = pci::PCIState::get();

    {
        using enum pci::command_register::bit;
        pci_state.config_write_word(bus, slot, func_number, 
                                    pci::Register::Command, 0x7); // Enable I/O, mem, bus master
    }

    {
        using enum PortCommandMasks;
        auto const mask = ~(u32(RFISEnable) | u32(Start));

        // Note: |=, +=, &=, etc are deprecated for volatile variables in C++20
        _port_registers.command_and_status = _port_registers.command_and_status & mask;

        while (_port_registers.command_and_status & (u32(CommandRunning) | u32(RFISRunning))) {
            x86::pause();
        }
    }

    util::memset<u8>((void*)(&_dma), 0_u8, sizeof(_dma));

    for (auto i = 0; i < 32; ++i) {
        _dma.ch[i].command_table_address 
            = util::kernel_to_physical_addr(uptr(&_dma.ct[i]));
    }

    _port_registers.cmdlist_addr 
        = util::kernel_to_physical_addr(uptr(&_dma.ch[0]));

    _port_registers.rfis_base_addr
        = util::kernel_to_physical_addr(uptr(&_dma.rfis));

    // Clear all SATA errors/interrupt status, and power up
    _port_registers.serror = ~0U;

    _port_registers.command_mask 
        = _port_registers.command_mask | u32(PortCommandMasks::PowerUp);

    _port_registers.interrupt_status = ~0U;
    _drive_registers.interrupt_status = ~0U;

    {
        using enum InterruptMasks;
        _port_registers.interrupt_enable = u32(DeviceToHost) | 
                                           u32(NCQComplete)  | 
                                           u32(ErrorMask);
    }



    _port_registers.command_and_status
        = _port_registers.command_and_status | u32(PortCommandMasks::RFISEnable);

    auto const busy = u32(RStatusMasks::Busy) | u32(RStatusMasks::DataReq);

    while ((_port_registers.tfd & busy) != 0 || 
            !sstatus_active(_port_registers.sstatus)) {
        x86::pause();
    }

    {
        using enum PortCommandMasks;

        _port_registers.command_and_status 
            = (_port_registers.command_and_status & ~u32(InterfaceMask)) | 
            u32(InterfaceActive);

        while ((_port_registers.command_and_status & u32(InterfaceMask)) != 
                u32(InterfaceIdle)) {
            x86::pause();
        }
    }

    _port_registers.command_and_status = 
        _port_registers.command_and_status | u32(PortCommandMasks::Start);

    Array<volatile u16, 256> id_buf;

    for (usize i = 0; i < id_buf.len(); ++i) {
        id_buf[i] = 0;
    }

    this->clear_slot(0);
    this->push_buffer(0, (void*)&id_buf, id_buf.size());
    this->issue_meta(0, pci::IDEController::Command::Identify, 0);
    this->await_basic(0);

    _num_sectors = id_buf[100] | (id_buf[101] << 16);

    // | (u64(id_buf[102]) << 32) | (u64(id_buf[103]) << 48);

    // count slots
    _num_ncq_slots = ((_drive_registers.capabilities >> 8) & 0x1F) + 1; // slots per controller
    if ((id_buf[75] & 0x1F) + 1U < _num_ncq_slots) {         // slots per disk
        _num_ncq_slots = (id_buf[75] & 0x1F) + 1;
    }
    _slots_full_mask = (_num_ncq_slots == 32 ? 0xFFFFFFFFU : (1U << _num_ncq_slots) - 1);
    _num_slots_available = _num_ncq_slots;

    // set features
    this->clear_slot(0);
    this->issue_meta(0, pci::IDEController::Command::SetFeatures, 0x02);  // write cache enable
    this->await_basic(0);

    this->clear_slot(0);
    this->issue_meta(0, pci::IDEController::Command::SetFeatures, 0xAA);  // read lookahead enable
    this->await_basic(0);

    // determine IRQ
    auto intr_line = pci_state.config_read_byte(bus, slot, func_number, 
                                                pci::Register::InterruptLine);
    _irq = intr_line;

    // finally, clear pending interrupts again
    _port_registers.interrupt_status = ~0U;
    _drive_registers.interrupt_status = ~0U;
}

// Prepare `slot` to receive a command
inline void AHCIState::clear_slot(u16 const slot) {
    _dma.ch[slot].num_buffers = 0;
    _dma.ch[slot].buffer_byte_pos = 0;
}

void AHCIState::push_buffer(u32 const slot, void* data, usize const size) {
    auto const phys_addr = util::kernel_to_physical_addr(uptr(data));
    
    auto const num_buffers = _dma.ch[slot].num_buffers;

    _dma.ct[slot].prdt[num_buffers].address = phys_addr;
    _dma.ct[slot].prdt[num_buffers].data_byte_count = size - 1;

    _dma.ch[slot].num_buffers = num_buffers + 1;
    _dma.ch[slot].buffer_byte_pos += size;
}

// Issue an NCQ (Native Command Queueing) command to the disk
// Must preceed with clear_slot(slot) and push_buffer(slot).
// `fua`: If true, then don't acknowledge the write until data has been durably written to disk.
// `priority`: 0 is normal priority, 2 is high priority
void AHCIState::issue_ncq(u32 const slot, 
                          pci::IDEController::Command const command,
                          usize const sector, 
                          bool const fua, 
                          u32 const priority) {
    using enum pci::IDEController::Command;

    usize const nsectors = _dma.ch[slot].buffer_byte_pos / SECTOR_SIZE;
    _dma.ct[slot].cfis[0] = CFIS_COMMAND | (u32(command) << 16)
        | ((nsectors & 0xFF) << 24);
    _dma.ct[slot].cfis[1] = (sector & 0xFFFFFF)
        | (u32(fua) << 31) | 0x40000000U;
    _dma.ct[slot].cfis[2] = (sector >> 24) | ((nsectors & 0xFF00) << 16);
    _dma.ct[slot].cfis[3] = (slot << 3) | (priority << 14);

    _dma.ch[slot].flags = 4 /* # words in `cfis` */
        | u32(CHFlag::Clear)
        | (command == WriteFPDMAQueued ? u32(CHFlag::Write) : 0);
    _dma.ch[slot].buffer_byte_pos = 0;

    // ensure all previous writes have made it out to memory
    // IMPORTANT: Add this back in when we have multicore and have implemented atomic
    // std::atomic_thread_fence(std::memory_order_release);
    
    _port_registers.ncq_active = 1U << slot;  // tell interface NCQ slot used
    _port_registers.command_mask = 1U << slot; // tell interface command available
    // The write to `command_mask` wakes up the device.

    _slots_outstanding_mask |= 1U << slot;   // remember slot
    --_num_slots_available;
}

void AHCIState::issue_meta(u32 const slot, 
                           pci::IDEController::Command const command,
                           u32 const features, 
                           u32 const count) {
    using enum pci::IDEController::Command;

    usize nsectors = _dma.ch[slot].buffer_byte_pos / SECTOR_SIZE;
    if (command == SetFeatures && count != u32(-1)) {
        nsectors = count;
    }

    _dma.ct[slot].cfis[0] = CFIS_COMMAND | (u32(command) << 16) | (u32(features) << 24);
    _dma.ct[slot].cfis[1] = 0;
    _dma.ct[slot].cfis[2] = (u32(features) & 0xFF00) << 16;
    _dma.ct[slot].cfis[3] = nsectors;

    _dma.ch[slot].flags = 4 | u16(CHFlag::Clear);
    _dma.ch[slot].buffer_byte_pos = 0;

    // IMPORTANT: Uncomment once multicore and atomic are done
    // std::atomic_thread_fence(std::memory_order_release);

    _port_registers.command_mask = 1U << slot;    // tell interface command is available

    _slots_outstanding_mask |= 1U << slot;
    --_num_slots_available;
}

void AHCIState::await_basic(u32 const slot) {
    while (_port_registers.command_mask & (1U << slot)) {
        x86::pause();
    }

    this->acknowledge(slot, 0);
}

// Acknowledge a command waiting in `slot`
void AHCIState::acknowledge(u32 const slot, u32 const result) {
    _slots_outstanding_mask ^= 1U << slot;
    ++_num_slots_available;

    if (_slot_status[slot] != nullptr) {
        *_slot_status[slot] = result;
        _slot_status[slot] = nullptr;
    }
}

auto AHCIState::read_or_write(pci::IDEController::Command const command,
                              Slice<u8>& buf, usize const offset) -> Result<Null, IOError> {
    // IMPORTANT: this whole function needs to protected by a lock when we add multicore
    // up until we start polling (which should block instead)
    
    // TODO: We should block here in a multicore/async environment, waiting for there to be
    // a free slot using _slots_outstanding_mask

    volatile u32 r;

    {
        InterruptGuard guard;

        r = u32(IOError::TryAgain);
        _slot_status[0] = &r;

        this->clear_slot(0);
        this->push_buffer(0, (void*)(buf.to_raw_ptr()), buf.len());
        this->issue_ncq(0, command, offset / SECTOR_SIZE);
    }

    // TODO: This should block instead of polling after we add wait queues
    // interrupts.
    while (r == u32(IOError::TryAgain)) {
        x86::pause();
    }

    _slot_status[0] = nullptr;
    
    return Result<Null, IOError>::Ok({});
}
