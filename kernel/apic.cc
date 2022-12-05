#include "../klib/apic.hh"
#include "../klib/x86.hh"
#include "../klib/console.hh"

namespace apic {
    auto LocalApic::get() -> LocalApic& {
        auto const ptr = reinterpret_cast<LocalApic*>(get_pa());
        return *ptr;
    }

    auto LocalApic::can_use_apic() -> bool {
        auto const [_, hi] = x86::cpuid(1);
        return hi & CPUID_EDX_HAS_APIC;
    }

    auto LocalApic::get_pa() -> uptr {
        auto const [lo, _] = x86::rdmsr(IA32_APIC_BASE_MSR);
        return (lo & 0xFFFFF000);
    }

    void LocalApic::set_pa(uptr const addr) {
        u32 lo = (addr & 0xFFFFF000) | IA32_APIC_BASE_MSR_ENABLE;
        x86::wrmsr(IA32_APIC_BASE_MSR, lo, 0);
    }
    
    auto LocalApic::get_id() const -> u8 {
        return read_register(REG_LAPIC_ID) >> 24;
    }

    void LocalApic::end_of_interrupt() {
        write_register(REG_EOI, 0xFF);
    }
    
    void LocalApic::enable() {
        auto const pa = get_pa();
        auto const val = read_register(REG_SPURIOUS_INTERRUPT);
        write_register(REG_SPURIOUS_INTERRUPT, val | APIC_ENABLE | 0xFF);
    }

    auto LocalApic::read_register(usize offset) const -> u32 {
        return _registers[offset].value;
    }
    
    void LocalApic::write_register(usize offset, u32 value) {
        _registers[offset].value = value;
    }
}
