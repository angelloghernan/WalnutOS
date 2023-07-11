#include "ahci.hh"
#include "../ports.hh"
#include "../pci/pci.hh"
#include "../x86.hh"

using namespace ahci;

AHCIState::AHCIState(u32 pci_addr, u32 sata_port, volatile registers& mr) 
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
        _port_registers.command_mask = _port_registers.command_mask & mask;
        while (_port_registers.command_mask & (u32(CommandRunning) | u32(RFISRunning))) {
            x86::pause();
        }
    }

    

}
