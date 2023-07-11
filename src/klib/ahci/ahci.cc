#include "ahci.hh"
#include "../ports.hh"
#include "../pci/pci.hh"
#include "../x86.hh"
#include "../util.hh"

using namespace ahci;

// Prepare `slot` to receive a command
inline void AHCIState::clear_slot(u16 const slot) {
    _dma.ch[slot].number_buffers = 0;
    _dma.ch[slot].buffer_byte_pos = 0;
}

AHCIState::AHCIState(u32 const pci_addr, 
                     u32 const sata_port, 
                     volatile registers& mr) 
    : _pci_addr(pci_addr), _sata_port(sata_port),
      _drive_registers(mr), _port_registers(mr.port_regs[sata_port]),
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
