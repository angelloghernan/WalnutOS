#include "ahci.hh"
#include "../ports.hh"
#include "../pci/pci.hh"
#include "../x86.hh"
#include "../util.hh"

using namespace ahci;

// Create a new object to keep track of AHCI-relevant state
// `pci_addr`: the relevant PCI bus/slot/function for the AHCI controller
// `sata_port`: the port for this device on the AHCI controller
// `dr`: the drive registers, as pointed to by BAR 5 of the AHCI controller
//
// Recommended to call `find(addr, port)` unless one is already searching for 
// PCI devices and wishes to avoid repeating work.
AHCIState::AHCIState(u32 const pci_addr, 
                     u32 const sata_port, 
                     volatile registers& dr) 
    : _pci_addr(pci_addr), _sata_port(sata_port),
      _drive_registers(dr), _port_registers(dr.port_regs[sata_port]),
     _num_ncq_slots(1), _num_slots_available(1), _slots_outstanding_mask(0) {

    for (auto& slot : _slot_status) {
        slot.make_none();
    }

    {
        using enum pci::command_register::bit;

        // Enable this PCI slot's bus master, memory space and I/O space
        ports::outw(pci_addr + u32(pci::Register::Command), 
                    u8(BusMaster) | u8(MemorySpace) | u8(IOSpace));
    }

    {
        using enum PortCommandMasks;
        auto const mask = ~(u32(RFISEnable) | u32(RFISClear));

        // Note: |=, +=, &=, etc are deprecated for volatile variables in C++20
        _port_registers.command_mask = _port_registers.command_mask & mask;

        while (_port_registers.command_mask & (u32(CommandRunning) | u32(RFISRunning))) {
            x86::pause();
        }
    }

    util::memset(uptr(&_dma), 0_u8, sizeof(_dma));

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

    _drive_registers.global_hba_control 
        = _drive_registers.global_hba_control | u32(GHCMasks::InterruptEnable);


    _port_registers.command_mask 
        = _port_registers.command_mask | u32(PortCommandMasks::RFISEnable);

    auto const busy = u32(RStatusMasks::Busy) | u32(RStatusMasks::DataReq);

    while ((_port_registers.command_and_status & busy) != 0 || 
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

    Array<u16, 256> id_buf;
    id_buf.filled(0_u16);

    // Read into devid here

    // Fill out these functions
    this->clear_slot(0);
    this->push_buffer(0, uptr(&id_buf), id_buf.size());
    this->issue_meta(0, pci::IDEController::Command::Identify, 0);
    this->await_basic(0);
}

// Prepare `slot` to receive a command
inline void AHCIState::clear_slot(u16 const slot) {
    _dma.ch[slot].num_buffers = 0;
    _dma.ch[slot].buffer_byte_pos = 0;
}

void AHCIState::push_buffer(u32 const slot, uptr const data, usize const size) {
    auto const phys_addr = util::kernel_to_physical_addr(data);
    
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
void AHCIState::issue_ncq(u32 slot, pci::IDEController::Command command,
                          usize sector, bool fua, u32 priority) {
    using enum pci::IDEController::Command;

    usize const nsectors = _dma.ch[slot].buffer_byte_pos / SECTOR_SIZE;
    _dma.ct[slot].cfis[0] = CFIS_COMMAND | (u32(command) << 16)
        | ((nsectors & 0xFF) << 24);
    _dma.ct[slot].cfis[1] = (sector & 0xFFFFFF)
        | (uint32_t(fua) << 31) | 0x40000000U;
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


// Acknowledge a command waiting in `slot`
void AHCIState::acknowledge(u32 slot, u32 result) {
    _slots_outstanding_mask ^= 1U << slot;
    ++_num_slots_available;

    if (_slot_status[slot].some()) {
        _slot_status[slot].unwrap() = result;
        _slot_status[slot].make_none();
    }
}

auto AHCIState::read(Slice<u8>& buf, usize const offset) -> Result<Null, IOError> {
    // IMPORTANT: this whole function needs to protected by a lock when we add multicore
    
    // TODO: We should block here in a multicore/async environment, waiting for there to be
    // a free slot using _slots_outstanding_mask

    volatile u32 r = u32(IOError::TryAgain);

    this->clear_slot(0);
    this->push_buffer(0, buf.to_uptr(), buf.len());
    this->issue_ncq(0, pci::IDEController::Command::ReadFPDMAQueued, offset / SECTOR_SIZE);

    _slot_status[0].assign(&r);

    // TODO: This should block instead of poll after we add wait queues
    while (r == u32(IOError::TryAgain)) {}
    
    return Result<Null, IOError>::Ok({});
}
